#include "eval.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <cstdlib> //rand
#include <ctime>   //time
#include <unistd.h>//getpid, getopt

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
    int c;
    bool fmode = false;
    bool imode = false;
    opterr = 0; // don't print error messages.
    while ((c = getopt(argc, argv, "fhi")) != -1) {
        switch (c) {
            case 'f':
                std::cout << "financial-mode\n";
                fmode = true;
                break;
            case 'h':
                std::cout << "this is the placeholder help text.\n";
                return 0;
            case 'i':
                std::cout << "interactive-mode\n";
                imode = true;
                break;
            case '?':
            default:
                std::cerr << "Unknown option: " << char(c) << '\n';
                return 1;
        }
    }
    
    // Needed for dice rolling
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);

    if (!(imode || fmode)) {
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
            #ifdef DRAWGRAPH
            std::cerr << evaluate(s) << '\n';
            #else
            std::cout << std::setprecision(15)  << evaluate(s) << '\n';
            #endif
        }
        catch(std::runtime_error& e){
            std::cerr << e.what() << std::endl;
            return 1;
        }
        catch(...){
            std::cerr << "An unknown error occured.\n";
            return 2;
        }
    }
    return 0;
}
