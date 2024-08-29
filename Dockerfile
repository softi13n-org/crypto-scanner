FROM ubuntu:22.04

COPY . /crypto
WORKDIR /crypto

RUN apt-get update

RUN apt-get install -y \
  build-essential \
  cmake \
  llvm \
  clang-14 \
  clang \
  lld-14 \
  lld \
  libssl-dev \
  ;

RUN apt-get install -y \
  wget \
  curl \
  pkg-config \
  zip unzip \
  git \
  ;

ARG BOOST_MINOR_VERSION=82

RUN apt-get install -y \
  build-essential \
  g++ \
  python3-dev \
  autotools-dev \
  libicu-dev \
  libbz2-dev \
  libboost-all-dev \
  ;

RUN cd /tmp/ \
  && wget https://boostorg.jfrog.io/artifactory/main/release/1.${BOOST_MINOR_VERSION}.0/source/boost_1_${BOOST_MINOR_VERSION}_0.tar.bz2 \
  && tar --bzip2 -xf boost_1_${BOOST_MINOR_VERSION}_0.tar.bz2

RUN cd /tmp/boost_1_${BOOST_MINOR_VERSION}_0/ \
  && ./bootstrap.sh --prefix=/usr/local \
  && ./b2 install \
  ;

RUN cd /tmp/ \
  && wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz \
  && mkdir /opt/vcpkg \
  && tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg \
  && /opt/vcpkg/bootstrap-vcpkg.sh \
  && ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg \
  && /usr/local/bin/vcpkg install quill \
  && cd /crypto && rm -rf /tmp/ \
  ;

CMD ["bash", "-c", "cd /crypto && rm $(ls -d $(PWD)/logs/spread/* | grep -v $(PWD)/logs/spread/columns.csv) && (rm -f logs/pure/*.log logs/*.log || true) && cmake -S . -B build && cd build && make && cd .. && ./crypto"]
