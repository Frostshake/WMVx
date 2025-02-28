vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ladislav-zezula/CascLib
    REF 07ab5f37ad282cc101d5c17793c550a0a6d4637f
    SHA512 18a284380ada9b79f57f6aa85f35f0621d51af7215ef854762ee65d1e507271ad7fa566cc0533784b53c33e2a23096321c1fd4e99a3f4fef0809c0f5ba833684
    HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" CASC_BUILD_SHARED_LIB)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" CASC_BUILD_STATIC_LIB)

set(CASC_UNICODE ON)

# if(VCPKG_TARGET_IS_WINDOWS)
#     message(STATUS "This version of CascLib is built in ASCII mode. To switch to UNICODE version, create an overlay port of this with CASC_UNICODE set to ON.")
#     message(STATUS "This recipe is at ${CMAKE_CURRENT_LIST_DIR}")
#     message(STATUS "See the overlay ports documentation at https://github.com/microsoft/vcpkg/blob/master/docs/specifications/ports-overlay.md")
# endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DCMAKE_REQUIRE_FIND_PACKAGE_ZLIB=ON
        -DCASC_BUILD_SHARED_LIB=${CASC_BUILD_SHARED_LIB}
        -DCASC_BUILD_STATIC_LIB=${CASC_BUILD_STATIC_LIB}
        -DCASC_UNICODE=${CASC_UNICODE}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/CascLib)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)