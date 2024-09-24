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

# Check docs when build for Windows
add_library(storage STATIC ${STORAGE_SRC} ${STORAGE_HEADERS})

target_include_directories(storage PRIVATE .)
