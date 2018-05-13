#pragma once

#include <string>
#include <vector>

struct ParseException : public std::exception {
   const char * what () const throw () {
      return "Parsing exception";
   }
};


//---------------------------------[Class and Struct declarations]----------------------------------

//Nodes and tokens may be different types of elements: numbers, operators, or functions.
enum element {null_t, num_t, op_t, func_t};

struct Node{
    enum element type;
    std::string value = "";
    Node* left = nullptr;
    Node* right = nullptr;
};

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

class SyntaxTree{
    public:    
        SyntaxTree(std::string e);
        SyntaxTree();
        SyntaxTree(const SyntaxTree&);
        double evaluate();
        std::string str();
        void setExpr(std::string);
        bool validate(Node*);
        bool operator==(const SyntaxTree& b){ return expr == b.expr; }
        bool operator!=(const SyntaxTree& b){ return expr != b.expr; }
        SyntaxTree& operator=(const SyntaxTree&);
        ~SyntaxTree();
    private:
        Node* root;
        std::string expr;
        bool isBuilt;
        bool valid;
        TokenStack exprstack;
        void build(Node*);
        double evaluate(Node*);
        int roll(int, int);
};

double evaluate(std::string);
