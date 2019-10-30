#! /bin/bash
cd "`dirname \"$0\"`"
./premake/premake5_mac xcode4
git submodule init
git submodule update
