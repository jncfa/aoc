#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstddef>
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
#include <stdexcept>
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

template <typename IterTA, typename IterTB, typename InitVal,
          typename BinaryOperation1, typename BinaryOperation2>
InitVal adj_diff_reduce(IterTA first, IterTA last, IterTB first2, IterTB last2,
                        InitVal init, BinaryOperation1 op1,
                        BinaryOperation2 op2) {

  for (; first != last && first2 != last2; ++first, ++first2) {
    init = op1(init, op2(*first, *first2));
  }
  return init;
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

template <typename T, typename S, typename... ArgsT>
void print_vec(const std::format_string<ArgsT..., std::string_view> fmt,
               const std::vector<T> &vec, ArgsT &&...args) {
  return print_vec(
      fmt, vec, [](const T &val) { return std::to_string(val); },
      std::forward<ArgsT>(args)...);
}

template <typename T, typename Functor, typename... ArgsT>
void print_vec(const std::format_string<ArgsT..., std::string_view> fmt,
               const std::set<T> &vec, Functor to_str, ArgsT &&...args) {
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
void print_vec(const std::set<T> &vec, Functor to_str) {
  return print_vec("[{}]", vec, to_str);
}

template <typename T> void print_vec(const std::set<T> &vec) {
  return print_vec(vec, [](const T &val) { return std::to_string(val); });
}

template <typename T, typename S, typename... ArgsT>
void print_vec(const std::format_string<ArgsT..., std::string_view> fmt,
               const std::set<T> &vec, ArgsT &&...args) {
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

std::unordered_map<unsigned long, unsigned long>
transform(const std::unordered_map<unsigned long, unsigned long> &line) {
  std::unordered_map<unsigned long, unsigned long> lineres{};

  for (const auto &k : line) {
    if (k.first == 0) {
      lineres[1] += k.second;
      continue;
    }

    auto digits =
        static_cast<unsigned long>(std::ceil(std::log10(k.first + 1)));
    if (digits % 2 == 0) {
      auto lsb = k.first % static_cast<unsigned long>(std::pow(10, digits / 2));
      auto msb = k.first / static_cast<unsigned long>(std::pow(10, digits / 2));
      lineres[lsb] += k.second;
      lineres[msb] += k.second;
      continue;
    }

    lineres[k.first * 2024] += k.second;
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

auto get_neighbours_with_same_value(
    const std::vector<std::string> &map, const Vec2d<int> &pos,
    const std::array<Vec2d<int>, 4> &directions) {

  std::vector<Vec2d<int>> neighbours{};
  for (const auto &dir : directions) {
    const auto neighbour_pos = pos + dir;
    if (neighbour_pos.x < 0 ||
        neighbour_pos.x >= static_cast<int>(map.size()) ||
        neighbour_pos.y < 0 ||
        neighbour_pos.y >=
            static_cast<int>(map[static_cast<unsigned long>(pos.x)].size())) {
      continue;
    }

    if (map[static_cast<unsigned long>(neighbour_pos.x)]
           [static_cast<unsigned long>(neighbour_pos.y)] ==
        map[static_cast<unsigned long>(pos.x)]
           [static_cast<unsigned long>(pos.y)]) {
      neighbours.push_back(neighbour_pos);
    }
  }
  return neighbours;
}

template <typename T, typename Y, typename... TArgs>
void emplace_or_push(std::unordered_map<T, std::vector<Y>, TArgs...> &map,
                     const T &key, const Y &value) {
  if (map.contains(key)) {
    map.at(key).push_back(value);
  } else {
    map.emplace(key, std::vector{value});
  }
}

template <typename T, typename Y, typename... TArgs>
void emplace_or_push(std::unordered_map<T, std::set<Y>, TArgs...> &map,
                     const T &key, const Y &value) {
  if (map.contains(key)) {
    map.at(key).insert(value);
  } else {
    map.emplace(key, std::set{value});
  }
}

template <typename K1, typename K2, typename V>
void emplace_or_push(
    std::unordered_map<K1, std::unordered_map<K2, std::set<V>>> &map,
    const K1 &key1, const K2 &key2, const V &value) {
  if (map.contains(key1)) {
    emplace_or_push(map.at(key1), key2, value);
  } else {
    map.emplace(key1,
                std::unordered_map<K2, std::set<V>>{{key2, std::set{value}}});
  }
}

template <typename K1, typename K2, typename V>
void emplace_or_push(
    std::unordered_map<K1, std::unordered_map<K2, std::vector<V>>> &map,
    const K1 &key1, const K2 &key2, const V &value) {
  if (map.contains(key1)) {
    emplace_or_push(map.at(key1), key2, value);
  } else {
    map.emplace(key1, std::unordered_map<K2, std::vector<V>>{
                          {key2, std::vector{value}}});
  }
}

enum class Directions { Up = 0, Down = 1, Left = 2, Right = 3 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  // perimeter per char = 4 - n of neighbours
  // area per char = 1
  constexpr auto directions = std::array{
      Vec2d{-1, 0},
      Vec2d{0, 1},
      Vec2d{1, 0},
      Vec2d{0, -1},
  };

  auto to_enum = [](const auto &dir) -> Directions {
    if (dir == Vec2d{-1, 0}) {
      return Directions::Up;
    }
    if (dir == Vec2d{0, 1}) {
      return Directions::Right;
    }
    if (dir == Vec2d{1, 0}) {
      return Directions::Down;
    }
    if (dir == Vec2d{0, -1}) {
      return Directions::Left;
    }
    throw std::runtime_error("invalid direction");
  };

  auto enum_to_str = [](const Directions &dir) -> std::string {
    switch (dir) {
    case Directions::Up:
      return "Up";
    case Directions::Right:
      return "Right";
    case Directions::Down:
      return "Down";
    case Directions::Left:
      return "Left";
    }
    throw std::runtime_error("invalid direction");
  };

  std::cout << "Hello World" << std::endl;

  auto lines = get_lines("input");
  std::vector<std::string> map{lines.begin(), lines.end()};

  unsigned long total_xmas = 0;
  std::vector<std::vector<bool>> visited{map.size(),
                                         std::vector<bool>(map[0].size())};

  auto is_valid = [&visited, &map](const auto &pos) -> bool {
    return pos.x >= 0 && pos.x < static_cast<int>(map.size()) && pos.y >= 0 &&
           pos.y <
               static_cast<int>(map[static_cast<unsigned long>(pos.x)].size());
  };

  for (auto i = 0UL; i < map.size(); ++i) {
    for (auto j = 0UL; j < map[i].size(); ++j) {
      if (visited[i][j]) {
        continue;
      }

      const auto start_pos =
          Vec2d<int>{static_cast<int>(i), static_cast<int>(j)};

      // position / direction of edge
      std::unordered_map<char,
                         std::unordered_map<Directions, std::set<Vec2d<int>>>>
          edge_map{};
      std::queue<Vec2d<int>> to_visit{};
      to_visit.push(start_pos);

      std::unordered_map<char, std::set<Vec2d<int>>> point_map{};

      while (!to_visit.empty()) {
        const auto current_pos = to_visit.front();
        to_visit.pop();

        visited[static_cast<unsigned long>(current_pos.x)]
               [static_cast<unsigned long>(current_pos.y)] = true;

        emplace_or_push(point_map,
                        map[static_cast<unsigned long>(current_pos.x)]
                           [static_cast<unsigned long>(current_pos.y)],
                        current_pos);

        std::vector<Vec2d<int>> neighbours{};
        if (current_pos.x == 2 && current_pos.y == 5) {
          print("current_pos: ({}, {})", current_pos.x, current_pos.y);
        }

        for (const auto &dir : directions) {
          const auto neighbour_pos = current_pos + dir;

          if (!is_valid(neighbour_pos)) {
            // found an edge!
            emplace_or_push(edge_map,
                            map[static_cast<unsigned long>(current_pos.x)]
                               [static_cast<unsigned long>(current_pos.y)],
                            to_enum(dir), current_pos);
            continue;
          }

          // ensure node was not visited
          if (map[static_cast<unsigned long>(neighbour_pos.x)]
                 [static_cast<unsigned long>(neighbour_pos.y)] ==
                  map[static_cast<unsigned long>(current_pos.x)]
                     [static_cast<unsigned long>(current_pos.y)] &&

              !visited[static_cast<unsigned long>(neighbour_pos.x)]
                      [static_cast<unsigned long>(neighbour_pos.y)]) {

            neighbours.push_back(neighbour_pos);
          } else {
            if (!visited[static_cast<unsigned long>(neighbour_pos.x)]
                        [static_cast<unsigned long>(neighbour_pos.y)]) {
              // found an edge!
              emplace_or_push(edge_map,
                              map[static_cast<unsigned long>(current_pos.x)]
                                 [static_cast<unsigned long>(current_pos.y)],
                              to_enum(dir), current_pos);
            }
          }
        }

        for (const auto &k : neighbours) {
          to_visit.push(k);
        }
      }

      for (const auto &edges_per_region : edge_map) {
        unsigned long sides_found = 0UL;

        for (const auto &p_edge_map : edges_per_region.second) {
          const auto dir = p_edge_map.first;
          const auto edges_for_dir = p_edge_map.second;

          // maps the fixed coordinate (e.g. if horizontal, it should be a vec
          // where all equal y=1 or y=2 for example obv) to the list of sides
          std::unordered_map<unsigned long, std::set<Vec2d<int>>>
              map_of_sides{};

          for (const auto &edge : edges_for_dir) {
            switch (dir) {
            case Directions::Up:
              [[fallthrough]];
            case Directions::Down:
              emplace_or_push(map_of_sides, static_cast<unsigned long>(edge.x),
                              edge);
              break;
            case Directions::Left:
              [[fallthrough]];
            case Directions::Right:
              emplace_or_push(map_of_sides, static_cast<unsigned long>(edge.y),
                              edge);
              break;
            default:
              throw std::runtime_error("Invalid direction");
            }
          }

          for (const auto &siders : map_of_sides) {
            auto list_of_edges =
                std::vector(siders.second.begin(), siders.second.end());

            std::sort(list_of_edges.begin(), list_of_edges.end(),
                      [&dir](const auto &a, const auto &b) {
                        switch (dir) {
                        case Directions::Up:
                          [[fallthrough]];
                        case Directions::Down:
                          return a.y < b.y;
                        case Directions::Right:
                          [[fallthrough]];
                        case Directions::Left:
                          return a.x < b.x;
                        }
                        throw std::runtime_error{"unreachable"};
                      });
            print_vec(
                "edge points from {} going {}: {}", list_of_edges,
                [](auto &abab) {
                  return std::format("({}, {})", abab.x, abab.y);
                },
                std::string{edges_per_region.first}, enum_to_str(dir));
            sides_found +=
                1 +
                adj_diff_reduce(list_of_edges.begin(), list_of_edges.end(),
                                std::next(list_of_edges.begin(), 1),
                                list_of_edges.end(), 0UL, std::plus<>(),
                                [&dir](auto &first, auto &second) {
                                  switch (dir) {
                                  case Directions::Up:
                                    [[fallthrough]];
                                  case Directions::Down:
                                    return std::min(second.x - first.x - 1, 1);
                                  case Directions::Left:
                                    [[fallthrough]];
                                  case Directions::Right:
                                    return std::min(second.y - first.y - 1, 1);
                                    break;
                                  }
                                  throw std::runtime_error("Invalid direction");
                                });
          }
        }

        print("{}: {}*{}", std::string{edges_per_region.first},
              point_map[edges_per_region.first].size(), sides_found);

        total_xmas += sides_found * point_map[edges_per_region.first].size();
      }
    }
  }

  print("result: {}", total_xmas);

  return 0;
}