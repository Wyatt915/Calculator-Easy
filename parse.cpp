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
    std::string err = "An error occured finding precedence. [";
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
    std::string prevTokenType;
    prevTokenType = NULL_T;
    while(first != std::end(s) || *first != '\0'){
        //Numbers
        if(isdigit(*first) || *first == '.'){
            last = first;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));

            out.push_back(Token(NUMBER_T, std::string(first, last)));
            first = last;
            prevTokenType = NUMBER_T;
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
            if(is_in_list(temp, constants)){
                if(temp == "e"){
                    out.push_back(Token(NUMBER_T, "2.718281828"));
                }
                else if(temp == "pi"){
                    out.push_back(Token(NUMBER_T, "3.141592653"));
                }
                else if(temp == "phi"){
                    out.push_back(Token(NUMBER_T, "1.618"));
                }
            }
            else{
                throw std::runtime_error(std::string("Unknown symbol: ") + temp);
            }
        }
        //Handle the case of negative numbers. A '-' char indicates a
        //negative number if it comes at the beginning of the string,
        //or if it follows a previous operator.
        else if(*first == '-' && prevTokenType != NUMBER_T && out.back().value != ")"){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            out.push_back(Token(NUMBER_T, std::string(first, last)));
            first = last;
            prevTokenType = NUMBER_T;
        }
        
        else if(is_in_list(std::string(first, first+1), operators)){
            //Handle implicit lval argument of 1 on dice rolls when not explicitly stated
            if(*first == 'd' && prevTokenType != NUMBER_T){
                out.push_back(Token(NUMBER_T, "1"));
            }    
            //Handle implicit multiplication of parentheticals
            if(*first == '(' && prevTokenType != NULL_T && (out.back().value == ")" || prevTokenType == NUMBER_T)){
                out.push_back(Token(OP_T, "*"));
                prevTokenType = OP_T;
            }
            out.push_back(Token(OP_T, std::string(first, first+1)));
            first++;
            last = first;
            prevTokenType = OP_T;
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
            //Case 1: Push operands (numbers) as they arrive. This is the only case
            //involving operands.
            if(t.type == NUMBER_T){
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
    return postfix;
}

