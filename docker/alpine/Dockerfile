FROM alpine:3.14.2 as builder

ARG CFLAGS
ARG LDFLAGS

# libc-dev is probably an alias to musl-dev
# somehow ld depends on gcc package (libgcc or something) even when linking clang objects
RUN apk add --no-cache \
  build-base clang clang-dev clang-static llvm-dev llvm-static musl-dev \
  make \
  binutils \
  openssl-libs-static \
  openssl-dev 

ENV CC=clang
WORKDIR /app

COPY . .

RUN make CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" release
