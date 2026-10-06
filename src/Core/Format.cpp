#include "Core/Format.h"

#include <charconv>
#include <cmath>
#include <cstring>

namespace sc2fla {
    void AppendRepr(std::string& out, double value) {
        if (value == 0.0) {
            out += std::signbit(value) ? "-0.0" : "0.0";
            return;
        }
        if (std::isnan(value)) {
            out += "nan";
            return;
        }
        if (std::isinf(value)) {
            out += value < 0 ? "-inf" : "inf";
            return;
        }

        const bool negative = value < 0.0;
        const double magnitude = negative ? -value : value;

        char scientific[40];
        const auto result = std::to_chars(scientific, scientific + sizeof(scientific), magnitude, std::chars_format::scientific);
        const char* end = result.ptr;
        const char* exponent_mark = static_cast<const char*>(std::memchr(scientific, 'e', static_cast<size_t>(end - scientific)));

        char digits[24];
        size_t digit_count = 0;
        for (const char* it = scientific; it != exponent_mark; ++it) {
            if (*it != '.')
                digits[digit_count++] = *it;
        }
        while (digit_count > 1 && digits[digit_count - 1] == '0')
            --digit_count;

        int exponent = 0;
        std::from_chars(exponent_mark + (exponent_mark[1] == '+' ? 2 : 1), end, exponent);
        const int decimal_point = exponent + 1;

        if (negative)
            out.push_back('-');

        if (decimal_point <= -4 || decimal_point > 16) {
            out.push_back(digits[0]);
            if (digit_count > 1) {
                out.push_back('.');
                out.append(digits + 1, digit_count - 1);
            }
            const int shown = decimal_point - 1;
            out.push_back('e');
            out.push_back(shown < 0 ? '-' : '+');
            const int magnitude_exponent = shown < 0 ? -shown : shown;
            if (magnitude_exponent < 10)
                out.push_back('0');
            char exponent_text[8];
            const auto exponent_result = std::to_chars(exponent_text, exponent_text + sizeof(exponent_text), magnitude_exponent);
            out.append(exponent_text, static_cast<size_t>(exponent_result.ptr - exponent_text));
        } else if (decimal_point <= 0) {
            out += "0.";
            out.append(static_cast<size_t>(-decimal_point), '0');
            out.append(digits, digit_count);
        } else if (static_cast<size_t>(decimal_point) >= digit_count) {
            out.append(digits, digit_count);
            out.append(static_cast<size_t>(decimal_point) - digit_count, '0');
            out += ".0";
        } else {
            out.append(digits, static_cast<size_t>(decimal_point));
            out.push_back('.');
            out.append(digits + decimal_point, digit_count - static_cast<size_t>(decimal_point));
        }
    }

    std::string ReprDouble(double value) {
        std::string out;
        AppendRepr(out, value);
        return out;
    }
}
