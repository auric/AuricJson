cmake_minimum_required(VERSION 3.20)

project(auric_json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTS ON CACHE BOOL "Enable Auric Json tests" FORCE)
set(BUILD_BENCHMARK ON CACHE BOOL "Enable Auric Json benchmark" FORCE)


add_subdirectory(tests)
add_subdirectory(benchmark)

add_executable(auric_json
    main.cpp
    auric_json.h
)

include(GNUInstallDirs)
install(TARGETS auric_json
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
