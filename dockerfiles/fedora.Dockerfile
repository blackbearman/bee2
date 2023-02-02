FROM rhub/fedora-gcc:latest

RUN dnf install -y cmake doxygen git

WORKDIR /usr/src
RUN git clone https://github.com/agievich/bee2.git

RUN mkdir bee2/build
WORKDIR /usr/src/bee2/build
RUN cmake ..
RUN make
RUN make test
RUN make install
RUN gcc --version

WORKDIR /usr/src