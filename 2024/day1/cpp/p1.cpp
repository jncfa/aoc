#include <algorithm>
#include <cassert>
#include <charconv>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <fstream>
#include <ostream>
#include <ranges>
#include <utility>
#include <vector>
#include <numeric>
#include <execution>
#include <sstream>

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

auto split_str(std::string_view str, std::string_view substr){    
    auto splits = str | std::ranges::views::split( substr);

    std::vector<int> result;

    for(const auto& k: splits){
        result.push_back(str_to<int>(k.begin(), k.end()));
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

template<typename T>
void print_vec(const std::vector<T>& vec){
    return print_vec(vec,[](const T& val){ return std::to_string(val); });
}
int main(int argc, char** argv){
	std::cout << "Hello World" << std::endl;

    std::ifstream myfile{};
    myfile.open("input");

    std::string line;
    std::vector<int> l1;
    std::vector<int> l2;

    if (myfile.is_open()) {
        for (auto& k: get_lines(myfile)){
            print("{}", k.c_str());
            auto vec = split_str(line.c_str(), "   ");
            print_vec(vec);
            assert(vec.size() == 2);
            l1.push_back(vec[0]);
            l2.push_back(vec[1]);            
        }

        myfile.close();

        std::sort(l1.begin(), l1.end());
        std::sort(l2.begin(), l2.end());
        print_vec(l1, [](int a){return std::to_string(a); });
        print("l1 has {} and l2 has {}", l1.size(), l2.size());

        const auto res = std::transform_reduce(l1.begin(), l1.end(), l2.begin(), 
            0,
            std::plus<>(), // sum everything
            [](const auto& v1,const auto& v2){return abs(v1 - v2);}); // transform: calc value diff between both vecs

        print("result: {}", res);
    }
    
	return 0;
}