#!/bin/bash
set -e

echo "Building Core Unit Tests container..."
docker build -t stradvision-unit-tests -f docker/test/unit_test.Dockerfile .

echo ""
echo "Running unit_tests (Payload, Session, etc)..."
docker run --rm stradvision-unit-tests
