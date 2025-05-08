cmake_minimum_required(VERSION 3.14)

project(WMVx VERSION 0.5.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Qt6 REQUIRED COMPONENTS Concurrent Gui Network OpenGLWidgets Widgets REQUIRED)

find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)

find_package(WDBReader CONFIG REQUIRED)
find_package(StormLib CONFIG REQUIRED)
find_package(CascLib CONFIG REQUIRED)

add_subdirectory(src)

file(GLOB_RECURSE PROJECT_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} 
    "src/*.cpp" 
    "src/*.h" 
    "src/*.ui"
)

file(GLOB_RECURSE CORE_FILES ${CMAKE_SOURCE_DIR} "src/core/**/*")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/core PREFIX "Core Files" FILES ${CORE_FILES})

file(GLOB_RECURSE FORM_FILES ${CMAKE_SOURCE_DIR} "src/*.ui")
source_group("Form Files" FILES ${FORM_FILES})

qt_add_executable(WMVx
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    src/WMVx.rc
)

target_link_libraries(WMVx PRIVATE Qt6::Concurrent)
target_link_libraries(WMVx PRIVATE Qt6::Gui)
target_link_libraries(WMVx PRIVATE Qt6::Network)
target_link_libraries(WMVx PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(WMVx PRIVATE Qt6::Widgets)

target_link_libraries(WMVx PRIVATE OpenGL::GL)
target_link_libraries(WMVx PRIVATE glm::glm-header-only)
target_link_libraries(WMVx PRIVATE GLEW::GLEW)

target_compile_definitions(WMVx PRIVATE CASCLIB_NO_AUTO_LINK_LIBRARY)
# if(BUILD_SHARED_LIBS)
    # target_link_libraries(WDBx PRIVATE CascLib::casc)
# else()
    target_link_libraries(WMVx PRIVATE CascLib::casc_static)
# endif()

target_compile_definitions(WMVx PRIVATE STORMLIB_NO_AUTO_LINK)
target_link_libraries(WMVx PRIVATE StormLib::storm)

target_link_libraries(WMVx PRIVATE WDBReader::WDBReader)

set_target_properties(WMVx PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if (MSVC)
    target_compile_options(WMVx PRIVATE /bigobj)
    set_property(TARGET WMVx PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:WMVx>")
endif()

if(WIN32)
    target_link_libraries(WMVx PRIVATE propsys.lib)
    target_compile_definitions(WMVx PUBLIC NOMINMAX)
endif()

target_compile_definitions(WMVx PUBLIC WMVX_VERSION="${CMAKE_PROJECT_VERSION}")

# FBX 
set(FBK_SDK_ROOT_DIR "" PATH)
target_include_directories(WMVx PRIVATE ${FBX_SDK_ROOT_DIR}/include)
if(WIN32)
target_link_libraries(WMVx PRIVATE "${FBX_SDK_ROOT_DIR}/lib/vs2022/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk.lib")
add_custom_command(TARGET WMVx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${FBX_SDK_ROOT_DIR}/lib/vs2022/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk.dll"
    $<TARGET_FILE_DIR:WMVx>
)
endif()
# END FBX

install(TARGETS WMVx
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_custom_command(TARGET WMVx POST_BUILD 
    COMMAND Qt6::windeployqt
    ARGS $<TARGET_FILE:WMVx>
)

# add_custom_command(TARGET WMVx POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/settings.ini.dist" "$<TARGET_FILE_DIR:WMVx>/settings.ini"
# )

# add_custom_command(TARGET WMVx POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Support files" "$<TARGET_FILE_DIR:WMVx>/Support Files"
# )

qt_finalize_executable(WMVx)
