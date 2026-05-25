#pragma once
#include <filesystem>
#include <list>
#include <ranges>

using std::operator""sv;
using Path = std::filesystem::path;

auto GetEnv(char const* name, char const* def = "") {
  char const* pszValue = std::getenv(name);
  return pszValue ?: def;
}

/** XDG_DATA_HOME and XDG_DATA_DIRS */
namespace xdg::data {
constexpr auto home() -> Path { return GetEnv("XDG_DATA_HOME", ".local/share"); }
constexpr auto dirs() -> std::string { return GetEnv("XDG_DATA_DIRS", "/usr/local/share:/usr/share"); }
constexpr auto append(std::string const& s) -> std::string { return dirs() + ":" + s; }
constexpr auto prepend(std::string const& s) -> std::string { return s + ":" + dirs(); }
constexpr auto transform(Path const& s) -> std::string {
  std::string out{};
  for (auto const data_path : std::views::split(dirs(), ":"sv)) {
    if (!out.empty())
      out += ":";
    out += Path(std::string_view(data_path) / s);
  }
  return out;
}
constexpr auto transform(std::list<Path> const& list) -> std::string {
  std::string out{};
  for (auto const data_path : std::views::split(dirs(), ":"sv)) {
    for (auto& item : list) {
      if (!out.empty())
        out += ":";
      out += Path(std::string_view(data_path) / item);
    }
  }
  return out;
}
} // namespace xdg::data

/** XDG_CONFIG_HOME and XDG_CONFIG_DIRS */
namespace xdg::config {
constexpr auto home() -> Path { return GetEnv("XDG_CONFIG_HOME", ".config"); }
constexpr auto dirs() -> std::string { return GetEnv("XDG_CONFIG_DIRS", "/usr/local/etc/xdg:/etc/xdg"); }
constexpr auto append(std::string const& s) -> std::string { return dirs() + ":" + s; }
constexpr auto prepend(std::string const& s) -> std::string { return s + ":" + dirs(); }
} // namespace xdg::config

/** *HOME */
namespace xdg::user {
constexpr auto home() -> Path { return GetEnv("HOME", "."); }
constexpr auto data() -> Path {
  return xdg::data::home().is_relative() ? home() / xdg::data::home() : xdg::data::home();
}
constexpr auto config() -> Path {
  return xdg::config::home().is_relative() ? home() / xdg::config::home() : xdg::config::home();
}
constexpr auto data_transform(std::list<Path> const& list) -> std::string {
  std::string out{};
  for (auto& item : list) {
    if (!out.empty())
      out += ":";
    out += data() / item;
  }
  return out;
}
constexpr auto config_transform(std::list<Path> const& list) -> std::string {
  std::string out{};
  for (auto& item : list) {
    if (!out.empty())
      out += ":";
    out += config() / item;
  }
  return out;
}
} // namespace xdg::user
