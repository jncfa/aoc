#include <algorithm>
#include <array>
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
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

template <typename T, typename... TArgs>
T str_to(const std::string &str, TArgs &&...args) {
  T result{};
  auto [ptr, ec] = std::from_chars(str.begin().base(), str.end().base(), result,
                                   std::forward<TArgs>(args)...);
  if (ec != std::errc()) {
    throw std::system_error(std::make_error_code(ec));
  }
  return result;
}

template <typename T, typename TIter, typename... TArgs>
T str_to(TIter iter_begin, TIter iter_end, TArgs &&...args) {
  T temp;
  std::from_chars(iter_begin, iter_end, temp, std::forward<TArgs>(args)...);
  return temp;
}

template <typename... ArgsT>
void print(const std::format_string<ArgsT...> fmt, ArgsT &&...args) {
  std::cout << std::format(fmt, std::forward<ArgsT>(args)...) << std::endl;
}

template <typename T, typename Functor>
void print_vec(const std::vector<T> &vec, Functor to_str) {
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
  print("[{}]", final);
}

template <typename T> void print_vec(const std::vector<T> &vec) {
  return print_vec(vec, [](const T &val) { return std::to_string(val); });
}

class Line : public std::string {};

std::istream &operator>>(std::istream &is, Line &l) {
  std::getline(is, l);
  return is;
}

struct LineWrapper {
  std::reference_wrapper<std::ifstream> ref;
  LineWrapper(std::ifstream &stream_) : ref(stream_){};
  auto begin() { return std::istream_iterator<Line>(ref.get()); }
  auto end() { return std::istream_iterator<Line>(); }
};

struct OwningLineWrapper {
  std::ifstream stream;

  OwningLineWrapper(std::ifstream &&stream_) : stream(std::move(stream_)){};
  auto begin() { return std::istream_iterator<Line>(stream); }
  auto end() { return std::istream_iterator<Line>(); }
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

std::optional<std::vector<std::tuple<unsigned long, unsigned long>>>
get_neighbours(const std::vector<std::string> &lines, unsigned long x,
               unsigned long y) {

  // search forward for XMAS
  // search backward for XMAS
  // search diagonally for XMAS
  // search final diagonal for XMAS

  const auto directions = std::array<std::tuple<int, int>, 8>({
      // diagonals
      {1, 1},
      {1, -1},
  });

  auto is_in_bounds = [&lines](int x, int y) -> bool {
    return (static_cast<signed long>(x) <
                static_cast<signed long>(lines.size()) &&
            static_cast<signed long>(x) >= 0 &&
            static_cast<signed long>(y) <
                static_cast<signed long>(lines[x].size()) &&
            static_cast<signed long>(y) >= 0);
  };

  const std::string_view MAS = "MAS";
  auto get_char = [&lines, &is_in_bounds](int x, int y) -> std::optional<char> {
    if (is_in_bounds(x, y)) [[likely]] {
      return lines[x][y];
    }
    return std::nullopt;
  };

  bool found_xmas = false;
  for (const auto &dir : directions) {
    const auto x_dir = std::get<0>(dir);
    const auto y_dir = std::get<1>(dir);
    const std::tuple<int, int> p1 = {static_cast<int>(x) - x_dir, static_cast<int>(y) - y_dir};
    const std::tuple<int, int> p2 = {static_cast<int>(x) + x_dir, static_cast<int>(y) + y_dir};

    const auto p1_char = get_char(std::get<0>(p1), std::get<1>(p1));
    const auto p2_char = get_char(std::get<0>(p2), std::get<1>(p2));

    if (! p1_char.has_value() || ! p2_char.has_value()) {
      return std::nullopt;
    }

    if ((p1_char.value() == MAS[0] && p2_char.value() == MAS[2]) || (p1_char.value() == MAS[2] && p2_char.value() == MAS[0])) {
      if (!found_xmas) {
        found_xmas  = true;
        continue;
      }
      
      return std::vector<std::tuple<unsigned long, unsigned long>>{
      {x, y}, {x + 1, y + 1}, {x - 1, y - 1}, {x + 1, y - 1}, {x - 1, y + 1}};
    }
  }

  return std::nullopt;
}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<std::string> lines;
  std::vector<std::string> masked_lines;

  for (auto &k : get_lines("input")) {
    print("{}", k.c_str());
    lines.push_back(k);
    masked_lines.push_back(std::string(k.size(), '.'));
  }

  auto total_xmas = 0;

  for (auto i = 0UL; i < lines.size(); ++i) {
    for (auto j = 0UL; j < lines[i].size(); ++j) {
      const auto current_char = lines[i][j];

      if (current_char == 'A') {
        const auto neighbours = get_neighbours(lines, i, j);
        if (!neighbours.has_value()) {
          print("no neighbours for {}", current_char);
          continue;
        }
        total_xmas += 1;

        // for each neighbour, mark points in masked lines with the correct char
        for (const auto &k : neighbours.value()) {
          masked_lines[std::get<0>(k)][std::get<1>(k)] =
              lines[std::get<0>(k)][std::get<1>(k)];
        }
      }
    }
  }

  std::for_each(masked_lines.begin(), masked_lines.end(),
                [](const auto &k) { print("{}", k.c_str()); });
  print("total xmas: {}", total_xmas);

  return 0;
}