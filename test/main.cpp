#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "audaki/u8string.h"



TEST_CASE("Test u8_iequal", "[string, utf8, u8_iequal]")
{
    CHECK(u8_iequal("asdäöüß", "ASDÄÖÜẞ"));
    CHECK(u8_iequal("asdäöüß", "asdäöüß"));
}


TEST_CASE("Test u8_iless", "[string, utf8, u8_iless]")
{
    CHECK_FALSE(u8_iless("asdäöüß", "ASDÄÖÜẞ"));
    CHECK_FALSE(u8_iless("ASDÄÖÜẞ", "asdäöüß"));
    CHECK(u8_iless("ASDÄÖÜẞ", "bsdäöüß"));
    CHECK_FALSE(u8_iless("BSDÄÖÜẞ", "asdäöüß"));
    CHECK(u8_iless("asdäöüß", "BSDÄÖÜẞ"));
    CHECK_FALSE(u8_iless("bsdäöüß", "ASDÄÖÜẞ"));
    CHECK(u8_iless("asdäöü", "ASDÄÖÜẞ"));
    CHECK_FALSE(u8_iless("asdäöüß", "ASDÄÖÜ"));
}
