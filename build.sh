#!/bin/bash

cd build/tizen
cmake -DCMAKE_INSTALL_PREFIX=$DESKTOP_PREFIX .
make install -j8
cd ../..

