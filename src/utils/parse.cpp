#include <expected>

#include "main.hpp"
#include "parse.hpp"

#include "Utils/rdb.hpp"
#include "core/console.hpp"

// extern void ShutdownGamescope();

namespace opt
{
    template<typename T>
    auto handle_error(error_msg_t err) -> std::expected<T, error_t>
    {
        console_log.error("{}", err);
        // ShutdownGamescope();
        std::exit(1);
        return std::unexpected(err);
    }
} // namespace opt

namespace opt::aspect_ratio
{
    auto set(float val) -> aspect_ratio_t
    {
        g_aspectRatio = val;
        return {val};
    }

#if 0
    auto parse(arg_t arg) -> aspect_ratio_t
    {
        std::string_view str{arg};
        return parser(str).and_then(set).or_else(error);
    }
#else

    void parse(arg_t arg)
    {
        std::string_view str{arg};
        [[maybe_unused]]
        auto ret = parser(str).and_then(set).or_else(error);
    }
#endif
    auto error(error_msg_t err) -> aspect_ratio_t
    {
        return handle_error<float>(err);
    }

    auto parser(parser_arg_t arg) -> aspect_ratio_t
    {
        if (arg == "auto")
        {
            return {rdb::AspectRatioValue.at(0)};
        }

        auto const* it = std::ranges::find(rdb::AspectRatioName, arg);

        if (it != rdb::AspectRatioName.end())
        {
            auto i = std::ranges::distance(rdb::AspectRatioName.begin(), it);
            return {rdb::AspectRatioValue.at(i)};
        }

        auto msg = std::format("invalid value for --{}, \"{}\" is not supported", name, arg);
        return std::unexpected(msg);
    }

} // namespace opt::aspect_ratio
