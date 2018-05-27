#pragma once
#include <string>
#include <vector>
#include <iostream>

struct ParseException : public std::exception {
   const char * what () const throw () {
      return "Parsing exception";
   }
};

//---------------------------------[Class and Struct declarations]----------------------------------

struct Token{
    Token(std::string t, std::string v):type(t), value(v){}
    Token():type("null"), value(""){}
    std::string type;
    std::string value;
};

class TokenStack{
    public:
        TokenStack();
        TokenStack(const TokenStack&);
        TokenStack& operator=(const TokenStack&);
        friend std::ostream& operator<< (std::ostream&, const TokenStack&);
        ~TokenStack();
        void push(Token);
        Token pop();
        Token peek();
        size_t size();
        bool is_empty();
        bool not_empty();
    private:
        Token* data;
        size_t numTokens;
        int top;
};


std::vector<Token> tokenize(std::string);
TokenStack infix_to_postfix(std::vector<Token> list);
double evaluate(std::string);
