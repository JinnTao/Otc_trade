#include "quserstrategy.h"
#include <qsettings.h>
#include <qreadwritelock.h>
#include <qsemaphore.h>
#include <atomic>
#include <qmutex.h>
#include <iostream>
#include <iomanip>
#include "StructFunction.h"
#define ELPP_THREAD_SAFE

#include "easylogging++.h"

extern QReadWriteLock g_lock;
extern QMap<QString,CThostFtdcDepthMarketDataField > instrumentMarketData;
extern QMap<QString,QAtomicInt> gOrderUnfinished;
extern QMutex g_mutex;

QUserStrategy::QUserStrategy(QOrderManager  *pOrderManager,QObject *parent)
 : QThread(parent)
{
 this->m_status = false; // 默认策略不开启

 this->m_pOrderManager = pOrderManager;
 
 //connect(this,SIGNAL(strategyMsg(QString)),parent->parent(),SLOT(onStrategyMsg(QString)) ,Qt::QueuedConnection ); 
 //connect(this,SIGNAL(subscribe_inst_data(QString)),parent,SLOT(subscribe_inst_data(QString))  ); 
 //connect(this,SIGNAL(hedgeOrder(QString,QString,int)),pOrderManager,SLOT(updateToOrderBook(QString,QString,int)),Qt::QueuedConnection);


 this->m_oldTickUpdate = "";
 this->m_tryInsertOrderTimes = 0;
 this->m_testTimes = 30;
 // 导入

}

QUserStrategy::~QUserStrategy()
{

}


void QUserStrategy::run(){

 QString msg;

 this-> remainT = 1/12;
 this->riskRate = 0.02;
 this->discount = std::exp(-riskRate * this->remainT);

 this-> expectReturn = 20;


 bool bFirst = true;
 while(this->m_status){
  //载入配置
  if(bFirst){
   //分批订阅合约
   for(auto i = m_optionArbitrageList.begin();i != m_optionArbitrageList.end();i++){
    emit subscribe_inst_data(i->toStdString().c_str());
   }
   emit subscribe_inst_data(this->m_futureCode.toStdString().c_str());//订阅基础合约
   emit strategyMsg( QString(" Option Arbitrage subscribe inst "));
   this->m_instMessage_map_stgy = this->m_pOrderManager->getInstMessageMap();
  }
  //策略运算间隔
  _sleep(1000);
  // K1 K2
  g_lock.lockForRead();
  
  this->updateData();
  this->optionArbitrageMode1();
  this->printData();

  g_lock.unlock();
  //this->optionArbitrage(C_K1,C_K2,P_K1,P_K2);
  //this->optionArbitrage(C_K1,C_K3,P_K1,P_K3);
  //this->optionArbitrage(C_K1,C_K4,P_K1,P_K4);
  //this->optionArbitrage(C_K1,C_K5,P_K1,P_K5);

  //this->optionArbitrage(C_K2,C_K3,P_K2,P_K3);
  //this->optionArbitrage(C_K2,C_K4,P_K2,P_K4);
  //this->optionArbitrage(C_K2,C_K5,P_K2,P_K5);

  //this->optionArbitrage(C_K3,C_K4,P_K3,P_K4);
  //this->optionArbitrage(C_K3,C_K5,P_K3,P_K5);

  //this->optionArbitrage(C_K4,C_K5,P_K4,P_K5);
  bFirst = false;
 
 }
 emit strategyMsg( QString("Option Arbitrage stopped "));
}

//期权箱式套利
bool QUserStrategy::optionArbitrage(optionInstrument C_K1,optionInstrument C_K2,optionInstrument P_K1,optionInstrument P_K2){
 // 期权策略套利
 double left = 0;
 double right = 0;
 bool status = false;

 if(instrumentMarketData.contains(C_K1.code) && instrumentMarketData.contains(C_K2.code) && 
  instrumentMarketData.contains(P_K1.code) && instrumentMarketData.contains(P_K2.code)){
  //C(K1)-C(K2)-(P(K1)-P(K2)) = PV(K2-K1)
   left = instrumentMarketData[C_K1.code].LastPrice - instrumentMarketData[C_K2.code].LastPrice - 
    (instrumentMarketData[P_K1.code].LastPrice - instrumentMarketData[P_K2.code].LastPrice);

   right = this->discount * (C_K2.Strike - C_K1.Strike);
   //大于期望收益
   if(left - right > this->expectReturn){
    
   }
   //小于期望收益
   if(right - left > this->expectReturn){
   
   }
   status = true;
 }
 else{
  status = false;
 }
 return status;
}

void QUserStrategy::optionArbitrageMode1(){
 //double priceDistance = this->m_instMessage_map_stgy[this->m_futureCode.toStdString()]->PriceTick * 5; // 5倍价格间距认为为
 //double upDownMaxRatio = (instrumentMarketData[this->m_futureCode].OpenPrice - instrumentMarketData[this->m_futureCode].LowestPrice) / instrumentMarketData[this->m_futureCode].OpenPrice;
 //
 //for(auto i = this->m_optionInstrumentList.begin(); i != this->m_optionInstrumentList.end(); i++){
 // //需要保证市场流动性 
 // // 当前盘口至少需要有5手的买卖量
 // switch (i->ot)
 // {
 // case optionModel::CALL:
 //  {
 //   //1
 //   double intrinsicValue = max( i->underlyingPrice - i->Strike,0);
 //   if(i->askPrice < intrinsicValue){
 //    //buy askprice 卖出价买入
 //    this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,i->askPrice);
 //    //short bidPrice 买入价卖出
 //    this->m_pOrderManager->insertOrder(this->m_futureCode,DIRECTION::sell,OFFSETFLAG::open,1,instrumentMarketData[this->m_futureCode].BidPrice1);
 //    cout << "C < max(S-K,0) 当看涨期权的权利金价格小于其内在价值，买入看涨期权，同时卖出相同数量的期货" << endl;
 //   }

 //   //4
 //   //与到期日之间的最大涨幅 
 //   double maxUpperPrice = std::pow(1+upDownMaxRatio,i->expiryDayNum) * i->underlyingPrice;
 //   //虚值期权
 //   if(i->Strike < i->underlyingPrice){
 //    if(maxUpperPrice < (i->underlyingPrice - i->Strike)){
 //     //short bidPrice 买入价卖出
 //     this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,i->bidPrice);
 //     cout << "当标的期货价格在临近到期日与最后到期日之间的最大涨幅，小于虚值看涨期权行权价与标的期货价格的价差，卖出虚值看涨期权。无风险收益 = 看跌期权的权利金" << endl;
 //    }
 //   }
 //  }

 //  
 //  
 //  
 //  break;
 // case optionModel::PUT:
 //  {
 //   // 2
 //   double intrinsicValue = max( i->Strike -  i->underlyingPrice,0);
 //   if(i->askPrice < intrinsicValue){
 //    //buy askprice 卖出价买入
 //    this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,i->askPrice);
 //    //short bidPrice 卖出价买入
 //    this->m_pOrderManager->insertOrder(this->m_futureCode,DIRECTION::buy,OFFSETFLAG::open,1,instrumentMarketData[this->m_futureCode].AskPrice1); //
 //    cout << "P < max(K-S,0) 当看跌期权的权利金价格小于其内在价值，买入看跌期权，同时买入相同数量的期货" << endl;
 //   }
 //   //3
 //   //与到期日之间的最大跌幅 
 //   double maxDownPrice = std::pow(1-upDownMaxRatio,i->expiryDayNum) * i->underlyingPrice;
 //   //虚值期权
 //   if(i->Strike > i->underlyingPrice){
 //    if(maxDownPrice < (i->Strike - i->underlyingPrice)){
 //     //short bidPrice 买入价卖出
 //     this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,i->bidPrice);
 //     cout << "标的期货价格在临近到期日与最后到期日之间的最大跌幅，小于标的期货价格与虚值看跌期权行权价的价差，卖出虚值看跌期权。无风险收益 = 看涨期权的权利金" << endl;
 //    }
 //   }
 //  }
 //  break;
 // default:
 //  break;
 // }
 // //针对 5 6 7 8 9 10 11 12 13 14 15 16
 // for(auto j = i+1; j != this->m_optionInstrumentList.end(); j++){

 //  if(i->ot == optionModel::OptionType::CALL){
 //   if(j->ot == optionModel::OptionType::PUT){
 //    if(std::abs(i->Strike - i->underlyingPrice) <= priceDistance && std::abs(j->Strike - j->underlyingPrice) <= priceDistance){
 //     // 5
 //     if(i->bidPrice > j->askPrice){
 //      
 //      this->m_pOrderManager->insertOrder(this->m_futureCode,DIRECTION::buy,OFFSETFLAG::open,1,0); //
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      cout << "当行权价与标的期货价格非常接近的看涨期权权利金大于看跌期权权利金，买入标的期货，卖出相同数量看涨期权，买入相同数量看跌期权。无风险收益=（看涨期权的权利金价格-看跌期权的权利金价格)-(标的期货价格-期权行权价格)" <<endl;
 //     }
 //     // 6
 //     if(i->askPrice < j->bidPrice){
 //      this->m_pOrderManager->insertOrder(this->m_futureCode,DIRECTION::sell,OFFSETFLAG::open,1,0); //
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      cout << "当行权价与标的期货价格非常接近的看跌期权价格大于看涨期权价格，卖出标的期货，买入相同数量看涨期权，卖出相同数量看跌期权。无风险收益=(看跌期权权利金价格-看涨期权权利金价格)-(期权行权价格-标的期货价格)" << endl;
 //     }
 //    }
 //    //9
 //    // j:put i:call
 //    if(j->Strike > i->Strike && (j->bidPrice + i->askPrice) < (j->Strike - i->Strike)){
 //     this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //     this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //     cout << "当到期日相同，行权价较高的看跌期权与行权价较低的看涨期权的权利金价格之和，小于两者行权价之差时，买入行权价较低的看涨期权，买入相同数量行权价较高的看跌期权。无风险收益=（高行权价－低行权价）－（看涨期权权利金价格+看跌期权权利金价格）" << endl;
 //    }
 //    //10
 //    //与到期日之间的最大涨幅 
 //    double maxUpperPrice = std::pow(1+upDownMaxRatio,i->expiryDayNum) * i->underlyingPrice;
 //    //与到期日之间的最大跌幅 
 //    double maxDownPrice = std::pow(1-upDownMaxRatio,i->expiryDayNum) * i->underlyingPrice;
 //    //i:call j:put
 //    if(i->Strike < j->Strike && (maxUpperPrice - maxDownPrice) < (j->Strike - i->Strike)){
 //     if((i->bidPrice + j->bidPrice) > (j->Strike - i->Strike)){
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      cout<< "当临近期权最后到期日，标的期货的最大波动在行权价较低的看涨期权与行权价较高的看跌期权两者行权价之间，两期权的权利金价格之和大于其高低行权价之差，那么卖出行权价较低看涨期权，卖出相同数量行权价较高的看跌期权。其中，看涨期权与看跌期权最后到期日相同。无风险收益＝（看涨期权权利金价格＋看跌期权权利金价格）－（高行权价－低行权价）。" << endl;
 //     }
 //    }

 //   }
 //   if(j->ot == optionModel::OptionType::CALL){
 //    // 7
 //    if(i->Strike > j->Strike && i->bidPrice > j->askPrice){
 //     this->m_pOrderManager->insertOrder(j->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //     this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //     cout << "当到期日相同，行权价较高的看涨期权的权利金价格大于行权价较低的看涨期权权利金价格，买入行权价较低的看涨期权，卖出相同数量行权价较高的看涨期权。最小无风险收益=行权价较高的看涨期权权利金价格-行权价较低的看涨期权权利金价格" << endl;
 //    }
 //   }
 //  }
 //  if(i->ot == optionModel::OptionType::PUT){
 //   if(j->ot == optionModel::OptionType::PUT){
 //    // 8
 //    if(i->Strike > j->Strike && j->bidPrice > i->askPrice){
 //     this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //     this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //     cout << "当到期日相同，行权价较低的看跌期权的权利金价格大于行权价较高的看跌期权权利金价格，买入行权价较高的看跌期权，卖出相同数量行权价较低的看跌期权。最小无风险收益=行权价较低的看跌期权权利金价格-行权价较高的看跌期权权利金价格" << endl;
 //    }
 //   }
 //  }
 //  // 13 14 15 16
 //  for(auto m = j+1;m != this->m_optionInstrumentList.end();m++){
 //   if(i->ot == optionModel::OptionType::CALL && j->ot == optionModel::OptionType::CALL && m->ot == optionModel::OptionType::CALL){
 //    //等差数列
 //    if((i->Strike + m->Strike) == j->Strike *2){
 //     double BothSideAsk = i->askPrice + m->askPrice;
 //     double BothSideBid = i->bidPrice + m->bidPrice;
 //     if(BothSideBid > j->askPrice){
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::buy,OFFSETFLAG::open,2,0);
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(m->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      cout << "14 call:buy mid sell both side"<<endl;
 //     }
 //     if(BothSideAsk > j->bidPrice){
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,2,0);
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(m->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      cout << "16 call:sell mid buy both side"<<endl;
 //     }
 //    }
 //   }
 //   if(i->ot == optionModel::OptionType::PUT && j->ot == optionModel::OptionType::PUT && m->ot == optionModel::OptionType::PUT){
 //    //等差数列
 //    if((i->Strike + m->Strike) == j->Strike *2){
 //     double BothSideAsk = i->askPrice + m->askPrice;
 //     double BothSideBid = i->bidPrice + m->bidPrice;
 //     if(BothSideBid > j->askPrice){
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::buy,OFFSETFLAG::open,2,0);
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(m->code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //      cout << "13 put buy mid sell both side"<<endl;
 //     }
 //     if(BothSideAsk < j->askPrice){
 //      this->m_pOrderManager->insertOrder(j->code,DIRECTION::sell,OFFSETFLAG::open,2,0);
 //      this->m_pOrderManager->insertOrder(i->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      this->m_pOrderManager->insertOrder(m->code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //      cout << "15 put sell mid buy both side"<<endl;
 //     }
 //    }
 //   }
 //  }
 // }
 //}
}


void QUserStrategy::optionArbitrageMode2(){
 //for(auto i = this->m_optionPairList.begin();i!=this->m_optionPairList.end(); i++){
 // for(auto j = i+1;j!=this->m_optionPairList.end();j++){
 //  // 11
 //  if(i.key() > j.key()){
 //   double leftBid = instrumentMarketData[j->callOption.code].BidPrice1 + instrumentMarketData[i->putOption.code].BidPrice1;
 //   double rightAsk = instrumentMarketData[j->putOption.code].AskPrice1 + instrumentMarketData[i->callOption.code].AskPrice1;

 //   double leftAsk = instrumentMarketData[i->callOption.code].AskPrice1 + instrumentMarketData[j->putOption.code].AskPrice1;
 //   double rightBid = instrumentMarketData[i->putOption.code].BidPrice1 + instrumentMarketData[j->callOption.code].BidPrice1;
 //   if((leftBid - rightAsk) > (i.key()-j.key())){
 //    this->m_pOrderManager->insertOrder(j->callOption.code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //    this->m_pOrderManager->insertOrder(i->putOption.code,DIRECTION::sell,OFFSETFLAG::open,1,0);

 //    this->m_pOrderManager->insertOrder(i->callOption.code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //    this->m_pOrderManager->insertOrder(j->putOption.code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //    cout << "11 Box Sell C-Low-K,P-High-K; Buy C-High-K,P-Low-K" <<endl;
 //   }

 //   if((leftAsk - rightBid) < (i.key()-j.key())){
 //    this->m_pOrderManager->insertOrder(j->callOption.code,DIRECTION::buy,OFFSETFLAG::open,1,0);
 //    this->m_pOrderManager->insertOrder(i->putOption.code,DIRECTION::buy,OFFSETFLAG::open,1,0);

 //    this->m_pOrderManager->insertOrder(i->callOption.code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //    this->m_pOrderManager->insertOrder(j->putOption.code,DIRECTION::sell,OFFSETFLAG::open,1,0);
 //    cout << "12 Box Buy C-Low-K,P-High-K; Sell C-High-K,P-Low-K" <<endl;
 //   
 //   }

 //  }
 // }
 //
 //}

}
void QUserStrategy::onRunClicked(){

 this->m_status = true;
 this->start();
}
void QUserStrategy::onStopClicked(){

 this->m_status = false;
}
void QUserStrategy::updateData(){
 //// updateData
 //for(auto i = this->m_optionArbitrageList.begin();i != this->m_optionArbitrageList.end(); i++){
 // //需保证基础合约行情以及该标的期权
 // if(instrumentMarketData.contains(*i) && instrumentMarketData.contains(this->m_futureCode)){
 //  if(this->m_optionInstrumentList.contains(*i)){
 //   this->m_optionInstrumentList[*i].askPrice = instrumentMarketData[*i].AskPrice1;
 //   this->m_optionInstrumentList[*i].bidPrice = instrumentMarketData[*i].BidPrice1;
 //   this->m_optionInstrumentList[*i].askVol = instrumentMarketData[*i].AskVolume1;
 //   this->m_optionInstrumentList[*i].bidVol = instrumentMarketData[*i].BidVolume1;
 //   this->m_optionInstrumentList[*i].underlyingPrice = instrumentMarketData[this->m_futureCode].LastPrice; //标的期货价格
 //   if(instrumentMarketData[*i].AskPrice1 > instrumentMarketData[*i].UpperLimitPrice)
 //   {
 //    this->m_optionInstrumentList[*i].askPrice  = 0;
 //   }
 //   if(instrumentMarketData[*i].BidPrice1 > instrumentMarketData[*i].UpperLimitPrice)
 //   {
 //    this->m_optionInstrumentList[*i].bidPrice  = 0;
 //   }
 //  }else{
 //   optionInstrument cur;
 //   QString type;
 //   cur.code = *i;
 // 
 //   cur.askPrice = instrumentMarketData[*i].AskPrice1;
 //   cur.bidPrice = instrumentMarketData[*i].BidPrice1;
 //   if(instrumentMarketData[*i].AskPrice1 > instrumentMarketData[*i].UpperLimitPrice)
 //   {
 //    cur.askPrice  = 0;
 //   }
 //   if(instrumentMarketData[*i].BidPrice1 > instrumentMarketData[*i].UpperLimitPrice)
 //   {
 //    cur.bidPrice  = 0;
 //   }

 //   cur.lastPrice = instrumentMarketData[*i].LastPrice;
 //   
 //   //type = (*i).mid(m_futureCodeIndex,1);
 //   //call
 //   if(this->m_instMessage_map_stgy[i->toStdString()]->OptionsType == '1'){
 //    cur.ot = optionModel::CALL;
 //    cur.optionType = "call";

 //   }
 //   //put
 //   if(this->m_instMessage_map_stgy[i->toStdString()]->OptionsType == '2'){
 //    cur.ot = optionModel::PUT;
 //    cur.optionType = "put";
 //   }
 //   double strike =  this->m_instMessage_map_stgy[i->toStdString()]->StrikePrice;
 //   cur.Strike = strike;
 //   
 //   cur.tradeDate =  instrumentMarketData[*i].ActionDay;
 //   cur.bidVol = instrumentMarketData[*i].BidVolume1;
 //   cur.askVol = instrumentMarketData[*i].AskVolume1;
 //   cur.expiryDate = this->m_instMessage_map_stgy[i->toStdString()]->ExpireDate;
 //   cur.underlyingPrice = instrumentMarketData[this->m_futureCode].LastPrice; //标的期货价格
 //   //剩余天数 等于 到期日减去交易日
 //   cur.expiryDayNum = this->m_workDayMap[cur.expiryDate.toStdString()] - this->m_workDayMap[cur.tradeDate.toStdString()];
 //   cur.priceTick = this->m_instMessage_map_stgy[i->toStdString()]->PriceTick;
 //   cur.UnderlyingInstrID =  this->m_instMessage_map_stgy[i->toStdString()]->UnderlyingInstrID; // 基础合约代码
 //   this->m_optionInstrumentList.insert(*i,cur); // 将具体消息插入队列

 //   //相同执行价配对组成列表
 //   if(this->m_optionPairList.contains(strike)){
 //    if(cur.ot == optionModel::OptionType::CALL){
 //     this->m_optionPairList[strike].callOption = cur;
 //    }else{
 //     this->m_optionPairList[strike].putOption = cur;
 //    }
 //   }else{
 //    optionPair curOptionPair;
 //    if(cur.ot == optionModel::OptionType::CALL){
 //     curOptionPair.callOption = cur;
 //    }else{
 //     curOptionPair.putOption = cur;
 //    }
 //    this->m_optionPairList.insert(strike,curOptionPair);
 //   }

 //  }
 // }
 //
 //}

}

void QUserStrategy::printData(){
 std::cout  << "TradeDate" << std::setw(15)<< " Code " << std::setw(15)<< " lastPrice "<< std::setw(15) << "bidPrice "<< std::setw(15) << " bidvol "<< std::setw(15) << " askPrice "<< std::setw(15) <<  " askVol "<< std::setw(15)<< " Strike "<< std::setw(15)<<" expireDate "<< std::setw(15)<<" optionType " <<endl;
 for(auto i = this->m_optionInstrumentList.begin();i != m_optionInstrumentList.end();i++){
  //std::cout << std::setw(15) <<  i->tradeDate.toStdString() << std::setw(15) <<  i->code.toStdString()<<std::setw(15)  <<i->lastPrice << " \t "  << i->bidPrice<< " \t " << i->bidVol<< " \t " << i->askPrice<< " \t " << i->askVol << " \t "<< i->Strike<< "\t" << i->expiryDate.toStdString() <<" \t " << i->optionType << endl;
  std::cout <<  i->tradeDate.toStdString() << std::setw(15) <<  i->code.toStdString()<<std::setw(15)  <<i->lastPrice << std::setw(15)  << i->bidPrice<< std::setw(15) << i->bidVol<< std::setw(15) << i->askPrice<< std::setw(15) << i->askVol <<std::setw(15) << i->Strike<< std::setw(15) << i->expiryDate.toStdString() << std::setw(15) << i->optionType << endl;
 }

}
void QUserStrategy::loadConf(){
 //导入工作日
// readWorkDay("input/workday.txt",this->m_workDayMap);
}
