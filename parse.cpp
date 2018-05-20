#include "parse.hpp"
#include "utils.hpp"

#include <cmath>
#include <ctype.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <stdlib.h>
std::vector<std::string> operators = {"d", "D", "x", "X", "+", "-", "*", "/", "^", "@", "(", ")"};
std::vector<std::string> functions = {"sin", "cos", "tan", "log", "sqrt"};

static std::map<std::string, std::string> constants = {
    { "phi", "1.61803398874989484820" },
    { "pi",  "3.14159265358979323846" },
    { "e",   "2.71828182845904523536" }
};

//---------------------------------------------[Token]---------------------------------------------

static std::map<std::string, int> precedence = {
    { "+", 0 },
    { "-", 0 },
    { "*", 1 },
    { "/", 1 },
    { "d", 2 },
    { "^", 2 },
    { "sin", 3}
};

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
        //Numbers
        if(isdigit(*first) || *first == '.'){
            last = first;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            prevToken = Token(NUMBER_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }

        //functions and constants start with a letter and may contain letters
        //and numbers.
        else if(isalpha(*first)){
            last = first;
            do{
                last++;
            }while(last != std::end(s) && isalpha(*last));
            std::string temp(first, last);
            first = last;
            if(constants.count(temp)){
                prevToken = Token(NUMBER_T, constants[temp]);
                out.push_back(prevToken);
            }
            else if(is_in_list(temp, functions)){
                //Handle implicit multiplication
                if(prevToken.type == NUMBER_T || prevToken.value == ")"){
                    out.push_back(Token(OP_T, "*"));
                }
                prevToken = Token(FUNC_T, temp);
                out.push_back(prevToken);
            }
            else{
                throw std::runtime_error(std::string("Unknown symbol: ") + temp);
            }
        }
        //Handle the case of negative numbers. A '-' char indicates a
        //negative number if it comes at the beginning of the string,
        //or if it follows a previous operator.
        else if(*first == '-' && prevToken.type != NUMBER_T && prevToken.value != ")"){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            prevToken = Token(NUMBER_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }

        else if(is_in_list(std::string(first, first+1), operators)){
            //Handle implicit lval argument of 1 on dice rolls when not explicitly stated
            if(*first == 'd' && prevToken.type != NUMBER_T){
                prevToken = Token(NUMBER_T, "1");
                out.push_back(prevToken);
            }    
            //Handle implicit multiplication of parentheticals
            if(*first == '(' && prevToken.type != NULL_T && (out.back().value == ")" || prevToken.type == NUMBER_T)){
                prevToken = Token(OP_T, "*");
                out.push_back(prevToken);
                prevToken.type = OP_T;
            }
            prevToken = Token(OP_T, std::string(first, first+1));
            out.push_back(prevToken);
            first++;
            last = first;
        }

        else{ throw std::runtime_error("Error in tokenization: unknown symbol."); }
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
            if(t.type == NUMBER_T){
                postfix.push(t);
                i++;
            }
            //Case 2: If the incoming symbol is a left parenthesis or a function, push it on
            //the stack.
            else if (t.value == "(" || t.type == FUNC_T){
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
                //If there is a function on the stack, push it now.
                if(opstack.peek().type == FUNC_T){
                    postfix.push(opstack.pop());
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
            else if (precedence[t.value] > precedence[opstack.peek().value]){
                opstack.push(t);
                i++;
            }
            //Case 6: If the incoming symbol has equal precedence with the
            //top of the stack, pop opstack and push it to postfix and then
            //push the incoming operator.
            else if (precedence[t.value] == precedence[opstack.peek().value]){
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
            if((temp.type == OP_T) && (temp.value == "(" || temp.value == ")")){
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
    //std::cout << postfix;
    return postfix;
}

