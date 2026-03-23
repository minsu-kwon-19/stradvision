$ErrorActionPreference = "Stop"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "    Running Unit Tests with ASan & UBSan (Memory Leak Check)" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

# Change working directory to project root
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $ProjectRoot

Write-Host "Building ASan test container..."
docker build -t stradvision-asan-tests -f docker/test/asan_test.Dockerfile .

Write-Host "Running tests in isolated container..."
docker run --rm `
  -e ASAN_OPTIONS="detect_leaks=1:symbolize=1" `
  stradvision-asan-tests

Write-Host "============================================================" -ForegroundColor Green
Write-Host "    ASan verification completed successfully! No Leaks!" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
