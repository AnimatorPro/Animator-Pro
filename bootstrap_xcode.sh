#!/bin/bash
if [ -d xcode ]
then
	rm -rf xcode
fi

mkdir xcode
pushd xcode
cmake -G Xcode ..
open animator-pro.xcodeproj
popd

