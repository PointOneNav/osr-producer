ARG ARCH_PREFIX=
FROM ${ARCH_PREFIX}ubuntu

# Application requirements:
# - OpenSSL (Polaris)
# - Boost (Polaris, OSRProducer examples)
# - Glog (Polaris, OSRProducer examples)
# - Gflags (OSRProducer examples)
RUN apt-get update && apt-get install -y \
        build-essential \
        cmake \
        libboost-all-dev \
        libgflags-dev \
        libgoogle-glog-dev \
        libssl-dev \
        unzip \
        wget \
        && rm -rf /var/lib/apt/lists/*

WORKDIR /home/pointone

RUN mkdir -p ci
COPY compile.sh ci/

CMD bash
