#pragma once

#include "fmt/format.h"

namespace axp {

    struct source_location {
        const char *file, *function;
        std::size_t line,  column;

        struct location_vis_conf {
            int num_pred_lines;
            int num_succ_lines;
            int line_num_space;
        };

        std::string generate_visualized_location(location_vis_conf props) const;

        std::string to_string() const {
            return fmt::format("{}:{}:{} ({})", file, line, column, function);
        }

    };

    // should be used as a default parameter, allows to get point where function is called:
    inline source_location current_location(
            const char* file = __builtin_FILE(), const char* function = __builtin_FUNCTION(),
            std::size_t line = __builtin_LINE(), std::size_t   column = __builtin_COLUMN()) {
        return { file, function, line, column };
    }

}
