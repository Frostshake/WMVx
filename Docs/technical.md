#Technical

## Build Process and Dependencies

Build Requirements:
- Windows
- Visual Studio 2022
- CMake (with vcpkg highly advised)
- QT 6.5 (https://doc.qt.io/qt-6.5/)
- FBX SDK 2020.3.4 (https://aps.autodesk.com/developer/overview/fbx-sdk)

Dependencies:
- CascLib (https://github.com/ladislav-zezula/CascLib)
- glew (https://glew.sourceforge.net/)
- StormLib (https://github.com/ladislav-zezula/StormLib)
- glm (https://github.com/g-truc/glm)
- fast-cpp-csv-parser (https://github.com/ben-strasser/fast-cpp-csv-parser)
- WDBReader (https://github.com/Frostshake/WDBReader)


Build with cmake and vcpkg.

```
cmake --preset=default-win-64-mixed -DFBX_SDK_ROOT_DIR="C:\path\to\FBX SDK\2020.3.4"
```

Example cmake preset:
```
{
    "name": "default-win-64-mixed",
    "generator": "Visual Studio 17 2022",
    "architecture": "x64",
    "binaryDir": "${sourceDir}/build",
    "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "D:/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_CHAINLOAD_TOOLCHAIN_FILE": "D:/vcpkg/scripts/toolchains/windows.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows-mixed",
        "VCPKG_CRT_LINKAGE": "dynamic",
        "VCPKG_TRACE_FIND_PACKAGE": "ON", 
        "CMAKE_INSTALL_PREFIX": "./install",
        "VCPKG_OVERLAY_PORTS": "${sourceDir}/vcpkg/ports",
        "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/vcpkg/triplets"
    }
}
```

## Project Structure

`Support Files` - Supporting files to be supplied with the output executable.

`src` - Source files, in the top level directory these files relate to the UI and display.

`src/core` - WMVx internals and game client abstractions.

## Game Client Abstractions

The foundation of WMVx works by utilising a common API to read different version game clients.

Specific game client versions need to implement the `GameClientAdaptor` interface, this is essentially a factory which specifies the client implementation of the database, filesystem and model structure.

### Filesystem

Game client files are accessed using the `GameFileSystem` interface, and `GameFileUri` for handling paths independant of the underlying MPQ or CASC implementation.

### Database

The game database can be queried using the `GameDatabase` interface, entities are managed by "Record" adaptors inside of "Datasets". (Datasets arent a direct relation to the client tables as they could change between versions). The abstraction of datasets and records effectively handles the variation in client tables and structures. Reading of DBC and DB2 database files is handles by [WDBReader](https://github.com/Frostshake/WDBReader).

### Models

Model loading is handled in `M2.h` & `M2.cpp`. Structure defintions are in `M2Definitions.h`. Handling of the file contents varies based on the header version & chunks read.
