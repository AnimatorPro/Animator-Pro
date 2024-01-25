#!/bin/bash
if [ -d ios ]
then
	rm -rf ios
fi

mkdir ios
pushd ios
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../thirdparty/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_BITCODE=OFF -DENABLE_ARC=ON -DENABLE_VISIBILITY=OFF ..
popd

