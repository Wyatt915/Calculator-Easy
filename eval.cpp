#include "eval.hpp"
#include "parse.hpp"
#include "utils.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <map>
//#include <pair>
#include <stdexcept>
#include <string>
#include <vector>

struct Node{
    enum element type;
    std::string value = "";
    Node* left = nullptr;
    Node* right = nullptr;
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

//Operators and Functions

std::map<std::string, std::string> constants;
constants.insert(std::make_pair("phi", "1.61803398874989484820"));
constants.insert(std::make_pair("pi",  "3.14159265358979323846"));
constants.insert(std::make_pair("e",   "2.71828182845904523536"));

//----------------------------------------[Tree operations]----------------------------------------

void freeTree(Node* n){
    if(n == nullptr) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

Node* copyTree(Node* n){
    if(n == nullptr) return nullptr;
    Node* clone = new Node;
    clone->type = n->type;
    clone->value = n->value;
    clone->left = copyTree(n->left);
    clone->right = copyTree(n->right);
    return clone;
}

//-------------------------------[SyntaxTree Constructors/Destructor]-------------------------------

SyntaxTree::SyntaxTree(std::string e):expr(e){
    root = new Node;
    tolower(expr);
    try{
        exprstack = infix_to_postfix(tokenize(expr));
    }
    catch(std::runtime_error& p){
        throw p;
    }
    catch(...){
        throw std::runtime_error("An Error occurred converting to postfix.");
    }
    build(root);
    isBuilt = true;
    valid = validate(root);
    if(!valid){
        throw std::runtime_error("Malformed syntax.");
    }
}

SyntaxTree::SyntaxTree(){
    root = nullptr;
    isBuilt = false;
}

SyntaxTree::SyntaxTree(const SyntaxTree& other){
    root = copyTree(other.root);
    isBuilt = other.isBuilt;
    expr = other.expr;
}

SyntaxTree& SyntaxTree::operator=(const SyntaxTree& other){
    root = copyTree(other.root);
    isBuilt = other.isBuilt;
    expr = other.expr;
    return *this;
}

SyntaxTree::~SyntaxTree(){
    freeTree(root);
}

//----------------------------------[SyntaxTree Member Functions]----------------------------------

void SyntaxTree::setExpr(std::string e){
    expr = e;
    freeTree(root);
    root = new Node;
    tolower(expr);
    try{
        exprstack = infix_to_postfix(tokenize(expr));
    }
    catch(std::runtime_error& p){
        throw p;
    }
    build(root);
    isBuilt = true;
    valid = validate(root);
    if(!valid){
        throw std::runtime_error("Malformed syntax.");
    }
}

void SyntaxTree::build(Node* n){
    if(exprstack.size() == 0) return;
    Token t = exprstack.pop();
    n->type = t.type;
    n->value = t.value;
    if(n->type == op_t){
        n->right = new Node;
        build(n->right);
        if(exprstack.size() == 0) return;
        n->left = new Node;
        build(n->left);
    }
    else{
        return;
    }
}

int SyntaxTree::roll(int numdice, int numsides){
    int total = 0;
    for(int i = 0; i < numdice; i++){
        total += rand() % numsides + 1;
    }
    return total;
}

double SyntaxTree::evaluate(){
    return evaluate(root);
}

double SyntaxTree::evaluate(Node* n){
    if(n->type == num_t) return std::stod(n->value);
    char c = n->value[0];
    double lval, rval;

    lval = evaluate(n->left);
    rval = evaluate(n->right);
    
    double result;
    switch (c){
        case 'd':
            result = roll(lval, rval);
            break;
        case '+':
            result = lval + rval;
            break;
        case '-':
            result = lval - rval;
            break;
        case 'x':
        case '*':
            result = lval * rval;
            break;
        case '/':
            result = lval / rval;
            break;
        case '@':
            result = lval - rval;
            break;
        case '^':
            result = pow(lval, rval);
            break;            
    }
    return result;
}

bool SyntaxTree::validate(Node* n){
   if(n == nullptr) return true;
   //If the current node is an operator and either child is null,
   //that means the tree is invalid.
   if(n->type == op_t){
       if(n->left == nullptr || n->right == nullptr){
           return false;
       }
   }

   //If the current node is a number, and either child is not null,
   //then the tree is invalid.
   if(n->type == num_t){
       if(!(n->left == nullptr || n->right == nullptr)){
           return false;
       }
   }

   bool l = validate(n->left);
   bool r = validate(n->right);

   return l && r;
}


std::string SyntaxTree::str(){
    return expr;
}

double evaluate(std::string expr){
    double result;
    try{
        SyntaxTree s(expr);
        result = s.evaluate();
    }
    catch(std::runtime_error& e){
        throw e;
    }
    return result;
}
