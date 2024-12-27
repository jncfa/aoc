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

struct Node : public std::enable_shared_from_this<Node> {
  unsigned int page_number;
  std::vector<std::shared_ptr<Node>> children;

  Node(unsigned int page_number_) : page_number(page_number_), children(){};

  void add_child(std::shared_ptr<Node> child) { children.push_back(child); }

  std::shared_ptr<Node> find_child(unsigned int page_number) {

    for (const auto &k : children) {
      if (k->page_number == page_number) {
        return k;
      }
    }
    return nullptr;

    // breadth first search
    std::queue<std::shared_ptr<Node>> children{};
    children.push(this->shared_from_this());

    std::set<unsigned int> visited{};

    while (!children.empty()) {
      auto current = children.front();
      children.pop();
      visited.insert(current->page_number);

      if (current->page_number == page_number) {
        return current;
      }

      for (auto &child : current->children) {
        if (visited.find(child->page_number) == visited.end()) {
          children.push(child);
        }
      }
    }

    return nullptr;
  }

  static auto &get_nodes() {
    static std::map<unsigned int, std::shared_ptr<Node>> nodes{};
    return nodes;
  }

  static std::optional<std::shared_ptr<Node>>
  get_node(unsigned int page_number) {
    const auto &nodes = get_nodes();
    if (nodes.find(page_number) == nodes.end()) {
      return std::nullopt;
    }
    return nodes.at(page_number);
  }

  static auto get_or_create_node(unsigned int page_number) {
    auto &nodes = get_nodes();
    if (nodes.find(page_number) == nodes.end()) {
      nodes[page_number] = std::make_shared<Node>(page_number);
    }
    return nodes[page_number];
  }

  static void print_nodes() {
    const auto &nodes = get_nodes();
    for (const auto &k : nodes) {
      print_vec(
          "{} -> [{}]", k.second->children,
          [](const auto &k) { return std::to_string(k->page_number); },
          k.first);
    }
  }
};

std::tuple<unsigned int, unsigned int> parse_rule(const std::string &rule) {
  auto splits = rule | std::ranges::views::split('|');

  std::vector<unsigned int> result;

  for (const auto &k : splits) {
    result.push_back(str_to<unsigned int>(k.begin().base(), k.end().base()));
  }

  assert(result.size() == 2);
  return {result[0], result[1]};
}

enum class Direction { Up = '^', Down = 'v', Left = '<', Right = '>' };

constexpr std::tuple<int, int> get_velocity(Direction dir) {
  switch (dir) {
  case Direction::Up:
    return {-1, 0};
  case Direction::Down:
    return {1, 0};
  case Direction::Left:
    return {0, -1};
  case Direction::Right:
    return {0, 1};
  }
}

constexpr Direction turn_clockwise(Direction dir) {
  // basic clockwise
  switch (dir) {
  case Direction::Up:
    return Direction::Right;
  case Direction::Right:
    return Direction::Down;
  case Direction::Down:
    return Direction::Left;
  case Direction::Left:
    return Direction::Up;
  }
}

auto get_current_position(const std::vector<std::string> &world_map) {
  auto current_pos = std::tuple{0U, 0U};
  std::any_of(world_map.begin(), world_map.end(),
              [&current_pos, i = 0](const auto &k) mutable {
                print("checking position {}, {}", i, k);
                auto result = std::any_of(
                    k.begin(), k.end(),
                    [&current_pos, i, j = 0](const auto &l) mutable {
                      if (l != '#' && l != '.') {
                        print("found current position: ({}, {}), with char {}",
                              i, j, l);
                        current_pos = {i, j};
                        return true;
                      }
                      ++j;
                      return false;
                    });
                i++;
                return result;
              });
  return current_pos;
}

std::tuple<unsigned int, unsigned int>
get_next_position(std::tuple<unsigned int, unsigned int> current_pos,
                  std::tuple<int, int> velocity) {
  return {static_cast<unsigned int>(static_cast<int>(std::get<0>(current_pos)) +
                                    std::get<0>(velocity)),
          static_cast<unsigned int>(static_cast<int>(std::get<1>(current_pos)) +
                                    std::get<1>(velocity))};
}

bool update_tick(
    std::vector<std::string> &world_map,
    std::set<std::tuple<unsigned int, unsigned int>> &travelled_places,
    std::tuple<unsigned int, unsigned int> &current_pos) {

  const auto current_direction =
      Direction(world_map[std::get<0>(current_pos)][std::get<1>(current_pos)]);

  const auto velocity = get_velocity(current_direction);
  const auto next_pos = get_next_position(current_pos, velocity);

  if (std::get<0>(next_pos) >= world_map.size() ||
      std::get<1>(next_pos) >= world_map[0].size()) {
    print("reached end of map, stopping");
    return false;
  }

  if (world_map[std::get<0>(next_pos)][std::get<1>(next_pos)] == '#') {
    // will collide with wall, need to turn clockwise
    print("will crash into wall at ({},{})! turning", std::get<0>(next_pos),
          std::get<1>(next_pos));

    const auto next_direction = turn_clockwise(current_direction);
    world_map[std::get<0>(current_pos)][std::get<1>(current_pos)] =
        static_cast<char>(next_direction);
  } else {
    print("moving from ({},{}) to ({}, {})", std::get<0>(current_pos),
          std::get<1>(current_pos), std::get<0>(next_pos),
          std::get<1>(next_pos));
    world_map[std::get<0>(current_pos)][std::get<1>(current_pos)] = '.';
    world_map[std::get<0>(next_pos)][std::get<1>(next_pos)] =
        static_cast<char>(current_direction);
    travelled_places.insert(next_pos);
    current_pos = next_pos;
  }

  print("current map");
  // std::for_each(world_map.begin(), world_map.end(),
  //               [](auto &k) { print("{}", k); });
  return true;
}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;

  std::vector<std::string> world_map;
  std::set<std::tuple<unsigned int, unsigned int>> travelled_places;

  auto lines = get_lines("input");
  std::transform(lines.begin(), lines.end(), std::back_inserter(world_map),
                 [](const auto &k) { return k; });

  std::for_each(world_map.begin(), world_map.end(),
                [](auto &k) { print("{}", k); });

  auto current_pos = get_current_position(world_map);
  travelled_places.insert(current_pos);
  do {
    print("travelling....");
  } while (update_tick(world_map, travelled_places, current_pos));
  print("Done!");

  std::for_each(travelled_places.begin(), travelled_places.end(),
                [&world_map](auto &k) {
                  world_map[std::get<0>(k)][std::get<1>(k)] = 'X';
                });

  print("final map");
  std::for_each(world_map.begin(), world_map.end(),
                [](auto &k) { print("{}", k); });

  print("total travelled: {}", travelled_places.size());
  return 0;
}