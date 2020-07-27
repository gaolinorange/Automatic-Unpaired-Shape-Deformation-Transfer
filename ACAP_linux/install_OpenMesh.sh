#!/bin/bash
if [ ! -d "./ACAP-build" ]; then
  mkdir ./ACAP-build
fi
cd ./ACAP-build

if [ ! -d "OpenMesh-build" ]; then
  mkdir OpenMesh-build
fi
cd OpenMesh-build

cmake ../../3rd/OpenMesh -DCMAKE_INSTALL_PREFIX=$(pwd)/install-custom -DBUILD_APPS=false
make
make install