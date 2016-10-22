FROM ubuntu:xenial

MAINTAINER Gerard Choinka <gerard.choinka@gmail.com>

#docker build --build-arg N_BUILD_JOBS=4 -f Dockerfile -t ambrosys/gpcxx .

ENV DEBIAN_FRONTEND noninteractive
ENV TERM xterm
RUN locale-gen --no-purge en_US.UTF-8
ENV LC_ALL en_US.UTF-8
RUN update-locale LANG=en_US.UTF-8

ARG N_BUILD_JOBS=1
ARG GPCXX_GIT_URL=https://github.com/gchoinka/gpcxx.git

RUN apt-get  update && \
    apt-get -y install \
        build-essential \
        cmake \
        cmake-curses-gui \
        ninja-build \
        gcc \
        g++ \
        git \
        doxygen \
        xsltproc

#installing git from source to get the --jobs option in git clone, not realy necessary but it make cloning boost faster
RUN git clone --recurse-submodules --depth 1 -b v2.10.1 https://github.com/git/git.git /opt/git
RUN apt-get -y remove git && apt-get -y build-dep git && apt-get -y install libssl-dev libcurl4-openssl-dev gettext 
RUN cd /opt/git/ && make -j${N_BUILD_JOBS} && make prefix=/usr/local install

RUN git clone --recurse-submodules --recursive --jobs ${N_BUILD_JOBS} "${GPCXX_GIT_URL}" /opt/gpcxx
RUN apt-get -y build-dep libboost-all-dev && \
    apt-get -y install libbz2-dev python-dev && \
    cd /opt/gpcxx/third_party/boost && ./bootstrap.sh && ./b2 -j${N_BUILD_JOBS} cxxflags=-std=c++14
RUN mkdir /opt/gpcxx/build && cd /opt/gpcxx/build && cmake .. -DGPCXX_BUILD_DOCS=0 -G Ninja && ninja && ninja test
