FROM ubuntu:22.04

# Install basic toolchain
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build for all test
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
RUN cmake --build build

WORKDIR /app
CMD ["ctest", "--test-dir", "build", "--output-on-failure", "-VV"]
