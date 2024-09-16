include(GoogleTest)

find_package(storage REQUIRED)

add_executable(
	storage_init
	# $<TARGET_OBJECTS:storage>
	func/storage_init.cc
)



target_link_libraries(storage_init 
	GTest::gtest_main
	storage::storage
	# $<TARGET_OBJECTS:storage>
)

target_include_directories(storage_init PRIVATE ${CMAKE_INSTALL_INCLUDEDIR})
# include_directories(${CMAKE_SOURCE_DIR}/storage/include)
# link_directories(${CMAKE_SOURCE_DIR}/storage)

file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
    func/*.cc
    smoke/*.cc
    unit/*.cc
)

gtest_discover_tests(storage_init)

