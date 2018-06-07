#pragma    once
#include    <iostream>
#include    <armadillo>
#include    "structure.h"
using    namespace    arma;

typedef    double    OptionValue;
typedef    double    DeltaValue;
typedef    double    GammaValue;
typedef    double    ThetaValue;
typedef    double    VegaValue;
typedef    double    RhoValue;
using    namespace    std;
using    namespace    optionSpace;

//    this    class    for    asian    Barrier    down    out
class    asianBarrier
{
public:
                OptionValue    price(OptionType    type,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                DeltaValue    delta(OptionType    type,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                GammaValue    gamma(OptionType    type,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                ThetaValue    theta(OptionType    type,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                VegaValue    vega(OptionType    type    ,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                RhoValue    rho(OptionType    type,    double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    Nrep,    int    yearBase=252);

                asianBarrier(void);
                ~asianBarrier(void);

private:
                    bool    checkParaValid(double    S0,double    K,double    A,double    r,double    b,double    T,double    t1,\
                int    totalPoint,int    averagingPoint    ,    double    barrier,double    sigma,double    compensation,double    initPrice,int    &Nrep);

};

