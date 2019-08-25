# Geoflow
flowchart tool for geo-spatial data processing

![badge](https://github.com/tudelft3d/geoflow/workflows/CI%20macOS/badge.svg)
![badge](https://github.com/tudelft3d/geoflow/workflows/CI%20linux/badge.svg)


## Building
Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

### Build without GUI
```
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=OFF
make
```

### Build with GUI
Install additional dependencies `glm` and `glfw`.

```
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=ON
make
```