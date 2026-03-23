$ErrorActionPreference = "Stop"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "    Running StradVision System with Live ASan / UBSan       " -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $ProjectRoot

docker-compose -f docker/test/docker-compose-asan.yml build
docker-compose -f docker/test/docker-compose-asan.yml up

Write-Host "============================================================" -ForegroundColor Green
Write-Host "    ASan System Terminated. Check logs for memory leaks.    " -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
