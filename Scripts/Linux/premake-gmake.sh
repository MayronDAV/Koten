#!/bin/bash

if [ $TRAVIS_OS_NAME == linux ]; then
chmod u+r+x Tools/Linux/premake5
Tools/Linux/premake5 gmake2
else
Tools/premake5 gmake2
fi
make config=debug CC=gcc-11 CXX=g++-11 -j4
