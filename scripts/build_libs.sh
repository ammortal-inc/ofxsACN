#!/bin/bash

# Get the directory of the script
script_dir=$(dirname "$0")
cd ${script_dir}
cd ../deps/sACN

# Make directory, replacing if it already exists
if [ -d build ]; then
    rm -rf build
fi

# Create a new build directory
mkdir build
# Change to the build directory
cd build
# Run cmake to configure the build system
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF
# Build the project
cmake --build .
# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

# Change to addon directory
cd ../../../

# Create libs directory if it doesn't exist
mkdir -p libs

# Copy the static library to the parent directory
cp deps/sACN/build/src/libsACN.a libs/
cp deps/sACN/build/_deps/etcpal-build/src/libEtcPal.a libs/libEtcPal.a

# Create include directory if it doesn't exist
mkdir -p include/sacn
mkdir -p include/etcpal

# Copy header files to the include directory (adjust patterns if needed)
cp -r deps/sACN/include/sacn/* include/sacn/
cp -r deps/sACN/build/_deps/etcpal-src/include/etcpal/* include/etcpal/
cp -r deps/sACN/build/_deps/etcpal-src/include/os/linux/etcpal/* include/etcpal/

echo "Build completed successfully"