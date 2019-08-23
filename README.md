# Geoflow
flowchart tool for geo-spatial data processing

![badge](https://github.com/tudelft3d/geoflow/workflows/CI%20macOS/badge.svg)


## Building
Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

Prior to building you'll need to install at least the `nlohmann-json` dependency.

### Build without GUI
```
git submodule update --init thirdparty/filesystem
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=OFF
make
```

### Build with GUI
Install additional dependencies `glm` and `glfw`.

```
git submodule update --init thirdparty/imgui thirdparty/osdialog thirdparty/filesystem
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=ON
make
```