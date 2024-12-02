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
template <typename T>
std::vector<T> remove_at(const std::vector<T>& vec, unsigned long at){
    auto pruned_vec = std::vector<int>(vec);
    pruned_vec.erase(std::next(pruned_vec.begin(), std::clamp(at, 0UL, pruned_vec.size())));
    return pruned_vec;
}

bool is_safe(const auto& levels, bool found_bad_level_already = false){
    // get adjacent differences
    std::vector<int> adjacent_difference;
    std::adjacent_difference(levels.begin(), levels.end(), std::back_inserter(adjacent_difference));
    adjacent_difference.erase(adjacent_difference.begin());

    std::vector<bool> adj_dif_values;
    std::transform(adjacent_difference.begin(), adjacent_difference.end(), std::back_inserter(adj_dif_values), [](const int& val) -> bool {
        auto abs_val = std::abs(val);
        return abs_val >= 1 && abs_val <= 3;
    });

    std::vector<bool> swapped_sign;
    std::adjacent_difference(adjacent_difference.begin(), adjacent_difference.end(), std::back_inserter(swapped_sign), [](const int& valA, const int& valB) -> bool {
        return std::signbit(valA) == std::signbit(valB);
    });

    std::vector<bool> combined;
    std::transform(adj_dif_values.begin(), adj_dif_values.end(), swapped_sign.begin(), std::back_inserter(combined), [](bool a, bool b){return a && b;});

    // i am lazy so i will try the following
    //print("Got level");
    //print_vec(levels);
    //print_vec(adj_dif_values);
    //print_vec(swapped_sign);
    //print_vec(combined);

    // found adj value diff
    if (auto iter = std::find(combined.begin(), combined.end(), false); iter != combined.end()){
        if (found_bad_level_already){
            return false;
        }
        auto dist = std::clamp(static_cast<unsigned long>(std::distance(combined.begin(), iter)), 0UL, levels.size());
        
        // check surrounding vectors
        return is_safe(remove_at(levels, dist-1), true) || 
            is_safe(remove_at(levels, dist), true) || 
            is_safe(remove_at(levels, dist+1), true);
    }
    
    

    return true;
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
            if (is_safe(levels)){
                ++safe_report;
            }
        }

        myfile.close();

        print("result: {}", safe_report);
    }
    
	return 0;
}