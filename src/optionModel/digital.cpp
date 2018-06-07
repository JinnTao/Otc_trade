#include    "digital.h"


americanDigital::americanDigital(void)
{
    this->m_payoutTag    =    false;
}


americanDigital::~americanDigital(void)
{

}

double    americanDigital::price(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T,PayoutType    pT){
    double    greeks    =    0,    lamudaPositive    =    0    ,    lamudaNegative    =    0,k    =    0,k_    =    0,alpha    =    0    ,    beta    =    0;
    double    value;
    const    double    payAmountK    =    1;;
    double    dPositive,dNegative;
    k    =    r    /    (0.5    *    v    *    v);
    //    for    future
    //if(uT    ==    underlyingType::Future){
        k_    =    0;
    //}
    ////    for    stock
    //else{
    //    k_    =    (r    -    b)    /    (0.5    *    v    *    v);
    //}
    alpha    =    0.5    *    (1-    k_);
    beta    =    -(0.25    *    (1    -    k_)    *        (1    -    k_)    +    k);

    if(pT    ==    PayoutType::AtExpiry){
        if(oT    ==    OptionType::CALL){
            if(S    >=    K    ||    this->m_payoutTag){
                value    =    std::exp(-r    *    T);
            }
            else    if    (S    >    0){
                value    =    this->m_eDigital.cashOrNothingPrice(CALL,K,r,b,payAmountK,S,v,T)    +    std::pow(S    /    K,2*alpha)    *    this->m_eDigital.cashOrNothingPrice(PUT,K,r,b,payAmountK,K    *    K    /    S,v,T)    ;
            }
    
        }
        else    if(oT    ==    OptionType::PUT){
            if(S    <=    K    ||    this->m_payoutTag){
                value    =    std::exp(-r    *    T);
            }
            else    {
                value    =    this->m_eDigital.cashOrNothingPrice(PUT,K,r,b,payAmountK,S,v,T)    +    std::pow(S    /    K,2*alpha)    *    this->m_eDigital.cashOrNothingPrice(CALL,K,r,b,payAmountK,K    *    K    /    S,v,T)    ;
            }
    

        }
    }else{
        if(b    !=    0){
            lamudaPositive    =    alpha    +    std::sqrt(-beta);
            lamudaNegative    =    alpha    -    std::sqrt(-beta);
        }    else{
            lamudaPositive    =    1;
            lamudaNegative    =    -k;
        }

        dPositive    =    (std::log(S    /    K)    +    v    *    v        *    std::sqrt(-beta)    *    T    )    /    (v    *    std::sqrt(T));
        dNegative    =    (std::log(S    /    K)    -    v    *    v        *    std::sqrt(-beta)    *    T    )    /    (v    *    std::sqrt(T));
    
        if(oT    ==    OptionType::CALL){
            //    或者触发
            if(S    >=    K    ||    this->m_payoutTag){
                return    1;
            }else    if(    S    >    0){
                if(    T    ==    0){
                    return    0;//At    expiry
                }
                value    =    std::pow(S    /    K,lamudaPositive)    *    this->m_math.N(dPositive,0,1)    +    
                    std::pow(S    /    K,    lamudaNegative)    *        this->m_math.N(dNegative,0,1);
    
            }
        }
        else    if(oT    ==    OptionType::PUT){
            if(S    <=    K    ||    this->m_payoutTag){
                return    1;
            }else{
                if(T    ==    0){
                    return    0;
                }
                value    =    std::pow(S    /    K,lamudaPositive)    *    this->m_math.N(-dPositive,0,1)    +    
                    std::pow(S    /    K,    lamudaNegative)    *        this->m_math.N(-dNegative,0,1);
            }

        }
    }
    return    value;
}
double    americanDigital::delta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T,    PayoutType    pT){
    double    upPrice    =        S    +    S    *    0.005L;
    double    downPrice    =S    -    S    *    0.005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->price(    oT,        K,        r,        b,        upPrice,            v,        T,    pT);
    downOption    =    this->price(    oT,        K,        r,        b,        downPrice,            v,        T,        pT);
    greeks    =    (upOption    -    downOption)        /    (S    *    2.0L    *        0.005L);
    return    greeks;
}
double    americanDigital::gamma(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T    ,PayoutType    pT){
    double    upPrice    =        S    +    S    *    0.0005L;
    double    midPrice    =    S;
    double    downPrice    =S    -    S    *    0.0005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    double    midOption    =    0;
    upOption    =    this->price(    oT,        K,        r,        b,        upPrice,            v,        T,        pT);
    midOption    =    this->price(    oT,        K,        r,        b,        midPrice,            v,        T,    pT);
    downOption    =    this->price(    oT,        K,        r,        b,        downPrice,            v,        T,        pT);
    greeks    =    (upOption    -    2.0F*    midOption    +        downOption)        /    (S    *S    *    0.0005L    *        0.0005L);
    return    greeks;
}
double    americanDigital::theta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T,const    double    dt,PayoutType    pT){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->price(    oT,        K,        r,        b,        S,            v,        T,        pT);
    downOption    =        this->price(    oT,        K,        r,        b,        S,            v,        T-    dt,        pT);
    greeks    =    (upOption    -    downOption)        /    dt;
    return    greeks;
}
double    americanDigital::vega(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T,PayoutType    pT){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->price(    oT,        K,        r,        b,        S,            v    +    0.001L,        T    ,        pT);
    downOption    =    this->price(    oT,        K,        r,        b,        S,            v    -    0.001L,        T,        pT);
    greeks    =    (upOption    -    downOption)        /    (2.0L    *    0.001L);
    return    greeks;

}
double    americanDigital::rho(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v,const    double    T,PayoutType    pT){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->price(    oT,        K,        r    +    0.0001L,        b,        S,            v        ,        T    ,        pT);
    downOption    =        this->price(    oT,        K,        r,        b,        S,            v    ,        T    ,        pT);
    greeks    =    (upOption    -    downOption)        /    (0.0001L)    /    100;
    return    greeks;

}

void    americanDigital::touchPayoutTag(bool    tag){
    this->m_payoutTag    =    tag;
}




europeanDigital::europeanDigital(void)
{

}


europeanDigital::~europeanDigital(void)
{

}


double    europeanDigital::cashOrNothingPrice(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T){
                double    value    =    0;
                double    d    =    0,greeks    =    0;;

                if(T    <=    0){
                                if(oT    ==    OptionType::CALL){
                                                return    S>K    ?    payAmountK    :    0;
                                }
                                if(oT    ==    OptionType::PUT){
                                                return    S<K    ?    payAmountK    :    0;
                                }
                
                }
                d    =    (std::log(S/K)    +    (b    -    v    *    v    *    0.5)    *    T)    /    (v    *    std::sqrt(T));

                if(oT    ==    OptionType::CALL){
                                            value    =    payAmountK    *    std::exp(-r    *    T    )    *    m_math.N(d,0,1);
                }
                else    if(oT    ==    OptionType::PUT){
                                value    =    payAmountK    *    std::exp(-r    *    T    )    *    m_math.N(-d,0,1);
                }
                return    value;
}
double    europeanDigital::cashOrNothingDelta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T){
    double    upPrice    =        S    +    S    *    0.005L;
    double    downPrice    =S    -    S    *    0.005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,upPrice,    v,T);
    downOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,downPrice,    v,T);
    greeks    =    (upOption    -    downOption)        /    (S    *    2.0L    *        0.005L);
    return    greeks;
}
double    europeanDigital::cashOrNothingGamma(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T){
    double    upPrice    =        S    +    S    *    0.0005L;
    double    midPrice    =    S;
    double    downPrice    =S    -    S    *    0.0005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    double    midOption    =    0;
    upOption    =        this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,upPrice,    v,T);
    midOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,midPrice,    v,T);
    downOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,downPrice,    v,T);
    greeks    =    (upOption    -    2.0F*    midOption    +        downOption)        /    (S    *S    *    0.0005L    *        0.0005L);
    return    greeks;
}
double    europeanDigital::cashOrNothingTheta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T,const    double    dt    ){

    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,S,    v,T    );
    downOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,S,    v,T-    dt);
    greeks    =    (upOption    -    downOption)        /    dt;
    return    greeks;
}
double    europeanDigital::cashOrNothingVega(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,S,    v    +    0.001L,T);
    downOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,S,    v    -    0.001L,T);
    greeks    =    (upOption    -    downOption)        /    (2.0L    *    0.001L);
    return    greeks;
}
double    europeanDigital::cashOrNothingRho(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    payAmountK,const    double    S,    const    double    v,const    double    T){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->cashOrNothingPrice(    oT,    K,r+    0.0001L,    b,payAmountK,S,    v,T);
    downOption    =    this->cashOrNothingPrice(    oT,    K,r,    b,payAmountK,S,    v,T);
    greeks    =    (upOption    -    downOption)        /    (0.0001L)    /    100;
    return    greeks;
}


//    ========================================================华丽分割线=============================================================================

double    europeanDigital::assetOrNothingPrice(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T){
    double    value    =    0;
    double    d    =    0,greeks    =    0;;

    d    =    (std::log(S/K)    +    (b+    v    *    v    *    0.5)    *    T)    /    (v    *    std::sqrt(T));


    if(oT    ==    OptionType::CALL){
        value    =    S    *    std::exp(b    -r    *    T    )    *    this->m_math.N(d,0,1);
    }
    else    if(oT    ==    OptionType::PUT){
        value    =    S    *    std::exp(b    -r    *    T    )    *    this->m_math.N(-d,0,1);
    }
    
    return    value;
}
double    europeanDigital::assetOrNothingDelta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T){
    double    upPrice    =        S    +    S    *    0.005L;
    double    downPrice    =S    -    S    *    0.005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->assetOrNothingPrice(    oT,        K,        r,        b,        upPrice    ,        v,        T);
    downOption    =    this->assetOrNothingPrice(    oT,        K,        r,        b,        downPrice    ,        v,        T);
    greeks    =    (upOption    -    downOption)        /    (S    *    2.0L    *        0.005L);
    return    greeks;
}
double    europeanDigital::assetOrNothingVega(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =            this->assetOrNothingPrice(    oT,        K,        r,        b,        S    ,        v    +    0.001L,        T);
    downOption    =        this->assetOrNothingPrice(    oT,        K,        r,        b,        S    ,        v    -    0.001L,        T);
    greeks    =    (upOption    -    downOption)        /    (2.0L    *    0.001L);
    return    greeks;
}
double    europeanDigital::assetOrNothingTheta(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T,const    double    dt){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =            this->assetOrNothingPrice(    oT,        K,        r,        b,        S    ,        v    ,        T);
    downOption    =        this->assetOrNothingPrice(    oT,        K,        r,        b,        S    ,        v    ,        T    -    dt);
    greeks    =    (upOption    -    downOption)        /    dt;
    return    greeks;
}
double    europeanDigital::assetOrNothingGamma(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T){
    double    upPrice    =        S    +    S    *    0.0005L;
    double    midPrice    =    S;
    double    downPrice    =S    -    S    *    0.0005L;
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    double    midOption    =    0;
    upOption    =            this->assetOrNothingPrice(    oT,        K,        r,        b,        upPrice    ,        v,        T);
    midOption    =    this->assetOrNothingPrice(    oT,        K,        r,        b,        midPrice    ,        v,        T);
    downOption    =        this->assetOrNothingPrice(    oT,        K,        r,        b,        downPrice    ,        v,        T);
    greeks    =    (upOption    -    2.0F*    midOption    +        downOption)        /    (S    *S    *    0.0005L    *        0.0005L);
    return    greeks;
}
double    europeanDigital::assetOrNothingRho(OptionType    oT,const    double    K,const    double    r,const    double    b,const    double    S    ,const    double    v,const    double    T){
        double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;

    upOption    =        this->assetOrNothingPrice(    oT,        K,        r    +    0.0001L,        b,        S    ,        v,        T);
    downOption    =this->assetOrNothingPrice(    oT,        K,        r,        b,        S    ,        v,        T);
    greeks    =    (upOption    -    downOption)        /    (0.0001L)    /    100;
    return    greeks;
}