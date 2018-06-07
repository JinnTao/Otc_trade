#include    "americanPrice.h"

americanPrice::americanPrice(void)
{
}


americanPrice::~americanPrice(void)
{
}

double    americanPrice::price    (OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        ){
        double    dt    =    T    /    double(step);
        //    check    dt
        double    dtCondition    =    v*v    /    ((r    -    b)*(r    -    b));
        while(dt    >    dtCondition){
            step    +=    100;
        dt    =        T    /    double(step);
    }
        double    u    =    0,    p    =    0,d    =    0,    rf    =    0,direction    =    -1.0F;
        int    j    =    0,i    =    0;
        double    optionValue,neihanValue,p0,    p1;
        double    EuropeanBinomial    =    0,EuropeanAnalytic,AmericanBinomial,AmericanAnalytic;
        double        discount    =    std::exp(-r*dt);
        vector<double>    AOx(step+1,-1);
        vector<double>    EOx(step+1,-1);
        
                        rf    =    b;
        u    =    std::exp(v    *    std::sqrt(dt));
        d    =    1.0    /    u;
        p    =    (std::exp(rf    *    dt)    -    d)    /    (u    -    d);
        p0    =    discount    *    p;
        p1    =    discount        -    p0;


        if(oT    ==    OptionType::CALL){
        direction    =    1.0F;
        }
    for(    i    =    0;    i    <=    step;    i++){
        AOx[i]    =    max((S    *    std::pow(u,double(2*i    -    step))    -    K)    *    direction,0.0);
        EOx[i]    =    AOx[i];
    }
    for(    j    =    step-1;j    >=    0    ;    j--){
        for(    i    =    0    ;    i    <=    j;    i    ++){
            optionValue    =    (p1    *    AOx[i]    +    p0*    AOx[i+1]);
            neihanValue    =        (S    *    std::pow(u,double(2*i    -    j))    -    K)*direction;
            AOx[i]    =    max(neihanValue,optionValue);
            EOx[i]    =        (p1    *    EOx[i]    +        p0    *    EOx[i+1]);
        }
    }
    EuropeanBinomial    =    EOx[0];
    EuropeanAnalytic    =    this->m_europeanPricing.price(oT,K,r,b,S,v,T);    
    AmericanBinomial    =    AOx[0];
    AmericanAnalytic    =    EuropeanAnalytic    +    (AmericanBinomial    -    EuropeanBinomial);    //control    variate    methods;
    
    return    AmericanAnalytic;
    }

double    americanPrice::delta(OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        ){
    double    dt    =    T    /    double(step);
    double    u=exp(v*sqrt(dt));
    double    d=exp(-v*sqrt(dt));
    double    p_up    =    (1-d)/(u-d);
                double    p_down    =    1.0    -    p_up;
    double    price1_u    =    price(    oT,            K,        r,        b,        S    *    u,            v        ,        T    *    (step    -    1)    /    step,        step    );
    double    price1_d    =    price(    oT,            K,        r,        b,        S    *    d,            v        ,        T    *    (step    -    1)    /    step,        step    );
    double    price0=    price(    oT,            K,        r,        b,        S    ,            v        ,        T    *    (step    -    1)    /    step,        step    );

    double    delta_up=(price1_u-price0)/(S*u-S);
    double    delta_down=(price1_d-price0)/(S*d-S);
    double    delta=p_up*delta_up+p_down*delta_down;
    return    delta;
}
double    americanPrice::vega(OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        ){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =            price(    oT,            K,        r,        b,        S    ,            v    +    0.001L    ,        T    ,        step    );
    downOption    =        price(    oT,            K,        r,        b,        S    ,            v        -    0.001L,        T    ,        step    );
    greeks    =    (upOption    -    downOption)        /    (2.0L    *        0.001L);
    return    greeks;
}
double    americanPrice::theta(OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,const    double    dt,    int    step    ){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =            price(    oT,            K,        r,        b,        S    ,            v    ,    T        ,        step    );
    downOption    =        price(    oT,            K,        r,        b,        S    ,            v    ,        T-    dt    ,        step    );
    greeks    =    (upOption    -    downOption)        /    (    dt);
    return    greeks;
}
double    americanPrice::rho(OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step        ){
    double    greeks=    0;
    double    upOption    =    0;    
    double    downOption    =    0;
    upOption    =            price(    oT,            K,        r    +    0.0001L,        b,        S    ,            v    ,    T        ,        step    );
    downOption    =        price(    oT,            K,        r    ,        b,        S    ,            v    ,    T        ,        step    );
    greeks    =    (upOption    -    downOption)        /    (0.0001L)    /    100;
    return    greeks;
}
double    americanPrice::gamma(OptionType    oT,    const    double    K,const    double    r,const    double    b,const    double    S,    const    double    v        ,const    double    T,    int    step    ){
    double    dt    =    T    /    double    (step);
    double    u=exp(v    *    sqrt(dt));
    double    d=exp(-v*sqrt(dt));
    double    p_up    =    (1-d)/(u-d);
                double    p_down    =    1.0    -    p_up;

    double    delta1_u=    delta(    oT,            K,        r,        b,        S    *    u,            v        ,        T    *    (step    -    1)    /    step,        step        );;
    double    delta1_d=    delta(    oT,            K,        r,        b,        S    *    d,            v        ,        T    *    (step    -    1)    /    step,        step        );
    double    delta0    =    delta(    oT,            K,        r,        b,        S,            v        ,        T,        step        );
    double    gamma_u=(delta1_u-delta0)/(S*u-S);
    double    gamma_d=(delta1_d-delta0)/(S*d-S);
    double    gamma=p_up*gamma_u+p_down*gamma_d;
    return    gamma;
}


//    =========================================»ªÀö·Ö¸îÏß=================================================================
double    bjsmodel::BSAmericanCallApprox2002(double    S,    double        X,    double    T,    double    r,    double    b,    double    v)
{
    double    BInfinity,    B0,    ht1,    ht2,    I1,    I2,    alfa1,    alfa2,    Beta,    t1;
    double    BSAmericanApprox2002;
    t1=0.5*(sqrt(5.0)-1)*T;
    if(b>=r)
        BSAmericanApprox2002=GBlackScholes('c',    S,    X,    T,r,    b,    v);
    else
    {
        Beta=(0.5-b/(v*v))+sqrt(    (b/(v*v)-0.5)*(b/(v*v)-0.5)+2*r/(v*v));
                    BInfinity=Beta*X/(Beta-1.0);
                    B0=Max(X,    r*X/(r-b));
    
                    ht1=-(b*t1+2*v*sqrt(t1))*    X*X/((BInfinity-B0)*B0);
                    ht2=-(b*T+2*v*sqrt(T))*    X*X/((BInfinity-B0)*B0);
                    I1=    B0+(BInfinity-B0)*    (1-exp(ht1));
                    I2=    B0+(BInfinity-B0)*    (1-exp(ht2));
                    alfa1=(I1-X)*    pow(I1,-Beta);
                    alfa2=(I2-X)*    pow(I2,-Beta);
        if(S>=    I2)
            BSAmericanApprox2002=    S-X;
        else
            BSAmericanApprox2002=    alfa2*    pow(S,    Beta)-    alfa2*    phi(S,    t1,    Beta,    I2,    I2,    r,    b,    v)
                                +    phi(S,    t1,    1.0,    I2,    I2,    r,    b,    v)-    phi(S,    t1,    1.0,    I1,    I2,    r,    b,    v)
                    -X*    phi(S,    t1,    0.0,    I2,    I2,    r,    b,    v)+    X*    phi(S,    t1,    0,    I1,    I2,    r,    b,    v)
                    +alfa1*    phi(S,    t1,    Beta,    I1,    I2,    r,    b,    v)-    alfa1*    ksi(S,    T,    Beta,    I1,    I2,    I1,    t1,    r,    b,    v)
                    +ksi(S,    T,    1.0,    I1,    I2,    I1,    t1,    r,    b,    v)-ksi(S,    T,    1.0,    X,    I2,    I1,    t1,    r,    b,    v)
                    -X*    ksi(S,    T,    0.0,    I1,    I2,    I1,    t1,    r,    b,    v)+    X*    ksi(S,    T,    0.0,    X,    I2,    I1,    t1,    r,    b,    v);
    }
    return    BSAmericanApprox2002;
}

double    bjsmodel::ksi(double    S,    double    T2,    double    gamma,    double    h,double    I2,    double    I1,    double    t1,    double    r,    double    b,    double    v)
{
    double    e1,    e2,    e3,    e4,    f1,    f2,    f3,    f4,    rho,    kappa,    lambda;
    double    ksi;
    e1=(log(S/I1)+(b+(gamma-0.5)*(v*v))*t1)/(v*sqrt(t1));
    e2=(log((I2*I2)/(S*I1))+(b+(gamma-0.5)*v*v)*t1)/(v*sqrt(t1));
    e3=(log(S/I1)-(b+(gamma-0.5)*(v*v))*t1)/(v*sqrt(t1));
    e4=(log((I2*I2)/(S*I1))-(b+(gamma-0.5)*v*v)*t1)/(v*sqrt(t1));
    f1=(log(S/h)+(b+(gamma-0.5)*(v*v))*T2)/(v*sqrt(T2));
    f2=(log((I2*I2)/(S*h))+(b+(gamma-0.5)*(v*v))*T2)/(v*sqrt(T2));
    f3=(log((I1*I1)/(S*h))+(b+(gamma-0.5)*(v*v))*T2)/(v*sqrt(T2));
    f4=(log((I1*I1*S)/(h*I2*I2))+(b+(gamma-0.5)*(v*v))*T2)/(v*sqrt(T2));

    rho=sqrt(t1/T2);
    lambda=-r+gamma*b+0.5*gamma*(gamma-1.0)*v*v;
    kappa=2.0*    b/(v*v)+(2*    gamma    -1);

    ksi=    exp(lambda*    T2)*pow(S,    gamma)*(    CBND(-e1,    -f1,    rho)
                -pow((I2    /S),    kappa)*CBND(-e2,    -f2,    rho)
                -pow((I1    /S),    kappa)*CBND(-e3,    -f3,    -rho)
                +pow((I1/    I2),    kappa)*    CBND(-e4,    -f4,    -rho));
    return    ksi;
}

double    bjsmodel::phi(double    S,    double    T,    double    gamma,    double    h,    double    i,    double    r,    double    b,    double    v)
{
    double    lambda,    kappa,    d;
    double    phi;
    lambda=(-r+gamma*    b+    0.5*    gamma    *    (gamma    -    1.0)*v*v    )*T;
    d=    -(log(S/h)+(b+(gamma    -0.5)*v*v)*T)/(v*sqrt(T));
    kappa=    2.0*    b/(v*v)+2.0*    gamma    -    1.0;
    phi    =    exp(lambda)*    pow(S,    gamma)*    (CND(d)-pow((i/S),    kappa)*CND(d-    2.0*log(i/S)/(v*sqrt(T))));
    return    phi;
}

double    bjsmodel::BSAmericanApprox2002(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)
{
    double    BSAmericanApprox2002;
    if(    CallPutFlag=='c')
                                    BSAmericanApprox2002=BSAmericanCallApprox2002(S,    X,    T,    r,    b,    v);
    else    
        BSAmericanApprox2002=BSAmericanCallApprox2002(X,    S,    T,    r-b,    -b,    v);
    return    BSAmericanApprox2002;
}
double    bjsmodel::Max(double    a,    double    b)
{
    if(a>b)
        return    a;    
    else
        return    b;
}
double    bjsmodel::GBlackScholes(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)
{
    double    d1,    d2;
    double    GBlackScholes;
    d1=    (log(S/X)    +    (b+    v*v/2.0)*T)    /    (    v*    sqrt(T));
                d2=d1-    v*sqrt(T);
    if(CallPutFlag=='c')
        GBlackScholes=    S*    exp((b-r)*T)*CND(d1)-X*exp(-r*T)*CND(d2);
    else
                                GBlackScholes=    X*    exp(-r*    T)    *    CND(-d2)    -    S*    exp((b-r)    *    T)*CND(-d1);
    return    GBlackScholes;
}


double    bjsmodel::CND(double    x)
{
    double    gamma    =    0.2316419;                    double    a1    =    0.319381530;
    double    a2    =    -0.356563782;                    double    a3    =    1.781477937;
    double    a4    =    -1.821255978;                    double    a5    =    1.330274429;
    double    pi    =    4.0*atan(1.0);                double    k    =    1.0    /    (1.0    +    gamma*x);
    if    (x    >=    0.0)
    {
        return    1.0    -    ((((a5*k    +    a4)*k    +    a3)*k    +    a2)*k    +    a1)
            *k*exp(-x*x    /    2.0)    /    sqrt(2.0*pi);
        
    }
    else    
            //return    1.0    -    CND(-x);
    {
                    k=    1.0    /    (1.0    +    -gamma*x);
            return    ((((a5*k    +    a4)*k    +    a3)*k    +    a2)*k    +    a1)
            *k*exp(-x*x    /    2.0)    /    sqrt(2.0*pi);
    }

}

double    bjsmodel::CBND(double    X,    double    y,    double    rho)
{
                int    i,    ISs,        LG,    NG;
                double    XX[10][3];
                double    W[10][3];
                double    h,    k,    hk,    hs;
                double    BVN,    Ass,    asr,    sn;
                double    A,    b,    bs,    c,    d;
                double    xs,    rs;
    double    pi    =    4.0*atan(1.0);
    W[0][0]=    0.17132449237917;
    XX[0][0]=    -0.932469514203152;
                W[1][0]    =    0.360761573048138;
                XX[1][0]=-0.661209386466265;    
                W[2][0]    =    0.46791393457269;    
    XX[2][0]    =    -0.238619186083197;
                W[0][1]=    0.0471753363865118;
                XX[0][1]=-0.981560634246719;
                W[1][1]=    0.106939325995318;    
                XX[1][1]    =    -0.904117256370475;
                W[2][1]    =    0.160078328543346;
                XX[2][1]=-    0.769902674194305;    
                W[3][1]=    0.203167426723066;    
                XX[3][1]=    -0.587317954286617;
                    W[4][1]    =    0.233492536538355;    
                XX[4][1]    =    -0.36783149899818;
                W[5][1]    =    0.249147045813403;
                XX[5][1]    =    -0.125233408511469;
                W[0][2]    =0.0176140071391521;    
                XX[0][2]    =-0.993128599185095;    
                W[1][2]=    0.0406014298003869;
                XX[1][2]    =    -0.963971927277914;    
                W[2][2]=    0.0626720483341091;
                XX[2][2]=    -0.912234428251326;    
                W[3][2]=    0.0832767415767048;
                XX[3][2]=    -0.839116971822219;
                W[4][2]=0.10193011981724;
                XX[4][2]=-0.746331906460151;
                W[5][2]=    0.118194531961518;
                XX[5][2]=    -0.636053680726515;
            W[6][2]    =    0.131688638449177;    
            XX[6][2]    =    -0.510867001950827;
                W[7][2]=    0.142096109318382;    
                XX[7][2]    =-    0.37370608871542;    
                    W[8][2]=0.149172986472604;
                XX[8][2]    =    -0.227785851141645;    
    W[9][2]    =    0.152753387130726;
                XX[9][2]        =    -0.0765265211334973;
                if(abs(rho)<0.3)
    {
        NG=1;
        LG=3;
    }
    else
    {
    if(abs(rho)<0.75)
    {
        NG=2;
        LG=6;
    }
    else
    {
                NG=3;
                LG=10;
    }
    }

    h=    -X;
    k=    -y;
    hk=h*k;
    BVN=0;
    if(abs(rho)<0.925)
    {
        if(abs(rho)>0)
        {
            hs=(h*h+k*k)/2.0;
            asr=    asin(rho);
            for(i=1;    i<LG+1;    i++)
            {
                for(ISs=-1;    ISs<2;    ISs=ISs+2)
                {
                    sn=sin(asr*(ISs*XX[i-1][NG-1]+1.0)/2.0);
                    BVN=BVN+W[i-1][NG-1]*exp((sn*hk-hs)/(1-sn*sn));
                }
            }
            BVN=    BVN*asr/(4*pi);
        }
        BVN=    BVN    +CND(-h)    *CND(-k);
    }
    else
    {
        if(rho<0)
        {
            h=-k;
                        hk=-hk;
        }
        if(abs(rho)<1.0)
        {
            Ass=(1.0-rho)*(1.0+rho);
            A=sqrt(Ass);
            bs=(h-k)*(h-k);
            c=(4.0-hk)/8.0;
            d=(12.0-hk)/16.0;
            asr=-(bs/Ass+hk)/2.0;
                if(asr>-100)
                    BVN=A*exp(asr)*(1-c*(bs-Ass)*(1-d*bs/5.0)/3.0+c*d*Ass*Ass/5.0);
                if(-hk<100)
                {
                    b=sqrt(bs);
                    BVN=BVN-exp(-hk/2)*sqrt(2*pi)*CND(-b/A)*b*(1-c*bs*(1-d*bs/5)/3);
                }
                A=A/2.0;
                for(i=1;i<LG+1;i++)
                {
                    for(ISs=-1;    ISs<2;    ISs=ISs+2)
                    {
                        xs=(A*(ISs*XX[i-1][NG-1]+1))*(A*(ISs*XX[i-1][NG-1]+1));
                        rs=sqrt(1-xs);
                        asr=-(bs/xs+hk)/2.0;
                        if(asr>100)
                            BVN=BVN+A*W[i-1][NG-1]*exp(asr)*(exp(-hk*(1-rs)/(2*(1+rs)))/rs-(1+c*xs*(1+d*xs)));
                    }
                }
                BVN=-BVN/(2*pi);
        }
        if(rho>0)
            BVN=BVN+CND(-Max(h,k));
        else
        {
            BVN=-BVN;
        if(k>h)
            BVN=BVN+CND(k)-CND(h);
        }

    }
    return    BVN;
    }

                        
double    bjsmodel::vega(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m)    
{
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r,    b,    v+v*5.0/m);
    double    price=BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r,    b,    v-v*5.0/m);
    double    vega=(price1-price)/(v*10.0/m);
    return    vega;
}

double    bjsmodel::ImpliedVol(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    cm,    double    epsilon,    int    m)
{
    double    vi,    ci,    vegai,    minDiff;
    int    l=1;
    vi=sqrt(abs(log(S/X)+r*T)*2/T);
    ci=BSAmericanApprox2002(CallPutFlag,    S,    X,    T,    r,    b,    vi);
    vegai=vega(CallPutFlag,    S,    X,    T,    r,    b,    vi,    m);
    minDiff=abs(cm-ci);

    while    ((abs(cm-ci)    >=epsilon)&&(abs(cm-ci)<=    minDiff))
    {
        vi=vi-(ci-cm)/vegai;
        ci=BSAmericanApprox2002(CallPutFlag,    S,    X,    T,    r,    b,    vi);
        vegai=vega(CallPutFlag,    S,    X,    T,    r,    b,    vi,m);
        minDiff=abs(cm-ci);
        l++;
        if(l==1000)    break;
    }
    if(abs(cm-ci)<epsilon)
        return    vi;
    else
        return    -1;
}

double    bjsmodel::delta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m)    
{
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S*(1.0+5.0/m),        X,    T,    r,    b,    v);
    double    price=BSAmericanApprox2002(CallPutFlag,    S*(1.0-5.0/m),        X,    T,    r,    b,    v);
    double    delta=(price1-price)/(S*10.0/m);
    return    delta;
}

double    bjsmodel::gamma(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m)    
{
    double    price1=    delta(CallPutFlag,    S*(1.0+5.0/m),        X,    T,    r,    b,    v,    m);
    double    price=delta(CallPutFlag,    S*(1.0-5.0/m),        X,    T,    r,    b,    v,    m);
    double    gamma=(price1-price)/(S*10.0/m);
    return    gamma;
}

double    bjsmodel::rho(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v,    int    m)    
{
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r*(1.0+5.0/m),    b,    v);
    double    price=BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r*(1.0-5.0/m),    b,    v);
    double    rho=(price1-price)/(r*10.0/m);
    return    rho/100.0;
}

double    bjsmodel::theta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    t,    double    r,    double    b,    double    v)    
{
    double    price_carry=    BSAmericanApprox2002(CallPutFlag,    S,        X,T-t,    r,    b,    v);
    double    price=BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r,    b,    v);
    double    theta=price-price_carry;
    return    theta;
}


double    bjsmodel::vega(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)    
{
    int    m=1000;
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r,    b,    v+v*5.0/m);
    double    price=BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r,    b,    v-v*5.0/m);
    double    vega=(price1-price)/(v*10.0/m);
    return    vega;
}

double    bjsmodel::ImpliedVol(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    cm)
{
    int    l=1;
                double    epsilon=10e-4;
    int    m=1000;
    double    vi,    ci,    vegai,    minDiff;
    vi=sqrt(abs(log(S/X)+r*T)*2/T);
    ci=BSAmericanApprox2002(CallPutFlag,    S,    X,    T,    r,    b,    vi);
    vegai=vega(CallPutFlag,    S,    X,    T,    r,    b,    vi,    m);
    minDiff=abs(cm-ci);

    while    ((abs(cm-ci)    >=epsilon)&&(abs(cm-ci)<=    minDiff))
    {
        vi=vi-(ci-cm)/vegai;
        ci=BSAmericanApprox2002(CallPutFlag,    S,    X,    T,    r,    b,    vi);
        vegai=vega(CallPutFlag,    S,    X,    T,    r,    b,    vi,m);
        minDiff=abs(cm-ci);
        l++;
        if(l==1000)    break;
    }
    if(abs(cm-ci)<epsilon)
        return    vi;
    else
        return    -1;
}

double    bjsmodel::delta(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)    
{
    int    m=1000;
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S*(1.0+5.0/m),        X,    T,    r,    b,    v);
    double    price=BSAmericanApprox2002(CallPutFlag,    S*(1.0-5.0/m),        X,    T,    r,    b,    v);
    double    delta=(price1-price)/(S*10.0/m);
    return    delta;
}

double    bjsmodel::gamma(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)    
{
    int    m=10000;
    double    price1=    delta(CallPutFlag,    S*(1.0+5.0/m),        X,    T,    r,    b,    v,    m);
    double    price=delta(CallPutFlag,    S*(1.0-5.0/m),        X,    T,    r,    b,    v,    m);
    double    gamma=(price1-price)/(S*10.0/m);
    return    gamma;
}

double    bjsmodel::rho(char    CallPutFlag,    double    S,    double        X,    double    T,    double    r,    double    b,    double    v)    
{
    int    m=1000;
    double    price1=    BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r*(1.0+5.0/m),    b,    v);
    double    price=BSAmericanApprox2002(CallPutFlag,    S,        X,    T,    r*(1.0-5.0/m),    b,    v);
    double    rho=(price1-price)/(r*10.0/m);
    return    rho/100.0;
}


double*    bjsmodel::bjsmodelresult(char    CallPutFlag,    double    S,    double        X,    double    T,double    t,    double    r,    double    b,    double    ImpliedVol,    int    m,    double    epsilon)
{
    double    *p    =    new    double[16];


                                    p[0]=ImpliedVol;
                        //price
            p[1]    =    BSAmericanApprox2002(CallPutFlag,S,X,T,r,b,p[0]);
            //price(S+)
            p[2]    =    BSAmericanApprox2002(CallPutFlag,S*(1.0+5.0/m),X,T,r,b,p[0]);
            //price(S-)
            p[3]    =    BSAmericanApprox2002(CallPutFlag,S*(1.0-5.0/m),X,T,r,b,p[0]);
            //price(v+)    
            p[4]    =    BSAmericanApprox2002(CallPutFlag,S,X,T,r,b,p[0]*(1.0+5.0/m));
            //price(T-t)
            p[5]    =    BSAmericanApprox2002(CallPutFlag,S,X,(T-t),r,b,p[0]);
            //price(r+)
            p[6]    =    BSAmericanApprox2002(CallPutFlag,S,X,T,r*(1.0+5.0/m),b,p[0]);

            //delta
            p[7]    =    (p[2]-p[3])/(S*10.0/m);
            //gamma
            p[8]=(p[2]+p[3]-2.0*p[1])/(S*5.0/m)/(S*5.0/m);
            //vega
            p[9]=(p[4]-p[1])/(p[0]*5.0/m);
            //theta
            p[10]=p[5]-p[1];
            //rho
            p[11]=(p[6]-p[1])/((r*5.0/m)*100.0);
                                                return    p;
}
    