# ================================================================
# EmergiCare Backend - Multi-Stage Docker Build
# C++ Drogon Framework
# ================================================================

# ----------------------------------------------------------------
# Stage 1: Builder
# Uses the official Drogon image which has all dev dependencies
# pre-installed (cmake, g++, libssl, libpq, jsoncpp, etc.)
# ----------------------------------------------------------------
FROM drogonframework/drogon:latest AS builder

WORKDIR /app

# Copy build system files first for better Docker layer caching.
# CMake will download jwt-cpp via FetchContent on first configure.
COPY CMakeLists.txt .

# Pre-configure to cache the FetchContent downloads (jwt-cpp)
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release 2>/dev/null || true

# Copy all source files
COPY main.cpp   .
COPY config.json .
COPY src/       src/

# Full configure + compile
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel $(nproc)

# Copy config to build output directory
RUN cp config.json build/

# ----------------------------------------------------------------
# Stage 2: Runtime
# Smaller image — only binary + runtime libraries
# ----------------------------------------------------------------
FROM ubuntu:22.04

# Install only the runtime shared libraries needed by the binary
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libpq5 \
    libjsoncpp25 \
    libcares2 \
    zlib1g \
    libbrotli1 \
    libcrypt1 \
    libhiredis0.14 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled server binary from builder stage
COPY --from=builder /app/build/emergicare_server /usr/local/bin/emergicare_server

# Copy Drogon server configuration
COPY --from=builder /app/build/config.json /app/config.json

EXPOSE 3001

CMD ["emergicare_server"]
