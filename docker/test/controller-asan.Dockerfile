FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build with ASan enabled
RUN cmake -S . -B build -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build build

FROM ubuntu:22.04 AS controller
# Install g++ simply to provide matching libasan and libubsan dynamically linked libraries
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/build/src/controller/controller /app/controller
COPY --from=builder /app/configs /app/configs
EXPOSE 9000
CMD ["/app/controller", "9000"]
