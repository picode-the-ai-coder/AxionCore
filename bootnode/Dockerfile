FROM ubuntu:20.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy bootnode files
COPY bootnode/ /app/bootnode/

# Build bootnode
RUN cd /app/bootnode && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .

# Create data and logs directories
RUN mkdir -p /app/data/chaindata /app/logs /app/metrics

# Expose ports
EXPOSE 30303/tcp 30303/udp 30304/udp 8545 8546 9090

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8545 || exit 1

# Set environment
ENV BOOTNODE_HOME=/app
ENV BOOTNODE_LOG_LEVEL=info

# Run bootnode
ENTRYPOINT ["/app/bootnode/build/bin/axion-bootnode"]
CMD ["--test-mode"]
