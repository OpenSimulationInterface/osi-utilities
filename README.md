# osi-utilities

![Build](https://github.com/OpenSimulationInterface/osi-utilities/actions/workflows/ubuntu.yml/badge.svg)

Utility library providing companion tools, helpers and best practices for working with the [Open Simulation Interface (OSI)](https://github.com/OpenSimulationInterface/open-simulation-interface/c) standard.

## Features
- Cross-platform C++ library and Python APIs
  - **Note:** Python API is planned but not yet implemented
  - **Note:** Windows support is planned but not yet tested, contributions welcome!
- Easily deal with OSI data and files into your own projects
- More OSI utility features planned, feature requests and pull requests welcome!

## Build Instructions

### Install required dependencies
- Compiler (e.g. GCC, Clang, MSVC)
- Python
- [CMake](https://cmake.org/download/)
- [zstd](https://github.com/facebook/zstd)
- [lz4](https://github.com/lz4/lz4)
- [Protobuf](https://github.com/protocolbuffers/protobuf)

**Note:** On Debian/Ubuntu, you can install the required dependencies using the following commands:
```bash
sudo apt install build-essential cmake libzstd-dev liblz4-dev protobuf-compiler
```

### Build 

Clone the repository
```bash
git clone https://github.com/OpenSimulationInterface/osi-utilities.git
```

Create build directory and configure using CMake
```bash
mkdir build && cd build
cmake ..
```

Build using CMake
```bash
cmake --build . -j4
```


## Contributing

We welcome contributions! This library aims to grow alongside OSI to provide the community with helpful tools and utilities. If you have ideas for new features or improvements, you are encouraged to open issues and submit pull requests.

