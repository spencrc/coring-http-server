FROM debian:bookworm-slim

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    clang \
    g++ \
    make \
    liburing-dev

WORKDIR /app
COPY . .