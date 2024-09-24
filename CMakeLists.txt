cmake_minimum_required(VERSION 3.12)

project(jjjdb
	LANGUAGES C
	VERSION 1.0
	)

add_subdirectory(storage)
# set(STORAGE_INCLUDE storage/storage.h)

add_executable(jjjdb $<TARGET_OBJECTS:storage> ./main.c ${STORAGE_HEADERS})
target_include_directories(jjjdb PRIVATE storage)




target_compile_options(jjjdb PRIVATE
		-Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes
        -Wno-unused-function -Wno-unused-but-set-variable
	# -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope -fsanitize=address
        )

#target_link_options(jjjdb BEFORE PRIVATE
#         -fvisibility=hidden -fvisibility-inlines-hidden
#)

# include(CTest)

# if(BUILD_TESTING)
# 	add_subdirectory(test)
# endif()
