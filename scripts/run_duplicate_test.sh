#!/bin/bash
set -e

echo "Building duplicate message test container..."
docker build -t stradvision-duplicate-test -f docker/test/duplicate_test.Dockerfile .

echo ""
echo "Running duplicate_message_test..."
docker run --rm stradvision-duplicate-test
