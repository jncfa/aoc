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

struct Equation {
  long int result{};
  std::vector<unsigned long> coefficients{};

  Equation() = default;

  Equation(long int result_, std::vector<unsigned long> coefficients_)
      : result(result_), coefficients(coefficients_) {}
};

enum class Operation {
  Add = '+',
  Mul = '*',
};

auto next_op(const auto &op) {
  switch (op) {
  case Operation::Add:
    return Operation::Mul;
  case Operation::Mul:
    return Operation::Add;
  }
  throw std::runtime_error("invalid operation");
}

auto apply_operation(const auto &op, const auto &lhs, const auto &rhs) {
  switch (op) {
  case Operation::Add:
    return std::plus<>{}(lhs, rhs);
  case Operation::Mul:
    return std::multiplies<>{}(lhs, rhs);
  }
}

struct SolvedEquation : public Equation {
  std::vector<Operation> operations;

  SolvedEquation(Equation equation, std::vector<Operation> operations_)
      : Equation(equation), operations(operations_) {
    assert(operations.size() == coefficients.size() - 1);
  }

  std::string to_string() const {
    std::string str_result{};
    str_result += std::format("{} = {}", std::to_string(result),
                              std::to_string(*coefficients.begin()));

    auto rhs_it = std::next(coefficients.begin());
    for (auto it = operations.begin();
         it != operations.end() && rhs_it != coefficients.end();
         ++it, ++rhs_it) {
      str_result += std::format(" {} {}", static_cast<char>(*it),
                                std::to_string(*rhs_it));
    }
    return str_result;
  }
};

auto apply_operations(const std::vector<unsigned long> &coeffs,
                      const std::vector<Operation> &ops) {
  assert(ops.size() == coeffs.size() - 1);

  auto result = *coeffs.begin();
  auto rhs_it = std::next(coeffs.begin());
  for (auto it = ops.begin(); it != ops.end() && rhs_it != coeffs.end();
       ++it, ++rhs_it) {
    // print("applying operation {} to {} and {}", static_cast<char>(*it),
    // result,*rhs_it);
    result = apply_operation(*it, result, static_cast<long int>(*rhs_it));
  }
  return result;
}

std::optional<std::vector<Operation>>
get_next_lexographically(std::vector<Operation> &operations) {
  // add -> sub -> mul -> div
  // if we have a div, then we change it to a add, and carry over the value
  auto result = std::any_of(operations.begin(), operations.end(), [](auto &k) {
    if (k == Operation::Mul) {
      k = next_op(k);
      return false;
    } else {
      k = next_op(k);
      return true;
    }
  });
  if (result) {
    return operations;
  }
  return std::nullopt;
}

std::optional<SolvedEquation> solve_equation(const Equation &equation) {
  auto operations =
      std::vector<Operation>{equation.coefficients.size() - 1, Operation::Add};

  bool found_solution = false;

  while (!(found_solution =
               (equation.result ==
                apply_operations(equation.coefficients, operations)))) {
    auto next_operations = get_next_lexographically(operations);
    if (!next_operations.has_value()) {
      break;
    }
  }

  if (!found_solution) {
    return std::nullopt;
  }

  return SolvedEquation{equation, operations};
}

Equation parse_equation(const std::string &equation) {
  // split string into "result" and "coefficients"
  auto splits =
      std::ranges::views::transform(equation | std::ranges::views::split(':'),
                                    [](const auto &k) { return k.data(); });
  std::vector<std::string> splits_vec(splits.begin(), splits.end());

  assert(splits_vec.size() == 2);

  std::transform(splits_vec.begin(), splits_vec.end(), splits_vec.begin(),
                 [](const auto &k) { return trim(std::string(k)); });

  long int result = str_to<long int>(splits_vec[0].data());
  auto coefficients_str = splits_vec[1];

  std::vector<unsigned long> coefficients{};
  for (auto k : coefficients_str | std::ranges::views::split(' ')) {
    coefficients.push_back(str_to<unsigned long>(k.data()));
  }

  if constexpr (false) {
    print("eq: '{}'", equation);
    print_vec(
        "result: {}, coeffs: {}", coefficients,
        [](const auto &k) { return std::to_string(k); }, result);
  }

  return {result, coefficients};
}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<Equation> equations;
  auto lines = get_lines("input");
  std::transform(lines.begin(), lines.end(), std::back_inserter(equations),
                 [](const auto &k) { return parse_equation(k); });

  long int total_value = 0L;
  for (const auto &k : equations) {
    const auto solved_eq = solve_equation(k);
    if (solved_eq.has_value()) {
      print("solved equation: {}", solved_eq->to_string());
      total_value += solved_eq->result;
    } else {
      print_vec(
          "no solution for {} = [{}]", k.coefficients,
          [](auto &k) { return std::to_string(k); }, k.result);
    }
  }
  print("total value: {}", total_value);
  return 0;
}