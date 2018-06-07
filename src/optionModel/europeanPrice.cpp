#include    "europeanPrice.h"


double    europeanPrice::price(OptionType    oType,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau    ){

    double    resultValue    =    0.0L    ,greeks    =    0.0L;
    double    d1    =    0.0L,d2    =    0.0L;

                if(tau<=0){
                                if(oType    ==    OptionType::CALL){
                                                return        std::max<double>(underlyingPrice    -    strike,0.0);
                                }
                                if(oType    ==    OptionType::PUT){
                                                return        std::max<double>(strike    -    underlyingPrice,0.0);
                                }
                }

                d1    =    (std::log(underlyingPrice/        strike)    +    (b    +    vol*    vol    *0.5)    *    tau)    /    (vol    *    std::sqrt(tau));
                d2    =    d1    -    vol    *    std::sqrt(tau);

    if(oType    ==    OptionType::CALL){                                                                                                        
                                resultValue    =    underlyingPrice    *        std::exp((b-rate)    *    tau)    *m_math.N(d1,0,1)    -    strike    *    std::exp(-rate    *    tau)    *    m_math.N(d2,0,1);
    }
    if(oType    ==    OptionType::PUT){
                                resultValue    =    strike    *    std::exp(-rate    *    tau)    *    m_math.N(-d2,0,1)    -        underlyingPrice    *        std::exp((b-rate)    *    tau)    *    m_math.N(-d1,0,1)    ;
    }

    return    resultValue;
}
double    europeanPrice::delta(OptionType    ot,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau){
    double    upPrice    =        underlyingPrice    +    underlyingPrice    *    0.005L;
    double    downPrice    =underlyingPrice    -    underlyingPrice    *    0.005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->price(    ot,        strike,    rate,    b,    upPrice    ,    vol,    tau)    ;
    downOption    =    this->price(    ot,        strike,    rate,    b,    downPrice    ,    vol,    tau)    ;
    greeks    =    (upOption    -    downOption)        /    (underlyingPrice    *    2.0L    *        0.005L);
    return    greeks;
}
double    europeanPrice::vega(OptionType    ot,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau    ){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =        this->price(    ot,        strike,    rate,    b,    underlyingPrice    ,    vol    +    0.001L,    tau)    ;
    downOption    =    this->price(    ot,        strike,    rate,    b,    underlyingPrice    ,    vol    -    0.001L,    tau)    ;
    greeks    =    (upOption    -    downOption)        /    (2.0L    *        0.001L);
    return    greeks;
}
double    europeanPrice::theta(OptionType    ot,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau,const    double    dt){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =        this->price(    ot,        strike,    rate,    b,    underlyingPrice    ,    vol,    tau    -    dt)    ;
    downOption    =    this->price(    ot,        strike,    rate,    b,    underlyingPrice    ,    vol,    tau)    ;
    greeks    =    (upOption    -    downOption)        /    (    dt);
    return    greeks;
}
double    europeanPrice::rho(OptionType    ot,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau    ){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =        this->price(    ot,        strike,    rate    +    0.0001L,    b,    underlyingPrice    ,    vol    ,    tau)    ;
    downOption    =    this->price(    ot,        strike,    rate    ,    b,    underlyingPrice    ,    vol    ,    tau)    ;
    greeks    =    (upOption    -    downOption)        /    (0.0001L)    /    100.0;
    return    greeks;
}
double    europeanPrice::gamma(OptionType    ot,    const    double    strike,const    double    rate,const    double    b,const    double    underlyingPrice,const    double    vol,const    double    tau    ){
    double    upPrice    =        underlyingPrice    +    underlyingPrice    *    0.0005L;
    double    downPrice    =underlyingPrice    -    underlyingPrice    *    0.0005L;
    double    midPrice    =    underlyingPrice;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    double    midOption;
    upOption    =    this->price(    ot,        strike,    rate,    b,    upPrice    ,    vol,    tau)    ;
    midOption    =    this->price(    ot,        strike,    rate,    b,    midPrice    ,    vol,    tau)    ;
    downOption    =    this->price(    ot,        strike,    rate,    b,    downPrice    ,    vol,    tau)    ;
    greeks    =        (upOption    -    2.0F*midOption    +    downOption)    /    (underlyingPrice*    underlyingPrice    *    0.0005L    *    0.0005L    );
    return    greeks;
}




europeanPrice::europeanPrice(void)
{
}


europeanPrice::~europeanPrice(void)
{
}