#!/bin/bash

if [ ! -d "./build" ]; then 
    mkdir build 
fi

pushd build
clang ../source/data_desk_main.c -DBUILD_LINUX -o ../source/data_desk
popd