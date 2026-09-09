#!/bin/bash

# Simple test script for the scheduler

echo "Building scheduler..."
make clean
make

if [ ! -f ./scheduler ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Running tests..."
echo ""

echo "Test 1: Rate-Monotonic with voo.txt"
./scheduler rate tests/voo.txt
if [ -f rate_mla.out ]; then
    echo "✓ Output file created: rate_mla.out"
    head -20 rate_mla.out
else
    echo "✗ Output file not created"
fi

echo ""
echo "Test 2: EDF with voo.txt"
./scheduler edf tests/voo.txt
if [ -f edf_mla.out ]; then
    echo "✓ Output file created: edf_mla.out"
    head -20 edf_mla.out
else
    echo "✗ Output file not created"
fi

echo ""
echo "Test 3: Rate-Monotonic with test1.txt"
./scheduler rate tests/test1.txt
if [ -f rate_mla.out ]; then
    echo "✓ Output file created"
else
    echo "✗ Output file not created"
fi

echo ""
echo "Test 4: Error handling - invalid algorithm"
./scheduler invalid tests/voo.txt
if [ $? -ne 0 ]; then
    echo "✓ Error correctly detected"
fi

echo ""
echo "Test 5: Error handling - missing file"
./scheduler rate nonexistent.txt
if [ $? -ne 0 ]; then
    echo "✓ Error correctly detected"
fi

echo ""
echo "Tests completed!"
