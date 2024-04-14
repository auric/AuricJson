cmake_minimum_required(VERSION 3.5)

project(auric_json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(Python REQUIRED)

if(MSVC)
    # Use /MDd for Debug builds
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    # Use /MD for Release builds
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

add_executable(auric_json
    main.cpp
    auric_json.h
    tests.h
    benchmark.cpp
)

target_link_libraries(auric_json
    benchmark::benchmark
    benchmark::benchmark_main
    nlohmann_json::nlohmann_json
    )

target_include_directories(auric_json PRIVATE
    "D:/dev/libs/matplotlib-cpp"
    "D:/dev/libs/RapidJSON/include"
)

include(GNUInstallDirs)
install(TARGETS auric_json
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
