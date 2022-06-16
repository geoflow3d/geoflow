# Geoflow
Flowchart tool for geo-spatial data processing. Highly experimental with many rough edges!

# Installation

## As part of [geoflow-bundle](https://github.com/geoflow3d/geoflow-bundle/)
This the recommended way since it will also include the commonly used plugins and flowcharts to get you started quickly. Also binary pacakges are available.

## Building from source

Requires compiler with c++17 support  (see https://en.cppreference.com/w/cpp/compiler_support).

```
mkdir build
cd build
cmake .. -DGF_BUILD_GUI=OFF
cmake --build . --parallel 4 --config Release
```

### dependencies
+ [nlohmann JSON](https://github.com/nlohmann/json/releases) at least version 3.10.5

# Usage
## Command line interface (`geof`)
```
Usage: 
   geof [-v | -p | -n | -h]
   geof <flowchart_file> [-V] [-g] [-w] [-c <file>] [--GLOBAL1=A --GLOBAL2=B ...]

Options:
   -v, --version                Print version information
   -p, --list-plugins           List available plugins
   -n, --list-nodes             List available nodes for plugins that are loaded
   -h, --help                   Print this help message

   <flowchart_file>             JSON flowchart file
   -V, --verbose                Print verbose messages during flowchart execution
   -g, --list-globals           List available flowchart globals. Cancels flowchart execution
   -w, --workdir                Set working directory to folder containing flowchart file
   -c <file>, --config <file>   Read globals from TOML config file
   --GLOBAL1=A --GLOBAL2=B ...  Specify globals for flowchart (list availale globals with -g)
```
### examples
Print version information:
```geof --version```

Print help:
```geof --help```

Running a flowchart with default globals:
```geof <flowchart file>```

List available globals:
```geof <flowchart file> --list-globals```

Running a flowchart with user-specified global values:
```geof <flowchart file> [--config <TOML config file with globals>] [--GLOBAL1=value1 --GLOBAL2=value2 ...]```

Command line specified globals have the highest priority and will override default values and/or config file provided values.