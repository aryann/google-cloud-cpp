include(vcpkg_common_functions)

vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH
    SOURCE_PATH
    REPO
    googleapis/google-cloud-cpp-common
    REF
    v0.13.0
    SHA512
    10fd22e4744b0a372dcfb4426b303e4f2b06efd8c14190aaa399f852ced4bdefcf495361c51969d9a43f8895f9d7a58d09c384301d6b1467bc75d77f077ee8ca
    HEAD_REF
    master)

vcpkg_configure_cmake(SOURCE_PATH
                      ${SOURCE_PATH}
                      PREFER_NINJA
                      DISABLE_PARALLEL_CONFIGURE
                      OPTIONS
                      -DGOOGLE_CLOUD_CPP_ENABLE_MACOS_OPENSSL_CHECK=OFF)

vcpkg_install_cmake(ADD_BIN_TO_PATH)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake TARGET_PATH share)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(INSTALL
     ${SOURCE_PATH}/LICENSE
     DESTINATION
     ${CURRENT_PACKAGES_DIR}/share/google-cloud-cpp-common
     RENAME copyright)

vcpkg_copy_pdbs()
