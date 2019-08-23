# Geoflow
flowchart tool for geo-spatial data processing

![badge](https://github.com/tudelft3d/geoflow/workflows/CI%20macOS/badge.svg)

## Building
Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

Install dependencies `nlohmann-json` and also 'glfw' and `glm`. Then build using:
```
git submodule update --init thirdparty/imgui thirdparty/osdialog thirdparty/filesystem
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=[ON|OFF]
make
```
