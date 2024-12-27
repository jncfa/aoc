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

std::vector<unsigned int> parse_update(const std::string &update) {
  auto splits = update | std::ranges::views::split(',');

  std::vector<unsigned int> result;

  for (const auto &k : splits) {
    result.push_back(str_to<unsigned int>(k.begin().base(), k.end().base()));
  }

  return result;
}

bool is_valid_update(const std::vector<unsigned int> &page_numbers) {
  auto bad_update = false;
  for (auto i = 0UL; i < page_numbers.size() && !bad_update; ++i) {
    // if node doesn't exist, skip
    if (!Node::get_node(page_numbers[i])) {
      continue;
    }
    auto i_node = Node::get_node(page_numbers[i]).value();

    for (auto j = i + 1; j < page_numbers.size() && !bad_update; ++j) {
      if (!Node::get_node(page_numbers[j])) {
        continue;
      }
      auto j_node = Node::get_node(page_numbers[j]).value();
      // if i is a child of j, then the rule is being broken
      if (j_node->find_child(i_node->page_number) != nullptr) {
        bad_update = true;
        break;
      }
    }
  }
  return !bad_update;
}

void fix_update_rule(std::vector<unsigned int> &page_numbers) {
  while (!is_valid_update(page_numbers)) {
    for (auto i = 0UL; i < page_numbers.size(); ++i) {
      // if node doesn't exist, skip
      if (!Node::get_node(page_numbers[i])) {
        continue;
      }
      auto i_node = Node::get_node(page_numbers[i]).value();

      for (auto j = i + 1; j < page_numbers.size(); ++j) {
        if (!Node::get_node(page_numbers[j])) {
          continue;
        }
        auto j_node = Node::get_node(page_numbers[j]).value();
        // if i is a child of j, then the rule is being broken
        if (j_node->find_child(i_node->page_number) != nullptr) {
          std::swap(page_numbers.at(i), page_numbers.at(j));
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  std::cout << "Hello World" << std::endl;

  bool input_is_rules = true;
  std::vector<std::vector<unsigned int>> valid_updates;

  for (auto &k : get_lines("input")) {
    // updates are next
    if (k.empty()) {
      input_is_rules = false;
      continue;
    }

    if (input_is_rules) {
      const auto [page_number, child_page_number] = parse_rule(k);
      auto node = Node::get_or_create_node(page_number);
      node->add_child(Node::get_or_create_node(child_page_number));
    } else {
      // Node::print_nodes();
      auto page_numbers = parse_update(k);
      auto bad_update = false;
      for (auto i = 0UL; i < page_numbers.size() && !bad_update; ++i) {
        // if node doesn't exist, skip
        if (!Node::get_node(page_numbers[i])) {
          continue;
        }
        auto i_node = Node::get_node(page_numbers[i]).value();

        for (auto j = i + 1; j < page_numbers.size() && !bad_update; ++j) {
          if (!Node::get_node(page_numbers[j])) {
            continue;
          }
          auto j_node = Node::get_node(page_numbers[j]).value();
          // if i is a child of j, then the rule is being broken
          if (j_node->find_child(i_node->page_number) != nullptr) {
            bad_update = true;
            break;
          }
        }
      }

      if (bad_update) {
        print_vec("invalid update, needs to be fixed: {}", page_numbers);

        fix_update_rule(page_numbers);

        print_vec("fixed update: {}", page_numbers);
        valid_updates.push_back(page_numbers);
      }
      // valid_updates.push_back(page_numbers);
    }
  }

  const auto sum_of_mid_values = std::transform_reduce(
      valid_updates.begin(), valid_updates.end(), 0U, std::plus<>(),
      [](const auto &k) { return k[(k.size() / 2)]; });
  for (const auto &k : valid_updates) {
    print_vec(k);
  }

  print("sum of mid values: {}", sum_of_mid_values);

  return 0;
}