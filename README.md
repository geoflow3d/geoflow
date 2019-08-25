# Geoflow
flowchart tool for geo-spatial data processing

![macOS CI](https://github.com/tudelft3d/geoflow/workflows/CI%20macOS/badge.svg)
![linux CI](https://github.com/tudelft3d/geoflow/workflows/CI%20linux/badge.svg)
![Windows CI](https://github.com/tudelft3d/geoflow/workflows/CI%20Windows/badge.svg)


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

### platform specific instructions
Have a look at the [workflow files](https://github.com/tudelft3d/geoflow/tree/master/.github/workflows).