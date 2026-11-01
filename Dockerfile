# ==============================================================================
# LifeLink - Blood Donor Finder (Production Dockerfile)
# Multi-stage build for C++ Crow Backend + SQLite3 + Frontend
# Compatible with Render, Railway, Fly.io, AWS, and GCP
# ==============================================================================

# ------------------------------------------------------------------------------
# Build Stage: Compile C++ application on Linux
# ------------------------------------------------------------------------------
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    libsqlite3-dev \
    libasio-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy application source code
COPY backend/ ./backend/
COPY database/ ./database/
COPY frontend/ ./frontend/

WORKDIR /app/backend

# Compile C++ Crow server with C++17, optimizations, POSIX threads, and SQLite amalgamation
RUN g++ -std=c++17 -O2 -Iinclude db_manager.cpp server.cpp sqlite3.c -o lifelink_server -lpthread -ldl
RUN chmod +x lifelink_server

# ------------------------------------------------------------------------------
# Production Runtime Stage: Ultra-lightweight, secure container
# ------------------------------------------------------------------------------
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    libsqlite3-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary and static assets from builder stage
COPY --from=builder /app/backend/lifelink_server /app/backend/lifelink_server
COPY --from=builder /app/frontend /app/frontend
COPY --from=builder /app/database /app/database

# Ensure execution rights for binary and full read/write rights for SQLite database
RUN chmod +x /app/backend/lifelink_server && chmod -R 777 /app/database

# Default port (Render overrides this dynamically via the PORT environment variable)
ENV PORT=8080
EXPOSE 8080

CMD ["/app/backend/lifelink_server"]
