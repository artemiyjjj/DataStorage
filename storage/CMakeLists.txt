file(GLOB_RECURSE STORAGE_SRC CONFIGURE_DEPENDS
        *.c
        blocks
        cells
        cli
)

file (GLOB_RECURSE STORAGE_HEADERS CMAKE_CONFIGURE_DEPENDS
        *.h
        blocks
        cells
        cli
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

# Check docs when build for Windows
add_library(storage STATIC ${STORAGE_SRC} ${STORAGE_HEADERS} ${GLIB_INCLUDE_DIRS})

target_include_directories(storage PRIVATE .)
target_include_directories(jjjdb PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(storage PkgConfig::deps)
