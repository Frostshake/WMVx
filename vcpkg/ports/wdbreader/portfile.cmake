vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Frostshake/WDBReader
    REF f4226da9d301f03ef1bc4e66d7edbc3755549f40
    SHA512 642504c85e7b25547f0bdc4b35fbf134f52f6121024bb4a02e28720b1847d152cd46a00a3e2d3af53713397e4ab8f2af16fe5405e9e511dcf133b0b89ccc4703
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