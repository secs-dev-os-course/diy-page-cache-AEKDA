get_filename_component(MONOLITH_BENCH_SOURCE_PATH "./${PROJECT_NAME}" ABSOLUTE)

set(MONOLITH_BENCH_TARGET_BASE "ItmoShell-bench")

macro(MONOLITH_BENCH_DECLARE suffix file)
    add_executable(
        ${MONOLITH_BENCH_TARGET_BASE}-${suffix}
        ${MONOLITH_BENCH_SOURCE_PATH}/${file}
    )

    target_link_libraries(
        ${MONOLITH_BENCH_TARGET_BASE}-${suffix}
        cachelib
        benchmark::benchmark
    )
endmacro()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

MONOLITH_BENCH_DECLARE(ema-search-str BenchmarkEmaSearchStr.cpp)
MONOLITH_BENCH_DECLARE(direct-ema-search-str BenchmarkDirectEmaSearchStr.cpp)
MONOLITH_BENCH_DECLARE(cache-ema-search-str BenchmarkCacheEmaSearchStr.cpp)
