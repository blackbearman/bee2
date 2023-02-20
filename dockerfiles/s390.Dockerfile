## Start Qemu support for different platform to Docker
# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

# Build: docker build --platform linux/s390x -f dockerfiles/s390.Dockerfile .

FROM s390x/debian:latest

RUN apt-get update && apt-get install -y cmake git

RUN gcc --version
RUN cat /etc/os-release

WORKDIR /usr/src
RUN git clone https://github.com/agievich/bee2.git

RUN mkdir bee2/build
WORKDIR /usr/src/bee2/build
RUN cmake -DCMAKE_BUILD_TYPE="Release" -DBUILD_FAST="ON" ..
RUN make
RUN make test
RUN make install
RUN gcc --version

WORKDIR /usr/src