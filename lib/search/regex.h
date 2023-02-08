#pragma once

#include <string>

class regex {
public:
    regex(std::string pattern);
    bool matches(std::string_view view);
};
