vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ladislav-zezula/StormLib
    REF 4c2fa7c7fd6d790f37c8833784df7a818ffba535
    SHA512 0f677223ee73a101bd3a2d1256688414b82ff8960a04a84d1e5a62c8e754d13b09ab9a3d7d30aea64ec8681d6cba124b16a375eed37efde303480fb94494ba79
    HEAD_REF master
)

set(STORM_UNICODE ON)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSTORM_UNICODE=${STORM_UNICODE}
        -DSTORM_USE_BUNDLED_LIBRARIES=ON
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME StormLib)
vcpkg_copy_pdbs()


file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")