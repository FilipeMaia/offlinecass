#!/bin/tcsh 

# Variables to change
# The prefix where hdf5 was installed.
setenv HDF5DIR /sw

# The rest of the script should not require changes as long as qmake is in the path

if ( ! -e $HDF5DIR/include/hdf5.h) then
	echo "Could not find hdf5.h! Please edit HDF5DIR in the script."
	exit 1
endif

if ( `uname -s` == "Linux") then
set arch = `uname -m`-linux
else
set arch = `uname -m`-`uname -s`
endif
setenv LCLSSYSINCLUDE `pwd`/LCLS
setenv LCLSSYSLIB `pwd`/LCLS/build/pdsdata/lib/$arch
echo LCLSSYSINCLUDE set to $LCLSSYSINCLUDE
echo LCLSSYSLIB set to $LCLSSYSLIB
echo HDF5DIR set to $HDF5DIR
echo Building LCLS...
rm -rf LCLS/build/pdsdata/lib/$arch
cd LCLS/pdsdata
make $arch
cd ../../
echo Building cass...
make clean
qmake -r
make

