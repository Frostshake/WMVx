#Technical

## Build Process and Dependencies

Build Requirements:
- Windows
- Visual Studio 2022
- QT VS Tools (https://doc.qt.io/qtvstools/index.html)
- QT 6.4 (https://doc.qt.io/qt-6.4/)
- Boost 1.79 (https://www.boost.org/users/history/version_1_79_0.html)

Dependencies:
- CascLib (https://github.com/ladislav-zezula/CascLib)
- CXImage (https://www.codeproject.com/Articles/1300/CxImage / See WMV source)
- glew (https://glew.sourceforge.net/)
- qtcsv (https://github.com/iamantony/qtcsv)
- StormLib (https://github.com/ladislav-zezula/StormLib)

Copy/link each of the dependencies into the `Dependencies/{name}` folder.

Open `WMVx.sln`

## Project Structure

`Dependencies` - Third party dependencies.
`Support Files` - Supporting files to be supplied with the output executable.
`WMVx` - Source files, in the top level directory these files relate to the UI and display.
`WMVx/core` - WMVx internals and game client abstractions.

## Game Client Abstractions

The foundation of WMVx works by utilising a common API to read different version game clients.

Specific game client versions need to implement the `GameClientAdaptor` interface, this is essentially a factory which specifies the client implementation of the database, filesystem and model structure.

### Filesystem

Game client files are accessed using the `GameFileSystem` interface, and `GameFileUri` for handling paths independant of the underlying MPQ or CASC implementation.

### Database

The game database can be queried using the `GameDatabase` interface, entities are managed by "Record" adaptors inside of "Datasets". (Datasets arent a direct relation to the client tables as they could change between versions). The abstraction of datasets and records effectively handles the variation in client tables and structures.

### Models

Similiarly to the filesystem and database, client version model variations are handled with the `RawModel` interface, this handles the reading and returning of the model data.
