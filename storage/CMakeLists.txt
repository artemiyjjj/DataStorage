file(GLOB_RECURSE STORAGE_SRC CONFIGURE_DEPENDS
    *.c
    *.h
    blocks
)

# Check docs when build for Windows
add_library(storage STATIC ${STORAGE_SRC})

target_include_directories(storage 
	PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS storage_targets
    EXPORT storageConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES storage.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(TARGETS
        storage_target
    NAMESPACE storage::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/storageConfig.cmake"
)

install(EXPORT storageConfig
    FILE storage_targets.cmake
    NAMESPACE storage::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/storage
)

# configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
#     "${CMAKE_CURRENT_BINARY_DIR}/storageConfig.cmake"
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/storage
# )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/storageConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/storage
)
