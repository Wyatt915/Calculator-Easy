#pragma once

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cstdlib>

double add_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return args[0] + args[1];
}

double sub_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return args[0] - args[1];
}

double mul_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return args[0] * args[1];
}

double div_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return args[0] / args[1];    
}

double mod_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return (int)args[0] % (int)args[1]; //TODO: This is wrong. use fmod() or somesuch
}

double pow_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    return pow(args[0], args[1]);
}

double rol_f(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Binary operator requires 2 arguments."); }
    int out = 0;
    for(unsigned int i = 0; i < (unsigned int)args[0]; i++){
        out += (rand() % (unsigned int)args[1]) + 1;
    }
    return out;
}

//double sin_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double cos_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double tan_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double csc_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double sec_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double cot_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double exp_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double log_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
//double srt_f(std::vector<double> args){
//    if(args.size() != 1){ throw std::runtime_error("Function requires 1 argument"); }
//
//}
