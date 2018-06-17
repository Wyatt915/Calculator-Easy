#include "eval.hpp"
#include "func.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <ctype.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>

#define LFT 0
#define RGT 1

extern std::vector<double> globalHistory;

//------------------------------------[Operators and Functions]-------------------------------------


static std::map<std::string, std::function<double(double, double)> > ops = {
    { "+", add_f },
    { "-", sub_f },
    { "*", mul_f },
    { "/", div_f },
    { "%", mod_f },
    { "^", pow_f },
    { "d", rol_f }
};

static std::map<std::string, std::function<double(double)> > funcs = {
    { "sin", sin },
    { "cos", cos },
    { "tan", tan },
    { "csc", csc },
    { "sec", sec },
    { "cot", cot },
    { "exp", exp },
    { "log", log },
    { "sqrt", sqrt },
    { "!", fact }
};

static std::map<std::string, std::string> constants = {
    { "phi", "1.61803398874989484820" },
    { "pi",  "3.14159265358979323846" },
    { "e",   "2.71828182845904523536" }
};

static std::map<std::string, int> precedence = {
    { "+", 0 },
    { "-", 0 },
    { "*", 1 },
    { "/", 1 },
    { "%", 1 },
    { "d", 2 },
    { "^", 2 }
};

//------------------------------------------------[]------------------------------------------------
//----------------------------------------[Parsing routines]----------------------------------------
//------------------------------------------------[]------------------------------------------------


//-------------------------------------------[TokenStack]-------------------------------------------

TokenStack::TokenStack():numTokens(0), top(-1){
    data = new Token[1024];
}

TokenStack::TokenStack(const TokenStack& other){
    numTokens = other.numTokens;
    top = other.top;
    data = new Token[1024];
    std::copy(other.data, other.data + numTokens, data);
}

TokenStack& TokenStack::operator=(const TokenStack& other){
    delete[] data; // This is guaranteed to be allocated beforehand.
    numTokens = other.numTokens;
    top = other.top;
    data = new Token[1024];
    std::copy(other.data, other.data + numTokens, data);
    return *this;
}

TokenStack::~TokenStack(){
    delete[] data;
}

size_t TokenStack::size(){
    return numTokens;
}

bool TokenStack::is_empty(){
    return numTokens <= 0;
}

bool TokenStack::not_empty(){
    return numTokens > 0;
}

Token TokenStack::peek(){
    if (numTokens > 0){
        return data[top];
    }
    else{
        return Token(NULL_T, "");
    }
}

void TokenStack::push(Token t){
    top++;
    data[top] = t;
    numTokens++;
}

Token TokenStack::pop(){
    if (numTokens <= 0){
        throw std::runtime_error("Attempt to pop empty stack.");
    }
    Token out = data[top];
    top--;
    numTokens--;
    return out;
}

std::ostream& operator<<(std::ostream& os, const TokenStack& t){
    os << "-----------------\n";
    for(size_t i = 0; i < t.numTokens; i++){
        os << i << '\t' <<  t.data[i].value << "\n";
    }
    os << "-----------------\n";
    return os;
}

//------------------------------------------[Tokenization]------------------------------------------

std::vector<Token> tokenize(std::string s){
    auto first = std::begin(s);
    auto last = std::begin(s);
    std::vector<Token> out;
    Token prevToken;
    while(first != std::end(s) || *first != '\0'){
        std::string temp(first, first+1);
        // Numbers
        if (isdigit(*first) || *first == '.'){
            last = first;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            prevToken = Token(NUMBER_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }
        
        else if (*first == '['){
            last = first++;
            do{
                last++;
            } while(last != std::end(s) && *last != ']');
            
            if (*last != ']'){
                std::cerr << *last << '\n';
                throw std::runtime_error("Missing ']'.");
            }
            double d = evaluate(std::string(first, last));
            if(d < 0){
                throw std::runtime_error("Error: cannot travel back in time.");
            }
            unsigned int val = d;
            if(val >= globalHistory.size()){
                throw std::runtime_error("Error: Cannot see the future.");
            }
            std::stringstream ss;
            ss << globalHistory[val];
            prevToken = Token(NUMBER_T, ss.str());
            out.push_back(prevToken);
            first=++last;
        }

        // Handle the case of negative numbers. A '-' char indicates a
        // negative number if it comes at the beginning of the string,
        // or if it follows a previous operator.
        else if (*first == '-' && prevToken.type != NUMBER_T && prevToken.value != ")"){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            prevToken = Token(NUMBER_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }
        else if (ops.count(temp) || temp == "(" || temp == ")"){
            // Handle implicit lval argument of 1 on dice rolls when not explicitly stated
            if (*first == 'd' && prevToken.type != NUMBER_T){
                prevToken = Token(NUMBER_T, "1");
                out.push_back(prevToken);
            }    
            // Handle implicit multiplication of parentheticals
            if (*first == '(' && prevToken.type != NULL_T && (out.back().value == ")" || prevToken.type == NUMBER_T)){
                prevToken = Token(OP_T, "*");
                out.push_back(prevToken);
                prevToken.type = OP_T;
            }
            prevToken = Token(OP_T, std::string(first, first+1));
            out.push_back(prevToken);
            first++;
            last = first;
        }
        
        // functions and constants start with a letter and may contain letters
        // and numbers.
        else if ((isalpha(*first) && !isspace(*first)) || *first == '!'){
            last = first;
            do{
                last++;
            }while(last != std::end(s) && isalpha(*last));
            std::string temp(first, last);
            first = last;
            if (constants.count(temp)){
                prevToken = Token(NUMBER_T, constants[temp]);
                out.push_back(prevToken);
            }
            else if (funcs.count(temp)){
                // Handle implicit multiplication
                if ((prevToken.type == NUMBER_T || prevToken.value == ")") && temp != "!"){
                    out.push_back(Token(OP_T, "*"));
                }
                prevToken = Token(FUNC_T, temp);
                out.push_back(prevToken);
            }
            else{
                throw std::runtime_error(std::string("Unknown symbol: ") + temp);
            }
        }

        else{ throw std::runtime_error("Unknown symbol."); }
    }
    return out;
}


TokenStack infix_to_postfix(std::vector<Token> list){
    TokenStack postfix;
    TokenStack opstack;
    try{
        size_t i = 0;
        while(i < list.size()){
            Token t = list[i];
            // Case 1: Push operands (numbers) as they arrive. This is the only case
            // involving operands. The factorial is already used as a postfix operator:
            // go ahead and put it on the output stack directly.
            if (t.type == NUMBER_T || t.value == "!"){
                postfix.push(t);
                i++;
            }
            // Case 2: If the incoming symbol is a left parenthesis or a function, push it on
            // the stack.
            else if (t.value == "(" || t.type == FUNC_T){
                opstack.push(t);
                i++;
            }
            // Case 3: If the incoming symbol is a right parenthesis, pop the operator
            // stack and push the operators onto the output until you see a left parenthesis.
            else if (t.value == ")"){
                // If there is a left parenthesis on the stack, the two will annihilate.
                if (opstack.peek().value == "("){
                    opstack.pop();
                }
                else{
                    while(opstack.peek().value != "(" && opstack.size() > 0){
                        postfix.push(opstack.pop());
                    }
                    opstack.pop();// discard parentheses
                }
                // If there is a function on the stack, push it now.
                if (opstack.peek().type == FUNC_T){
                    postfix.push(opstack.pop());
                }
                i++;
            }
            // Case 4: If the operator stack is empty or contains a left parenthesis on
            // top, push the incoming operator onto the operator stack.
            else if (opstack.is_empty() || opstack.peek().value == "("){
                opstack.push(t);
                i++;
            }
            // Case 5: If the incoming symbol has higher precedence than the
            // top of the stack, push it on the stack.
            else if (precedence[t.value] > precedence[opstack.peek().value]){
                opstack.push(t);
                i++;
            }
            // Case 6: If the incoming symbol has equal precedence with the
            // top of the stack, pop opstack and push it to postfix and then
            // push the incoming operator.
            else if (precedence[t.value] == precedence[opstack.peek().value]){
                postfix.push(opstack.pop());
                opstack.push(t);
                i++;
            }
            // Case 7: If the incoming symbol has lower precedence than the
            // symbol on the top of the stack, pop the stack and push it to
            // postfix. Then test the incoming operator against the new
            // top of stack.
            else{
                postfix.push(opstack.pop());
                // Don't increment
            }
        }

        // There should be no parentheses left.
        while(opstack.not_empty()){
            postfix.push(opstack.pop());
            Token temp = postfix.peek();
            if ((temp.type == OP_T) && (temp.value == "(" || temp.value == ")")){
                throw std::runtime_error("Mismatched Parentheses");
            }
        }
    }
    catch(std::runtime_error& e){
        throw e;
    }
    catch(...){
        throw std::runtime_error("Invalid expression. Please try again.");
    }
    // std::cout << postfix;
    return postfix;
}

//------------------------------------------------[]------------------------------------------------
//---------------------------------[Tree operations and Evaluation]---------------------------------
//------------------------------------------------[]------------------------------------------------

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
        bool valid(Node*);
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
};


//----------------------------------------[Tree operations]-----------------------------------------

void freeTree(Node* n){
    if (n == nullptr) return;
    freeTree(n->right);
    freeTree(n->left);
    delete n;
}

Node* copyTree(Node* n){
    if (n == nullptr) return nullptr;
    Node* clone = new Node;
    clone->type = n->type;
    clone->value = n->value;
    clone->right = copyTree(n->right);
    clone->left = copyTree(n->left);
    return clone;
}

//--------------------------------------[Pretty print a tree]---------------------------------------
// Using the graphviz dot program
void prettyprint(Node* p)
{
    std::cout << "\t\"" << p << "\" [label=\"" << p->value << "\"];\n";
    if (!p) return;
    if (p->left){
        std::cout << "\t\"" << p << "\" -> \"" << p->left << "\";\n";
        prettyprint(p->left);
    }
    if (p->right){
        std::cout << "\t\"" << p << "\" -> \"" << p->right << "\";\n";
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
    root = new Node;
    build(root);
    #ifdef DRAWGRAPH
        ppwrapper(root);
    #endif
    isBuilt = true;
    if (!valid(root)){
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
    tolower(expr);
    try{
        exprstack = infix_to_postfix(tokenize(expr));
    }
    catch(std::runtime_error& p){
        throw p;
    }
    root = new Node;
    build(root);
    prettyprint(root);
    isBuilt = true;
    if (!valid(root)){
        throw std::runtime_error("Malformed syntax.");
    }
}

void SyntaxTree::build(Node* n){
    if (exprstack.size() == 0){
        delete n;
        n = nullptr;
        return;
    }
    Token t = exprstack.pop();
    n->type = t.type;
    n->value = t.value;
    if (t.type == OP_T){
        n->numchildren = 2;
            // Build the righthand side of the tree first!!
            n->right = new Node;
            n->left = new Node;
            build(n->right);
            build(n->left);
    }
    if (t.type == NUMBER_T){
        n->numchildren = 0;
        n->left = nullptr;
        n->right = nullptr;
    }
    if (t.type == FUNC_T){
        n->numchildren = 1;
        n->left = new Node;
        build(n->left);
        n->right = nullptr;
    }
}

bool SyntaxTree::valid(Node* n){
    if (n == nullptr) return true;
    // If the current node is an operator and either child is null,
    // that means the tree is invalid.
    if (n->type == OP_T){
        if (n->left == nullptr || n->right == nullptr){ return false; }
    }

    // If the current node is a number, and either child is not null,
    // then the tree is invalid.
    else if (n->type == NUMBER_T){
        if (n->left != nullptr || n->right != nullptr){ return false; }
    }

    else if (n->type == FUNC_T){
        if (n->left == nullptr && n->right != nullptr){ return false; }
    }

    else{
        return false;
    }

    bool l = valid(n->left);
    bool r = valid(n->right);
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
    if (n->type == NUMBER_T)
        return std::stod(n->value);
    else if (n->type == OP_T)
        return ops[n->value](evaluate(n->left), evaluate(n->right));
    else if (n->type == FUNC_T)
        return funcs[n->value](evaluate(n->left));
    else
        throw std::runtime_error("Could not evaluate expression.");
}
