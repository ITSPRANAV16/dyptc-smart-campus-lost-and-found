# =====================================================================
# Dockerfile: Smart Campus Lost & Found Matching System (C++17 + Crow)
# Supports: Render.com, Railway.app, Fly.io, Self-Hosted Docker
# =====================================================================

FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    g++ \
    libsqlite3-dev \
    libboost-system-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Compile smart_campus binary for Linux 64-bit
RUN g++ -std=c++17 -O2 -Iinclude main.cpp -lsqlite3 -lpthread -o smart_campus

# Minimal Runtime Container
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libsqlite3-0 ca-certificates && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/smart_campus .
COPY --from=builder /app/templates ./templates
COPY --from=builder /app/data ./data
COPY --from=builder /app/style.css .
COPY --from=builder /app/*.html ./

# Default Cloud Port
EXPOSE 18080
ENV PORT=18080

CMD ["./smart_campus"]
