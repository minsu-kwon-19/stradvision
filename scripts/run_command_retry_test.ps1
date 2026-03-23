$ErrorActionPreference = "Stop"

Write-Host "Building test container..."
docker build -t stradvision-test -f docker/test/retry_test.Dockerfile .

Write-Host ""
Write-Host "Running command_retry_test..."
docker run --rm stradvision-test
