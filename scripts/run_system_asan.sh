#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "============================================================"
echo "    Running StradVision System with Live ASan / UBSan       "
echo "============================================================"

docker-compose -f docker/test/docker-compose-asan.yml build
docker-compose -f docker/test/docker-compose-asan.yml up

echo "============================================================"
echo "    ASan System Terminated. Check logs for memory leaks.    "
echo "============================================================"
