FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build the test. Ensure clean start.
RUN cmake -S . -B build && cmake --build build --config Debug

FROM ubuntu:22.04 AS tester
WORKDIR /app
COPY --from=builder /app/build/tests/command_retry_test /app/command_retry_test

CMD ["/app/command_retry_test"]
