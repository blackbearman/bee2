FROM s390x/fedora:rawhide

RUN dnf -y update

RUN dnf install -y gcc cmake clang doxygen git

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src
RUN git clone https://github.com/agievich/bee2.git

WORKDIR /usr/src/bee2
RUN mkdir build
RUN mkdir build_clang

WORKDIR /usr/src/bee2/build
RUN cmake ..
RUN make
# RUN make test
# RUN ctest --verbose
# RUN make install

WORKDIR /usr/src/bee2/build_clang
RUN export CC=/usr/bin/clang && cmake ..
RUN make
# RUN make test
RUN ctest --verbose

WORKDIR /usr/src
