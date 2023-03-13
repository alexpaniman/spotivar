#include "axp/exceptions.h"
#include "axp/source-location.h"

#include "fmt/format.h"
#include "fmt/color.h"

using fe = fmt::emphasis;
using fc = fmt::color;


std::string axp::details::generate_error_message(std::string error_message, source_location location,
                                                 source_location::location_vis_conf conf) {

    std::string message;

    message += fmt::format(fe::bold | fg(fc::indian_red), "error: ");
    message += fmt::format("“{}”\n", error_message);

    message += fmt::format("occured in ");
    message += fmt::format(fe::bold | fg(fc::sky_blue), "{}", location.to_string());
    message += fmt::format(":\n");

    message += location.generate_visualized_location(conf);

    return message;
}


std::string axp::details::generate_nested_error_message(std::exception_ptr nested) {
    std::string message =
        fmt::format(fe::bold | fg(fc::light_golden_rod_yellow), "\nbecause: ");

    try {
        std::rethrow_exception(nested);
    }
    catch (std::exception &exc) {
        message += exc.what();
    }
    catch (...) {
        message += "happened unknown nested exception!\n";
    }

    return message;
}
