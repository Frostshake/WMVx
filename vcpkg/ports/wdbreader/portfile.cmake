vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Frostshake/WDBReader
    REF 19a33a696879c3e57d7fedbb14859d36fa53ca4e
    SHA512 628c11d56b7dd0c2868a26a76c979c5e7504d51998c96cfe4ef7bf6e19112bf3c3d2f2ebb1f82626c82457218686ce74197ba0a64f16b92a1c8df36655b844b1
    HEAD_REF main 
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_APPS=OFF
        -DBUILD_TESTING=OFF
        -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/WDBReader)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)