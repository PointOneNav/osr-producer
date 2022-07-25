# Point One `OSRProducer` Example Applications

This project provides example applications demonstrating how to use Point One's `libosr_producer` library.
`libosr_producer` defines the `OSRProducer` C++ class, which decodes incoming observation-space representation (OSR) and
state-space representation (SSR) GNSS corrections data. It automatically selects the best available data for your
location, and forwards corrections to your receiver using standard RTCM 10403.3 messages, converting SSR data to OSR
measurements as needed.

The library is capable of receiving OSR and SSR data over IP (e.g., cellular internet connection) via a Polaris protocol
or NTRIP connection. Additionally, SSR can be received over the air using an L-band satellite connection.

## Example Applications

- [Septentrio GNSS receiver connected via serial](examples/septentro_osr_example)

## Requirements
- [CMake 3.14+](https://cmake.org/)
- [Google gflags 2.2.2+](https://github.com/gflags/gflags)
- [Google glog 0.4.0+](https://github.com/google/glog)
- [Boost 1.58+](https://www.boost.org/)
- [OpenSSL](https://www.openssl.org/) or [BoringSSL](https://boringssl.googlesource.com/boringssl/) (required for
  Polaris connections)

## Building From Source

1. If necessary, install dependencies:
   ```bash
   sudo apt-get install cmake libssl-dev libgflags-dev libgoogle-glog-dev libboost-all-dev
   ```

2. Then build with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
   - To cross-compile for a different architecture (e.g., compile an ARM executable on an Intel host) specify the
     `CMAKE_TOOLCHAIN_FILE` argument:
     ```bash
     cmake -DCMAKE_TOOLCHAIN_FILE=<your-toolchain-file> ..
     ```

When run, `cmake` will automatically download the correct pre-compiled version of `libosr_producer` from Point One
for your target architecture.