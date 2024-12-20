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
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories (${GLIB_LIBRARY_DIRS})
add_definitions (${GLIB_CFLAGS_OTHER})
# pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
# pkg_check_modules(deps REQUIRED IMPORTED_TARGET gtk+-2.0)
# include_directories(${GTK2_INCLUDE_DIRS})

# LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

find_library(MATH_LIB m)

# Check docs when build for Windows
add_library(storage STATIC ${STORAGE_SRC} ${STORAGE_HEADERS} ${GLIB_INCLUDE_DIRS})

target_compile_options(storage PRIVATE
		-Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes
        -Wno-unused-function -Wno-unused-but-set-variable
	# -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope -fsanitize=address
)

target_include_directories(storage PUBLIC . ${GLIB_INCLUDE_DIRS})
target_link_libraries(storage ${MATH_LIB} -lm ${GLIB_LIBRARIES})
