#include "parse.hpp"
#include "utils.hpp"

#include <stdlib.h>
#include <stdexcept>
#include <ctype.h>
#include <iostream>
#include <cmath>

std::string operators = "dDxX+-*/^@()";

//---------------------------------------------[Token]---------------------------------------------

int precedence(Token t){
    char c = t.value;
    switch(c){
        case '+':
        case '-':
            return 0;
        case 'x':
        case 'X':
        case '*':
        case '/':
            return 1;
        case 'd':
        case 'D':
        case '@':
        case '^':
            return 2;
    }
    throw std::runtime_error("An unknown error occured.");
}

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
    if(data){ delete data; }
    numTokens = other.numTokens;
    top = other.top;
    data = new Token[1024];
    std::copy(other.data, other.data + numTokens, data);
    return *this;
}

TokenStack::~TokenStack(){
    delete[] data;
}

int TokenStack::size(){
    return numTokens;
}

bool TokenStack::is_empty(){
    return numTokens <= 0;
}

bool TokenStack::not_empty(){
    return numTokens > 0;
}

Token TokenStack::peek(){
    return data[top];
}

void TokenStack::push(Token t){
    top++;
    data[top] = t;
    numTokens++;
}

Token TokenStack::pop(){
    if(numTokens <= 0){
        throw std::runtime_error("Error: attempt to pop empty stack");
    }
    Token out = data[top];
    top--;
    numTokens--;
    return out;
}

//------------------------------------------[Tokenization]------------------------------------------

std::string remove_spaces(std::string in){
    std::string out;
    for(char c : in){
        if(!isspace(static_cast<unsigned char>(c))){
            out += c;
        }
    }
    return out;
}

std::vector<Token> tokenize(std::string s){
    s = remove_spaces(s);
    auto first = std::begin(s);
    auto last = std::begin(s);
    std::vector<Token> out;
    enum tokentype{token_null, token_op, token_num} prevToken;
    prevToken = token_null;
    while(first != std::end(s)){
        if(isdigit(*first)){
            last = first;
            do{
                last++;
            } while(last != std::end(s) && isdigit(*last));

            out.push_back(Token(false, std::stoi(std::string(first, last))));
            first = last;
            prevToken = token_num;
        }
        
        //Handle the case of negative numbers. A '-' char indicates a
        //negative number if it comes at the beginning of the string,
        //or if it follows a previous operator.
        if(*first == '-' && prevToken != token_num){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && isdigit(*last));
            out.push_back(Token(false, std::stoi(std::string(first, last))));
            first = last;
            prevToken = token_num;
        }
        
        if(is_in_list(*first, operators)){
            //Handle implicit lval argument of 1 on dice rolls when not explicitly stated
            if(*first == 'd' && prevToken != token_num){
                out.push_back(Token(false, 1));
            }    
            out.push_back(Token(true, *first));
            first++;
            last = first;
            prevToken = token_op;
        }
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
            //Case 1: Push operands as they arrive. This is the only case
            //involving operands.
            if(!t.op){
                postfix.push(t);
                i++;
            }
            //Case 2: If the incoming symbol is a left parenthesis, push it on
            //the stack.
            else if (t.value == '('){
                opstack.push(t);
                i++;
            }
            //Case 3: If the incoming symbol is a right parenthesis, pop the
            //stack and print the operators until you see a left parenthesis.
            else if (t.value == ')'){
                //If there is a left parenthesis on the stack, the two will annihilate.
                if(opstack.peek().value == '('){
                    opstack.pop();
                }
                else{
                    while(opstack.peek().value != '(' && opstack.size() > 0){
                        postfix.push(opstack.pop());
                    }
                    opstack.pop();//discard parentheses
                }
                i++;
            }
            //Case 4: If the stack is empty or contains a left parenthesis on
            //top, push the incoming operator onto the stack.
            else if (opstack.is_empty() || opstack.peek().value == '('){
                opstack.push(t);
                i++;
            }
            //Case 5: If the incoming symbol has higher precedence than the
            //top of the stack, push it on the stack.
            else if (precedence(t) > precedence(opstack.peek())){
                opstack.push(t);
                i++;
            }
            //Case 6: If the incoming symbol has equal precedence with the
            //top of the stack, pop opstack and push it to postfix and then
            //push the incoming operator.
            else if (precedence(t) == precedence(opstack.peek())){
                postfix.push(opstack.pop());
                opstack.push(t);
                i++;
            }
            //Case 7: If the incoming symbol has lower precedence than the
            //symbol on the top of the stack, pop the stack and push it to
            //postfix. Then test the incoming operator against the new
            //top of stack.
            else{
                postfix.push(opstack.pop());
                //Don't increment
            }
        }

        //There should be no parentheses left.
        while(opstack.not_empty()){
            postfix.push(opstack.pop());
            Token temp = postfix.peek();
            if(temp.op && (temp.value == '(' || temp.value == ')')){
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
    return postfix;
}

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
    clone->op = n->op;
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
    build(root);
    isBuilt = true;
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
}

void SyntaxTree::build(Node* n){
    if(exprstack.size() == 0) return;
    Token t = exprstack.pop();
    n->op = t.op;
    n->value = t.value;
    if(n->op){
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
    if(n->left == nullptr && n->right == nullptr){
        //an operand will have two null children
        if(!n->op){
            return n->value;
        }
        else{
            throw std::runtime_error("Invalid Expression. Please try again.");
        }
    }
    
    char c = n->value;
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

std::string SyntaxTree::str(){
    return expr;
}

double evaluate(std::string expr){
    SyntaxTree s(expr);
    double result;
    try{
        result = s.evaluate();
    }
    catch(std::runtime_error& e){
        throw e;
    }
    return result;
}
