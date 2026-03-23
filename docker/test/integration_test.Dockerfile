FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build && cmake --build build --config Debug

FROM ubuntu:22.04 AS tester

WORKDIR /app
COPY --from=builder /app /app

RUN apt-get update && apt-get install -y cmake
CMD ["/app/build/tests/integration_test"]
