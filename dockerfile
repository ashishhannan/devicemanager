# 1. Base image
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# 2. Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libboost-system-dev libpqxx-dev libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# 3. Copy source
WORKDIR /app
COPY . .

# 4. Build
RUN mkdir -p build && cd build && cmake .. && make -j

# --------------------------
# Runtime image
# --------------------------
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libboost-system-dev libpqxx-dev libpq-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# copy binary only
COPY --from=builder /app/build/ev07b_server /app/ev07b_server

# Railway sets PORT env var automatically
ENV PORT=5050

EXPOSE 5050

# Start server
CMD ["./ev07b_server"]

