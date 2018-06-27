#include "asianBarrier.h"


asianBarrier::asianBarrier(void)
{
}


asianBarrier::~asianBarrier(void)
{
}

OptionValue asianBarrier::price(OptionType type, double S0,double K,double A,double r,double b,double T,double t1,\
                                int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
   if(!checkParaValid(  S0, K, A, r, b, T, t1,\
       totalPoint, averagingPoint ,  barrier, sigma, compensation, initPrice, Nrep)){
       std::cerr << "Parameter Invalid" << std::endl;
       return 0;
   }
   else{
       double tag = 1.0;
       if(type == OptionType::PUT){
            tag = -1.0;
       }
       __int32 tTag = __int32(t1 * yearBase) +1;
       double dt = 1.0 / double(yearBase);
       __int32 Nsteps = __int32(T * yearBase);
       double initPrice = 1.0;
       double price = 0.0;
       mat futuresMatrix(Nrep,Nsteps);
       mat onesMatrix(Nrep,Nsteps,arma::fill::ones);;
       mat averIndexMatrix(1,Nsteps - tTag + 2,arma::fill::ones);
       mat randMatrix = randn<mat>(Nrep,Nsteps);

       
       futuresMatrix = S0 * arma::exp((b - 0.5 * sigma * sigma) * dt * arma::cumsum(onesMatrix,1) + sigma * std::sqrt(dt) * arma::cumsum(randMatrix,1));
       mat F0Matrix(Nrep,1);
       F0Matrix.fill(S0);
       mat F0PathMatrix = arma::join_horiz(F0Matrix,futuresMatrix);// all underlying path
       
       
       mat AverMatrix = (averagingPoint * A + arma::cumsum(F0PathMatrix.cols(tTag-1,Nsteps),1))   /   arma::repmat(arma::cumsum(averIndexMatrix,1),Nrep,1);

       mat downOutPrice(Nrep,1);
       mat payoffPrice(Nrep,1); 

       for(__int32 i = 0; i < Nrep; i++){
           arma::uvec firstBarrierPoint = arma::find(AverMatrix.row(i) <= barrier,1,"first");
           if(firstBarrierPoint.is_empty()){
               downOutPrice(i) = AverMatrix(i,Nsteps - tTag + 1);
           }else{
               downOutPrice(i) =  AverMatrix(i,firstBarrierPoint(0));
           }
       }

       
       while(std::abs(initPrice > price) > 1e-6){
           initPrice = price;
           mat initPriceCompensation(Nrep,1);
           initPriceCompensation.fill(initPrice * compensation);

           payoffPrice = arma::max(arma::mat(arma::join_horiz(initPriceCompensation,(downOutPrice - K) * tag)),1);
           mat meanPrice = arma::mean(payoffPrice);
           price = meanPrice(0) * std::exp(-r * T); 

       }
       return price;
   
   }
}

DeltaValue asianBarrier::delta(OptionType type , double S0,double K,double A,double r,double b,double T,double t1,\
        int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
        double priceUp = this -> price(type,S0 * 1.0005,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double priceDown = this -> price(type,S0 * 0.9995,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double deltaV = (priceUp - priceDown) / (0.001 * S0);
        return deltaV;
}

GammaValue asianBarrier::gamma(OptionType type ,double S0,double K,double A,double r,double b,double T,double t1,\
        int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
        double priceUp = this -> price(type,S0 * 1.0005,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double price = this -> price(type,S0 ,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double priceDown = this -> price(type,S0 * 0.9995,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double gammaV = (priceUp - 2.0 * price + priceDown) / (0.0005 * 0.0005 * S0 * S0);
        return gammaV;

}

ThetaValue asianBarrier::theta(OptionType type , double S0,double K,double A,double r,double b,double T,double t1,\
        int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
        double priceUp = this -> price(type,S0,K,A,r,b,T - 1.0/252.0,0,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double priceDown = this -> price(type,S0 ,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double thetaV = (priceUp - priceDown);
        return thetaV;

}

VegaValue asianBarrier::vega(OptionType type , double S0,double K,double A,double r,double b,double T,double t1,\
        int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
        double priceUp = this -> price(type,S0,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double priceDown = this -> price(type,S0 ,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma - 0.01,compensation,initPrice,Nrep,yearBase);
        double vegaV = (priceUp - priceDown);
        return vegaV;

}

RhoValue asianBarrier::rho(OptionType type , double S0,double K,double A,double r,double b,double T,double t1,\
        int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int Nrep, int yearBase){
        double priceUp = this -> price(type,S0,K,A,r,b,T,t1,totalPoint,averagingPoint,barrier,sigma,compensation,initPrice,Nrep,yearBase);
        double priceDown = this -> price(type,S0 ,K,A,r - 0.01,b,T,t1,totalPoint,averagingPoint,barrier,sigma ,compensation,initPrice,Nrep,yearBase);
        double rhoV = (priceUp - priceDown);
        return rhoV;

}
bool asianBarrier::checkParaValid(double S0,double K,double A,double r,double b,double T,double t1,\
         int totalPoint,int averagingPoint , double barrier,double sigma,double compensation,double initPrice,int &Nrep){

    if(S0 < 0 || K < 0 || A <0 || r < 0 || b < 0 || T <0 || t1 < 0 || totalPoint < 0 || averagingPoint < 0 || sigma < 0 || compensation <0){
        std::cerr << "paramater invalid < 0 " << std::endl;
        return false;
    }
    if(Nrep <= 5000){
        Nrep = 5000;
    }
    return true;
}