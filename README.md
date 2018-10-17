# Geoflow
flowchart tool for geo-spatial data processing

## Building
Requires compiler with c++17 support. Can be installed on macOS with:
```
brew install llvm
```

All other dependencies are included. Build with cmake:
```
mkdir build
cd build
cmake ..
make
```

### Building without GUI
Set the CMake variable `GF_BUILD_GUI` to `OFF`, eg:
```
cmake .. -DGF_BUILD_GUI=OFF
```