#pragma once

#include <string>
#include <vector>

struct ParseException : public std::exception {
   const char * what () const throw () {
      return "Parsing exception";
   }
};


//---------------------------------[Class and Struct declarations]----------------------------------

struct Node{
    bool op;
    int value = 0;
    Node* left = nullptr;
    Node* right = nullptr;
};

struct Token{
    Token(bool o, int v):op(o), value(v){}
    Token():op(false), value(0){}
    bool op;
    int value;
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

class SyntaxTree{
    public:    
        SyntaxTree(std::string e);
        SyntaxTree();
        SyntaxTree(const SyntaxTree&);
        double evaluate();
        std::string str();
        void setExpr(std::string);
        bool operator==(const SyntaxTree& b){ return expr == b.expr; }
        bool operator!=(const SyntaxTree& b){ return expr != b.expr; }
        SyntaxTree& operator=(const SyntaxTree&);
        ~SyntaxTree();
    private:
        Node* root;
        std::string expr;
        bool isBuilt;
        TokenStack exprstack;
        void build(Node*);
        double evaluate(Node*);
        int roll(int, int);
};

double evaluate(std::string);
