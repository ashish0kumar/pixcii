#!/usr/bin/env bash

set -e

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is not installed"
    exit 1
fi

# Check if make is installed
if ! command -v make &> /dev/null; then
    echo "Error: make is not installed"
    exit 1
fi

echo "Building pixcii..."

rm -rf build
mkdir build
cd build
cmake ..
make -j"$(nproc)"

echo "Build completed successfully!"
echo "Run './build/pixcii -h' for usage information"