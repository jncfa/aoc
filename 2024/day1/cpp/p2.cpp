#include <algorithm>
#include <cassert>
#include <format>
#include <functional>
#include <iostream>
#include <fstream>
#include <ostream>
#include <ranges>
#include <string>
#include <vector>
#include <numeric>
#include <execution>

template <typename T>
std::string to_str(T a){ return std::to_string(a); }
template <typename T, typename TIter, typename ...TArgs>
T str_to(TIter iter_begin, TIter iter_end, TArgs&&... args){
    T temp{};
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

auto split_str(std::string_view str, std::string_view substr){    
    auto splits = str | std::ranges::views::split( substr);

    std::vector<int> result;

    for(const auto& k: splits){
        result.push_back(str_to<long>(k.begin(), k.end()));
    }
    print("origin {}", str);
    print_vec(result, to_str<int>);
    return result;
}

int main(int argc, char** argv){
	std::cout << "Hello World" << std::endl;

    std::ifstream myfile{};
    myfile.open("input");

    std::string line;


    std::vector<int> l1;
    std::vector<int> l2;  
    std::cout << "Before hell" << std::endl;
    if (myfile.is_open()) {
        while (std::getline(myfile, line)) {
            auto vec = split_str(line, "  ");
            assert(vec.size() == 2);
            l1.push_back(vec[0]);
            l2.push_back(vec[1]);
        }

        myfile.close();

        std::sort(l1.begin(), l1.end());
        std::sort(l2.begin(), l2.end());
        //print_vec(l1, [](int a){return std::to_string(a); });
        //print_vec(l2, [](int a){return std::to_string(a); });
        print("l1 has {} and l2 has {}", l1.size(), l2.size());

        const auto res = std::transform_reduce(l1.begin(), l1.end(), 0, std::plus<>(), [&l2](const auto& v1){return v1 * std::count(l2.begin(), l2.end(), v1);});

        print("result: {}", res);
    }
    
	return 0;
}