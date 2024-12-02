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
#include <utility>
#include <vector>
#include <numeric>
#include <execution>

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

auto split_str(std::string_view str, std::string_view substr){    
    auto splits = str | std::ranges::views::split( substr);

    std::vector<int> result;

    for(const auto& k: splits){
        result.push_back(str_to<int>(k.begin(), k.end()));
    }

    return result;
}
int main(int argc, char** argv){
	std::cout << "Hello World" << std::endl;

    std::ifstream myfile{};
    myfile.open("input");

    std::string line;
    
    unsigned int safe_report = 0;

    if (myfile.is_open()) {
        while (std::getline(myfile, line)) {
            auto levels = split_str(line, " ");

            // get adjacent differences
            std::vector<int> adjacent_difference;
            std::adjacent_difference(levels.begin(), levels.end(), std::back_inserter(adjacent_difference));

            print_vec(adjacent_difference);
            // get sign of first value
            const auto sign_bit = std::signbit(*next(adjacent_difference.begin()));
            const auto is_safe = std::all_of(next(adjacent_difference.begin()), adjacent_difference.end(), [sign_bit](const int& val) -> bool {
                auto abs_val = std::abs(val);

                // same sign and value between 1 and 3
                return (std::signbit(val) == sign_bit && abs_val >= 1 && abs_val <= 3);
            });

            if (is_safe){
                ++safe_report;
            }
        }

        myfile.close();

        print("result: {}", safe_report);
    }
    
	return 0;
}