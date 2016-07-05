FROM ubuntu:xenial

MAINTAINER Gerard Choinka <gerard.choinka@gmail.com>

#docker build -f Dockerfile -t ambrosys/gpcxx .

ENV DEBIAN_FRONTEND noninteractive
ENV TERM xterm
RUN locale-gen --no-purge en_US.UTF-8
ENV LC_ALL en_US.UTF-8
RUN update-locale LANG=en_US.UTF-8

RUN apt-get  update && \
    apt-get -y install \
        build-essential \
        cmake \
        cmake-curses-gui \
        ninja-build \
        gcc \
        g++ \
        git \
        libboost-all-dev \
        doxygen \
        xsltproc

RUN git clone --recurse-submodules --depth 1 -b v2.9.0 https://github.com/git/git.git /opt/git
RUN apt-get -y remove git && apt-get -y build-dep git && apt-get -y install libssl-dev libcurl4-openssl-dev gettext 
RUN cd /opt/git/ && make -j8 && make prefix=/usr/local install
RUN git clone --recurse-submodules --recursive --jobs 4 https://github.com/gchoinka/gpcxx.git /opt/gpcxx

RUN cd /opt/gpcxx/third_party/boost && apt-get -y build-dep libboost-all-dev && apt-get -y install libbz2-dev && ./bootstrap.sh && ./b2 -j4 cxxflags=-std=c++14 
RUN mkdir /opt/gpcxx/build && cd /opt/gpcxx/build && cmake .. -DGPCXX_BUILD_DOCS=0 -DGPCXX_BUILD_PERFORMANCE=0 -G Ninja && ninja && ninja test
