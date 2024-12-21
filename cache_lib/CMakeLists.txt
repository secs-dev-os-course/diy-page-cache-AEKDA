cmake_minimum_required(VERSION 3.12)
project(MRUCacheLib
        VERSION 0.0.0
        DESCRIPTION "Lib module"
        LANGUAGES C CXX
)

add_library(cachelib SHARED PageCache.cpp PageCacheFS.cpp)

target_include_directories(cachelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
