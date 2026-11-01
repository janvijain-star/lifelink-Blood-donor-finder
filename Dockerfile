# ==============================================================================
# LifeLink - Blood Donor Finder (Production Dockerfile)
# Compatible with Render, Railway, Fly.io, AWS, and GCP
# ==============================================================================

# Build Stage: Compile C++ application on Linux
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libsqlite3-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY backend/ ./backend/
COPY database/ ./database/
COPY frontend/ ./frontend/

WORKDIR /app/backend
# Compile server with POSIX threads and dynamic linking
RUN g++ -std=c++17 -O2 -Iinclude db_manager.cpp server.cpp sqlite3.c -o lifelink_server -lpthread -ldl

# Production Runtime Stage: Ultra-lightweight container
FROM ubuntu:22.04

WORKDIR /app
COPY --from=builder /app/backend/lifelink_server /app/backend/
COPY --from=builder /app/frontend/ /app/frontend/
COPY --from=builder /app/database/ /app/database/

ENV PORT=8080
EXPOSE 8080

CMD ["/app/backend/lifelink_server"]
