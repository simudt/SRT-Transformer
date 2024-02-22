#!/bin/bash

# Define variables
SOURCE="src/srt-processor.cpp"
OUTPUT="SRTTransformer"
CXXFLAGS="-std=c++11 -Wall -Wextra -O2 -g"

# Compile the code
g++ $CXXFLAGS -o $OUTPUT $SOURCE

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful"
else
    echo "Compilation failed"
    exit 1
fi
