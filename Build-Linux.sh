#!/bin/bash

chmod u+r+x Tools/Linux/premake5
Tools/Linux/premake5 gmake2 --dotnet=mono
make config=debug CC=gcc-11 CXX=g++-11 -j8