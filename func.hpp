#pragma once

#include <stdexcept>
#include <cmath>
#include <cstdlib>

double add_f(double lval, double rval){
    return lval + rval;
}

double sub_f(double lval, double rval){
    return lval - rval;
}

double mul_f(double lval, double rval){
    return lval * rval;
}

double div_f(double lval, double rval){
    return lval / rval;    
}

double mod_f(double lval, double rval){
    return (int)lval % (int)rval; //TODO: This is wrong. use fmod() or somesuch
}

double pow_f(double lval, double rval){
    return pow(lval, rval);
}

double rol_f(double lval, double rval){
    int out = 0;
    for(unsigned int i = 0; i < (unsigned int)lval; i++){
        out += (rand() % (unsigned int)rval) + 1;
    }
    return out;
}

//double sin_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double cos_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double tan_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double csc_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double sec_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double cot_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double exp_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double log_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double srt_f(double lval, double rval){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
