$ErrorActionPreference = "Stop"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "    Running All Test Suites (Standard Fast Execution)       " -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

# Change working directory to project root
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $ProjectRoot

Write-Host "Building unified test container..."
docker build -t stradvision-all-tests -f docker/test/all_tests.Dockerfile .

Write-Host "Running all registered CTests in container..."
docker run --rm stradvision-all-tests

Write-Host "============================================================" -ForegroundColor Green
Write-Host "    All tests completed successfully!                       " -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
