$ErrorActionPreference = "Stop"

Write-Host "Building integration test container..."
docker build -t stradvision-integration-test -f docker/test/integration_test.Dockerfile .

Write-Host ""
Write-Host "Running integration_test..."
docker run --rm stradvision-integration-test
