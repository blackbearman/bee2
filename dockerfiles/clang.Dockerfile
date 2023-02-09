FROM rhub/gcc13:latest

RUN dnf install -y cmake doxygen git clang

WORKDIR /usr/src

RUN clang --version
RUN cat /etc/os-release

RUN git clone https://github.com/agievich/bee2.git

RUN mkdir bee2/build

WORKDIR /usr/src/bee2/build

# CMake honors the environment variables CC and CXX upon detecting the C and C++ compiler to use
RUN export CC=/usr/bin/clang && cmake ..
RUN make
RUN make test
RUN make install

WORKDIR /usr/src