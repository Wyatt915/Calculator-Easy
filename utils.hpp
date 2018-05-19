#pragma once

#include <vector>
#include <string>

#define NULL_T "null"
#define NUMBER_T "number"
#define OP_T "operator"
#define FUNC_T "function"

template<class Element_type, class Container_type>
bool is_in_list(Element_type item, Container_type list){
    for(Element_type element : list){
        if (item == element) { return true; }
    }
    return false;
}

void tolower(std::string& s);
void toupper(std::string& s);
int max_length_in_list(std::vector<std::string>&);
