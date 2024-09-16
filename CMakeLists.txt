cmake_minimum_required(VERSION 3.12)

project(jjjdb
	LANGUAGES C CXX
	VERSION 1.0
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_executable(jjjdb $<TARGET_OBJECTS:storage> ./main.c)

add_subdirectory(storage)

set_target_properties(jjjdb
	PROPERTIES
	CMAKE_CXX_STANDARD 14
	CMAKE_CXX_STANDARD_REQUIRED ON
	CMAKE_CXX_EXTENSIONS OFF
)

include_directories(${CMAKE_INSTALL_INCLUDEDIR})
# target_link_libraries(jjjdb PUBLIC storage)

target_compile_options(jjjdb PRIVATE
		-Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes
        -Wno-unused-function -Wno-unused-but-set-variable
	# -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope -fsanitize=address
        )

#target_link_options(jjjdb BEFORE PRIVATE
#         -fvisibility=hidden -fvisibility-inlines-hidden
#)

include(CTest)

include(FetchContent)
FetchContent_Declare(
	  googletest
	  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(BUILD_TESTING)
	add_subdirectory(test)
endif()

