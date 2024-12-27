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

enum class Direction { Up = 0, Down = 1, Left = 2, Right = 3 };

template <typename T> struct Vec2d {
  T x = std::numeric_limits<T>::max();
  T y = std::numeric_limits<T>::max();

  Vec2d operator+(const Vec2d &other) const {
    return {x + other.x, y + other.y};
  }
  Vec2d operator-(const Vec2d &other) const {
    return {x - other.x, y - other.y};
  }

  auto operator<=>(const Vec2d &other) const = default;
};

Vec2d<int> get_velocity(const Direction &dir) {
  switch (dir) {
  case Direction::Up:
    return {-1, 0};
  case Direction::Down:
    return {1, 0};
  case Direction::Left:
    return {0, -1};
  case Direction::Right:
    return {0, 1};
  default:
    throw std::runtime_error("invalid direction");
  }
}

struct key_hash {
  std::size_t operator()(const Vec2d<unsigned long> &k) const {
    auto a = k.x, b = k.y;
    return a >= b ? a * a + a + b : a + b * b;
  }
};

struct Node : public std::enable_shared_from_this<Node> {
  unsigned int x, y, value;
  std::vector<std::shared_ptr<Node>> children;

  Node(unsigned int x_, unsigned int y_, unsigned int value_)
      : x(x_), y(y_), value(value_), children() {}

  void add_child(std::shared_ptr<Node> child) { children.push_back(child); }

  std::vector<std::shared_ptr<Node>> find_child(unsigned int value_) {
    std::vector<std::shared_ptr<Node>> node_list{};

    // breadth first search
    std::queue<std::shared_ptr<Node>> children_to_visit{};
    children_to_visit.push(this->shared_from_this());

    std::set<Vec2d<unsigned long>> visited{};

    while (!children_to_visit.empty()) {
      auto current = children_to_visit.front();
      children_to_visit.pop();
      visited.insert({current->x, current->y});
      // print("current node: ({}, {}, {})", current->x, current->y,
      //       current->value);
      if (current->value == value_) {
        //  print("found node with value {}", current->value);
        node_list.push_back(current);
      }

      for (auto &child : current->children) {
        if (visited.find({child->x, child->y}) == visited.end()) {
          children_to_visit.push(child);
        }
      }
    }

    return node_list;
  }

  static auto &get_nodes() {
    static std::unordered_map<Vec2d<unsigned long>, std::shared_ptr<Node>,
                              key_hash>
        nodes{};
    return nodes;
  }

  static std::vector<std::shared_ptr<Node>>
  get_nodes_with_value(unsigned long value) {
    std::vector<std::shared_ptr<Node>> node_list{};
    const auto &nodes = get_nodes();
    for (const auto &k : nodes) {
      if (k.second->value == value) {
        node_list.push_back(k.second);
      }
    }
    return node_list;
  }

  static std::optional<std::shared_ptr<Node>> get_node(unsigned long x,
                                                       unsigned long y) {
    const auto &nodes = get_nodes();
    if (auto it = nodes.find({x, y}); it != nodes.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  static auto get_or_create_node(unsigned long x, unsigned long y,
                                 unsigned long value) {
    auto node = get_node(x, y);
    if (node.has_value()) {
      return node.value();
    }

    auto &nodes = get_nodes();
    nodes[{x, y}] = std::make_shared<Node>(x, y, value);
    return nodes[{x, y}];
  }

  static void print_nodes() {
    const auto &nodes = get_nodes();
    for (const auto &k : nodes) {
      print_vec(
          "{} -> [{}]", k.second->children,
          [](const auto &l) {
            return std::format("({}, {}, {})", l->value, l->x, l->y);
          },
          std::format("({}, {}, {})", k.second->value, k.second->x,
                      k.second->y));
    }
  }
};

void create_nodes(const std::vector<std::vector<unsigned char>> &map) {
  constexpr auto directions = std::array{
      Vec2d{-1, 0},
      Vec2d{1, 0},
      Vec2d{0, -1},
      Vec2d{0, 1},
  };

  for (auto i = 0UL; i < map.size(); ++i) {
    for (auto j = 0UL; j < map[i].size(); ++j) {
      for (auto &vel : directions) {
        auto current_pos = Vec2d<int>{static_cast<int>(i), static_cast<int>(j)};
        auto next_pos = current_pos + vel;
        auto is_invalid =
            next_pos.x < 0 || next_pos.x >= static_cast<int>(map.size()) ||
            next_pos.y < 0 || next_pos.y >= static_cast<int>(map[i].size());

        if (is_invalid) {
          continue;
        }
        auto char_next_pos = map[static_cast<unsigned long>(next_pos.x)]
                                [static_cast<unsigned long>(next_pos.y)];
        auto char_current_pos = map[static_cast<unsigned long>(current_pos.x)]
                                   [static_cast<unsigned long>(current_pos.y)];

        if (char_next_pos > 9 || char_current_pos > 9) {
          continue;
        }

        auto diff = char_next_pos - char_current_pos;

        if (diff == 1) {
          Node::get_or_create_node(i, j,
                                   static_cast<unsigned long>(char_current_pos))
              ->add_child(Node::get_or_create_node(
                  static_cast<unsigned long>(next_pos.x),
                  static_cast<unsigned long>(next_pos.y),
                  static_cast<unsigned long>(char_next_pos)));
        }
      }
    }
  }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<std::vector<unsigned char>> map{};

  auto lines = get_lines("input");

  std::for_each(lines.begin(), lines.end(), [&map](auto &k) {
    std::vector<unsigned char> line{};
    std::transform(
        k.begin(), k.end(), std::back_inserter(line), [](const auto &s) {
          return s == '.' ? std::numeric_limits<unsigned char>::max() : s - '0';
        });
    map.push_back(std::vector<unsigned char>(line.begin(), line.end()));
  });

  create_nodes(map);
  Node::print_nodes();

  auto vec_to_search = Node::get_nodes_with_value(0);

  print("vec_to_search size: {}", vec_to_search.size());
  std::vector<unsigned long> vec{};
  std::transform(vec_to_search.begin(), vec_to_search.end(),
                 std::back_inserter(vec),
                 [](const auto &k) { return k->find_child(9).size(); });

  print_vec(vec);
  auto sum = std::accumulate(vec.begin(), vec.end(), 0UL);

  print("sum is {}", sum);
  return 0;
}