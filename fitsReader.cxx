#include "fitsReader.h"

#include <cmath>

#include <iostream>     // std::cout
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "cxxsupport/string_utils.h"

/*
 *
// The fits reading functions here are from fitsio examples in cookbook.c
*/

//----------------------------------------------------------------------------
fitsReader::fitsReader()
{
    this->filename[0]='\0';
    this->xStr[0]='\0';
    this->yStr[0]='\0';
    this->zStr[0]='\0';
    this->title[0]='\0';
    this->is3D=true;

    for (int i=0; i<3; i++)
    {
        crval[i]=0;
        cpix[i]=0;
        cdelt[i]=0;
        naxes[i]= 10;
    }
    


}

//----------------------------------------------------------------------------
fitsReader::~fitsReader()
{
}

void fitsReader::SetFileName(std::string name) {


    if (name.empty()) {
        printf("No Datafile\n");
        return;
    }


    filename= name;


}

void fitsReader::ComputeTF(COLOUR& e, float I)
{
/*
 * Sort of TF currently
*/
float step=(datamax-2*rms)/20;
if((I>2*rms)&&(I<=3*step))
    e.g=(I-2*rms)/(3*step);

if((I>3*step)&&(I<=7*step))
{   e.r=1.0;
    e.g=(I-3*rms)/(4*step);
}

    if((I>7*rms)&&(I<=11*step))
    {   e.r=1.0;
        e.b=(I-7*step)/(4*step);
    }
    if((I>11*step)&&(I<=15*step))
    {   e.b=1.0;
        e.r=1.0-(I-11*step)/(4*step);
        e.g=(I-11*step)/(4*step);
    }
    if(I>15*step)
    {e.r=0.0;
       e.g=1.0-(I-15*rms)/(5*step);
        e.b=1.0;
    }

}
void fitsReader::NormaliseIR(float& r, float& I)
{
    float scale=(datamax);
    I/=scale;
   // TODO:Below are current patches with magic numbers not to change shaders
    r=exp(-I)*22;
    //r=std::clamp(r,27,200);
    I*=0.02;

}

//----------------------------------------------------------------------------
void fitsReader::ReadFits(std::vector<particle_sim> &particles)
{
    //Calculating contour value to further through away the points
                 std::vector<VALS> vals;
                  CalculateRMS(vals);

                int num2=0; //number of particles





                float point[3];
                float bmin[3]={0,0,0};
                float norm=fmax(naxes[0],naxes[1]);
                norm=fmax(norm,naxes[1])/100;

                float bmax[3]={naxes[0]/norm,naxes[1]/norm,naxes[2]/norm};
                float delta[3]={(bmax[0]-bmin[0])/naxes[0],(bmax[1]-bmin[1])/naxes[1],(bmax[2]-bmin[2])/naxes[2]};

                float normilise=naxes[0]/naxes[2];
                    int i,j,k;
                    i=0;
                    j=0;
                    k=0;


                    //For every pixel
                    for (std::vector<VALS>::iterator it = vals.begin() ; it != vals.end(); ++it)

                    {


                        float I= it->val;

                        COLOUR e(1,0,0);
                       if((I>2*rms))//&&(I<=(7*rms))) //check if still should skip
                        {
                         //   I=0.00001;



                        ComputeTF(e,I);
                        float32 r=I;
                        NormaliseIR(r,I);
                        int type=0;
                        bool active=true;
                        float p[3];

                        float approx=float(it->ii)/float(naxes[1]*naxes[0]);
                        k=int(floor(approx));
                        float rest=approx-k;

                        if (rest>0)
                        {
                            int ii2=it->ii-naxes[1]*naxes[0]*k;
                            approx=float(ii2)/float(naxes[0]);
                            j=int(floor(approx));
                            if (approx-float(j)>0){
                                i=ii2-naxes[0]*j;

                            } else i=0;
                        }
                        else {
                            j=0;
                            i=0;
                        }

                        p[0]=bmin[0]+delta[0]*i;
                        p[1]=bmin[1]+delta[1]*j;
                        p[2]=bmin[2]+delta[2]*k;
                        //std::cout<<"nums "<< naxes[0]<<" "<< naxes[1]<<" "<< naxes[2]<<" "<<std::endl;

                        //TODO: here I do flip of data
                        //it is a patch, ideally I should not do it but something is
                        //wrong with reading



                    particles.push_back(particle_sim(e, p[0],p[1],p[2], r,I,type,active));
                    num2++;

                    }
                    }




        particles.resize(num2);




    }




void fitsReader::ReadHeader() {



    fitsfile *fptr;       /* pointer to the FITS file */

    int status, nkeys, keypos, hdutype, ii, jj;
    char card[FLEN_CARD];   /* standard string lengths defined in fitsioc.h */
    
    
    char crval1[80];
    char crval2[80];
    char crval3[80];
    char crpix1[80];
    char crpix2[80];
    char crpix3[80];
    char cdelt1[80];
    char cdelt2[80];
    char cdelt3[80];
    char naxis1[80];
    char naxis2[80];
    char naxis3[80];
    
    
    crval1[0] ='\0';
    crval2[0] ='\0';
    crval3[0] ='\0';
    crpix1[0] ='\0';
    crpix2[0] ='\0';
    crpix3[0] ='\0';
    cdelt1[0] ='\0';
    cdelt2[0] ='\0';
    cdelt3[0] ='\0';
    
    std::string val1, val2, val3, pix1,pix2, pix3, delt1, delt2, delt3, nax1, nax2, nax3;

    status = 0;


    char *fn=new char[filename.length() + 1];;
    strcpy(fn, filename.c_str());

    if ( fits_open_file(&fptr, fn, READONLY, &status) )
        printerror( status );
    delete []fn;

    /* attempt to move to next HDU, until we get an EOF error */
    for (ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status) ); ii++)
    {

        /* get no. of keywords */
        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status) )
            printerror( status );

        for (jj = 1; jj <= nkeys; jj++)  {

            if ( fits_read_record(fptr, jj, card, &status) )
                printerror( status );

            if (!strncmp(card, "CTYPE", 5)) {

                char *first = strchr(card, '\'');
                char *last = strrchr(card, '\'');

                *last = '\0';
                if (card[5] == '1')
                    strcpy(xStr, first+1);
                if (card[5] == '2')
                    strcpy(yStr, first+1);
                if (card[5] == '3')
                    strcpy(zStr, first+1);

                        }

            if (!strncmp(card, "OBJECT", 6)) {
                std::cerr << card << std::endl;
                char *first = strchr(card, '\'');
                char *last = strrchr(card, '\'');
                *last = '\0';
                strcpy(title, first+1);
            }

            if (!strncmp(card, "CRVAL", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';

                // char *last = strrchr(card, '/');
                //*last = '\0';

                if (card[5] == '1')
                {
                    strcpy(crval1, first+1);
                    char *pch = strtok (crval1," ,");
                    strcpy(crval1, pch);
                    
                }
                
                if (card[5] == '2')
                {
                    strcpy(crval2, first+1);
                    char *pch = strtok (crval2," ,");
                    strcpy(crval2, pch);

                }
                
                if (card[5] == '3')
                {
                    strcpy(crval3, first+1);
                    char *pch = strtok (crval3," ,");
                    strcpy(crval3, pch);

                }
            }

            if (!strncmp(card, "CRPIX", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';
                
                
                if (card[5] == '1')
                {
                    strcpy(crpix1, first+1);

                    char *pch = strtok (crpix1," ,");
                    strcpy(crpix1, pch);
                }
                
                if (card[5] == '2')
                {
                    strcpy(crpix2, first+1);

                    char *pch = strtok (crpix2," ,");
                    strcpy(crpix2, pch);
                }
                if (card[5] == '3')
                {
                    strcpy(crpix3, first+1);

                    char *pch = strtok (crpix3," ,");
                    strcpy(crpix3, pch);
                }
            }

            if (!strncmp(card, "CDELT", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';
                
                if (card[5] == '1')
                {
                    strcpy(cdelt1, first+1);
                    char *pch = strtok (cdelt1," ,");
                    strcpy(cdelt1, pch);
                    
                }
                
                if (card[5] == '2')
                {
                    strcpy(cdelt2, first+1);
                    char *pch = strtok (cdelt2," ,");
                    strcpy(cdelt2, pch);
                }
                
                if (card[5] == '3')
                {
                    strcpy(cdelt3, first+1);
                    char *pch = strtok (cdelt3," ,");
                    strcpy(cdelt3, pch);
                }
            }
            
            

        }
    }


    val1=crval1;
    val2=crval2;
    val3=crval3;
    pix1=crpix1;
    pix2=crpix2;
    pix3=crpix3;
    delt1=cdelt1;
    delt2=cdelt2;
    delt3=cdelt3;


    
    crval[0]=::atof(val1.c_str());
    crval[1]=::atof(val2.c_str());
    crval[2]=::atof(val3.c_str());
    cpix[0]=::atof(pix1.c_str());
    cpix[1]=::atof(pix2.c_str());
    cpix[2]=::atof(pix3.c_str());
    cdelt[0]=::atof(delt1.c_str());
    cdelt[1]=::atof(delt1.c_str());
    cdelt[2]=::atof(delt1.c_str());

    initSlice=crval[2]-(cdelt[2]*(cpix[2]-1));

//std::cout<<crval[0]<< " "<<crval[1]<< " "<<crval[2]<< " "<<cpix[0]<< " "<<cpix[1]<< " "<<cpix[2]<< " "<< cdelt[0]<< " "<<cdelt[1]<< " "<<cdelt[2]<< std::endl;
    
}

void fitsReader::printerror(int status) {


    if (status) {
        fits_report_error(stderr, status); /* print error report */
        exit( status );    /* terminate the program, returning error status */
    }
    return;
}



// TODO: first we read file to calculate RMS
void fitsReader::CalculateRMS(std::vector<VALS>& vals) {
    

    ReadHeader();

    fitsfile *fptr;
    int status = 0, nfound = 0, anynull = 0;
    long fpixel, nbuffer, npixels, ii, n=0;
    double meansquare=0;
    const int buffsize = 1000;
    
    
    float nullval, buffer[buffsize];
    char *fn=new char[filename.length() + 1];;
    strcpy(fn, filename.c_str());
    
    if ( fits_open_file(&fptr, fn, READONLY, &status) )
        printerror( status );
    
    delete []fn;

    if ( fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status) )
        printerror( status );
    
    npixels  = naxes[0] * naxes[1] * naxes[2];
    n=npixels;
    
    fpixel   = 1;
    nullval  = 0;
    datamin  = 1.0E30;
    datamax  = -1.0E30;

    int bad=0;



    //For every pixel

    int num2=0;

    while (npixels > 0) {




        nbuffer = npixels;
        if (npixels > buffsize)
            nbuffer = buffsize;
        
        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                           buffer, &anynull, &status) )
            printerror( status );



        for (ii = 0; ii < nbuffer; ii++)  {

            if (!std::isnan(buffer[ii])&&(buffer[ii]>=0.0f))
            {


                if ( buffer[ii] < datamin )
                    datamin = buffer[ii];
                if ( buffer[ii] > datamax   )
                    datamax = buffer[ii];

                meansquare+=buffer[ii]*buffer[ii];
                vals.push_back(VALS(num2,buffer[ii]));


            }
            else{
                bad++;
            }
            num2++;
        }
        npixels -= nbuffer;
        fpixel  += nbuffer;
    }

    n=n-bad;
    printf("pixesl=%d\n",n);
    double means=meansquare/n;
    rms=sqrt(means);


    if ( fits_close_file(fptr, &status) )
        printerror( status );
    


    return;
}
int fitsReader::GetNaxes(int i)
{

    return naxes[i];

}

