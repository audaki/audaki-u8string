cmake_minimum_required(VERSION 3.14)
project(audaki-u8string LANGUAGES CXX VERSION 1.0.0)


add_library(audaki-u8string src/audaki/u8string.cpp)


target_compile_options(audaki-u8string PRIVATE
    -Werror
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-c++98-c++11-compat-binary-literal
    -Wno-padded
)

target_compile_features(audaki-u8string PUBLIC cxx_std_17)

target_include_directories(audaki-u8string
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(audaki-u8string PRIVATE $<$<CONFIG:Release>:-flto=thin>)
target_link_options(audaki-u8string PRIVATE
        -fuse-ld=lld
        -Wl,--threads
        $<$<CONFIG:Release>:-Wl,--strip-all,--thinlto-cache-dir=${PROJECT_BINARY_DIR}/lto.cache>
        $<$<CONFIG:Debug>:-Wl,--gdb-index>
        $<$<CONFIG:Release>:-flto=thin>)

add_subdirectory(test)