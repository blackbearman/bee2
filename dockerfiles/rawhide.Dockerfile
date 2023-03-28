FROM --platform=$TARGETPLATFORM fedora:rawhide

RUN dnf -y update

RUN dnf install -y gcc clang cmake doxygen

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src
