#include "parse.hpp"
#include "utils.hpp"

#include <stdlib.h>
#include <stdexcept>
#include <ctype.h>
#include <iostream>
#include <cmath>

std::string operators = "dDxX+-*/^@()";
std::vector<std::string> functions = {"sin", "cos", "tan", "log", "sqrt"};
std::vector<std::string> constants = {"e", "pi", "phi"};

//---------------------------------------------[Token]---------------------------------------------

int precedence(Token t){
    char c = t.value[0];
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
    std::string err = "An unknown error occured. [";
    throw std::runtime_error(err + c + "]");
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
    delete[] data; //This is guaranteed to be allocated beforehand.
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

std::vector<Token> tokenize(std::string s){
    auto first = std::begin(s);
    auto last = std::begin(s);
    std::vector<Token> out;
    enum element prevToken;
    prevToken = null_t;
    while(first != std::end(s)){
        //Numbers
        if(isdigit(*first) || *first == '.'){
            last = first;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));

            out.push_back(Token(num_t, std::string(first, last)));
            first = last;
            prevToken = num_t;
        }

        //functions and constants start with a letter and may contain letters
        //and numbers.
        if(isalpha(*first)){
            last = first;
            do{
                last++;
            }while(last != std::end(s) && isalpha(*last));
            std::string temp(first, last);
            first = last;
            if(is_in_list(temp, constants)){
                if(temp == "e"){
                    out.push_back(Token(num_t, "2.718281828"));
                }
                else if(temp == "pi"){
                    out.push_back(Token(num_t, "3.141592653"));
                }
                else if(temp == "phi"){
                    out.push_back(Token(num_t, "1.618"));
                }
            }
            else{
                throw std::runtime_error("Unknown symbol");
            }
        }
        //Handle the case of negative numbers. A '-' char indicates a
        //negative number if it comes at the beginning of the string,
        //or if it follows a previous operator.
        else if(*first == '-' && prevToken != num_t && out.back().value != ")"){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            out.push_back(Token(num_t, std::string(first, last)));
            first = last;
            prevToken = num_t;
        }
        
        else if(is_in_list(*first, operators)){
            //Handle implicit lval argument of 1 on dice rolls when not explicitly stated
            if(*first == 'd' && prevToken != num_t){
                out.push_back(Token(num_t, "1"));
            }    
            //Handle implicit multiplication of parentheticals
            if(*first == '(' && prevToken != null_t && (out.back().value == ")" || prevToken == num_t)){
                out.push_back(Token(op_t, "*"));
                prevToken = op_t;
            }
            out.push_back(Token(op_t, std::string(first, first+1)));
            first++;
            last = first;
            prevToken = op_t;
        }

        else{ first++; }
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
            //Case 1: Push operands (numbers) as they arrive. This is the only case
            //involving operands.
            if(t.type == num_t){
                postfix.push(t);
                i++;
            }
            //Case 2: If the incoming symbol is a left parenthesis, push it on
            //the stack.
            else if (t.value == "("){
                opstack.push(t);
                i++;
            }
            //Case 3: If the incoming symbol is a right parenthesis, pop the operator
            //stack and push the operators onto the output until you see a left parenthesis.
            else if (t.value == ")"){
                //If there is a left parenthesis on the stack, the two will annihilate.
                if(opstack.peek().value == "("){
                    opstack.pop();
                }
                else{
                    while(opstack.peek().value != "(" && opstack.size() > 0){
                        postfix.push(opstack.pop());
                    }
                    opstack.pop();//discard parentheses
                }
                i++;
            }
            //Case 4: If the operator stack is empty or contains a left parenthesis on
            //top, push the incoming operator onto the operator stack.
            else if (opstack.is_empty() || opstack.peek().value == "("){
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
            if((temp.type == op_t) && (temp.value == "(" || temp.value == ")")){
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
