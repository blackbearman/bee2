FROM debian:latest

RUN apt-get update && apt-get install -y cmake git gcc doxygen clang

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src