#!/bin/sh

if [ ! -d build_normal ];
then
  mkdir build_normal;
  cd build_normal;
  cmake .. -DCMAKE_BUILD_TYPE=Debug
else
  cd build_normal;
fi;

make
if [ $? != 0 ]
then
  exit
fi
rm -f example.archive
./example/example.bin check
./example/example.bin run
./example/example.bin invalidate -i 'fibonacci'
./example/example.bin clean
./example/example.bin run
cd ..

if [ ! -d build_mpi ];
then
  mkdir build_mpi;
  cd build_mpi;
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_MPI=True
else
  cd build_mpi;
fi;

make
if [ $? != 0 ]
then
  exit
fi
rm -f example.archive
./example/example.bin check
mpirun -np 2 ./example/example.bin run
./example/example.bin invalidate -i 'fibonacci'
./example/example.bin clean
mpirun -np 2 ./example/example.bin run
cd ..
