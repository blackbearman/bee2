## Start Qemu support for different platform to Docker
# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

# Build: docker build --platform linux/s390x -f dockerfiles/s390.Dockerfile .

FROM s390x/debian:latest

RUN apt-get update && apt-get install -y cmake git gcc clang

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src