#pragma once

#include <vector>

class ce_func{
    public:
        ce_func() {};
        virtual double operator()(std::vector<double> args) = 0;
};

//----------------------------------------[Basic Arithmetic]----------------------------------------

class add_f:public ce_func{
    virtual double operator()(std::vector<double>);
};
class sub_f:public ce_func{
    virtual double operator()(std::vector<double>);
};
class mul_f:public ce_func{
    virtual double operator()(std::vector<double>);
};
class div_f:public ce_func{
    virtual double operator()(std::vector<double>);
};
class mod_f:public ce_func{
    virtual double operator()(std::vector<double>);
};

//-------------------------------------------[Roll Dice]--------------------------------------------

class rol_f:public ce_func{ virtual double operator()(std::vector<double>); };

//------------------------------------------[Trigonometry]------------------------------------------

//class sin_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class cos_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class tan_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class sec_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class csc_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class cot_f:public ce_func{ virtual double operator()(std::vector<double>); };

//------------------------------------[Powers, roots, and logs]-------------------------------------

//class log_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class sqrt_f:public ce_func{ virtual double operator()(std::vector<double>); };
//class exp_f:public ce_func{ virtual double operator()(std::vector<double>); };
