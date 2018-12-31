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

double csc(double arg){
    return 1/sin(arg);
}

double sec(double arg){
    return 1/cos(arg);
}

double cot(double arg){
    return 1/tan(arg);
}

double fact(double arg){
    double intpart;
    //If our value is an integer
    double out = 1;
    if(modf(arg, &intpart) == 0.0){
        for(int i = 2; i <= intpart; i++){
            out *= i;
        }
    }
    else{
        out = tgamma(arg+1);
    }
    return out;
}
