#pragma once

#include <vector>
#include <string>

template<class Element_type, class Container_type>
bool is_in_list(Element_type item, Container_type list){
    for(Element_type element : list){
        if (item == element) { return true; }
    }
    return false;
}

//Nodes and tokens may be different types of elements: numbers, operators, or functions.
enum element {null_t, num_t, op_t, func_t, paren_t};

void tolower(std::string& s);
void toupper(std::string& s);
int max_length_in_list(std::vector<std::string>&);
