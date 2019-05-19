#pragma once

#include <cstddef>
#include <cassert>

#include <string_view>

#include <algorithm>
#include <vector>
#include <numeric>
#include <utility>
#include <array>





enum class Utf8_byte_type: uint8_t {
    continuation_byte = 0,
    single_byte_ascii = 1,
    first_of_two_bytes = 2,
    first_of_three_bytes = 3,
    first_of_four_bytes = 4
};


/**
 * Examine most significant bits to get UTF-8 byte type.
 */
inline Utf8_byte_type get_utf8_byte_type(std::byte byte)
{
    using Type = Utf8_byte_type;

    if ((byte >> 7) == std::byte{0b0})
        return Type::single_byte_ascii;

    if ((byte >> 6) == std::byte{0b10})
        return Type::continuation_byte;

    if ((byte >> 5) == std::byte{0b110})
        return Type::first_of_two_bytes;

    if ((byte >> 4) == std::byte{0b1110})
        return Type::first_of_three_bytes;

    return Type::first_of_four_bytes;
}



/**
 * Is char c the begin of a code point? Otherwise it's a continuation byte.
 */
inline bool is_utf8_code_point_begin(char c) noexcept
{
    uint8_t byte = static_cast<uint8_t>(c);
    bool is_ascii = (byte & 0b1000'0000) == 0;
    if (is_ascii)
        return true;

    bool is_two_bytes_begin = (byte & 0b1110'0000) == 0b1100'0000;
    if (is_two_bytes_begin)
        return true;

    bool is_three_bytes_begin = (byte & 0b1111'0000) == 0b1110'0000;
    if (is_three_bytes_begin)
        return true;

    bool is_four_bytes_begin = (byte & 0b1111'1000) == 0b1111'0000;
    if (is_four_bytes_begin)
        return true;

    return false;
}


struct Unicode_code_point {

    constexpr Unicode_code_point(uint32_t v): v_{v}
    {
    }

    constexpr Unicode_code_point(char c): v_{static_cast<uint8_t>(c)}
    {
    }

    constexpr Unicode_code_point(char c1, char c2): v_{
            (static_cast<uint32_t>(static_cast<uint8_t>(c1) & 0b0001'1111u) << 6) |
            static_cast<uint32_t>(static_cast<uint8_t>(c2) & 0b0011'1111u)}
    {
    }

    constexpr Unicode_code_point(char c1, char c2, char c3): v_{
            (static_cast<uint32_t>(static_cast<uint8_t>(c1) & 0b0000'1111u) << 12) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c2) & 0b0011'1111u) << 6) |
            static_cast<uint32_t>(static_cast<uint8_t>(c3) & 0b0011'1111u)}
    {
    }

    constexpr Unicode_code_point(char c1, char c2, char c3, char c4): v_{
            (static_cast<uint32_t>(static_cast<uint8_t>(c1) & 0b0000'0111u) << 18) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c2) & 0b0011'1111u) << 12) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c3) & 0b0011'1111u) << 6) |
            static_cast<uint32_t>(static_cast<uint8_t>(c4) & 0b0011'1111u)}
    {
    }

    bool operator==(char c) const noexcept
    {
        uint8_t uc = static_cast<uint8_t>(c);
        return uc <= 0x7F && uc == v_;
    }

    bool operator!=(char c) const noexcept
    {
        uint8_t uc = static_cast<uint8_t>(c);
        return uc >= 0x80 || uc != v_;
    }

    bool operator==(const Unicode_code_point& other) const noexcept
    {
        return v_ == other.v_;
    }

    bool operator!=(const Unicode_code_point& other) const noexcept
    {
        return v_ != other.v_;
    }

    bool is_ascii() const noexcept
    {
        return v_ <= 0x7F;
    }

    bool is_ascii_digit() const noexcept
    {
        return v_ >= 0x30 && v_ <= 0x39;
    }

    bool is_ascii_upper() const noexcept
    {
        return v_ >= 0x41 && v_ <= 0x5A;
    }

    bool is_ascii_lower() const noexcept
    {
        return v_ >= 0x61 && v_ <= 0x7A;
    }

    bool is_ascii_alpha() const noexcept
    {
        return is_ascii_upper() || is_ascii_lower();
    }

    bool is_ascii_alnum() const noexcept
    {
        return is_ascii_digit() || is_ascii_upper() || is_ascii_lower();
    }

    bool is_ascii_control() const noexcept
    {
        return v_ <= 0x1F || v_ == 0x7F;
    }

    bool is_ascii_blank() const noexcept
    {
        return v_ == U'\t' || v_ == U' ';
    }

    std::array<char, 5> to_utf8() const noexcept
    {
        if (v_ <= 0x7F)
            return {static_cast<char>(v_), 0, 0, 0, 0};

        if (v_ <= 0x7FF)
            return {
                    static_cast<char>(0b1100'0000 | static_cast<char>((v_ >> 6) & 0b0001'1111)),
                    static_cast<char>(0b1000'0000 | static_cast<char>(v_ & 0b0011'1111)),
                    0, 0, 0};

        if (v_ <= 0xFFFF)
            return {
                    static_cast<char>(0b1110'0000 | static_cast<char>((v_ >> 12) & 0b0000'1111)),
                    static_cast<char>(0b1000'0000 | static_cast<char>((v_ >> 6) & 0b0011'1111)),
                    static_cast<char>(0b1000'0000 | static_cast<char>(v_ & 0b0011'1111)),
                    0, 0};

        return {
                static_cast<char>(0b1111'0000 | static_cast<char>((v_ >> 18) & 0b0000'0111)),
                static_cast<char>(0b1000'0000 | static_cast<char>((v_ >> 12) & 0b0011'1111)),
                static_cast<char>(0b1000'0000 | static_cast<char>((v_ >> 6) & 0b0011'1111)),
                static_cast<char>(0b1000'0000 | static_cast<char>(v_ & 0b0011'1111)),
                0};
    }

    bool icompare(const Unicode_code_point& other) const noexcept
    {
        switch(v_) {
            case U'a': case U'A': return other.v_ == U'a' || other.v_ == U'A';
            case U'b': case U'B': return other.v_ == U'b' || other.v_ == U'B';
            case U'c': case U'C': return other.v_ == U'c' || other.v_ == U'C';
            case U'd': case U'D': return other.v_ == U'd' || other.v_ == U'D';
            case U'e': case U'E': return other.v_ == U'e' || other.v_ == U'E';
            case U'f': case U'F': return other.v_ == U'f' || other.v_ == U'F';
            case U'g': case U'G': return other.v_ == U'g' || other.v_ == U'G';
            case U'h': case U'H': return other.v_ == U'h' || other.v_ == U'H';
            case U'i': case U'I': return other.v_ == U'i' || other.v_ == U'I';
            case U'j': case U'J': return other.v_ == U'j' || other.v_ == U'J';
            case U'k': case U'K': return other.v_ == U'k' || other.v_ == U'K';
            case U'l': case U'L': return other.v_ == U'l' || other.v_ == U'L';
            case U'm': case U'M': return other.v_ == U'm' || other.v_ == U'M';
            case U'n': case U'N': return other.v_ == U'n' || other.v_ == U'N';
            case U'o': case U'O': return other.v_ == U'o' || other.v_ == U'O';
            case U'p': case U'P': return other.v_ == U'p' || other.v_ == U'P';
            case U'q': case U'Q': return other.v_ == U'q' || other.v_ == U'Q';
            case U'r': case U'R': return other.v_ == U'r' || other.v_ == U'R';
            case U's': case U'S': return other.v_ == U's' || other.v_ == U'S';
            case U't': case U'T': return other.v_ == U't' || other.v_ == U'T';
            case U'u': case U'U': return other.v_ == U'u' || other.v_ == U'U';
            case U'v': case U'V': return other.v_ == U'v' || other.v_ == U'V';
            case U'w': case U'W': return other.v_ == U'w' || other.v_ == U'W';
            case U'x': case U'X': return other.v_ == U'x' || other.v_ == U'X';
            case U'y': case U'Y': return other.v_ == U'y' || other.v_ == U'Y';
            case U'z': case U'Z': return other.v_ == U'z' || other.v_ == U'Z';

            case U'à': case U'À': return other.v_ == U'à' || other.v_ == U'À';
            case U'á': case U'Á': return other.v_ == U'á' || other.v_ == U'Á';
            case U'â': case U'Â': return other.v_ == U'â' || other.v_ == U'Â';
            case U'ã': case U'Ã': return other.v_ == U'ã' || other.v_ == U'Ã';
            case U'ä': case U'Ä': return other.v_ == U'ä' || other.v_ == U'Ä';
            case U'å': case U'Å': return other.v_ == U'å' || other.v_ == U'Å';
            case U'æ': case U'Æ': return other.v_ == U'æ' || other.v_ == U'Æ';
            case U'ç': case U'Ç': return other.v_ == U'ç' || other.v_ == U'Ç';
            case U'è': case U'È': return other.v_ == U'è' || other.v_ == U'È';
            case U'é': case U'É': return other.v_ == U'é' || other.v_ == U'É';
            case U'ê': case U'Ê': return other.v_ == U'ê' || other.v_ == U'Ê';
            case U'ë': case U'Ë': return other.v_ == U'ë' || other.v_ == U'Ë';
            case U'ì': case U'Ì': return other.v_ == U'ì' || other.v_ == U'Ì';
            case U'í': case U'Í': return other.v_ == U'í' || other.v_ == U'Í';
            case U'î': case U'Î': return other.v_ == U'î' || other.v_ == U'Î';
            case U'ï': case U'Ï': return other.v_ == U'ï' || other.v_ == U'Ï';
            case U'ð': case U'Ð': return other.v_ == U'ð' || other.v_ == U'Ð';
            case U'ñ': case U'Ñ': return other.v_ == U'ñ' || other.v_ == U'Ñ';
            case U'ò': case U'Ò': return other.v_ == U'ò' || other.v_ == U'Ò';
            case U'ó': case U'Ó': return other.v_ == U'ó' || other.v_ == U'Ó';
            case U'ô': case U'Ô': return other.v_ == U'ô' || other.v_ == U'Ô';
            case U'õ': case U'Õ': return other.v_ == U'õ' || other.v_ == U'Õ';
            case U'ö': case U'Ö': return other.v_ == U'ö' || other.v_ == U'Ö';
            case U'÷': case U'×': return other.v_ == U'÷' || other.v_ == U'×';
            case U'ø': case U'Ø': return other.v_ == U'ø' || other.v_ == U'Ø';
            case U'ù': case U'Ù': return other.v_ == U'ù' || other.v_ == U'Ù';
            case U'ú': case U'Ú': return other.v_ == U'ú' || other.v_ == U'Ú';
            case U'û': case U'Û': return other.v_ == U'û' || other.v_ == U'Û';
            case U'ü': case U'Ü': return other.v_ == U'ü' || other.v_ == U'Ü';
            case U'ý': case U'Ý': return other.v_ == U'ý' || other.v_ == U'Ý';
            case U'þ': case U'Þ': return other.v_ == U'þ' || other.v_ == U'Þ';
            case U'ß': case U'ẞ': return other.v_ == U'ß' || other.v_ == U'ẞ';

            default:
                return v_ == other.v_;
        }
    }

    Unicode_code_point as_upper_case() const noexcept
    {
        switch(v_) {
            case U'a': return U'A';
            case U'b': return U'B';
            case U'c': return U'C';
            case U'd': return U'D';
            case U'e': return U'E';
            case U'f': return U'F';
            case U'g': return U'G';
            case U'h': return U'H';
            case U'i': return U'I';
            case U'j': return U'J';
            case U'k': return U'K';
            case U'l': return U'L';
            case U'm': return U'M';
            case U'n': return U'N';
            case U'o': return U'O';
            case U'p': return U'P';
            case U'q': return U'Q';
            case U'r': return U'R';
            case U's': return U'S';
            case U't': return U'T';
            case U'u': return U'U';
            case U'v': return U'V';
            case U'w': return U'W';
            case U'x': return U'X';
            case U'y': return U'Y';
            case U'z': return U'Z';

            case U'à': return U'À';
            case U'á': return U'Á';
            case U'â': return U'Â';
            case U'ã': return U'Ã';
            case U'ä': return U'Ä';
            case U'å': return U'Å';
            case U'æ': return U'Æ';
            case U'ç': return U'Ç';
            case U'è': return U'È';
            case U'é': return U'É';
            case U'ê': return U'Ê';
            case U'ë': return U'Ë';
            case U'ì': return U'Ì';
            case U'í': return U'Í';
            case U'î': return U'Î';
            case U'ï': return U'Ï';
            case U'ð': return U'Ð';
            case U'ñ': return U'Ñ';
            case U'ò': return U'Ò';
            case U'ó': return U'Ó';
            case U'ô': return U'Ô';
            case U'õ': return U'Õ';
            case U'ö': return U'Ö';
            case U'÷': return U'×';
            case U'ø': return U'Ø';
            case U'ù': return U'Ù';
            case U'ú': return U'Ú';
            case U'û': return U'Û';
            case U'ü': return U'Ü';
            case U'ý': return U'Ý';
            case U'þ': return U'Þ';
            case U'ß': return U'ẞ';

            default:
                return v_;
        }
    }

    Unicode_code_point as_lower_case() const noexcept
    {
        switch(v_) {
            case U'A': return U'a';
            case U'B': return U'b';
            case U'C': return U'c';
            case U'D': return U'd';
            case U'E': return U'e';
            case U'F': return U'f';
            case U'G': return U'g';
            case U'H': return U'h';
            case U'I': return U'i';
            case U'J': return U'j';
            case U'K': return U'k';
            case U'L': return U'l';
            case U'M': return U'm';
            case U'N': return U'n';
            case U'O': return U'o';
            case U'P': return U'p';
            case U'Q': return U'q';
            case U'R': return U'r';
            case U'S': return U's';
            case U'T': return U't';
            case U'U': return U'u';
            case U'V': return U'v';
            case U'W': return U'w';
            case U'X': return U'x';
            case U'Y': return U'y';
            case U'Z': return U'z';

            case U'À': return U'à';
            case U'Á': return U'á';
            case U'Â': return U'â';
            case U'Ã': return U'ã';
            case U'Ä': return U'ä';
            case U'Å': return U'å';
            case U'Æ': return U'æ';
            case U'Ç': return U'ç';
            case U'È': return U'è';
            case U'É': return U'é';
            case U'Ê': return U'ê';
            case U'Ë': return U'ë';
            case U'Ì': return U'ì';
            case U'Í': return U'í';
            case U'Î': return U'î';
            case U'Ï': return U'ï';
            case U'Ð': return U'ð';
            case U'Ñ': return U'ñ';
            case U'Ò': return U'ò';
            case U'Ó': return U'ó';
            case U'Ô': return U'ô';
            case U'Õ': return U'õ';
            case U'Ö': return U'ö';
            case U'×': return U'÷';
            case U'Ø': return U'ø';
            case U'Ù': return U'ù';
            case U'Ú': return U'ú';
            case U'Û': return U'û';
            case U'Ü': return U'ü';
            case U'Ý': return U'ý';
            case U'Þ': return U'þ';
            case U'ẞ': return U'ß';

            default:
                return v_;
        }
    }



    uint32_t v_;
};







struct Utf8_view {

    Utf8_view(const std::string& v): v_{v}
    {
    }

    Utf8_view(std::string_view v): v_{v}
    {
    }

    Utf8_view(const char* const v): v_{v}
    {
    }

    struct Iterator {

        using Type = Utf8_byte_type;

        Iterator(Utf8_view* v, std::size_t pos): v_{v}, pos_{pos}, code_point_{to_code_point()}
        {
        }




        std::byte byte() const noexcept
        {
            return std::byte{static_cast<uint8_t>(v_->v_[pos_])};
        }

        Type type() const noexcept
        {
            return get_utf8_byte_type(byte());
        }

        const Unicode_code_point& get() const noexcept
        {
            return code_point_;
        }

        const Unicode_code_point& operator*() const noexcept
        {
            return code_point_;
        }

        const Unicode_code_point* operator->() const noexcept
        {
            return &code_point_;
        }

        operator const Unicode_code_point&() const noexcept
        {
            return code_point_;
        }

        bool operator==(const Iterator& other) const noexcept
        {
            return &v_->v_ == &other.v_->v_ && pos_ == other.pos_;
        }

        bool operator!=(const Iterator& other) const noexcept
        {
            return !(*this == other);
        }



        Iterator& operator++() noexcept
        {
            switch (type()) {
                case Type::continuation_byte:
                case Type::single_byte_ascii:
                    pos_ = std::min(pos_ + 1, v_->byte_count());
                    break;

                case Type::first_of_two_bytes:
                    pos_ = std::min(pos_ + 2, v_->byte_count());
                    break;

                case Type::first_of_three_bytes:
                    pos_ = std::min(pos_ + 3, v_->byte_count());
                    break;

                case Type::first_of_four_bytes:
                    pos_ = std::min(pos_ + 4, v_->byte_count());
                    break;
            }

            code_point_ = to_code_point();

            return *this;
        }


        Iterator operator++(int) const noexcept
        {
            return ++Iterator{*this};
        }


    private:

        Unicode_code_point to_code_point() noexcept
        {
            switch (type()) {

                case Type::continuation_byte:
                    return {0xFFFDu};

                case Type::single_byte_ascii:
                    return Unicode_code_point{v_->v_[pos_]};

                case Type::first_of_two_bytes:
                    if (pos_ + 1 >= v_->byte_count())
                        return {0xFFFDu};

                    return Unicode_code_point{v_->v_[pos_], v_->v_[pos_ + 1]};

                case Type::first_of_three_bytes:
                    if (pos_ + 2 >= v_->byte_count())
                        return {0xFFFDu};

                    return Unicode_code_point{v_->v_[pos_], v_->v_[pos_ + 1], v_->v_[pos_ + 2]};

                case Type::first_of_four_bytes:
                    if (pos_ + 3 >= v_->byte_count())
                        return {0xFFFDu};

                    return Unicode_code_point{v_->v_[pos_], v_->v_[pos_ + 1], v_->v_[pos_ + 2], v_->v_[pos_ + 3]};
            }
        }

        Utf8_view* v_;
        std::size_t pos_;
        Unicode_code_point code_point_;
    };


    Iterator begin()
    {
        return {this, 0};
    }

    Iterator end()
    {
        return {this, byte_count()};
    }


    std::size_t byte_count()
    {
        return v_.size();
    }


    bool icompare(Utf8_view other)
    {
        auto it = begin();
        auto other_it = other.begin();

        while (it != end() && other_it != other.end()) {
            if (!it->icompare(*other_it))
                return false;

            ++it;
            ++other_it;
        }

        return (it == end() && other_it == other.end());
    }


    bool iless(Utf8_view other)
    {
        auto it = begin();
        auto other_it = other.begin();

        while (it != end() && other_it != other.end()) {
            auto lc1 = it->as_lower_case();
            auto lc2 = other_it->as_lower_case();

            if (lc1.v_ < lc2.v_)
                return true;

            if (lc1.v_ > lc2.v_)
                return false;

            ++it;
            ++other_it;
        }

        return (it == end() && other_it != other.end());
    }



    std::string_view v_;
};


inline std::string as_lower_cased_string(Utf8_view v)
{
    std::string string;
    for (auto& c: v) {
        string += c.as_lower_case().to_utf8().data();
    }
    return string;
}


inline std::string as_upper_cased_string(Utf8_view v)
{
    std::string string;
    for (auto& c: v) {
        string += c.as_upper_case().to_utf8().data();
    }
    return string;
}


/**
 * This will truncate the string if either max_length or max_lines is reached.
 */
inline std::string truncate_by_length_and_lines(std::string s, std::size_t max_length, std::size_t max_lines, std::string truncate_marker = " ...")
{
    std::size_t glyph_count{0};
    std::size_t line_count{1};
    for (std::size_t i{0}; i != s.size(); ++i)
    {
        char c = s[i];
        if (is_utf8_code_point_begin(c))
            ++glyph_count;

        if (c == '\n')
            ++line_count;

        bool limit_reached = glyph_count > max_length || line_count > max_lines;
        if (limit_reached) {
            s.resize(i);
            s += truncate_marker;
            break;
        }
    }

    return s;
}




// Remove when string_view is finished in stdc++
inline std::string& operator +=(std::string& out, const std::string_view& in)
{
    return out.append(in.data(), in.size());
}


inline constexpr bool is_utf8(char c) {
    return (c & 0b1000'0000);
}

template <char trim_c>
inline std::string trim(const std::string& string)
{
    size_t first = string.find_first_not_of(trim_c);
    if (first == std::string::npos) {
        return {};
    }
    size_t last = string.find_last_not_of(trim_c);
    return string.substr(first, 1 + last - first);
}

/**
 * Split by template-supplied delimiter char.
 */
template<unsigned char delimiter>
inline std::vector<std::string> split(const std::string& string)
{
    return std::accumulate(string.begin(), string.end(), std::vector<std::string>{""}, [](auto& v, char c) {
        switch (c) {
            case delimiter: v.push_back(""); break;
            default: v.back().push_back(c); break;
        }
        return v;
    });
}

/**
 * Split by delimiter chars.
 */
template<std::size_t N>
inline std::vector<std::string> split(const std::string& string, std::array<char, N> delimiters)
{
    return std::accumulate(string.begin(), string.end(), std::vector<std::string>{""}, [&](auto& v, char c) {
        for (char d: delimiters) {
            if (c == d) {
                v.push_back("");
                return v;
            }
        }
        v.back().push_back(c);
        return v;
    });
}

/**
    * Split once by template-supplied delimiter char.
    */
template<unsigned char delimiter>
inline std::pair<std::string, std::string> split_once(const std::string& string)
{
    std::pair<std::string, std::string> parts;
    size_t size{string.size()}, i{0};
    while (i < size && string[i] != delimiter) {
        parts.first += string[i];
        ++i;
    }
    if (i < size && string[i] == delimiter) {
        ++i;
        while (i < size) {
            parts.second += string[i];
            ++i;
        }
    }
    return parts;
}

template<unsigned char delimiter>
static inline std::string join(const std::vector<std::string>& vector)
{
    switch (vector.size()) {
        case 0:
            return {""};

        case 1:
            return vector.back();

        default: {
            assert(vector.size() >= 2);

            size_t size = vector.size() - 1;
            for (const std::string& string: vector)
                size += string.size();

            std::string out;
            out.reserve(size);

            auto it = vector.begin();
            out = *it;

            while (++it != vector.end()) {
                out += delimiter;
                out += *it;
            }

            return out;
        }
    }
}

template<unsigned char delimiter, size_t Count>
inline std::string join(const std::array<std::string, Count>& strings)
{
    if (strings.empty()) {
        return {""};
    }
    return std::accumulate(strings.begin() + 1, strings.end(), std::string{strings.front()}, [](auto& out, auto& in) {
        out.push_back(delimiter);
        out += in;
        return out;
    });
}

inline std::string join(const std::vector<std::string>& strings, std::string_view glue)
{
    if (strings.empty()) {
        return {""};
    }
    return std::accumulate(strings.begin() + 1, strings.end(), std::string{strings.front()}, [&](auto& out, auto& in) {
        out += glue;
        out += in;
        return out;
    });
}

inline std::vector<std::string> prefix(const std::vector<std::string>& strings, const std::string_view& prefix)
{
    std::vector<std::string> prefixed_strings;
    for (const std::string& string: strings) {
        std::string t;
        t.reserve(prefix.size() + string.size());
        prefixed_strings.emplace_back((t += prefix) += string);
    }
    return prefixed_strings;
}

template <size_t Count>
inline std::array<std::string, Count> prefix(const std::array<std::string, Count>& strings, const std::string_view& prefix)
{
    std::array<std::string, Count> prefixed_strings;
    size_t i{0};
    for (const std::string& string: strings) {
        prefixed_strings[i].reserve(prefix.size() + string.size());
        prefixed_strings[i] += prefix;
        prefixed_strings[i] += string;
        ++i;
    }
    return prefixed_strings;
}

/**
 * Checks if needle (text to find) is in haystack (text which is searched) case insensitive.
 */
bool icontains(Utf8_view haystack, Utf8_view needle) noexcept;

/**
 * Compare two utf8 strings case insensitive.
 */
inline bool u8_iequal(Utf8_view v1, Utf8_view v2) noexcept
{
    return v1.icompare(v2);
}

/**
 * Sort two utf8 strings case insensitive.
 */
inline bool u8_iless(Utf8_view v1, Utf8_view v2) noexcept
{
    return v1.iless(v2);
}


