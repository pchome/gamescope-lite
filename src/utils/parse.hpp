#pragma once

#include <expected>
#include <string>

namespace opt
{
    using std::operator""sv;

    /** optarg type */
    using arg_t = char const*;

    /** parser optarg type */
    using parser_arg_t = std::string_view const&;

    /** expected error type */
    using error_t = std::string;

    /** error message type */
    using error_msg_t = std::string const&;

    /** generic error handler */
    template<typename T>
    auto handle_error(error_msg_t err) -> std::expected<T, error_t>;

} // namespace opt

/**
 * Aspect ratio
 * -a, --aspect-ratio option handlers
 */
namespace opt::aspect_ratio
{
    /** option long name */
    constexpr auto name = "aspect-ratio"sv;

    /** option short name */
    constexpr auto alias = "a"sv;

    /** expected type */
    using aspect_ratio_t = std::expected<float, error_t>;

    /** apply parsed value */
    auto set(float val) -> aspect_ratio_t;
#if 0
    /** parse optarg */
    auto parse(arg_t arg) -> aspect_ratio_t;
#else
    /** parse optarg */
    void parse(arg_t arg);
#endif
    /** handle arg error */
    auto error(error_msg_t err) -> aspect_ratio_t;

    /** option argument parser */
    auto parser(parser_arg_t arg) -> aspect_ratio_t;

} // namespace opt::aspect_ratio
