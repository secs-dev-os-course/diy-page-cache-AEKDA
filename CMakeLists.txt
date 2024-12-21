cmake_minimum_required(VERSION 3.12)

project(ItmoShell
        VERSION 0.0.0
	DESCRIPTION "itmoPageCache"
        LANGUAGES C CXX
)

option(MONOLITH_DEVELOPER "Enable ${PROJECT_NAME} developer mode" OFF)
option(MONOLITH_BENCHMARK "Enable ${PROJECT_NAME} benchmark module" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(cmake/CompileOptions.cmake)

add_subdirectory(cache_lib)
add_subdirectory(app)

if(MONOLITH_BENCHMARK)
    find_package(GBenchmark REQUIRED)
    add_subdirectory(benchmark)
endif()


