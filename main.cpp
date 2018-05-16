#include "eval.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib> //rand
#include <ctime>   //time
#include <unistd.h>//getpid

//Robert Jenkins' 96 bit Mix Function
//https://stackoverflow.com/a/323302
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

int main(int argc, char** argv){
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);
    std::string s;
    if (argc > 1){
        for(int i = 1; i < argc; i++){
            s += argv[i];
        }
    }
    else{
        std::cin >> s;
    }

    try{
        std::cout << evaluate(s) << '\n';
    }
    catch(std::runtime_error& e){
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch(...){
        std::cerr << "An unknown error occured.\n";
    }
    return 0;
}
