#pragma    once
#include    "structure.h"
#include    "europeanPrice.h"
#include    <iostream>
#include    <vector>

using    std::vector;
using    std::max;
using    namespace    optionSpace;
//二叉树定价
class    DLL_API_CLASS    americanPrice
{
public:
    americanPrice(void);
    ~americanPrice(void);
    //    二叉树
    double    price    (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        );
    double    delta    (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        );
    double    theta    (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,const    double    dt,    int    step    );
    double    vega        (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        );
    double    rho        (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step    );
    double    gamma    (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step    );
private:
    europeanPrice    m_europeanPricing;
};

class    DLL_API_CLASS    bjsmodel
{
public:    
        double    BSAmericanCallApprox2002(double    S,    double        X,    double    T,    double    r,    double    b,    double    v);
        double    phi(double    S,    double    T,    double    gamma,    double    h,    double    i,    double    r,    double    b,    double    v);
        double    ksi(double    S,    double    T2,    double    gamma,    double    h,double    I2,    double    I1,    double    t1,    double    r,    double    b,    double    v);
        double    BSAmericanApprox2002(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);
        double    GBlackScholes(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);
        double    CBND(double    X,    double    y,    double    rho);
        double    CND(double    x);    
        double    vega(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m);
        double    ImpliedVol(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    cm,    double    epsilon,    int    m);
        double    Max(double    a,    double    b);
        double    delta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m);    
        double    gamma(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m);    
        double    rho(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m);    
        double    theta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    todayToNextTradeDayNum,    double    r,    double    b,    double    v);    

        double    vega(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);
        double    ImpliedVol(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    cm);
        double    delta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);    
        double    gamma(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);    
        double    rho(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v);    
        double*    bjsmodelresult(char    CallPutFlag,    double    S,    double        X,    double    T,double    t,    double    r,    double    b,    double    ImpliedVol,    int    m,    double    epsilon);
    
};
