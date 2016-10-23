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
ARG GPCXX_ROOT=/opt/gpcxx
ARG BOOST_ROOT=/opt/gpcxx/third_party/boost
ARG THIRD_PARTY_INSTALL_ROOT=/opt/gpcxx/third_party/install

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
        xsltproc \
        wget

#installing git from source to get the --jobs option in git clone, not realy necessary but it make cloning boost faster
RUN git clone --recurse-submodules --depth 1 -b v2.10.1 https://github.com/git/git.git /opt/git
RUN apt-get -y remove git && apt-get -y build-dep git && apt-get -y install libssl-dev libcurl4-openssl-dev gettext 
RUN cd /opt/git/ && make -j${N_BUILD_JOBS} && make prefix=/usr/local install

RUN git clone --recurse-submodules --recursive --jobs ${N_BUILD_JOBS} "${GPCXX_GIT_URL}" /opt/gpcxx
RUN apt-get -y build-dep libboost-all-dev && \
    apt-get -y install libbz2-dev python-dev && \
    mkdir -p "${BOOST_ROOT}" && \
    wget -qO- https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.bz2 | tar xvj -C "${BOOST_ROOT}" --strip=1 && \
    cd "${BOOST_ROOT}" && ./bootstrap.sh && ./b2 --prefix="${THIRD_PARTY_INSTALL_ROOT}" -j${N_BUILD_JOBS} cxxflags=-std=c++14 install
RUN mkdir "${GPCXX_ROOT}/build" && cd "${GPCXX_ROOT}/build" && cmake .. -DGPCXX_BUILD_DOCS=0 -G Ninja -DBOOST_ROOT="${THIRD_PARTY_INSTALL_ROOT}" && ninja && ninja test
