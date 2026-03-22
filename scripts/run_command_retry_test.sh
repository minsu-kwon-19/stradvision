#!/bin/bash
set -e

echo "Building test container..."
docker build -t stradvision-test -f docker/test/retry_test.Dockerfile .

echo ""
echo "Running command_retry_test..."
docker run --rm stradvision-test
