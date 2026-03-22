FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build the unit tests. Ensure clean start.
RUN cmake -S . -B build && cmake --build build --target unit_tests --config Debug

FROM ubuntu:22.04 AS tester
WORKDIR /app
COPY --from=builder /app /app

RUN apt-get update && apt-get install -y cmake
CMD ["/app/build/tests/unit_tests"]
