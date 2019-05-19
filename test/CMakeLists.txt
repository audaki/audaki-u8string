

add_executable(audaki-u8string-test EXCLUDE_FROM_ALL main.cpp)

find_package(Catch2 REQUIRED)

target_link_libraries(audaki-u8string-test
    Catch2::Catch2
    audaki-u8string)