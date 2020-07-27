#!/bin/bash
if [ ! -d "./ACAP-build" ]; then
  mkdir ./ACAP-build
fi
cd ./ACAP-build

if [ ! -d "ACAP-build" ]; then
  mkdir ACAP-build
fi
cd ACAP-build

# export EIGEN_DIR=../../3rd
cmake ../.. -DEIGEN3_INCLUDE_DIRS=../../3rd
make
cp ../../3rd/gurobi801/linux64/lib/libgurobi80.so ./