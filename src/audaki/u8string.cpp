#include "audaki/u8string.h"



bool icontains(Utf8_view haystack, Utf8_view needle) noexcept
{
    if (needle.byte_count() == 0)
        return true;

    // If the haystack has less bytes it certainly can't contain the full needle string
    if (haystack.byte_count() < needle.byte_count())
        return false;

    assert(haystack.byte_count() > 0 && haystack.byte_count() >= needle.byte_count());

    auto needle_begin = needle.begin();
    auto needle_it = needle_begin;
    auto needle_end = needle.end();
    assert(needle_begin != needle_end);

    auto first_code_point = needle_begin.get();

    auto haystack_begin = haystack.begin();
    auto haystack_it = haystack_begin;
    auto haystack_end = haystack.end();
    assert(haystack_begin != haystack_end);

    auto next_haystack_begin = haystack_begin;

    while (haystack_it != haystack_end) {

        auto needle_val = needle_it.get();
        auto haystack_val = haystack_it.get();

        if (needle_val == first_code_point && needle_it != needle_begin && next_haystack_begin == haystack_begin)
            next_haystack_begin = haystack_it;

        bool is_matched = needle_val.icompare(haystack_val);
        if (is_matched) {

            ++needle_it;
            if (needle_it == needle_end)
                return true;

            ++haystack_it;
            continue;
        }


        assert(!is_matched);
        needle_it = needle_begin;
        if (next_haystack_begin != haystack_begin) {
            haystack_it = next_haystack_begin;
            next_haystack_begin = haystack_begin;
        }
        else {
            ++haystack_it;
        }
    }

    return false;
}
