FROM --platform=$TARGETPLATFORM btls/debian:cdev
ARG BUILDPLATFORM
ARG TARGETPLATFORM

COPY . /usr/src

RUN gcc --version
RUN clang --version
RUN cat /etc/os-release

WORKDIR /usr/src
RUN rm -rf ./build_gcc; mkdir build_gcc
RUN rm -rf ./build_clang; mkdir build_clang

WORKDIR /usr/src/build_gcc
RUN echo "I am running on $BUILDPLATFORM, building for $TARGETPLATFORM" 
RUN if [ \( "$TARGETPLATFORM" = "linux/386" \) -o \( "$TARGETPLATFORM" = "linux/arm/v7" \) ] ; then CC=gcc cmake -DBASH_PLATFORM=BASH_32 .. ; else CC=clang cmake .. ; fi
RUN make
RUN ctest --verbose

WORKDIR /usr/src/build_clang
RUN CC=clang cmake ..
RUN make
RUN ctest --verbose

WORKDIR /usr/src