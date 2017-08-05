# This sets up a debian environment with a cross-compiler, suitable for building SILVOS
FROM debian:stable
WORKDIR /root/packages
RUN apt-get update -qq && \
    apt-get install -y build-essential wget git && \
    apt-get install -y qemu-system-x86 && \
    wget https://s3-us-west-2.amazonaws.com/colin.cool/gcc-cross-deb/binutils-x86-64-elf_2.28-5%2B1_amd64.deb && \
    wget https://s3-us-west-2.amazonaws.com/colin.cool/gcc-cross-deb/gcc-x86-64-elf_6.3.0-18%2B1_amd64.deb && \
    apt-get install -y ./binutils-x86-64-elf_2.28-5+1_amd64.deb ./gcc-x86-64-elf_6.3.0-18+1_amd64.deb
