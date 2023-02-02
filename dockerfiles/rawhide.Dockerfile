FROM fedora:rawhide

# RUN dnf install -y dnf-plugins-core

# RUN dnf copr enable -y kashcheyeu/bee2

# RUN dnf install -y bee2 --nogpgcheck

RUN dnf install -y gcc cmake doxygen git

RUN gcc --version
RUN cat /etc/os-release


WORKDIR /usr/src
RUN git clone https://github.com/agievich/bee2.git

WORKDIR /usr/src/bee2
RUN mkdir build
RUN ls

WORKDIR /usr/src/bee2/build
RUN cmake ..
RUN make
RUN make test
RUN make install

WORKDIR /usr/src
