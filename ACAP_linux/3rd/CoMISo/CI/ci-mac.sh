#!/bin/bash

#Exit on any error
set -e

LANGUAGE=$1

echo "============================="
echo "Build information"
echo "============================="

PATH=$PATH:/opt/local/bin
export PATH

OPTIONS=""

if [ "$LANGUAGE" == "C++98" ]; then
  echo "Building with C++98";
elif [ "$LANGUAGE" == "C++11" ]; then
  echo "Building with C++11";
  OPTIONS="$OPTIONS -DCMAKE_CXX_FLAGS='-std=c++11' "
fi

echo "============================="
echo "Software Directory content:  "
echo "============================="

ls ~/sw

echo "============================="
echo "Starting Build: "
echo "============================="

#########################################
# Base Submodule init
git submodule init
git submodule update


#########################################
# Build release version
#########################################

if [ ! -d build-release ]; then
  mkdir build-release
fi

cd build-release

cmake -DCMAKE_BUILD_TYPE=Release -DSTL_VECTOR_CHECKS=ON $OPTIONS ../

#build it
make

cd ..


#########################################
# Build Debug version and Unittests
#########################################

if [ ! -d build-debug ]; then
  mkdir build-debug
fi

cd build-debug

cmake -DCMAKE_BUILD_TYPE=Debug -DSTL_VECTOR_CHECKS=ON $OPTIONS ../

#build it
make
