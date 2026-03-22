$ErrorActionPreference = "Stop"

Write-Host "Building duplicate message test container..."
docker build -t stradvision-duplicate-test -f docker/test/duplicate_test.Dockerfile .

Write-Host ""
Write-Host "Running duplicate_message_test..."
docker run --rm stradvision-duplicate-test
