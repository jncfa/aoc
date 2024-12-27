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
#include <system_error>
#include <utility>
#include <vector>
#include <numeric>
#include <execution>
#include <regex>

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

auto get_mul_count(const std::string& str){
    static const std::regex mul_values(R"(mul\((\d{1,3}),(\d{1,3})\)|don't\(\)|do\(\))", std::regex_constants::optimize);

    unsigned long result = 0;

    bool is_enabled = true;

    for(auto it = std::sregex_iterator(str.begin(), str.end(), mul_values); it != std::sregex_iterator(); ++it){
        if (it->str().compare("do()") == 0){
            if (!is_enabled){
                print("mul is disabled, enabling... {}", it->str());
                is_enabled = true;
            }
            else {
                 print("mul is still enabled... {}", it->str());           
            }
        }
        else if (it->str().compare("don't()") == 0){
            if (is_enabled){
                print("mul is enabled, disabling... {}", it->str());
                is_enabled = false;
            }
            else{
                print("mul is still disabled.. {}", it->str());
            }
        }
        else{
            if (!is_enabled){
                print("mul is disabled, skipping this: {}", it->str());
                continue;
            }
            // get regex values
            auto base = it->str(), val1 = it->str(1), val2 = it->str(2);
            print("got value: {}, calc {} * {}", base, val1, val2);
            result += str_to<unsigned long>(val1) * str_to<unsigned long>(val2);
        }
    }

    return result;
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


std::string read_file( std::ifstream& stream ){
    std::ostringstream sstr;
    sstr << stream.rdbuf();
    return sstr.str();
}

auto get_lines(std::ifstream& stream){
    return LineWrapper(stream);
}

int main(int argc, char** argv){
	std::cout << "Hello World" << std::endl;

    std::ifstream myfile{};
    myfile.open("input");

    std::string line;
    
    if (myfile.is_open()) {
        print("got result {}", get_mul_count(read_file(myfile)));
        myfile.close();
    }
    
	return 0;
}