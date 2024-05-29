#!/bin/bash

CPP_FLAGS="-std=c++20 -O3 -static"
SRC_DIR=./src/*.cpp
COMPRESSOR=./compressor_main.cpp
COMPRESSOR_NAME=./exec_linux/compressor
DECOMPRESSOR=./decompressor_main.cpp
DECOMPRESSOR_NAME=./exec_linux/decompressor

g++ $CPP_FLAGS $SRC_DIR $COMPRESSOR -o $COMPRESSOR_NAME > compressor_compile.log 2>&1

if [ $? -eq 0 ]; then
    echo Compressor compilation done.
else
    echo Compressor compilation failed.
    exit 1
fi

g++ $CPP_FLAGS $SRC_DIR $DECOMPRESSOR -o $DECOMPRESSOR_NAME > decompressor_compile.log 2>&1

if [ $? -eq 0 ]; then
    echo Decompressor compilation done.
else
    echo Decompressor compilation failed.
    exit 1
fi