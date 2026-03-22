#!/bin/bash
set -e

echo "Building integration test container..."
docker build -t stradvision-integration-test -f docker/test/integration_test.Dockerfile .

echo ""
echo "Running integration_test..."
docker run --rm stradvision-integration-test
