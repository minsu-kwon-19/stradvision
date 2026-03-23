#!/bin/bash
set -e

echo "Building Message Parser Test container..."
docker build -t stradvision-message-parser-test -f docker/test/message_parser_test.Dockerfile .

echo ""
echo "Running message_parser_test (CRC verification)..."
docker run --rm stradvision-message-parser-test
