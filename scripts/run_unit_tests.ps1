$ErrorActionPreference = "Stop"

Write-Host "Building Core Unit Tests container..."
docker build -t stradvision-unit-tests -f docker/test/unit_test.Dockerfile .

Write-Host ""
Write-Host "Running unit_tests (Payload, Session, etc)..."
docker run --rm stradvision-unit-tests
