# Geoflow
flowchart tool for geo-spatial data processing

![macOS CI](https://github.com/tudelft3d/geoflow/workflows/CI%20macOS/badge.svg)
![linux CI](https://github.com/tudelft3d/geoflow/workflows/CI%20linux/badge.svg)
![Windows CI](https://github.com/tudelft3d/geoflow/workflows/CI%20Windows/badge.svg)


## Building
Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

```
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=[ON|OFF]
cmake --build . --parallel 4 --target install --config Release
```

### Building with GUI
Requires additional dependencies `glm` and `glfw` that need to be installed by  the user.

### Platform specific instructions
Have a look at the [workflow files](https://github.com/tudelft3d/geoflow/tree/master/.github/workflows).