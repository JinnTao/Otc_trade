
#include "asianPrice.h"
#include <math.h>

#define Pi 3.1415926535898


asianPrice::asianPrice()
{
}


asianPrice::~asianPrice()
{
}

double asianPrice::AsianCurranApprox(OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double dt, my, myi;
 double vxi, vi, vx;
 double Km, sum1, sum2;
 double ti, EA;
 int z;
 long i;
 double price;
 z = 1;
    if(T<= 0){
        if(cpFlag == OptionType::PUT){
            return std::max<double>((X-SA),0);
        }else{
            return std::max<double>((SA-X),0);
        }
    
    }

 if (cpFlag == OptionType::PUT) z = -1;
 dt = (T - t1) / (n - 1);
 if (b == 0){
  EA = S;
 }else{
  EA = S / n * exp(b*t1)*(1 - exp(b*dt*n)) / (1 - exp(b*dt));
 }
 if (m > 0){
  if (SA > n / m *X){
   if (cpFlag ==  OptionType::PUT){
    price = 0;
   }
   else{
    SA = SA * m / n + EA * (n - m) / n;
    price = (SA - X) * exp(-r * T);
   }
   return price;
  }
 }
 if (m == (n - 1)){
  X = n *X - (n - 1) *SA;
  price = GBlackScholes(cpFlag,S,X,T,r,b,v) * 1.0 / n;
  return price;
 }
 if (m > 0){
  X = n / (n - m)* X - m / (n - m)*SA;
 }
 vx = v * sqrt(t1 + dt * (n - 1) * (2 * n - 1) / (6.0 * n));
 my = log(S) + (b - v*v *0.5) * (t1 + (n - 1) * dt / 2.0);
 sum1 = 0;
 for (i = 1; i <= n; i++){
  ti = dt * i + t1 - dt;
  vi = v * sqrt(t1 + (i - 1) * dt);
  vxi = v* v * (t1 + dt * ((i - 1) - i * (i - 1) / (2.0 * n)));
  myi = log(S) + (b - v * v * 0.5) * ti;
  sum1 = sum1 + exp(myi + vxi / (vx * vx) * (log(X) - my) + (vi *vi - vxi * vxi / (vx * vx))  * 0.5);
 }
 Km = 2.0 * X - 1.0 / n *sum1;
 sum2 = 0;
 for (i = 1; i <= n; i++){
  ti = dt * i + t1 - dt;
  vi = v * sqrt(t1 + (i - 1) * dt);
  vxi = v * v * (t1 + dt * ((i - 1) - i  * (i - 1) / (2.0*n)));
  myi = log(S) + (b - v*v *0.5) * ti;
  sum2 = sum2 + exp(myi + vi * vi * 0.5) * CND(z * ((my - log(Km)) / vx + vxi / vx));
 }
 price = exp(-r  * T)*z * (1.0 / n * sum2 - X * CND(z*(my - log(Km)) / vx)) * (n - m) / n;
 return price;
}
double asianPrice::GBlackScholes( OptionType cpFlag, double S, double X, double T, double r, double b, double v){
 double d1, d2;
 double price;
 d1 = (log(S / X) + (b + v*v / 2.0) * T) / (v * sqrt(T));
 d2 = d1 - v *sqrt(T);
 if (cpFlag ==  OptionType::CALL){
  price = S * exp((b - r)*T) * CND(d1) - X * exp(-r * T) * CND(d2);
 }
 else{
  price = X * exp(-r*T) * CND(-d2) - S * exp((b-r)*T) * CND(-d1);
 }
 return price;
}


double asianPrice::CND(double x){
 double L, k;
 double result;
 if (x == 0)
 {
  result = 0.5;
 }
 else{
  const double a1 = 0.31938153;
  const double a2 = -0.356563782;
  const double a3 = 1.781477937;
  const double a4 = -1.821255978;
  const double a5 = 1.330274429;

  L = abs(x);
  k = 1.0 / (1.0 + 0.2316419*L);
  result = 1 - 1.0 / sqrt(2.0 * Pi) * exp(-L * L / 2.0) *
   (a1*k + a2*k*k + a3 *k *k*k + a4 * k * k*k*k + a5 *k *k *k *k*k);
  if (x < 0){
   result = 1 - result;
  }
 }
 return result;
}

double asianPrice::delta( OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double greeks, p1,p2;
 p1 = AsianCurranApprox(cpFlag, S * 1.0005, SA, X, t1, T, n, m, r,  b, v);
 p2 = AsianCurranApprox(cpFlag, S * 0.9995, SA, X, t1, T, n, m, r,  b, v);
 greeks = (p1 - p2) / (0.001 * S);
 return greeks;
}
double asianPrice::vega( OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double greeks, p1,p2;
 p1 = AsianCurranApprox(cpFlag, S, SA, X, t1, T, n, m, r,  b, v* 1.005);
 p2 = AsianCurranApprox(cpFlag, S, SA, X, t1, T, n, m, r,  b, v*0.995);
 greeks = (p1 - p2) / (v * 0.01);
 return greeks;
}
double asianPrice::gamma( OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double greeks, p1,p2,p3;
 p1 = AsianCurranApprox(cpFlag, S * 1.0005, SA, X, t1, T, n, m, r,  b, v);
 p2 = AsianCurranApprox(cpFlag, S , SA, X, t1, T, n, m, r,  b, v);
 p3 = AsianCurranApprox(cpFlag, S * 0.9995, SA, X, t1, T, n, m, r,  b, v);
 greeks = (p1 - 2*p2 + p3) / (0.0005 * S * 0.0005 * S);
 return greeks;
}
double asianPrice::rho( OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double greeks, p1,p2;
 p1 = AsianCurranApprox(cpFlag, S, SA, X, t1, T, n, m, r * 1.005,  b, v);
 p2 = AsianCurranApprox(cpFlag, S, SA, X, t1, T, n, m, r * 0.995,  b, v);
 greeks = (p1 - p2) / (0.01 * r);
 return greeks;
}
double asianPrice::theta( OptionType cpFlag, double S, double SA, double X, double t1, double T, double n, double m, double r, double b, double v){
 double greeks, p1,p2;
 p1 = AsianCurranApprox(cpFlag, S, SA, X, t1, T, n, m, r,  b, v);
 p2 = AsianCurranApprox(cpFlag, S, SA, X, t1, T - 1.0/365.0, n, m, r,  b, v);
 greeks = (p1 - p2) / (1.0/ 365);
 return greeks;
}