# osr-producer
OSRProducer Library And Sample Applications

This project contains example applications making use of the `OSProducer`
library.

# Building the Examples

If necessary, install dependencies:
```bash
sudo apt-get install cmake libgflags-dev libgoogle-glog-dev libboost-all-dev
```

Then build with CMake:
```bash
mkdir build
cd build
cmake ..
make
```

To cross compile (eg build an Arm executable on an Intel host) pass `-DCMAKE_TOOLCHAIN_FILE=<your-toolchain-file>` to `cmake ..`.
