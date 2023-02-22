FROM s390x/fedora:rawhide

RUN dnf -y update

RUN dnf install -y gcc cmake clang doxygen git

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src
