if (NOT BUILD_BENCHMARK)
    return()
endif()

include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_Declare(googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)

FetchContent_Declare(
    nlohman_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG        v1.1.0
)

set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Enable RapidJSON tests" FORCE)

FetchContent_MakeAvailable(
    googletest
    googlebenchmark
    nlohman_json
    rapidjson
)

FetchContent_GetProperties(rapidjson
    SOURCE_DIR     RAPIDJSON_SOURCE_DIR
)

add_executable(auric_json_benchmark
    benchmark.cpp
)

target_link_libraries(auric_json_benchmark PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    nlohmann_json::nlohmann_json
)

target_include_directories(auric_json_benchmark PRIVATE
    "${RAPIDJSON_SOURCE_DIR}/include"
)
