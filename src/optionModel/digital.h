#pragma once
#include "structure.h"
#include "algorithmLib.h"
#include <iostream>
 using namespace optionSpace;
class  DLL_API_CLASS europeanDigital
{
public:
 europeanDigital(void);

 ~europeanDigital(void);

 double cashOrNothingPrice(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T);
 double cashOrNothingDelta(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T);
 double cashOrNothingVega(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T);
 double cashOrNothingTheta(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T,const double dt);
 double cashOrNothingGamma(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T);
 double cashOrNothingRho(OptionType oT,const double K,const double r,const double b,const double payAmountK,const double S, const double v,const double T);

 double assetOrNothingPrice(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T);
 double assetOrNothingDelta(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T);
 double assetOrNothingVega(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T);
 double assetOrNothingTheta(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T,const double dt);
 double assetOrNothingGamma(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T);
 double assetOrNothingRho(OptionType oT,const double K,const double r,const double b,const double S ,const double v,const double T);
private:
 algorithmLib m_math;

};



class DLL_API_CLASS americanDigital
{
public:
 americanDigital(void);

 ~americanDigital(void);
 double price(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,PayoutType pT);
 double delta(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,PayoutType pT);
 double gamma(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,PayoutType pT);
 double theta(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,const double dt,PayoutType pT);
 double vega(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,PayoutType pT);
 double rho(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T, PayoutType pT);
 void touchPayoutTag(bool);
private:
 algorithmLib m_math;
 europeanDigital m_eDigital;
 bool m_payoutTag;

};
