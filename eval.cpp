#include "eval.hpp"
#include "func.hpp"
#include "utils.hpp"

#include <iomanip>
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
    { "pi",  "3.14159265358979323846" }
};

static std::map<std::string, int> prec = {
    { "+", 0 },
    { "-", 0 },
    { "*", 1 },
    { "/", 1 },
    { "%", 1 },
    { "d", 2 },
    { "^", 2 }
};

inline int precedence(const Token &t){
    return prec[t.value];
}

//------------------------------------------------[]------------------------------------------------
//----------------------------------------[Parsing routines]----------------------------------------
//------------------------------------------------[]------------------------------------------------


//-------------------------------------------[TokenStack]-------------------------------------------

TokenStack::TokenStack():numTokens(0), top(-1){
    data = new Token[capacity];
}

TokenStack::TokenStack(const TokenStack& other){
    numTokens = other.numTokens;
    top = other.top;
    data = new Token[capacity];
    std::copy(other.data, other.data + numTokens, data);
}

TokenStack& TokenStack::operator=(const TokenStack& other){
    delete[] data; // This is guaranteed to be allocated beforehand.
    numTokens = other.numTokens;
    top = other.top;
    capacity = other.capacity;
    data = new Token[capacity];
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
    if(numTokens >= capacity){
        Token* temp = new Token[capacity];
        std::copy(data, data+numTokens, temp);
        delete[] data;
        data = new Token[capacity*2];
        std::copy(temp, temp+numTokens, data);
        capacity *= 2;
        delete[] temp;
    }
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
    for(size_t i = 0; i < t.numTokens; i++){
        os << t.data[i].value << " ";
    }
    return os;
}

//------------------------------------------[Tokenization]------------------------------------------

std::vector<Token> tokenize(std::string s){
    auto first = std::begin(s);
    auto last = std::begin(s);
    std::vector<Token> out;
    Token prevToken;
    while(first != std::end(s) || *first != '\0'){
        // Ignore spaces
        if (isspace(*first)){
            first++;
        }

        // Numbers
        else if (isdigit(*first) || *first == '.'){
            last = first;
            bool keepgoing = true;
            while(keepgoing){
                do{
                    last++;
                } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
                if(last == std::end(s)) break;
                //handle e notation
                else if(*last == 'e' && !isalpha(*(last+1))){
                    //if the char after 'e' is not a letter, we want to include it
                    last++;
                    keepgoing = true;
                }
                else keepgoing = false;
            }
            prevToken = Token(NUMB_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }

        // Evaluate expressions within brackets.
        else if (*first == '['){
            last = first++;

            // Find outermost matching brackets
            int bracecount = 1;
            do{
                last++;
                if(*last == '[') bracecount++;
                if(*last == ']') bracecount--;
            } while(last != std::end(s) && (*last != ']' || bracecount > 0));

            if (*last != ']'){
                std::cerr << *last << '\n';
                throw std::runtime_error("Missing ']'.");
            }
            // Done matching braces

            double d = evaluate(std::string(first, last));
            if(d < 0){
                throw std::runtime_error("Error: values within [brackets] must be >= 0.");
            }
            unsigned int val = d;
            if(val >= globalHistory.size()){
                throw std::runtime_error("Error: values within [brackets] must not exceed the number of solved expressions");
            }
            std::stringstream ss;
            ss << std::setprecision(15) << globalHistory[val];
            prevToken = Token(NUMB_T, ss.str());
            out.push_back(prevToken);
            first = ++last;
        }

        // Handle the case of negative numbers. A '-' char indicates a
        // negative number if it comes at the beginning of the string,
        // or if it follows a previous operator.
        else if (*first == '-' && prevToken.type != NUMB_T && prevToken.value != ")"){
            first = last;
            do{
                last++;
            } while(last != std::end(s) && (isdigit(*last) || *last == '.'));
            prevToken = Token(NUMB_T, std::string(first, last));
            out.push_back(prevToken);
            first = last;
        }

        // parenthesis
        else if (*first == '(' || *first == ')'){
            prevToken = Token(BRAC_T, std::string(first, first+1));
            out.push_back(prevToken);
            first = ++last;
        }

        // Operators
        else if (!isalnum(*first) && *first != '!'){
            last = first;
            do{
                last++;
                // keep going until we reach a number, letter, space, or paren.
            } while (last != std::end(s) && ispunct(*last) && !is_in_list(*last, std::string("()[].")));
            std::string op(first, last);
            if (ops.count(op)){
                prevToken = Token(OPER_T, op);
                out.push_back(prevToken);
            }
            else {
                throw std::runtime_error(std::string("Unknown operator: ") + op);
            }
            first = last;
        }

        // functions and constants start with a letter and may contain letters
        // and numbers.
        else if ((isalpha(*first) && !isspace(*first)) || *first == '!'){
            last = first;
            do{
                last++;
            }while(last != std::end(s) && isalpha(*last));
            std::string temp(first, last);
            if (constants.count(temp)){
                prevToken = Token(NUMB_T, constants[temp]);
                out.push_back(prevToken);
            }
            else if (funcs.count(temp)){
                prevToken = Token(FUNC_T, temp);
                out.push_back(prevToken);
            }
            else if (ops.count(temp)){
                prevToken = Token(OPER_T, temp);
            }
            else{
                throw std::runtime_error(std::string("Unknown symbol: ") + temp);
            }
            first = last;
        }

        // Catch any other cases
        else{

            throw std::runtime_error(std::string("Unknown symbol.") + std::string(first, last));
        }
    }
    return out;
}

void parse(std::vector<Token>& tkstream){
    if (tkstream.size() <= 1) return;
    auto it = tkstream.begin();
    Token prv;
    while(it != tkstream.end()){
        // Handle implicit multiplication
        if ((it->type == FUNC_T || it->type == NUMB_T || it->value == "(") && (it->value != "!") &&
            (prv.type == NUMB_T || prv.value == ")")){
            it = tkstream.insert(it, Token(OPER_T, "*"));
        }

        // Handle implicit lval argument of 1 on dice rolls when not explicitly stated
        if (it->value == "d" && (prv.type == NULL_T || prv.type == OPER_T)){
            it = tkstream.insert(it, Token(NUMB_T, "1"));
        }
        prv = *it;
        it++;
    }
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
            if (t.type == NUMB_T || t.value == "!"){
                postfix.push(t);
                i++;
            }
            // Case 2: If the incoming symbol is a left parenthesis or a function, push it on
            // the stack.
            else if (t.value == "(" || t.type == FUNC_T){
                opstack.push(t);
                i++;
            }

            // Case 3: the incoming token is an operator
            else if (t.type == OPER_T){
                bool shouldPopOp;
                while (!opstack.is_empty()){
                    shouldPopOp =
                        (opstack.peek().type == FUNC_T ||
                            precedence(opstack.peek())  >  precedence(t) ||
                            (precedence(opstack.peek()) == precedence(t) &&
                                opstack.peek().value != "^"     //Right-associativity behaves differently
                            )
                        ) &&
                        (opstack.peek().value != "(");
                    if (shouldPopOp) {
                        postfix.push(opstack.pop());
                    }
                    else break;
                }
                opstack.push(t);
                i++;
            }

            // Case 3: If the incoming symbol is a right parenthesis, pop the operator
            // stack and push the operators onto the output until you see a left parenthesis.
            else if (t.value == ")"){
                while(opstack.peek().value != "(" && opstack.size() > 0){
                    postfix.push(opstack.pop());
                }
                opstack.pop();// discard parentheses
                // If there is a function on the stack, push it now.
                if (opstack.peek().type == FUNC_T){
                    postfix.push(opstack.pop());
                }
                i++;
            }

            else{
                postfix.push(opstack.pop());
                // Don't increment
            }
        }

        // There should be no parentheses left.
        while(opstack.not_empty()){
            postfix.push(opstack.pop());
            Token temp = postfix.peek();
            if ((temp.type == OPER_T) && (temp.value == "(" || temp.value == ")")){
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
        SyntaxTree(const std::vector<Token>&);
        SyntaxTree();
        SyntaxTree(const SyntaxTree&);
        double evaluate();
        std::string str();
        bool valid(Node*);
        SyntaxTree& operator=(const SyntaxTree&);
        ~SyntaxTree();
    private:
        Node* root;
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

SyntaxTree::SyntaxTree(const std::vector<Token>& tkstream){
    try{
        exprstack = infix_to_postfix(tkstream);
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
}

SyntaxTree& SyntaxTree::operator=(const SyntaxTree& other){
    root = copyTree(other.root);
    isBuilt = other.isBuilt;
    return *this;
}

SyntaxTree::~SyntaxTree(){
    freeTree(root);
}

//----------------------------------[SyntaxTree Member Functions]-----------------------------------

void SyntaxTree::build(Node* n){
    if (exprstack.size() == 0){
        delete n;
        n = nullptr;
        return;
    }
    Token t = exprstack.pop();
    n->type = t.type;
    n->value = t.value;
    if (t.type == OPER_T){
        n->numchildren = 2;
            // Build the righthand side of the tree first!!
            n->right = new Node;
            n->left = new Node;
            build(n->right);
            build(n->left);
    }
    if (t.type == NUMB_T){
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
    if (n->type == OPER_T){
        if (n->left == nullptr || n->right == nullptr){ return false; }
    }

    // If the current node is a number, and either child is not null,
    // then the tree is invalid.
    else if (n->type == NUMB_T){
        //TODO: Make sure whatever the value of this token is is a valid number.
        if (n->left != nullptr || n->right != nullptr){ return false; }
    }

    else if (n->type == FUNC_T){
        if (n->left == nullptr && n->right != nullptr){ return false; }
    }

    else{
        return false;
    }

    // The whole tree is valid iff the left and right subtrees are both valid.
    return valid(n->left) && valid(n->right);
}

// TODO
std::string SyntaxTree::str(){
    return "";
}

//------------------------------------[Evaluate the Expression]-------------------------------------

double evaluate(std::string expr){
    double result;
    tolower(expr);
    try{
        std::vector<Token> tkstream = tokenize(expr);
        parse(tkstream);
        SyntaxTree s(tkstream);
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
    if (n->type == NUMB_T)
        return std::stod(n->value);
    else if (n->type == OPER_T)
        return ops[n->value](evaluate(n->left), evaluate(n->right));
    else if (n->type == FUNC_T)
        return funcs[n->value](evaluate(n->left));
    else
        throw std::runtime_error("Could not evaluate expression.");
}
