#pragma once

#include "axp/source-location.h"
#include "axp/arg-pass-utility.h"

#include <vector>
#include <exception>

namespace axp {


    namespace details {

        inline constexpr source_location::location_vis_conf default_location_vis = {
            .num_pred_lines = 5,
            .num_succ_lines = 3,
            .line_num_space = 4
        };

        std::string generate_error_message(std::string error_message, source_location location,
                                           source_location::location_vis_conf conf = default_location_vis);

        std::string generate_nested_error_message(std::exception_ptr ptr);
    }


    template <typename... formatted_types>
    class exception: public std::exception {
    public:
        exception(fmt::string_view format, formatted_types&&... printed_data,
                  axp::source_location location = axp::current_location())
            : m_message(fmt::vformat(format, fmt::make_format_args(FWD(printed_data)...))),
              m_error_location(location) {}

        const char* what() const noexcept final {
            generate_error_message_lazily();
            return m_cached_what_message.c_str();
        }

    protected:
        virtual std::string generate_error_message() const {
            return details::generate_error_message(m_message, m_error_location);
        }

    private:
        std::string m_message;
        axp::source_location m_error_location;

        mutable std::string m_cached_what_message;
        void generate_error_message_lazily() const {
            // already calculated, use cached string:
            if (!m_cached_what_message.empty())
                return;

            m_cached_what_message = generate_error_message();
        }

    };

    exception(std::string_view format, auto&&... printed_data) -> exception<decltype(printed_data)...>;


    template <template <typename...> class exception_base = exception, typename... formatted_types>
    class nested_exception: public exception_base<formatted_types...> {
    public:
        using exception_base<formatted_types...>::exception_base;

        std::string generate_error_message() const override {
            std::string message = exception_base<formatted_types...>::generate_error_message();
            return message += axp::details::generate_nested_error_message(m_nested_exception);
        }

    private:
        std::exception_ptr m_nested_exception = std::current_exception();
    };

    template <template <typename...> class exception_base = exception>
    nested_exception(std::string_view format, auto&&... printed_data) -> nested_exception<exception_base, decltype(printed_data)...>;


}


// macro to define you're own marker exception that differs from axp::exception
// only by name, allowing it to be catched separately by a catch block
#define AXP_CREATE_NEW_EXCEPTION(new_exception_name)                      \
    template <typename... formatted_types>                                \
    class new_exception_name: public axp::exception<formatted_types...> { \
    public:                                                               \
        using axp::exception<formatted_types...>::exception;              \
    };                                                                    \
