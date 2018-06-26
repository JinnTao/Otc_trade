#include "barrierPrice.h"


double barrierPrice:: price(OptionType oT,const double K,const double r,const double b,const double S, const double v,
       const double T,double barrier,double rebate,PayoutType rebateStyle ){

 double rebateValue = 0;
 double greeks = 0,value = 0; 
 OptionType OTType;

 if(oT == OptionType::CallDownIn || oT == OptionType::CallDownOut || oT == OptionType::PutDownOut || oT == OptionType::PutDownIn){
  OTType = OptionType::PUT;
  //putupout putdownin calldownin callupout
 }else{
  // callupin putupin putdownout calldownout
  OTType = OptionType ::CALL;
 }

 if(oT == OptionType::CallDownIn || oT == OptionType::CallUpIn || oT == OptionType::PutDownIn || oT == OptionType::PutUpIn){
  rebateStyle = PayoutType::AtExpiry; // In style only at Expiry 
 }
 if(rebateStyle == PayoutType::AtExpiry){
  if(oT == OptionType::CallDownIn || oT == OptionType::CallUpIn || oT == OptionType::PutUpIn || oT == OptionType::PutDownIn){
   rebateValue = rebate * std::exp(-r * T)- rebate * this->m_aDigital.price(OTType,barrier,r,b,S,v,T,PayoutType::AtExpiry);
  }
  else{
   rebateValue = rebate *  this->m_aDigital.price(OTType,barrier,r,b,S,v,T,PayoutType::AtExpiry);
  }
 }

 if(rebateStyle == PayoutType::Immediate){
  if(oT == OptionType::CallDownIn || oT == OptionType::CallUpIn || oT == OptionType::PutUpIn || oT == OptionType::PutDownIn){
   rebateValue = rebate -  rebate * this->m_aDigital.price(OTType,barrier,r,b,S,v,T,PayoutType::Immediate);
  }else{
   rebateValue = rebate *  this->m_aDigital.price(OTType,barrier,r,b,S,v,T,PayoutType::Immediate);
  }
 }
 value = BarrierStaticReplication( oT,  K, r, b, S, v, T,barrier) + rebateValue ;

 return value;
}

barrierPrice::~barrierPrice(void)
{
}
double barrierPrice::BarrierStaticReplication(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,const double B ){
 
 
 double greeks = 0, lamudaPositive = 0 , lamudaNegative = 0,k = 0,k_ = 0,alpha = 0 , beta = 0;
 double value;
 double barrieStrike =  B * B / K ; // B^2 / K 
 double barrieMutiple = K / B;
 double rebateValue = 0;


 switch (oT)
 {
  case CALL:
   break;
  case PUT:
   break;
  case CallUpIn:
   if(B > S && B > K && !this->m_touchBarrier){
    value = barrieMutiple * this->m_ePrice.price(CALL,barrieStrike,r,b,S,v,T) + (B - K) * this->m_aDigital.price(CALL,B,r,b,S,v,T,PayoutType::AtExpiry);
   }else{
    value = this->m_ePrice.price(CALL,K,r,b,S,v,T);
   }
   break;
  case CallUpOut:
   if(B > S && !this->m_touchBarrier){
    value = this->m_ePrice.price(CALL,K,r,b,S,v,T) - barrieMutiple * this->m_ePrice.price(CALL,barrieStrike,r,b,S,v,T) - 
     (B - K) * this->m_aDigital.price(CALL,B,r,b,S,v,T,PayoutType::AtExpiry);  
   }else{
    value = 0;
   }
   break;
  case CallDownIn:
   if(S > B && !this->m_touchBarrier){
    //value = barrieMutiple * this->EuropeanCalculate(OptionType::PUT,Greeks::PRICE,barrieStrike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  +
    //rebateValue;
    value = BarrierStaticReplication(PutDownIn,K,r,b,S,v,T,B) - (K - B) * this->m_aDigital.price(PUT,B,r,b,S,v,T,PayoutType::AtExpiry);
   }else{
    value = this->m_ePrice.price(CALL,K,r,b,S,v,T);
   }
   break;
  case CallDownOut:
   if(S > B && !this->m_touchBarrier){
    //value = this->EuropeanCalculate(OptionType::CALL,Greeks::PRICE,_strike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  - 
    //barrieMutiple * this->EuropeanCalculate(OptionType::PUT,Greeks::PRICE,barrieStrike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  +
    //rebateValue;
    value = this ->m_ePrice.price(CALL,K,r,b,S,v,T) - BarrierStaticReplication(CallDownIn,K,r,b,S,v,T,B);
   }else{
    value = 0;
   }
   break;
  case PutUpIn:
   if(B > S && !this->m_touchBarrier){
    //value = barrieMutiple * this->EuropeanCalculate(OptionType::CALL,Greeks::PRICE,barrieStrike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  +
    //rebateValue;
    value = BarrierStaticReplication(CallUpIn,K,r,b,S,v,T,B) - 
     (B- K ) * this->m_aDigital.price(CALL,B,r,b,S,v,T,PayoutType::AtExpiry);
   }else{
    value = this->m_ePrice.price(PUT,K,r,b,S,v,T);
   }

   break;
  case PutUpOut:
   if(B > S && !this->m_touchBarrier){
    //value =  this->EuropeanCalculate(OptionType::PUT,Greeks::PRICE,_strike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  -
    //barrieMutiple * this->EuropeanCalculate(OptionType::CALL,Greeks::PRICE,barrieStrike,_riskFreeRate,_dividendYield,_underlyingPrice,_vol,_elapsedTime,_remainTime)  +
    //rebateValue;
    value = this->m_ePrice.price(PUT,K,r,b,S,v,T) - BarrierStaticReplication(PutUpIn,K,r,b,S,v,T,B);
   }else{
    value = 0;
   }

   break;
  case PutDownIn:
   if(S > B && B < K && !this->m_touchBarrier){
    value = barrieMutiple * this->m_ePrice.price(PUT,barrieStrike,r,b,S,v,T)  + 
     (K - B ) * this->m_aDigital.price(PUT,B,r,b,S,v,T,PayoutType::AtExpiry);
   }else{
    value = this->m_ePrice.price(PUT,K,r,b,S,v,T);
   }

   break;
  case PutDownOut:
   if(S > B  && !this->m_touchBarrier){
    value = this->m_ePrice.price(PUT,K,r,b,S,v,T) - 
     barrieMutiple * this->m_ePrice.price(PUT,barrieStrike,r,b,S,v,T) - (K - B) * this->m_aDigital.price(PUT,B,r,b,S,v,T,PayoutType::AtExpiry);
   }
   else{
    value = 0;
   }
   break;
  default:
   break;
 }
 if(value < 0){
  value = 0;
 }
 return value;
}

double barrierPrice::delta(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle){
    this->checkBarrier( S, barrier);
 double greeks, p1,p2;
 p1 = price( oT,  K,  r,  b,  S * 1.0005,   v,T, barrier, rebate, rebateStyle );
 p2 = price( oT,  K,  r,  b,  S * 0.9995,   v, T, barrier, rebate, rebateStyle );
 greeks = (p1 - p2) / (0.001 * S);
 return greeks;
}
double barrierPrice::vega(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle ){ 
 this->checkBarrier( S, barrier);
 double greeks, p1,p2;
 p1 = price( oT,  K,  r,  b,  S ,   v + 0.001,T, barrier, rebate, rebateStyle );
 p2 = price( oT,  K,  r,  b,  S ,   v, T, barrier, rebate, rebateStyle   );
 greeks = (p1 - p2) / (0.001) /100.0;
 return greeks;
}
double barrierPrice::theta(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle){
 this->checkBarrier( S, barrier);
 double greeks, p1,p2;
 p1 = price( oT,  K,  r,  b,  S ,   v,T , barrier, rebate, rebateStyle   );
 p2 = price( oT,  K,  r,  b,  S ,   v,T - 1.0/252.0, barrier, rebate, rebateStyle   );
 greeks = (p1 - p2) / (1.0/252.0)/252.0;
 return greeks;
}
double barrierPrice::gamma(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle){
 this->checkBarrier( S, barrier);
 double greeks, p1,p2,p3;
 p1 = price( oT,  K,  r,  b,  S * 1.0005,   v,T , barrier, rebate, rebateStyle   );
 p2 = price( oT,  K,  r,  b,  S ,   v,T, barrier, rebate, rebateStyle   );
 p3 = price( oT,  K,  r,  b,  S * 0.9995,   v,T , barrier, rebate, rebateStyle );
 greeks = (p1 - 2.0*p2 + p3) / (0.0005 * 0.0005 * S * S);
 return greeks;
}
double barrierPrice::rho(OptionType oT,const double K,const double r,const double b,const double S, const double v,const double T,double barrier,double rebate,PayoutType rebateStyle){
 this->checkBarrier( S, barrier);
 double greeks, p1,p2;
 p1 = price( oT,  K,  r + 0.001L,  b,  S ,   v,T, barrier, rebate, rebateStyle);
 p2 = price( oT,  K,  r,  b,  S ,   v,T, barrier, rebate, rebateStyle );
 greeks = (p1 - p2) / 0.001L / 100.0;
 return greeks;
} 
void barrierPrice::checkBarrier(double S,double B){
 if( abs(S - B) < abs(1E-12)){
  this->m_touchBarrier = true;
  this->m_aDigital.touchPayoutTag(true);
 }
}