#include "Core/GroupPattern.h"

namespace sc2fla {
    bool NameMatchesGroupPattern(const std::string& name, const std::string& pattern) {
        if (pattern.find('*') == std::string::npos) {
            return name.find(pattern) != std::string::npos;
        }

        size_t n = 0;
        size_t p = 0;
        size_t star = std::string::npos;
        size_t mark = 0;
        while (n < name.size()) {
            if (p < pattern.size() && pattern[p] == '*') {
                star = p++;
                mark = n;
            } else if (p < pattern.size() && pattern[p] == name[n]) {
                ++p;
                ++n;
            } else if (star != std::string::npos) {
                p = star + 1;
                n = ++mark;
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*')
            ++p;
        return p == pattern.size();
    }
}
