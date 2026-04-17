FROM alpine:3.23.3

RUN adduser -D user

RUN mkdir /Bootloader /workspaces
WORKDIR /Bootloader

# add required packages
RUN apk update \
    && apk add \
    clang \
    clang-extra-tools \
    gdb-multiarch \
    cmake \
    samurai \
    g++ \
    g++-arm-none-eabi \
    qemu-system-arm \
    python3 \
    py3-pycryptodomex \
    xxd \
    gzip \
    git \
    openssl \
    asciidoctor \
    && gem install asciidoctor-pdf \
    && true

