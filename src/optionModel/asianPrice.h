#pragma once

#include "structure.h"
using namespace optionSpace;
//enum CallPutFlag{CALL,PUT};
class DLL_API_CLASS asianPrice
{
 public:
 asianPrice();
 // n : totalPoint m : AveragedPoint t1:timeToFisrtAveragePoint
 double AsianCurranApprox(OptionType t , double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
 double delta(OptionType, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
 double vega(OptionType, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
 double gamma(OptionType, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
 double rho(OptionType, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
 double theta(OptionType, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v);
  
 ~asianPrice();
 double GBlackScholes(OptionType, double S, double X, double T, double r, double b, double v);

 double CND(double);
private:

};


