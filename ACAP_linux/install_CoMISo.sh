#!/bin/bash
if [ ! -d "./ACAP-build" ]; then
  mkdir ./ACAP-build
fi
cd ./ACAP-build

if [ ! -d "CoMISo-build" ]; then
  mkdir CoMISo-build
fi
cd CoMISo-build

tar -xf ../../3rd/gurobi8.0.1_linux64.tar.gz

export GMM_DIR=../../3rd/gmm/include
export EIGEN_DIR=../../3rd
export GUROBI_HOME=../../3rd/gurobi801/linux64
cmake ../../3rd/CoMISo
make
