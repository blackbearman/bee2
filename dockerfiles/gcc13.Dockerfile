FROM rhub/gcc13:latest

RUN dnf install -y cmake doxygen git

RUN gcc --version
RUN cat /etc/os-release

WORKDIR /usr/src
RUN git clone https://github.com/agievich/bee2.git

WORKDIR /usr/src/bee2
RUN mkdir build

WORKDIR /usr/src/bee2/build
RUN cmake ..
RUN make
RUN make test
RUN make install

WORKDIR /usr/src