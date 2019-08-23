# Geoflow
flowchart tool for geo-spatial data processing

## Building
Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

Install dependencies `nlohmann-json` and also 'glfw' and `glm` when building with GUI (the default). Then build using:
```
git submodule update --init thirdparty/imgui thirdparty/osdialog
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=[ON|OFF]
make
```
