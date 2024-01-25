#!/bin/bash
./autogen.sh
./configure ${LIBFFI_DEBUG_FLAGS} --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --disable-shared --disable-docs
