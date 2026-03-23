#!/bin/bash
set -e

echo "============================================================"
echo "    Running All Test Suites (Standard Fast Execution)       "
echo "============================================================"

# Change working directory to project root
cd "$(dirname "$0")/.."

echo "Building unified test container..."
docker build -t stradvision-all-tests -f docker/test/all_tests.Dockerfile .

echo "Running all registered CTests in container..."
docker run --rm stradvision-all-tests
  
echo "============================================================"
echo "    All tests completed successfully!                       "
echo "============================================================"
