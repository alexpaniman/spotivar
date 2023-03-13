#include "axp/strings.h"

#include <numeric>

void skip_lines(std::istream &ifs, int line_count) {
    for (int i = 0; ifs && i < line_count; ++ i)
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}          

std::vector<std::string> read_lines(std::istream &ifs, int line_count) {
    std::vector<std::string> lines;
    for (int i = 0; ifs && i < line_count; ++ i) {
        std::string line;
        std::getline(ifs, line);

        lines.push_back(line);
    }

    return lines;
}          

std::vector<std::string> read_selected_lines(std::istream &ifs, int from, int to) {
    ifs.seekg(std::ios::beg);

    skip_lines(ifs, from - 1);
    return read_lines(ifs, to - from);
}

std::string repeat(std::string repeated, std::size_t times) {
    if (times == 0) {
        repeated.clear();
        repeated.shrink_to_fit();
        return repeated;
    }

    if (times == 1 || repeated.empty())
        return repeated;

    const auto period = repeated.size();
    if (period == 1) {
        repeated.append(times - 1, repeated.front());
        return repeated;
    }

    repeated.reserve(period * times);

    std::size_t repeated_times = 2;
    for (; repeated_times < times; repeated_times *= 2)
        repeated += repeated;

    repeated.append(repeated.c_str(),
        (times - (repeated_times / 2)) * period);

    return repeated;
}
