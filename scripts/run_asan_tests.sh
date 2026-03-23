#!/bin/bash
set -e

echo "============================================================"
echo "    Running Unit Tests with ASan & UBSan (Memory Leak Check)"
echo "============================================================"

cd "$(dirname "$0")/.."

echo "Building ASan test container..."
docker build -t stradvision-asan-tests -f docker/test/asan_test.Dockerfile .

echo "Running tests in isolated container..."

docker run --rm \
  -e ASAN_OPTIONS=detect_leaks=1:symbolize=1 \
  stradvision-asan-tests
  
echo "============================================================"
echo "    ASan verification completed successfully! No Leaks!"
echo "============================================================"
