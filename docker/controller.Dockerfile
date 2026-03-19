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

FROM ubuntu:22.04 AS controller
WORKDIR /app
COPY --from=builder /app/build/src/controller/controller /app/controller
COPY --from=builder /app/configs /app/configs
EXPOSE 9000
CMD ["/app/controller", "9000"]
