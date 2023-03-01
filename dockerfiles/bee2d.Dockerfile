FROM debian:latest

RUN apt-get update && apt-get install -y cmake git gcc doxygen clang

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

COPY . /usr/src

WORKDIR /usr/src
RUN rm -rf ./build_gcc; mkdir build_gcc
RUN rm -rf ./build_clang; mkdir build_clang

WORKDIR /usr/src/build_gcc
RUN cmake ..
RUN make
RUN make test

WORKDIR /usr/src/build_clang
RUN cmake ..
RUN make
RUN make test

WORKDIR /usr/src