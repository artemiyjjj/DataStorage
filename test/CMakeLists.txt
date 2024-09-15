include(CTest)

set(TIMEOUT 20)
set(TIMEOUT_SIGNAL_NAME SIGTERM)

file(GLOB_RECURSE testing_tools_sources CONFIGURE_DEPENDS
    utils/*.c
    utils/*.h
)

add_library(testing_tools OBJECT ${testing_tools_sources})
target_include_directories(testing_tools PUBLIC utils)
target_link_libraries(testing_tools PUBLIC $<TARGET_OBJECTS:storage>)

file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
    func/*.c
    smoke/*.c
    unit/*.c
)

foreach(test_source IN LISTS test_sources)
    string(REPLACE "/" ";" name_components ${test_source})
    list(GET name_components -1 name)
    string(REGEX REPLACE "\.c$" "" name ${name})
    list(APPEND test_targets test_${name})

    add_executable(test_${name} ${test_source})
    target_link_libraries(test_${name} PUBLIC testing_tools)
    add_test(NAME test_${name} COMMAND test_${name})
endforeach()

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} ${CMAKE_CTEST_ARGUMENTS}
    DEPENDS ${test_targets}
)

