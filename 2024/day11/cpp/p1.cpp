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
#include <limits>
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
#include <unordered_map>
#include <utility>
#include <vector>

template <typename IterTA, typename IterTB>
std::tuple<IterTA, IterTB> iter_swap_range(IterTA first1, IterTA last1,
                                           IterTB first2, IterTB last2) {
  for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
    std::iter_swap(first1, first2);
  }
  return {first1, first2};
}

template <typename Container>
std::tuple<unsigned long, unsigned long>
iter_swap_range(Container &container, unsigned long first1, unsigned long last1,
                unsigned long first2, unsigned long last2) {
  auto f1 = std::next(container.begin(), static_cast<long>(first1));
  auto l1 = std::next(container.begin(), static_cast<long>(last1));
  auto f2 = std::next(container.begin(), static_cast<long>(first2));
  auto l2 = std::next(container.begin(), static_cast<long>(last2));

  auto [final1, final2] = iter_swap_range(f1, l1, f2, l2);

  return {std::distance(container.begin(), final1),
          std::distance(container.begin(), final2)};
}

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
  LineWrapper(std::ifstream &stream_) : ref(stream_) {}
  const auto begin() { return std::istream_iterator<Line>(ref.get()); }
  const auto end() { return std::istream_iterator<Line>(); }
};

struct OwningLineWrapper {
  std::ifstream stream;

  OwningLineWrapper(std::ifstream &&stream_) : stream(std::move(stream_)) {}
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

template <typename T> void print_linevec(const std::vector<T> &line) {
  std::string str_line{};
  std::for_each(line.begin(), line.end(), [&str_line](auto &k) {
    str_line += ((k != std::numeric_limits<T>::max())
                     ? std::to_string(static_cast<unsigned int>(k))
                     : ".");
  });
  print("{}", str_line);
}

std::vector<unsigned long> transform(const std::vector<unsigned long> &line) {
  std::vector<unsigned long> lineres{};
  lineres.reserve(line.size());

  for (auto i = 0UL; i < line.size(); ++i) {
    if (line[i] == 0) {
      lineres.push_back(1);
      continue;
    }

    auto digits =
        static_cast<unsigned long>(std::ceil(std::log10(line[i] + 1)));
    if (digits % 2 == 0) {
      auto lsb = line[i] % static_cast<unsigned long>(std::pow(10, digits / 2));
      auto msb = line[i] / static_cast<unsigned long>(std::pow(10, digits / 2));
      lineres.push_back(msb);
      lineres.push_back(lsb);
      continue;
    }

    lineres.push_back(line[i] * 2024);
  }
  return lineres;
}

auto split_str(std::string_view str, std::string_view substr) {
  auto splits = str | std::ranges::views::split(substr);

  std::vector<unsigned long> result;

  for (const auto &k : splits) {
    result.push_back(str_to<unsigned long>(k.begin(), k.end()));
  }

  return result;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<std::vector<unsigned char>> map{};

  auto lines = get_lines("input");
  auto line_str = std::string(*lines.begin());
  auto line = split_str(line_str, " ");

  auto do_n_times = 25UL;

  // print_vec(line);

  for (auto i = 0UL; i < do_n_times; ++i) {
    line = transform(line);
    // print_vec(line);
  }

  print("result: {}", line.size());

  return 0;
}