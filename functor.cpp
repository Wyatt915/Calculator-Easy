#include "functor.hpp"
#include <stdexcept>

double add_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return args[0] + args[1];
}

double sub_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return args[0] - args[1];
}

double mul_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return args[0] * args[1];
}

double div_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return args[0] / args[1];
}

double mod_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return int(args[0]) % int(args[1]);
}

double rol_f::operator()(std::vector<double> args){
    if(args.size() != 2){ throw std::runtime_error("Invalid arguement"); }
    return 0;
}

//double sin_f::operator()(std::vector<double> args){
//    return 0;
//}
//
//double cos_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double tan_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double sec_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double csc_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double cot_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double log_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double sqrt_f::operator()(std::vector<double> args){
//    return 0;
//
//}
//
//double exp_f::operator()(std::vector<double> args){
//    return 0;
//
//}
