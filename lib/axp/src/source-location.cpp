#include "axp/source-location.h"
#include "axp/strings.h"

#include "fmt/color.h"
#include <fstream>

std::string axp::source_location::generate_visualized_location(location_vis_conf props) const {
    std::ifstream ifs { file };

    int first_line_number = line - props.num_pred_lines;
    auto lines = read_selected_lines(ifs, first_line_number, line + props.num_succ_lines);

    int max_line_length = std::numeric_limits<int>::min();
    for (auto &&element: lines)
        max_line_length = std::max<int>(max_line_length, element.size());

    std::string boxed_text;

    boxed_text += "┌" + repeat("─", props.line_num_space + 2) + "┬" + repeat("─", max_line_length + 2) + "┐\n";

    for (int i = 0; i < (int) lines.size(); ++ i) {
        fmt::text_style printing_style;

        bool is_target_line = i == props.num_pred_lines;
        if (is_target_line)
            printing_style = fmt::emphasis::bold | fg(fmt::color::indian_red);

        // print current line number:
        int current_line = i + first_line_number;
        boxed_text += "│ " + fmt::format(printing_style, "{:>{}}", current_line, props.line_num_space) + " │ ";

        // print current source string:
        boxed_text += fmt::format(printing_style, "{:<{}}", lines[i], max_line_length) + " │\n";
        if (is_target_line) {
            std::string pointer = fmt::format("{:>{}}", "^", column);

            fmt::text_style pointer_style = fmt::fg(fmt::color::sky_blue) | fmt::emphasis::bold;

            boxed_text += "│ " + fmt::format(pointer_style, "{:>{}}", "!!", props.line_num_space) + " │ ";
            boxed_text += fmt::format(pointer_style, "{:<{}}", pointer, max_line_length) + " │\n";
        }
    }

    boxed_text += "└" + repeat("─", props.line_num_space + 2) + "┴" + repeat("─", max_line_length + 2) + "┘\n";
    return boxed_text;
}
