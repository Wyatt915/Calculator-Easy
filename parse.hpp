#pragma once
#include "utils.hpp"
#include <string>
#include <vector>

struct ParseException : public std::exception {
   const char * what () const throw () {
      return "Parsing exception";
   }
};

//---------------------------------[Class and Struct declarations]----------------------------------

struct Token{
    Token(enum element t, std::string v):type(t), value(v){}
    Token():type(num_t), value(""){}
    enum element type;
    std::string value;
};

class TokenStack{
    public:
        TokenStack();
        TokenStack(const TokenStack&);
        TokenStack& operator=(const TokenStack&);
        ~TokenStack();
        void push(Token);
        Token pop();
        Token peek();
        int size();
        bool is_empty();
        bool not_empty();
    private:
        Token* data;
        int numTokens;
        int top;
};

std::vector<Token> tokenize(std::string);
TokenStack infix_to_postfix(std::vector<Token> list);
