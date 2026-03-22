$ErrorActionPreference = "Stop"

Write-Host "Building Message Parser Test container..."
docker build -t stradvision-message-parser-test -f docker/test/message_parser_test.Dockerfile .

Write-Host ""
Write-Host "Running message_parser_test (CRC verification)..."
docker run --rm stradvision-message-parser-test
