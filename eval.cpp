#include "eval.hpp"
#include "func.hpp"
#include "parse.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <iomanip>

#define LFT 0
#define RGT 1

struct Node{
    std::string type;
    std::string value = "";
    Node* left = nullptr;
    Node* right = nullptr;
    size_t numchildren = 0;
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
};

//------------------------------------[Operators and Functions]-------------------------------------


static std::map<std::string, std::function<double(double, double)> > ops = {
    { "+", add_f },
    { "-", sub_f },
    { "*", mul_f },
    { "/", div_f },
    { "^", pow_f },
    { "d", rol_f }
};

static std::map<std::string, std::function<double(double)> > funcs = {
    { "sin", sin }
};

//----------------------------------------[Tree operations]----------------------------------------

void freeTree(Node* n){
    if(n == nullptr) return;
    freeTree(n->right);
    freeTree(n->left);
    delete n;
}

Node* copyTree(Node* n){
    if(n == nullptr) return nullptr;
    Node* clone = new Node;
    clone->type = n->type;
    clone->value = n->value;
    clone->right = copyTree(n->right);
    clone->left = copyTree(n->left);
    return clone;
}

//--------------------------------------[Pretty print a tree]---------------------------------------
//Using graphviz dot program
void prettyprint(Node* p)
{
    if(!p) return;
    if(p->left){
        std::cout << "\t\"" << p->value << "\" -> \"" << p->left->value << "\";\n";
        prettyprint(p->left);
    }
    if(p->right){
        std::cout << "\t\"" << p->value << "\" -> \"" << p->right->value << "\";\n";
        prettyprint(p->right);
    }
}

void ppwrapper(Node* p){
    std::cout << "digraph G{ \n";
    prettyprint(p);
    std::cout << "}\n";
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
    //ppwrapper(root);
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

//----------------------------------[SyntaxTree Member Functions]-----------------------------------

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
    prettyprint(root);
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
    if(n->type == OP_T){
        n->numchildren = 2;
            //Build the righthand side of the tree first!!
            n->right = new Node;
            n->left = new Node;
            build(n->right);
            build(n->left);
    }
    if(n->type == NUMBER_T){
        n->numchildren = 0;
        n->left = nullptr;
        n->right = nullptr;
    }
    if(n->type == FUNC_T){
        n->numchildren = 1;
        n->left = new Node;
        build(n->left);
        n->right = nullptr;
    }
}

bool SyntaxTree::validate(Node* n){
   if(n == nullptr) return true;
   //If the current node is an operator and either child is null,
   //that means the tree is invalid.
   if(n->type == OP_T){
       if(n->left == nullptr || n->right  == nullptr){ return false; }
   }

   //If the current node is a number, and either child is not null,
   //then the tree is invalid.
   if(n->type == NUMBER_T){
       if(n->left != nullptr || n->right != nullptr){ return false; }
   }

   bool l = validate(n->left);
   bool r = validate(n->right);
   return l&&r;
}


std::string SyntaxTree::str(){
    return expr;
}

//------------------------------------[Evaluate the Expression]-------------------------------------

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

double SyntaxTree::evaluate(){
    return evaluate(root);
}

double SyntaxTree::evaluate(Node* n){
    if(n->type == NUMBER_T)
        return std::stod(n->value);
    else if(n->type == OP_T)
        return ops[n->value](evaluate(n->left), evaluate(n->right));
    else if(n->type == FUNC_T)
        return funcs[n->value](evaluate(n->left));
    else
        throw std::runtime_error("Could not evaluate expression.");
}
