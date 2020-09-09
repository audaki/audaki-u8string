cmake_minimum_required(VERSION 3.13)
project(audaki-u8string LANGUAGES CXX VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)

option(ENABLE_LTO "enable LTO" OFF)

option(USE_LLD "Use LLD" OFF)


add_library(audaki-u8string src/audaki/u8string.cpp)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(compiler_specific_compile_options
        -Wno-return-type
    )
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(compiler_specific_compile_options
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-c++98-c++11-compat-binary-literal
        -Wno-padded
    )
endif()


target_compile_options(audaki-u8string PRIVATE
    -Werror
    ${compiler_specific_compile_options}
)

target_link_options(audaki-u8string PRIVATE -Wl,--threads$<$<CONFIG:Release>:,--strip-all>)

if(ENABLE_LTO)
    target_compile_options(audaki-u8string PRIVATE $<$<CONFIG:Release>:-flto=thin>)

    target_link_options(audaki-u8string PRIVATE $<$<CONFIG:Release>:-flto=thin -Wl,--thinlto-cache-dir=${PROJECT_BINARY_DIR}/lto.cache>)
endif()

if (USE_LLD)
    target_link_options(audaki-u8string PRIVATE -fuse-ld=lld)
endif()


target_compile_features(audaki-u8string PUBLIC cxx_std_17)

target_include_directories(audaki-u8string
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

install(TARGETS audaki-u8string
    ARCHIVE DESTINATION lib64)

install(DIRECTORY
    include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h*"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)



add_subdirectory(test)
