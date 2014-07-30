#!/bin/sh

if [ ! -d build_normal ];
then
  mkdir build_normal;
  cd build_normal;
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DNO_MPI=True
else
  cd build_normal;
fi;

make test
cd ..

if [ ! -d build_mpi ];
then
  mkdir build_mpi;
  cd build_mpi;
  cmake .. -DCMAKE_BUILD_TYPE=Debug
else
  cd build_mpi;
fi;

make test
cd ..
