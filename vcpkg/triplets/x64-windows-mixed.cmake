set(VCPKG_TARGET_ARCHITECTURE x64)

if(${PORT} MATCHES "wdbreader|casclib|stormlib|bzip2|zlib|glew")
    set(VCPKG_CRT_LINKAGE dynamic)
	set(VCPKG_LIBRARY_LINKAGE static)
else()
    set(VCPKG_CRT_LINKAGE dynamic)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()