#pragma    once
#include    "structure.h"
#include    "algorithmLib.h"
#include    <iostream>
using    namespace    optionSpace;
class    DLL_API_CLASS    europeanPrice
{
public:
    europeanPrice(void);
    double    price(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau);
    double    delta(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau);
    double    vega(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau);
    double    theta(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau,const    double    dt);
    double    rho(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau);
    double    gamma(OptionType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau);

    ~europeanPrice(void);
private:
    algorithmLib    m_math;
};

