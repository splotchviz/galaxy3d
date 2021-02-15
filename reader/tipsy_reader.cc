/*
 * Copyright (c) 2004-2014
 *              Claudio Gheller ETH-CSCS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "cxxsupport/arr.h"
#include "cxxsupport/paramfile.h"
#include "cxxsupport/string_utils.h"
#include "cxxsupport/mpi_support.h"
#include "cxxsupport/bstream.h"
#include "splotch/splotchutils.h"

using namespace std;

void tipsy_reader (paramfile &params, vector<particle_sim> &points, std::vector<MyIDType> &ids, int snr)
{

  float smooth_factor = params.find<float>("smooth_factor",1.0);
  long headersize = sizeof(double)+5*sizeof(int)+sizeof(int); //expected size of the header in bytes (last term is the "pad"
  int ptypes = params.find<int>("ptypes",1);
	
	int interpol_mode = params.find<int>("interpolation_mode", 0);
	if(interpol_mode>1)
	planck_fail("Interpolation mode larger than 1 not supported for tipsy files");

	string datafile = params.find<string>("infile");
	string idfile;
	if(interpol_mode==1)
	{	
	// For interpolation, infile is filename up to (including) the period before the file id
	// The ID is added here from snr
	int id_pad = params.find("tipsy_id_pad",6);
	stringstream ss;
	ss << setfill('0') << setw(id_pad) << snr;
	datafile.append(ss.str());
  idfile = datafile+".iord";
	}

  if (mpiMgr.master())cout << "Reading TIPSY file format: " << datafile << endl;

// set the array that identifies the quantities to read
  arr<int> qty_idx;
  qty_idx.alloc(8);
// for all, the coordinates are at position 1, 2, 3 (0 is the mass)
  qty_idx[0] = 1;
  qty_idx[1] = 2;
  qty_idx[2] = 3;

// open input file
  bifstream inp;
  bool doswap = params.find<bool>("swap_endian",true);
  inp.open (datafile.c_str(),doswap);
  planck_assert (inp,"could not open input file '" + datafile + "'");
// read the header
  double time ;
  int nbodies ;
  int ndim ;
  const int nspecies=3;
  int * np_species;
  int * np_active;

  np_species = new int[nspecies];

  inp >> time;
  inp >> nbodies;
  inp >> ndim;
  inp >> np_species[0];
  inp >> np_species[1];
  inp >> np_species[2];

  np_active = new int [ptypes];

  if (mpiMgr.master())
  cout << "File containing "
       << nbodies << " particles\n "
       << ndim << " dim\n "
       << np_species[0] << " gas\n "
       << np_species[1] << " DM\n "
       << np_species[2] << " stars\n "
       << "at time " << time << "\n";

  // Get index file ready (Note, assumes binary index file with one leading int)
  bifstream id_inp;
  if(interpol_mode==1)
  {
    id_inp.open(idfile.c_str(),doswap);
    planck_assert(id_inp,"could not open id file '"+idfile+"'");  
    // This corresponds to msr->N in gasoline, we dont use it
    int N;
    id_inp >> N;
  }
// Divide work between processors

  long * npar_species;
  long * nstart_species;
  npar_species = new long[nspecies];
  nstart_species = new long[nspecies];
  int64 mybegin, myend;


// Check which and how many particles to visualize
  long npart = 0;
  for(int itype=0;itype<ptypes;itype++)
  {
     int type = params.find<int>("ptype"+dataToString(itype),0);
     np_active[itype]=type;
  }

  for(int itype=0;itype<ptypes;itype++)
  {
     mpiMgr.calcShare (0, np_species[np_active[itype]], mybegin, myend);
     npar_species[np_active[itype]] = myend-mybegin;
     //cout << "MYEND " << myend << "  MYBEGIN " << mybegin << "\n";
     nstart_species[np_active[itype]] = mybegin;
     npart += npar_species[np_active[itype]];
  }


// allocate main vector
  if (mpiMgr.master())
    cout << "rank 0 particle count: " << npart << "\n";
  points.resize(npart);
  if(interpol_mode==1)
  {
    ids.resize(npart);  
  }
// allocate auxiliary vector for reading data
  arr<float> buffer(3);

// load data for each species
  int nfields;
  long skipbytes, skipidbytes;
  long ipart=0;
  const int np_per_read = 100000;
  for(int itype=0;itype<ptypes;itype++)
  {

      qty_idx[3] = params.find<int>("r_"+dataToString(itype),-1)-1;
      qty_idx[4] = params.find<int>("I_"+dataToString(itype),-1)-1;
      qty_idx[5] = params.find<int>("C1_"+dataToString(itype),-1)-1;
      qty_idx[6] = params.find<int>("C2_"+dataToString(itype),-1)-1;
      qty_idx[7] = params.find<int>("C3_"+dataToString(itype),-1)-1;
      float size_fac = params.find<float>("size_fac"+dataToString(itype),1.0);
	
      switch (np_active[itype])
      {
        case 0:
          nfields = 12;
          skipbytes = nstart_species[0]*12*sizeof(float);
	  skipidbytes = nstart_species[0] * sizeof(int);
          break;
        case 1:
          nfields = 9;
          skipbytes = (np_species[0]*12+nstart_species[1]*9)*sizeof(float);
	  skipidbytes = (np_species[0]+nstart_species[1]) * sizeof(int);
          break;
        case 2:
          nfields = 11;
          skipbytes = (np_species[0]*12+np_species[1]*9+nstart_species[2]*11)*sizeof(float);
	  skipidbytes = (np_species[0] + np_species[1] +nstart_species[2]) * sizeof(int);
          break;
        default:
          planck_fail("must not arrive here");
      }
	int bufsize = nfields*np_per_read;
      buffer.resize(bufsize);


      bool have_c2c3 = (qty_idx[6]>=0) && (qty_idx[7]>=0);
      inp.seekg(headersize+skipbytes);
      if(interpol_mode==1)
      {
       	
       id_inp.seekg(skipidbytes+1);
       id_inp.get(&ids[ipart],npar_species[np_active[itype]]);
      }
      long total_p = npar_species[np_active[itype]];
      for(long ip=0; ip<total_p; ip+=np_per_read)
      {
	int p_left = total_p - ip;
	int this_np = min(np_per_read,p_left);
         inp.get(&buffer[0],nfields*this_np);
        
	for(long ipp=0; ipp < this_np; ipp++)
	{
	 int step = ipp*nfields;
	 points[ipart].x = buffer[step+qty_idx[0]];
         points[ipart].y = buffer[step+qty_idx[1]];
         points[ipart].z = buffer[step+qty_idx[2]];
         points[ipart].r = (qty_idx[3]>=0) ? size_fac*buffer[step+qty_idx[3]] : size_fac*smooth_factor;
         points[ipart].I = (qty_idx[4]>=0) ? buffer[step+qty_idx[4]] : 0.5;
         points[ipart].e.r = (qty_idx[5]>=0) ? buffer[step+qty_idx[5]] : 1.0;
         points[ipart].e.g = have_c2c3 ? buffer[step+qty_idx[6]] : 0.0;
         points[ipart].e.b = have_c2c3 ? buffer[step+qty_idx[7]] : 0.0;
         points[ipart].type = itype;
         //points[ipart].type = np_active[itype];
	
         //if(ipart<1000)
         //cout << points[ipart].x << " " << points[ipart].y << " " << points[ipart].z << " " << points[ipart].e.r << " " << points[ipart].type <<"\n";
         ipart++;
        }
      }
  }
  inp.close();

}
