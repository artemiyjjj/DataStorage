file(GLOB_RECURSE STORAGE_SRC CONFIGURE_DEPENDS
        *.c
        *.h
        blocks
)

# Check docs when build for Windows
add_library(storage STATIC ${STORAGE_SRC})
