#pragma once

#include <string>
#include <cstddef>
#include <istream>
#include <vector>

std::string repeat(std::string repeated, std::size_t times);

// skip /line_count/ lines form selected stream
void skip_lines(std::istream &ifs, int line_count);

// reads lines from current point in the stream
// until /line_count/ lines is read or file ended
std::vector<std::string> read_lines(std::istream &ifs, int line_count);

// reads selected range of lines counting from the beginning 
// of passed input stream
std::vector<std::string> read_selected_lines(std::istream &ifs, int from, int to);

