FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build && cmake --build build --config Release

FROM ubuntu:22.04 AS agent
WORKDIR /app
COPY --from=builder /app/build/src/agent/agent /app/agent
ENTRYPOINT ["/app/agent"]
