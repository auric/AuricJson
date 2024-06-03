if (NOT BUILD_TESTS)
    return()
endif()

include(FetchContent)
include(GoogleTest)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(
    googletest
)

enable_testing()

add_executable(auric_json_tests
    tests.cpp
)
target_link_libraries(auric_json_tests
    GTest::gtest GTest::gtest_main
)
add_test(AllTests auric_json_tests)
gtest_discover_tests(auric_json_tests)
