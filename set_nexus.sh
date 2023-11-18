#!/bin/bash
#DEFINE PATHS
export GEANT4_DIR=opt/geant4-v11.1.3-install
export ROOT_DIR=opt/root_v6.28.06
export GSL_DIR=opt/gsl-2.7.1
export HDF5_DIR=opt/hdf5-1.14.3/hdf5

#CONFIGURE GEANT4 ENV
source ${GEANT4_DIR}/bin/geant4.sh

#CONFIGURE ROOT ENV
source ${ROOT_DIR}/bin/thisroot.sh 

#HDF5 LIBRARIES
export HDF5_LIB=${HDF5_DIR}/lib
export HDF5_INC=${HDF5_DIR}/include

#ADD TO (DY)LD_LIBRARY_PATH
LD_LIBRARY_PATH=/usr/local/lib:${HDF5_LIB}:${GEANT4_DIR}/lib:${LD_LIBRARY_PATH}
if [ "$OSTYPE" = "darwin23" ]
then
    DYLD_LIBRARY_PATH=/usr/local/lib:${HDF5_LIB}:${GEANT4_DIR}/lib:${DYLD_LIBRARY_PATH}
fi
