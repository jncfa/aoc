#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>
#include <numeric>
#include <execution>
#include <regex>
#include <optional>


template <typename T, typename ...TArgs>
T str_to(const std::string& str, TArgs&&... args){
    T result{};
    auto [ptr, ec] = std::from_chars(str.begin().base(), str.end().base(), result, std::forward<TArgs>(args)...);
    if (ec != std::errc()){
        throw std::system_error(std::make_error_code(ec));
    }
    return result;
} 

template <typename T, typename TIter, typename ...TArgs>
T str_to(TIter iter_begin, TIter iter_end, TArgs&&... args){
    T temp;
    std::from_chars(iter_begin, iter_end, temp, std::forward<TArgs>(args)...);
    return temp;
} 

template <typename... ArgsT>
void print(const std::format_string<ArgsT...> fmt, ArgsT&&... args) {
    std::cout << std::format(fmt, std::forward<ArgsT>(args)...) << std::endl;
}

template <typename T, typename Functor>
void print_vec(const std::vector<T>& vec, Functor to_str) {
    const auto join_vec = [](const std::string& val1, const std::string& val2) -> std::string {
        if (val1 != "") [[likely]] {
            return std::format("{}, {}", val1, val2);
        }
        else{
            return val2;
        }
    };
    const auto final = std::transform_reduce(vec.begin(), vec.end(), std::string(), join_vec, [&to_str](const T& val) -> std::string {return to_str(val);});
    print("[{}]", final);
}

template<typename T>
void print_vec(const std::vector<T>& vec){
    return print_vec(vec,[](const T& val){ return std::to_string(val); });
}

class Line : public std::string {};

std::istream &operator>>(std::istream &is, Line &l)
{
    std::getline(is, l);
    return is;
}

struct LineWrapper {
    std::reference_wrapper<std::ifstream> ref;
    LineWrapper(std::ifstream& stream_): ref(stream_) {};
    auto begin(){ return std::istream_iterator<Line>(ref.get()); }
    auto end(){ return  std::istream_iterator<Line>(); }
};

struct OwningLineWrapper {
    std::ifstream stream;

    OwningLineWrapper(std::ifstream&& stream_): stream(std::move(stream_)) {};
    auto begin(){ return std::istream_iterator<Line>(stream); }
    auto end(){ return  std::istream_iterator<Line>(); }
};


std::string read_file( std::ifstream& stream ){
    std::ostringstream sstr;
    sstr << stream.rdbuf();
    return sstr.str();
}

auto get_lines(std::ifstream& stream){
    return LineWrapper(stream);
}

auto get_lines(const std::string_view& file){
    std::ifstream myfile{};
    myfile.open(file.data());

    if (myfile.is_open()) {
        return OwningLineWrapper(std::move(myfile));
    }
    throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
}

std::tuple<int, std::vector<std::tuple<unsigned long, unsigned long>>> get_neighbours(const std::vector<std::string>& lines, unsigned long x, unsigned long y){

    // search forward for XMAS
    // search backward for XMAS
    // search diagonally for XMAS
    // search final diagonal for XMAS
    
    const auto directions = std::array<std::tuple<int, int>, 8>({
        {0, 1}, // search forward for XMAS
        {0, -1}, // search backward for XMAS
        {1, 0}, // search up for XMAS
        {-1, 0}, // search down for XMAS
        // diagonals
        {1, 1},
        {-1, -1},
        {1, -1},
        {-1, 1}
    });

    constexpr auto XMAS = std::string_view("XMAS");

    std::vector<std::tuple<unsigned long, unsigned long>> neighbours;
    int xmas_count = 0;
    for(const auto& dir: directions){
        const auto x_dir = std::get<0>(dir);
        const auto y_dir = std::get<1>(dir);

        // check if neighbour is valid
        for(auto i = 1UL; static_cast<signed long>(x + i*x_dir) < static_cast<signed long>(lines.size()) &&
            static_cast<signed long>(x + i*x_dir) >= 0 &&
            static_cast<signed long>(y + i*y_dir) < static_cast<signed long>(lines[x + i*x_dir].size()) &&
            static_cast<signed long>(y + i*y_dir) >= 0; ++i){

            if (lines[x + i*x_dir][y + i*y_dir] != XMAS[i]){
                break;
            }
            else if (i == XMAS.size() - 1) {
                neighbours.push_back({x, y});
                neighbours.push_back({x + x_dir, y + y_dir});
                neighbours.push_back({x + 2*x_dir, y + 2*y_dir});
                neighbours.push_back({x + 3*x_dir, y + 3*y_dir});
                ++xmas_count;
            }
        }
    }
    return {xmas_count, neighbours};
}

int main(int argc, char** argv){
	std::cout << "Hello World" << std::endl;

    std::vector<std::string> lines;
    std::vector<std::string> masked_lines;
    
    
    for (auto& k: get_lines("input")){
        print("{}", k.c_str());
        lines.push_back(k);
        masked_lines.push_back(std::string(k.size(), '.'));
    }

    auto total_xmas = 0;

    for(auto i = 0UL; i < lines.size(); ++i){
        for (auto j = 0UL; j < lines[i].size(); ++j) {
            const auto current_char = lines[i][j];

            if (current_char == 'X'){
                const auto neighbours = get_neighbours(lines, i, j);
                const auto xmas_count = std::get<0>(neighbours);
                if (xmas_count == 0){
                    print("no neighbours for {}", current_char);
                    continue;
                }
                total_xmas += xmas_count;

                // for each neighbour, mark points in masked lines with the correct char
                for(const auto& k: std::get<1>(neighbours)){
                    masked_lines[std::get<0>(k)][std::get<1>(k)] = lines[std::get<0>(k)][std::get<1>(k)];
                }                
            }
        }
    }

    std::for_each(masked_lines.begin(), masked_lines.end(), [](const auto& k){print("{}", k.c_str());});
    print("total xmas: {}", total_xmas);

	return 0;
}