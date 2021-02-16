cmake_minimum_required(VERSION 3.8)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES 
render_remote.cpp 
Base.cpp
VulkanTools.cpp 
VulkanBuffer.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c_utils)
set(SPLOTCH_SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/transform.cxx
   ${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/error_handling.cc
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/mpi_support.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/paramfile.cc 
${CMAKE_CURRENT_SOURCE_DIR}/reader/gadget_reader.cc 
${CMAKE_CURRENT_SOURCE_DIR}/fitsReader.cxx 
${CMAKE_CURRENT_SOURCE_DIR}/reader/bin_reader.cc 
${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotchutils.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotch.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/scenemaker.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotch_host.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/new_renderer.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/walltimer.cc 
${CMAKE_CURRENT_SOURCE_DIR}/c_utils/walltime_c.c
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/string_utils.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/announce.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/ls_image.cc 

)

#Build FITSIO

#ExternalProject_Add(fitsio
#  URL https://github.com/HEASARC/cfitsio.git
#  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/fitsio
#  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
#)

#include(${CMAKE_CURRENT_BINARY_DIR}/lib/fitsio/fitsio-targets.cmake)

#ExternalProject_Get_Property(fitsio install_dir)
#include_directories(${install_dir}/include)

#add_dependencies(project_b_exe project_a)
#target_link_libraries(project_b_exe ${install_dir}/lib/alib.lib)




#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/CFITSIO/include)
#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/CFITSIO/lib)




add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SPLOTCH_SOURCE} )


#find_package(glfw3 3.3 REQUIRED)



find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
  message(STATUS "VULKAN found")
  include_directories(${Vulkan_INCLUDE_DIR})
  link_libraries (${Vulkan_LIBRARY})
endif (Vulkan_FOUND)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})

include (ExternalProject)

# Begin: External Third Party Library
set (libFitsio ThirdPartyFitsioLibrary)
ExternalProject_Add (${libFitsio}
PREFIX          ${CMAKE_CURRENT_BINARY_DIR}/${libFitsio}


# Begin: Download Source from GIT Repository
  GIT_REPOSITORY  https://github.com/HEASARC/cfitsio.git
   GIT_TAG         master
   GIT_SHALLOW     ON
# End: Download Source from GIT Repository

# Begin: CMAKE Comamnd Argiments
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${libFitsio}
CMAKE_ARGS      -DUSE_SHARED_LIBRARY:BOOL=ON
# End: CMAKE Comamnd Argiments    
)

# The above ExternalProject_Add(...) construct wil take care of \
# 1. Downloading sources
# 2. Building Object files
# 3. Install under DCMAKE_INSTALL_PREFIX Directory

# Acquire Installation Directory of 
ExternalProject_Get_Property (${libFitsio} install_dir)

# Import librarues from External Project required by Target Project
add_library (cfitsio SHARED IMPORTED)
if (APPLE)
set_target_properties (cfitsio PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libcfitsio.dylib) 
else ()
set_target_properties (cfitsio PROPERTIES IMPORTED_LOCATION ${install_dir}/lib64/libcfitsio.so) #.so - find does not work so not sure how to fix it
endif (APPLE)

set_target_properties(cfitsio PROPERTIES IMPORTED_NO_SONAME ON)
link_directories(${install_dir}/lib)


# Begin: Target Project depends on Third Party Component
add_dependencies(${PROJECT_NAME} ${libFitsio})

# Begin: Importing Headers & Library of Third Party built using ExternalProject_Add(...)
# Include PATH that has headers required by Target Project
include_directories (${install_dir}/include)




target_link_libraries(${PROJECT_NAME} cfitsio )
#target_link_libraries(${PROJECT_NAME} glfw )