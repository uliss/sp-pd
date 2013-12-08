#/bin/sh

PD_PATH="/Applications/Pd-extended.app/Contents/Resources/include/pdextended"
FLAGS="-m32 -undefined suppress -flat_namespace -I${PD_PATH}"

cd ../genlib
rm -f *.o
gcc -c $FLAGS -fPIC -o Offt.o Offt.cpp
gcc -c $FLAGS -fPIC -o Ooscil.o Ooscil.cpp
gcc -c $FLAGS -fPIC -o RandGen.o RandGen.cpp
gcc -c $FLAGS -fPIC -o FFTReal.o FFTReal.cpp
gcc -c $FLAGS -fPIC -o Obucket.o Obucket.cpp
gcc -c $FLAGS -fPIC -o Odelay.o Odelay.cpp
cd ../Spectacle
rm -f *.o
gcc -c $FLAGS -fPIC -o SpectacleBase.o SpectacleBase.cpp
gcc -c $FLAGS -fPIC -o SpectEQ.o SpectEQ.cpp
gcc -c $FLAGS -fPIC -o Spectacle.o Spectacle.cpp
cd ../spectdelay~
rm -f *.o

gcc -m32 -DPD $FLAGS -I../Spectacle -o spectdelay~.o -c spectdelay~.cpp

gcc -shared $FLAGS \
        -o spectdelay~.pd_darwin \
        ../Spectacle/Spectacle.o ../Spectacle/SpectacleBase.o  ../genlib/Odelay.o ../genlib/Obucket.o ../genlib/FFTReal.o ../genlib/Offt.o spectdelay~.o
