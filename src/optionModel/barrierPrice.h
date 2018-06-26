#pragma once
#include "structure.h"
#include "europeanPrice.h"
#include "digital.h"
using namespace optionSpace;
class DLL_API_CLASS barrierPrice
{
public:


 //Barier Option
 barrierPrice(){this->m_touchBarrier = false;};
 // barrier Static Replication 
 double price(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle);
 double delta(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle );
 double vega(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle);
 double theta(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle);
 double gamma(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle);
 double rho(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle);

 double BarrierStaticReplication(OptionType oT,const double K,const double r,const double y,const double S, const double v,const double T,const double barrier);

 ~barrierPrice(void);
private:
 europeanPrice m_ePrice;;
 americanDigital m_aDigital;
 bool m_touchBarrier;
protected:
 void checkBarrier(double S,double B);
};

