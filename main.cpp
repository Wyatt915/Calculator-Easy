#include "parse.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv){
    std::string s;
    std::cin >> s;
    std::cout << evaluate(s);
    return 0;
}
