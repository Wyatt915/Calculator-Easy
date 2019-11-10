#include "eval.hpp"

#include <cstdlib> //rand
#include <ctime>   //time
#include <curses.h>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unistd.h>//getpid, getopt

#include <readline/readline.h>
#include <readline/history.h>

#define TXTRED      "\x1b[31;1m"
#define TXTBLD      "\x1b[1m"
#define TXTDEF      "\x1b[0m"

std::vector<double> globalHistory;

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
    // Needed for dice rolling
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);

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

    if (!(imode || fmode)) {
        std::string s;
        // If there are commandline arguments, ignore stdin.
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
    else if(imode){
        unsigned int counter = 0;
        double res;
        std::string expr;
        while(true){
            expr = readline(">>> ");
            if(expr == "q") break;
            try{
                res = evaluate(expr);
                globalHistory.push_back(res);
                std::cout << "[" << counter << "]\t" <<  std::setprecision(15) << res << "\n";
                counter++;
                add_history(expr.c_str());
            }
            catch(std::runtime_error& e){
                std::cerr << e.what() << std::endl;
            }
            catch(...){
                std::cerr << "An unknown error occured.\n";
            }
        }
    }
    else if (fmode){
        int c;
        std::string prev = "0";
        std::string cur = "0";
        std::string expr = "0";
        int count = 0;
        int y = 0; int x = 0;
        int decimal = -1;
        cbreak();
        noecho();
        timeout(-1);
        initscr();
        keypad(stdscr, TRUE);
        std::stringstream ss;
        const int left = 5;
        const int digits = 12 + left;
        move(0, left);
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        while ((c = getchar()) != 'q'){
            switch (c){
                case '-':
                    getyx(stdscr, y, x);
                    move(y, 0);
                    chgat(20, A_NORMAL, COLOR_RED, NULL);
                    attron(COLOR_PAIR(1));
                    move(y, x);
                case '+':
                    move(y, left);
                    clrtoeol();
                    if(decimal >=0){
                        mvaddstr(y, digits-decimal, cur.c_str());
                    }
                    else {
                        mvaddstr(y, digits-cur.length(), cur.c_str());
                    }

                    addch(c);
                    expr += c; expr += cur;
                    prev = cur;
                    cur = "";
                    move(y, 0);
                    ss << '[' << ++count << ']';
                    addstr(ss.str().c_str());
                    ss.str(std::string());
                    move(++y, left);
                    attrset(0);
                    decimal = -1;
                    break;
                case KEY_ENTER:
                case 10:
                case 13:
                    mvaddstr(y, 0, "---------------");
                    ss << evaluate(expr) << std::endl;
                    decimal = ss.str().find('.');
                    attron(A_STANDOUT);
                    mvaddch(++y, 0, '*');
                    mvaddstr(y, digits-decimal, ss.str().c_str());
                    attroff(A_STANDOUT);
                    ss.str(std::string());
                    move(++y, left);
                    break;
                case '.':
                    decimal = cur.length();
                default:
                    addch(c);
                    cur += c;
                    break;
            }
            refresh();
        }
        endwin();
    }
    return 0;
}
