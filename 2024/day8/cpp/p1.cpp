#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <execution>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

const char *ws = " \t\n\r\f\v";

// trim from end of string (right)
inline std::string rtrim(std::string s, const char *t = ws) {
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

// trim from beginning of string (left)
inline std::string ltrim(std::string s, const char *t = ws) {
  s.erase(0, s.find_first_not_of(t));
  return s;
}

// trim from both ends of string (right then left)
inline std::string trim(std::string s, const char *t = ws) {
  return ltrim(rtrim(s, t), t);
}

template <typename T, typename TIter, typename... TArgs>
T str_to(TIter iter_begin, TIter iter_end, TArgs &&...args) {
  T result{};
  auto [ptr, ec] = std::from_chars(iter_begin, iter_end, result,
                                   std::forward<TArgs>(args)...);
  if (ec != std::errc()) {
    throw std::system_error(std::make_error_code(ec));
  }
  return result;
}

template <typename T, typename... TArgs>
T str_to(const std::string &str, TArgs &&...args) {
  return str_to<T>(str.begin().base(), str.end().base(),
                   std::forward<TArgs>(args)...);
}

template <typename... ArgsT>
void print(const std::format_string<ArgsT...> fmt, ArgsT &&...args) {
  std::cout << std::format(fmt, std::forward<ArgsT>(args)...) << std::endl;
}

template <typename T, typename Functor, typename... ArgsT>
void print_vec(const std::format_string<ArgsT..., std::string_view> fmt,
               const std::vector<T> &vec, Functor to_str, ArgsT &&...args) {
  const auto join_vec = [](const std::string &val1,
                           const std::string &val2) -> std::string {
    if (val1 != "") [[likely]] {
      return std::format("{}, {}", val1, val2);
    } else {
      return val2;
    }
  };
  const auto final = std::transform_reduce(
      vec.begin(), vec.end(), std::string(), join_vec,
      [&to_str](const T &val) -> std::string { return to_str(val); });
  print(fmt, std::forward<ArgsT>(args)..., std::string_view(final));
}

template <typename T, typename Functor>
void print_vec(const std::vector<T> &vec, Functor to_str) {
  return print_vec("[{}]", vec, to_str);
}

template <typename T> void print_vec(const std::vector<T> &vec) {
  return print_vec(vec, [](const T &val) { return std::to_string(val); });
}

template <typename T, typename... ArgsT>
void print_vec(const std::format_string<ArgsT..., std::string_view> fmt,
               const std::vector<T> &vec, ArgsT &&...args) {
  return print_vec(
      fmt, vec, [](const T &val) { return std::to_string(val); },
      std::forward<ArgsT>(args)...);
}
class Line : public std::string {};

std::istream &operator>>(std::istream &is, Line &l) {
  std::getline(is, l);
  return is;
}

struct LineWrapper {
  std::reference_wrapper<std::ifstream> ref;
  LineWrapper(std::ifstream &stream_) : ref(stream_){};
  const auto begin() { return std::istream_iterator<Line>(ref.get()); }
  const auto end() { return std::istream_iterator<Line>(); }
};

struct OwningLineWrapper {
  std::ifstream stream;

  OwningLineWrapper(std::ifstream &&stream_) : stream(std::move(stream_)){};
  const auto begin() { return std::istream_iterator<Line>(stream); }
  const auto end() { return std::istream_iterator<Line>(); }
};

std::string read_file(std::ifstream &stream) {
  std::ostringstream sstr;
  sstr << stream.rdbuf();
  return sstr.str();
}

auto get_lines(std::ifstream &stream) { return LineWrapper(stream); }

auto get_lines(const std::string_view &file) {
  std::ifstream myfile{};
  myfile.open(file.data());

  if (myfile.is_open()) {
    return OwningLineWrapper(std::move(myfile));
  }
  throw std::system_error(
      std::make_error_code(std::errc::no_such_file_or_directory));
}

std::tuple<unsigned long, unsigned long>
get_next_location(const std::tuple<unsigned long, unsigned long> &location,
                  const std::tuple<int, int> &velocity) {
  return {
      static_cast<unsigned long>(std::get<0>(location) + std::get<0>(velocity)),
      static_cast<unsigned long>(std::get<1>(location) +
                                 std::get<1>(velocity))};
}

bool is_in_bounds(const std::tuple<unsigned long, unsigned long> &location,
                  const std::vector<std::string> &map) {
  return (std::get<0>(location) < map.size() &&
          std::get<1>(location) < map[std::get<0>(location)].size());
}

std::tuple<int, int>
get_velocity(const std::tuple<unsigned long, unsigned long> &location1,
             const std::tuple<unsigned long, unsigned long> &location2) {
  return {static_cast<int>(std::get<0>(location2)) -
              static_cast<int>(std::get<0>(location1)),
          static_cast<int>(std::get<1>(location2)) -
              static_cast<int>(std::get<1>(location1))};
}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<std::string> map;
  auto lines = get_lines("input");
  std::transform(lines.begin(), lines.end(), std::back_inserter(map),
                 [](const auto &k) { return k; });
  // map for each node type, and the locations of its nodes
  std::map<char, std::vector<std::tuple<unsigned long, unsigned long>>>
      node_map{};

  for (auto i = 0UL; i < map.size(); ++i) {
    auto current_row = map[i];
    for (auto j = 0UL; j < current_row.size(); ++j) {
      const auto antena_type = current_row[j];
      if (antena_type != '.') {
        print("found a antena of type {} at ({}, {})", antena_type, i, j);
        node_map[antena_type].push_back({i, j});
      }
    }
  }

  std::set<std::tuple<unsigned long, unsigned long>>
      unique_antinode_locations{};
  for (const auto &k : node_map) {
    const auto node_locations = k.second;
    print("node {} has {} locations", k.first, node_locations.size());
    for (auto i = 0UL; i < node_locations.size(); ++i) {
      const auto node_i = node_locations[i];
      for (auto j = i + 1; j < node_locations.size(); ++j) {
        const auto node_j = node_locations[j];

        const auto next_i =
            get_next_location(node_i, get_velocity(node_j, node_i));
        if (is_in_bounds(next_i, map)) {
          unique_antinode_locations.insert(next_i);
        }
        const auto next_j =
            get_next_location(node_j, get_velocity(node_i, node_j));
        if (is_in_bounds(next_j, map)) {
          unique_antinode_locations.insert(next_j);
        }
      }
    }
  }

  std::for_each(unique_antinode_locations.begin(),
                unique_antinode_locations.end(), [&map](auto &k) {
                  if (map[std::get<0>(k)][std::get<1>(k)] == '.')
                    map[std::get<0>(k)][std::get<1>(k)] = '#';
                });
  print("map");
  std::for_each(map.begin(), map.end(), [](auto &k) { print("{}", k); });

  print("total unique antenna locations: {}", unique_antinode_locations.size());
}