#include "qstrategythread.h"
#include <qsettings.h>
#include <qreadwritelock.h>
#include <qsemaphore.h>
#include <atomic>
#include <qmutex.h>
#include <time.h>
#include <float.h>
#include <limits.h>

#define ELPP_THREAD_SAFE

#include "easylogging++.h"

//! Used to simplify the throwing of formatted exceptions
#define XLW__HERE__ __FILE__ "(" _MAKESTRING(__LINE__) "): "
#define _MAKESTRING(a) __MAKESTRING(a)
#define __MAKESTRING(a) #a

#define ERROR__OUTPUT(ERROR_MSG_PARTS) \
 do { \
  std::ostringstream ostr; \
  ostr << ERROR_MSG_PARTS; \
  std::cerr << XLW__HERE__ << ostr.str() << std::endl; \
 } while(0)


#define  ACCURACY 1.0e-6


extern QReadWriteLock g_lock;
extern QMap<QString,CThostFtdcDepthMarketDataField > instrumentMarketData;
extern QMap<QString,QAtomicInt> gOrderUnfinished;
extern QMutex g_mutex;


QStrategyThread::QStrategyThread(QStandardItemModel* pModel,QOrderManager  *pOrderManager,QObject *parent)
 : QThread(parent)
{
 this->m_status = false; // Ĭ�ϲ��Բ�����
 //this->m_checkStatus = false; // Ĭ�ϲ���δ���
 this->m_strategyGreekModel = pModel;
 this->m_pOrderManager = pOrderManager;
 
 connect(this,SIGNAL(strategyMsg(QString)),parent->parent(),SLOT(onStrategyMsg(QString)) ,Qt::QueuedConnection ); 
 connect(this,SIGNAL(subscribe_inst_data(QString)),parent,SLOT(subscribe_inst_data(QString))  ); 
 connect(this,SIGNAL(dataChange(QModelIndex,QModelIndex)),parent,SLOT(dataChange(QModelIndex,QModelIndex)),Qt::QueuedConnection);
 connect(this,SIGNAL(updateToOrderBook(QString,QString,int,int,int)),pOrderManager,SLOT(updateToOrderBook(QString,QString,int,int,int)),Qt::QueuedConnection);

 this->m_iNoTradeHour1 = 3;
 this->m_iNoTradeHour2 = 9;
 this->m_oldTickUpdate = "";
 this->m_tryInsertOrderTimes = 0;
 this->m_testTimes = 15;
 this->m_instrumentStatus = true;
 this->m_tau = 0;
 this->m_t1 = 0;
 this->m_callExerPrice = 0;
 this->m_putExerPrice = 0;
 this->m_averagePriceAdjust = 0;
 this->m_averagedPointAdjust = 0;


// this->m_optionEngine.setUnderlyingType(optionModel::underlyingType::Future);
 this->m_isCarry = false;
}

QStrategyThread::~QStrategyThread()
{
 this->m_status = false;
}

void QStrategyThread::run(){
 
 QString msg;
 bool bFirst =true;
 while(this->m_status){

  if(bFirst){
   g_mutex.lock();
   //�ڳ�������Բ���
   //msg = this->productName + QString(" is subscribe Inst ") +this->instrumentCode;
   //QString productNameDir = QString("output/product/") + this->productName + QString(".ini") ;
   //this->m_oOptionCalculateEngine.loadConf(productNameDir);
   emit strategyMsg(msg);
   instrumentDetail iD;
   m_instrumentList.clear();
   if (abs(this->instrumentRatio)>0 ) { 
 emit subscribe_inst_data(this->instrumentCode);iD.instrumentCode = QString(this->instrumentCode);
    iD.ratio = this->instrumentRatio;iD.tickValue = this->tickValue;iD.updateToBook = true;this->m_instrumentList.append(iD);
    this->m_strategyCashGreeksMap.insert(std::make_pair(this->instrumentCode,cashGreeks()));
   }
   if (abs(this->instrumentRatio2)>0 ){ 
 emit subscribe_inst_data(this->instrumentCode2);iD.instrumentCode = this->instrumentCode2;
 iD.ratio = this->instrumentRatio2;iD.tickValue = this->tickValue2;iD.updateToBook = true;this->m_instrumentList.append(iD);
    this->m_strategyCashGreeksMap.insert(std::make_pair(this->instrumentCode2,cashGreeks()));
   }
   if (abs(this->instrumentRatio3)>0 ){ 
 emit subscribe_inst_data(this->instrumentCode3);iD.instrumentCode = this->instrumentCode3;
 iD.ratio = this->instrumentRatio3;iD.tickValue = this->tickValue3;iD.updateToBook = true;this->m_instrumentList.append(iD);
    this->m_strategyCashGreeksMap.insert(std::make_pair(this->instrumentCode3,cashGreeks()));
   }
   if (abs(this->instrumentRatio4)>0 ){ 
 emit subscribe_inst_data(this->instrumentCode4);iD.instrumentCode = this->instrumentCode4;
 iD.ratio = this->instrumentRatio4;iD.tickValue = this->tickValue4;iD.updateToBook = true;this->m_instrumentList.append(iD);
    this->m_strategyCashGreeksMap.insert(std::make_pair(this->instrumentCode4,cashGreeks()));
   }
   //�ڳ���������
   for(auto i = this->m_instrumentList.begin();i != this->m_instrumentList.end();i++){
 QList<QStandardItem *> tList = this->m_strategyGreekModel->findItems(this->productName + i->instrumentCode);
 if (tList.count()<=0)
 {
  m_strategyGreekModel->insertRow(0);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(0, 0), this->productName + i->instrumentCode);
 }


   }
   if(this->m_instrumentList.size() == 0){
 this->m_instrumentStatus = false;
   }


   g_mutex.unlock();
  }
  // ===================
  testFunc();
  //========================================Run===========================================
  sleep(this->m_spanTime);;
  g_lock.lockForRead();
  //�ж������Ƿ����
  for(auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
   if(!instrumentMarketData.contains(i->instrumentCode)){
 this->m_instrumentStatus = false;
   }
  }
  if(this->m_instrumentStatus ){

   //QString LastPrice = QString::number(instrumentMarketData[this->instrumentCode].LastPrice,'f', 2);
   //msg = this->productName + QString(" get data ") +LastPrice + QString( " updateTime  ") +QString(instrumentMarketData[this->instrumentCode].UpdateTime );
   //emit strategyMsg(msg);
   //���ļ���
   StrategyOperationMutil();

   //���ʲ�
   //StrategyOptionCalcultate(&instrumentMarketData[this->instrumentCode]);
  }else{
   msg = this->productName + QString(" don't get data");
   emit strategyMsg(msg);
  }
  bFirst = false;
  g_lock.unlock();

 }

 msg = this->productName + QString("  stopped ");
 m_testTimes = 30;
 emit strategyMsg(msg);

}
 bool QStrategyThread::timeCalculate(TThostFtdcTimeType updateTime,double underlyingPrice){
  time_t t = time(0);
  time_t curTimeTic,everyDayStratTime;
  char tmp[64],dateTime[64];
  strftime( tmp, sizeof(tmp), "%Y%m%d",localtime(&t) ); 
  strftime( dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S",localtime(&t) );
  int iYear,iMonth,iDay,iHour,iMinute,iSecond;
  sscanf(dateTime,"%d-%d-%d %d:%d:%d",&iYear,&iMonth,&iDay,&iHour,&iMinute,&iSecond);

  curTimeTic = dateTime2unix(iYear,iMonth,iDay,iHour,iMinute,iSecond);/// ��ǰʱ��ת����ʱ���
  everyDayStratTime = dateTime2unix(iYear,iMonth,iDay,this->strategyStartTime.hour(),this->strategyStartTime.minute(),this->strategyStartTime.second());/// ÿ�տ���ʱ���
  this->m_noonTime = dateTime2unix(iYear,iMonth,iDay,this->carryNoonTime.hour(),this->carryNoonTime.minute(),this->carryNoonTime.second());/// noon Time
  this->m_nightTime = dateTime2unix(iYear,iMonth,iDay,this->carryNightTime.hour(),this->carryNightTime.minute(),this->carryNightTime.second());/// Night Timee
  this->m_noonCloseTime =   dateTime2unix(iYear,iMonth,iDay,15,45,0);// ��������ʱ��
  // �������ڼ��
  string today(tmp);
  string sSettlementDate = this->settlementDate.toString("yyyyMMdd").toStdString();
  string sMaturityDate = this->maturityDate.toString("yyyyMMdd").toStdString();
  string snextAverageStartDate = this->nextAveragePoint.toString("yyyyMMdd").toStdString();
  int calendarNumsTotal = this->m_calendarDay[sMaturityDate] - this->m_calendarDay[sSettlementDate];
  int tradeDayNumsTotal = this->m_workDay[sMaturityDate] - this->m_workDay[sSettlementDate];
  int calendarNums = this->m_calendarDay[sMaturityDate] - this->m_calendarDay[today];
  int tradeDayNums = this->m_workDay[sMaturityDate] - this->m_workDay[today];
  int tradeDayNumToAverageStartDate  = 0;
  int calendarToAverageStartDate = 0;
  int holidayNums = calendarNums - tradeDayNums;
  int holidayNumsTotal = calendarNumsTotal - tradeDayNumsTotal;
  int holidayN2AverageTime = 0;
  double t1Plus = 0,tauPlus = 0,t1 = 0,tau = 0;
  time_t carryTime_t = 0;
  switch (this->optionType)
  {
  case 0:
  case 1:
  case 3:
  case 4:
    //carry������ʱ��
    if(curTimeTic > this->m_nightTime){
  carryTime_t = this->carryNightDistance*60;
    }
    //carry����������
    if(curTimeTic > this->m_noonTime && curTimeTic < this->m_noonCloseTime){
  carryTime_t = this->carryNoonDistance*60;
    }
   break;
  case 2:
    tradeDayNumToAverageStartDate = this->m_workDay[snextAverageStartDate] - this->m_workDay[today];
    calendarToAverageStartDate = this->m_calendarDay[snextAverageStartDate] - this->m_calendarDay[today];
    holidayN2AverageTime = calendarToAverageStartDate - tradeDayNumToAverageStartDate;
    this->m_averagedPointAdjust = this->averagedPoint ;
    this->m_averagePriceAdjust = this->averagePrice;
    //carry����������
    if(curTimeTic > this->m_nightTime){
  carryTime_t = this->carryNightDistance*60;
    } 
    //carry������ʱ��
    if(curTimeTic > this->m_noonTime && curTimeTic < this->m_noonCloseTime ){
  carryTime_t = this->carryNoonDistance*60;
  if(curTimeTic + carryTime_t > this->firstAveragePoint.toTime_t() ){
   this->m_averagedPointAdjust = this->averagedPoint + 1;
   this->m_averagePriceAdjust = (this->averagePrice * this->averagedPoint + underlyingPrice)/double(this->m_averagedPointAdjust );
  }
    }
    t1 = ( this->nextAveragePoint.toTime_t() - curTimeTic - holidayN2AverageTime * DAY_SECOND - carryTime_t) / YEAR_SECOND_WORKDAY;

   break;
  default:
   break;
  }

  tau = (this->maturityDate.toTime_t() - curTimeTic - holidayNums * DAY_SECOND - carryTime_t) / YEAR_SECOND_WORKDAY;
  if(t1 < 0){
   t1 = 0;
  }
  this->m_tradeN = tradeDayNums;
  this->m_tradeNTotal = tradeDayNumsTotal;
  this->m_canlendarN = calendarNums;
  this->m_canlendarNTotal = calendarNumsTotal;
  this->m_tradeT1N = tradeDayNumToAverageStartDate;
  this->m_calendarT1N = calendarToAverageStartDate;

  if(this->settlementDate.toTime_t() > curTimeTic || curTimeTic > this->maturityDate.toTime_t() || this->settlementDate.toTime_t() > this->maturityDate.toTime_t() || tau < 0)
  {
   return false;
  }
  this->m_t1 = t1;
  this->m_tau = tau;
  return true;
}
void QStrategyThread::StrategyOperationMutil(){
 //===================������ʼ��==================================
 CThostFtdcDepthMarketDataField *pDepthMarketData = &instrumentMarketData[this->m_instrumentList.begin()->instrumentCode];
 time_t curTimeTic = 0,everyDayStratTime;
 optionGreeks CallOptionGreeks,PutOptionGreeks;
 optionHedgeHands CurHedgeHands;
 optionParam _optionParam;
 int maxOpenNum = this->m_openNums;
 int hedge_level = this->hedgeLevel.toInt();
 double  dH0;
 //===================���·��Price==================================
 double  underlyingPrice = 0;
 for (auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
  underlyingPrice += instrumentMarketData[i->instrumentCode].LastPrice * i->ratio;
 }
 //===================timeCarry==================================
 // ��ʵ�ʹ���ʱ��Ϊ׼
 //this->GetTimeTic(pDepthMarketData->ActionDay,pDepthMarketData->UpdateTime,curTimeTic,everyDayStratTime);
 
 if(!this->timeCalculate(pDepthMarketData->UpdateTime,underlyingPrice)){
  emit strategyMsg(QString( this->productName + QString::fromLocal8Bit(" ��Ʒ�ѵ��ڻ��������")));
  return;
 }
 
 //======================Greeks����====================================

 
 if(!this->optionChoose(underlyingPrice,curTimeTic,CallOptionGreeks,PutOptionGreeks)){
  emit strategyMsg(QString( this->productName + QString::fromLocal8Bit(" optionChoose Error")));
  return;
 }
 // Product Total ???
 //if(this->m_instrumentList.size() > 1){
 // setModelData(this->productName,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,1.0,underlyingPrice,this->productName,this->strategyCash);
 //}

 //=====================ÿ����ĶԳ�����ȡ====================================
 for(auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
 i->updateToBook = true;
 double lastPrice = instrumentMarketData[i->instrumentCode].LastPrice;
 CurHedgeHands.DateTime = curTimeTic;
 /// only continue status can hedge
  if(this->m_bookStatus == 0){
  //optionGreeks callGreeks,putGreeks;
  if( this->callHedgeDirection == 0){
  CurHedgeHands.callhedgeHands = getHands(this->scaleUnit,this->callScale,i->tickValue) * CallOptionGreeks.delta * i->ratio;
  }else{
  CurHedgeHands.callhedgeHands = getHands(this->scaleUnit,this->callScale,i->tickValue) * CallOptionGreeks.delta * -1 * i->ratio;
  }
 
  if(this->putHedgeDirection == 0){
  CurHedgeHands.puthedgeHands = getHands(this->scaleUnit,this->putScale,i->tickValue) * PutOptionGreeks.delta * i->ratio;
  }
  else{
  CurHedgeHands.puthedgeHands = getHands(this->scaleUnit,this->putScale,i->tickValue) * PutOptionGreeks.delta * -1 * i->ratio;
  }

  CurHedgeHands.hedgeHands = (CurHedgeHands.callhedgeHands + CurHedgeHands.puthedgeHands);
  
  CurHedgeHands.RealHedgeHands =  (int)(CurHedgeHands.hedgeHands);//��������

  

  
  // cash Greeks
  this->m_strategyCashGreeksMap[i->instrumentCode]._deltaCash = (CurHedgeHands.callhedgeHands + CurHedgeHands.puthedgeHands) * lastPrice * i->tickValue *i->ratio;
  this->m_strategyCashGreeksMap[i->instrumentCode]._gammaCash = (CallOptionGreeks.gamma + PutOptionGreeks.gamma) * lastPrice * lastPrice * 100.0 *i->ratio;
  this->m_strategyCashGreeksMap[i->instrumentCode]._vegaCash = (CallOptionGreeks.vega + PutOptionGreeks.vega) * this->hedgeVolability *  100.0 *i->ratio;
  this->m_strategyCashGreeksMap[i->instrumentCode]._rho = (CallOptionGreeks.rho + PutOptionGreeks.rho) * 100.0 *i->ratio;
  this->m_strategyCashGreeksMap[i->instrumentCode]._thetaCash = (CallOptionGreeks.theta + PutOptionGreeks.theta)* i->ratio ;
  }else{


  CallOptionGreeks.reset();
  PutOptionGreeks.reset();
  this->m_strategyCashGreeksMap[i->instrumentCode]._deltaCash = 0;
  this->m_strategyCashGreeksMap[i->instrumentCode]._gammaCash = 0;
  this->m_strategyCashGreeksMap[i->instrumentCode]._vegaCash =0;
  this->m_strategyCashGreeksMap[i->instrumentCode]._rho = 0;
  this->m_strategyCashGreeksMap[i->instrumentCode]._thetaCash = 0;
  
  }





  setModelData(this->productName + i->instrumentCode,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,i->ratio,lastPrice, i->instrumentCode,this->m_strategyCashGreeksMap[i->instrumentCode]);

  // tickʱ��δ�����򲻽���
  if(strcmp(this->m_oldTickUpdate.toStdString().c_str(),instrumentMarketData[i->instrumentCode].UpdateTime) == 0){
   i->updateToBook = false;
  }else{
   this->m_oldTickUpdate =  QString(instrumentMarketData[i->instrumentCode].UpdateTime);// 
  }
  i->hedgeLots = CurHedgeHands;
  double gammaV = (PutOptionGreeks.gamma + CallOptionGreeks.gamma);
  dH0 = this->H0(this->m_tau,this->m_tradeCostPercent,this->hedgeVolability,this->m_riskAversion,gammaV,this->interest,instrumentMarketData[i->instrumentCode].LastPrice) ;
  i->hedgeThreshold = int(CurHedgeHands.hedgeHands * dH0 *  i->ratio +0.5);



 }

 // -============================���͵�����������=======================
 //PrintLogging( pDepthMarketData, CallOptionGreeks, PutOptionGreeks, CurHedgeHands, _optionParam);

 if(this->m_testTimes != 0){
  QString msg = this->productName + QString::fromLocal8Bit(" ���� ") + QString::number(this->m_testTimes,'f', 1) + QString::fromLocal8Bit (" �����ʽ��ʼ���ס�");
  emit strategyMsg(msg);
  this->m_testTimes--;
  // print message
  switch (this->optionType)
  {
  case 0:
  case 1:
  case 3:
  case 4:
   emit strategyMsg("optionType: " + QString::number(this->optionType) + " S: "  + QString::number(underlyingPrice,'f',2) + 
 " r " + QString::number(this->interest,'f',4) + " vol " + QString::number(this->hedgeVolability,'f',4)  + 
 " tau: " + QString::number(this->m_tau,'f',4) + " callExerPrice: " + QString::number(this->m_callExerPrice,'f',2)+
 " putExerPrice: " + QString::number(this->m_putExerPrice,'f',2) + " tradeNTotal: " + QString::number(this->m_tradeNTotal,'f',2) +
 " tradeN: " + QString::number(this->m_tradeN,'f',2) + " calendarTotal: " + QString::number(this->m_canlendarNTotal,'f',2) + 
 " calendarN: " + QString::number(this->m_canlendarN,'f',2)+" dH0: " + QString::number(dH0,'f',5));
   break;
  case 2:
   emit strategyMsg("optionType: " + QString::number(this->optionType) + " S: "  + QString::number(underlyingPrice,'f',2) + " SA: " + QString::number(this->m_averagePriceAdjust,'f',2) + 
 " r " + QString::number(this->interest,'f',4) + " vol " + QString::number(this->hedgeVolability,'f',4) + " t1: " + QString::number(this->m_t1,'f',4) + 
 " tau: " + QString::number(this->m_tau,'f',4) + " totalP: " + QString::number(this->totalAveragePoint) + " AveragedP: " + QString::number(this->m_averagedPointAdjust) +
 " callExerPrice: " + QString::number(this->m_callExerPrice,'f',2)+ " putExerPrice: " + QString::number(this->m_putExerPrice,'f',2)+ 
 " tradeNTotal: " + QString::number(this->m_tradeNTotal,'f',2) +" tradeN: " + QString::number(this->m_tradeN,'f',2) + 
 " calendarTotal: " + QString::number(this->m_canlendarNTotal,'f',2) + " calendarN: " + QString::number(this->m_canlendarN,'f',2)+
 " calendarT1: " + QString::number(this->m_calendarT1N,'f',2) + " tradeT1N: " + QString::number(this->m_tradeT1N,'f',2)+" dH0: " + QString::number(dH0,'f',5));
   break;


  default:
   break;
  }

  return;
 } 
 
 for (auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
  if(i->updateToBook){
   //QVariant data;
     // data.setValue( this->m_strategyCashGreeksMap[i->instrumentCode]);

   emit this->updateToOrderBook(this->productName+i->instrumentCode,i->instrumentCode,i->hedgeLots.RealHedgeHands,i->hedgeThreshold,this->m_openNums); 
   // _sleep(500);//������ֹ����
  }else{
   //emit  strategyMsg(QString::fromLocal8Bit(" δ�յ��µ�����Tick "));
  }
 }
 

}
//���ʲ���Ʒ�Ľ��������¸ú���Ŀǰ��˵�Ѿ�ʧЧ�ˣ���������߼����Բο������ǵ�ǰ��˵���ǿ�mutiOption hedge ���������
void QStrategyThread::StrategyOptionCalcultate(CThostFtdcDepthMarketDataField *pDepthMarketData){

 ////===================���Կ���ʱ����==================================
 //time_t curTimeTic,everyDayStratTime;
 //
 //optionGreeks CallOptionGreeks,PutOptionGreeks;
 //optionHedgeHands CurHedgeHands;
 //optionParam _optionParam;
 //int maxOpenNum = this->m_openNums;
 ////�Գ���ֵ
 //int hedge_level = this->hedgeLevel.toInt();

 //// ��ʵ�ʹ���ʱ��Ϊ׼
 //this->GetTimeTic(pDepthMarketData->ActionDay,pDepthMarketData->UpdateTime,curTimeTic,everyDayStratTime);
 ////carry������ʱ��
 //if(curTimeTic > this->m_noonTime){
 // curTimeTic+=this->carryNoonDistance*60;
 //}
 ////carry����������
 //if(curTimeTic > this->m_nightTime){
 // curTimeTic+=this->carryNightDistance*60;
 //} 

 //if(!this->m_oOptionCalculateEngine.optionCalculate(pDepthMarketData->LastPrice,curTimeTic,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,_optionParam)){
 // QString msg = this->productName + QString::fromLocal8Bit("   ��Ʒ�ѵ���");
 // emit strategyMsg(msg);
 //}

 //CurHedgeHands.RealHedgeHands =  (int)(CurHedgeHands.hedgeHands + 0.5);//��������



 //PrintLogging( pDepthMarketData, CallOptionGreeks, PutOptionGreeks, CurHedgeHands, _optionParam);

 ////setModelData(this->instrumentCode,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,1.0,0,NULL);


 //// ��ȡ��ʵ�ֲ���
 //if(this->m_testTimes != 0){
 // QString msg = this->productName + QString::fromLocal8Bit(" ���� ") + QString::number(this->m_testTimes,'f', 1) + QString::fromLocal8Bit (" �����ʽ��ʼ���ס�");
 // emit strategyMsg(msg);
 // this->m_testTimes--;
 // return;
 //}
 //// tickʱ���������
 //if(!strcmp(this->m_oldTickUpdate.toStdString().c_str(),instrumentMarketData[this->instrumentCode].UpdateTime) == 0){
 // //����ʱ��
 // this->m_oldTickUpdate =  QString(instrumentMarketData[this->instrumentCode].UpdateTime);//
 // //this->StrategyOperation(CurHedgeHands.hedgeDifference,CurHedgeHands.netPos,pDepthMarketData);
 // //emit this->updateToOrderBook(this->productName,this->instrumentCode,CurHedgeHands.RealHedgeHands,this->hedgeThreshold,m_openNums,); 
 //}
}

void QStrategyThread::testFunc(){

 
}

// ===========================����ʱ�����==================================
void QStrategyThread::GetTimeTic(TThostFtdcDateType ActionDay,TThostFtdcTimeType UpdateTime,time_t&curTimeTic,time_t&everyDayStratTime){

 time_t t = time(0); 
 char tmp[64]; 
 strftime( tmp, sizeof(tmp), "%Y%m%d",localtime(&t) ); 
 string today(tmp);
 string settlementDate = this->settlementDate.toString("yyyyMMdd").toStdString();

 int iYear,iMonth,iDay;
 sscanf(tmp,"%4d%2d%2d",&iYear,&iMonth,&iDay);

 // ������ ����  -  ��ǰ�������� = ����������
 //int iYear = std::atoi(date.substr(0,4).c_str());
 //int iMonth = std::atoi(date.substr(4,2).c_str());
 //int iDay = std::atoi(date.substr(6,2).c_str());
 int hour = 0,minute = 0,sec = 0;
 sscanf_s(UpdateTime,"%2d:%2d:%2d",&hour,&minute,&sec); // Trading Current Time
 curTimeTic = dateTime2unix(iYear,iMonth,iDay,hour,minute,sec);/// ��ǰʱ��ת����ʱ���

 everyDayStratTime = dateTime2unix(iYear,iMonth,iDay,this->strategyStartTime.hour(),this->strategyStartTime.minute(),this->strategyStartTime.second());/// ÿ�տ���ʱ���
 this->m_noonTime = dateTime2unix(iYear,iMonth,iDay,this->carryNoonTime.hour(),this->carryNoonTime.minute(),this->carryNoonTime.second());/// noon Time
 this->m_nightTime = dateTime2unix(iYear,iMonth,iDay,this->carryNightTime.hour(),this->carryNightTime.minute(),this->carryNightTime.second());/// Night Time


}



// ����ģ�� ����Ӧ�÷ֳ� Strategy��� Hedge�ࣿ hedgeDifference ΪҪ����������
void QStrategyThread::StrategyOperation(int hedgeDifference,int position,CThostFtdcDepthMarketDataField *pDepthMarketData){
  
 TThostFtdcInstrumentIDType instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
 TThostFtdcDirectionType    dir;//����,'0'��'1'��
 TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
 TThostFtdcPriceType     price;//�۸�0���м�,��������֧��
 TThostFtdcVolumeType    vol;//����
 string s_instId = pDepthMarketData->InstrumentID;
 string date = pDepthMarketData->TradingDay;
// double miniChangeTick = m_instMessage_map_stgy[pDepthMarketData->InstrumentID]->PriceTick * 2; // ��������С�䶯�۸� ��֤�ɽ�

 //ofstream logging("output/" + s_instId + "_" + date + "_logging" + ".txt",ios::app);
 strcpy_s(instId, this->instrumentCode.toStdString().c_str());
 // ģ���� ʹ��1���۸� ���������� ��ƽ��ƽ
 if(hedgeDifference>0){
  // ���뿪��
  if(position >= 0){
   
 dir = '0';
 strcpy_s(kpp, "0");
 price = pDepthMarketData->AskPrice1;//��1������������
 vol = hedgeDifference;
 TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;

  }
  //��ǰ�ֲ�
  else{
   // >0 ��������ƽ�� �����뿪��
   if((hedgeDifference + position) > 0){

 //��ƽ position
 dir = '0';
 strcpy_s(kpp, "3");
 price = pDepthMarketData->AskPrice1;
 vol = -position;
 if(vol >0 ){
  TDSpi_stgy->StraitClose(instId, dir, price, vol);
 }
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;
 //�� hedgeDifference + position
 strcpy_s(kpp, "0");
 price = pDepthMarketData->AskPrice1;
 vol = hedgeDifference + position;
 TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;
   }
   else{
 //��ƽ hedgeDifference
   
 dir = '0';
 strcpy_s(kpp, "3");
 price = pDepthMarketData->AskPrice1;
 vol = hedgeDifference;

 TDSpi_stgy->StraitClose(instId, dir, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;
   }
  }
 }
 /// �Գ��ֵ Ϊ �� Ҫô����ƽ�� Ҫô ��������
 else if(hedgeDifference < 0){
  // ��������
  if(position <= 0){
 dir = '1';
 strcpy_s(kpp, "0");
 price = pDepthMarketData->BidPrice1;
 vol = -hedgeDifference;

 TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;

  }
  //��ǰ�ֲ�
  else{
   // <0 ��������ƽ�� ����������
   if((hedgeDifference + position) < 0){
 // ����ƽ
 dir = '1';
 //strcpy_s(kpp, "3");
 price = pDepthMarketData->BidPrice1 ;
 vol = position;

 TDSpi_stgy->StraitClose(instId, dir, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;

 //������
   
 dir = '1';
 strcpy_s(kpp, "0");
 price =pDepthMarketData->BidPrice1;
 vol = -hedgeDifference - position;

 TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;

   }
   else{
 //��ƽ hedgeDifference
 // ����ƽ
 dir = '1';
 strcpy_s(kpp, "3");
 price =pDepthMarketData->BidPrice1;
 vol = -hedgeDifference;

 TDSpi_stgy->StraitClose(instId, dir, price, vol);
   // logging << instId << "  dir:" << dir << " kp:" << kpp << " p: " << price << "  v:" << vol << endl;
   }
  }
 }
// logging.close();

}



void QStrategyThread::loadConf(QString confName){

 QString saveproductName = QString("output/product/") + confName ;
 QSettings *configSetting = new QSettings(saveproductName, QSettings::IniFormat); 
 configSetting->setIniCodec("GB2312");
 this-> productName =configSetting->value("/OptionParam/productName").toString();
 this-> optionType = configSetting->value("/OptionParam/optionKind").toInt(); // combox�Ǵ�0��ʼ�����
 this-> scaleUnit = configSetting->value("/OptionParam/scaleUnit").toInt(); 
 this-> callScale = configSetting->value("/OptionParam/callScale").toDouble(); 
 this->  putScale = configSetting->value("/OptionParam/putScale").toDouble(); 
 this->  settlementDate = configSetting->value("/OptionParam/settlementDate").toDateTime();
 this->  maturityDate = configSetting->value("/OptionParam/maturityDate").toDateTime();
 this->  nextAveragePoint = configSetting->value("/OptionParam/nextAveragePoint").toDateTime();
 this->  firstAveragePoint = configSetting->value("/OptionParam/firstAveragePoint").toDateTime();
 this->  m_initialPrice = configSetting->value("/OptionParam/initialPrice").toDouble();
 this->  averagePrice = configSetting->value("/OptionParam/averagePrice").toDouble();
 this->  m_callExecPricePercent = configSetting->value("/OptionParam/callExecPricePercent").toDouble();
 this->  m_putExecPricePercent = configSetting->value("/OptionParam/putExecPricePercent").toDouble();
 this->  interest = configSetting->value("/OptionParam/interest").toDouble();
 this->  dividend = configSetting->value("/OptionParam/dividend").toDouble();
 this->  barrier = configSetting->value("/OptionParam/barrier").toDouble();
 
 this->  callHedgeDirection = configSetting->value("/OptionParam/callHedgeDirection").toInt();
 this->  putHedgeDirection = configSetting->value("/OptionParam/putHedgeDirection").toInt();
 //this->  priceVolability = configSetting->value("/OptionParam/priceVolability").toDouble();
 //this->  margin = configSetting->value("/OptionParam/margin").toString();
 //this->  openCommission = configSetting->value("/OptionParam/openCommission").toString();
 this->  strategyStartTime = configSetting->value("/OptionParam/strategyStartTime").toTime();
 //this->  settleTime = configSetting->value("/OptionParam/settleTime").toTime();
 this->  m_openNums = configSetting->value("/OptionParam/openNums").toInt();
 this->  hedgeLevel = configSetting->value("/OptionParam/hedgeLevel").toString();
 this->  hedgeVolability = configSetting->value("/OptionParam/hedgeVolability").toDouble();
 this->  carryNoonTime = configSetting->value("/OptionParam/carryNoonTime").toTime();
 this->  carryNightTime = configSetting->value("/OptionParam/carryNightTime").toTime();
 this->  carryNoonDistance = configSetting->value("/OptionParam/carryNoonDistance").toInt(); 
 this->  carryNightDistance = configSetting->value("/OptionParam/carryNightDistance").toInt();
 this->  totalAveragePoint = configSetting->value("/OptionParam/totalAveragePoint").toInt();
 this->  averagedPoint = configSetting->value("/OptionParam/averagedPoint").toInt();


 this->  instrumentCode = configSetting->value("/OptionParam/instrumentCode").toString(); 
 this->  instrumentCode2 = configSetting->value("/OptionParam/instrumentCode2").toString(); 
 this->  instrumentCode3 = configSetting->value("/OptionParam/instrumentCode3").toString(); 
 this->  instrumentCode4 = configSetting->value("/OptionParam/instrumentCode4").toString(); 

 this->  tickValue = configSetting->value("/OptionParam/tickValue").toDouble();
 this->  tickValue2 = configSetting->value("/OptionParam/tickValue2").toDouble();
 this->  tickValue3 = configSetting->value("/OptionParam/tickValue3").toDouble();
 this->  tickValue4 = configSetting->value("/OptionParam/tickValue4").toDouble();

 this->  instrumentRatio = configSetting->value("/OptionParam/instrumentRatio").toDouble();
 this->  instrumentRatio2 = configSetting->value("/OptionParam/instrumentRatio2").toDouble();
 this->  instrumentRatio3 = configSetting->value("/OptionParam/instrumentRatio3").toDouble();
 this->  instrumentRatio4 = configSetting->value("/OptionParam/instrumentRatio4").toDouble();

 this->  hedgeThreshold = configSetting->value("/OptionParam/hedgeThreshold").toDouble();

 this->m_tradeCostPercent = configSetting->value("/OptionParam/tradeCostPercent").toDouble();
 this->m_riskAversion = configSetting->value("/OptionParam/riskAversion").toDouble();

 // 20180516
 this->m_bookStatus = configSetting->value("/OptionParam/bookStatus").toInt();
 this->m_PNL = configSetting->value("/OptionParam/PNL").toDouble();
 this->m_daySettlePrice = configSetting->value("/OptionParam/daySettlePrice").toDouble();
 this->m_optionFee = configSetting->value("/OptionParam/optionFee").toDouble();
 this->m_counterParty = configSetting->value("/OptionParam/counterParty").toString();
 this->m_deliveryPrice = configSetting->value("/OptionParam/deliveryPrice").toDouble();
 this->m_paymentDate = configSetting->value("/OptionParam/paymentDate").toDateTime();
 this->m_deliveryDate = configSetting->value("/OptionParam/deliveryDate").toDateTime();
 this->m_terminationDate = configSetting->value("/OptionParam/terminationDate").toDateTime();
 this->m_tradeValue = configSetting->value("/OptionParam/tradeValue").toDouble();
 this->m_presentValue = configSetting->value("/OptionParam/presentValue").toDouble();
 this->m_returnProfit = configSetting->value("/OptionParam/returnProfit").toDouble();
 delete configSetting;
}



void QStrategyThread::setModelData(QString itemName,optionGreeks CallOptionGreeks,optionGreeks PutOptionGreeks,optionHedgeHands CurHedgeHands,double ratio,double lastPrice,QString inst,cashGreeks& greeksCash)
{
 QString InstrumentName = itemName;

 double dailyPNL = 0;
 double _delta = (CallOptionGreeks.delta + PutOptionGreeks.delta) * ratio;
 double deltaCash = (greeksCash._deltaCash) * ratio;

 //double _theta = (CallOptionGreeks.theta  + PutOptionGreeks.theta) * ratio;
 double thetaCash = (CallOptionGreeks.theta  + PutOptionGreeks.theta) * ratio;
 //double _vega = (CallOptionGreeks.vega + PutOptionGreeks.vega) * ratio;
 double vegaCash = (greeksCash._vegaCash) * ratio;
 //double _gamma = (CallOptionGreeks.gamma + PutOptionGreeks.gamma) * ratio;
 double gammaCash = (greeksCash._gammaCash) * ratio;
 double _rho = (CallOptionGreeks.rho + PutOptionGreeks.rho) * ratio;
 QString sdelta = QString::number(_delta,'f', 3);
 QString sdeltaCash = QString::number(deltaCash,'f', 0);
 QString stheta = QString::number(thetaCash,'f', 0);
 QString svega = QString::number(vegaCash,'f', 0);
 QString sgamma = QString::number(gammaCash,'f', 0);
 QString srho = QString::number(_rho,'f', 3);
 QString sLastPrice = QString::number(lastPrice,'f', 2);
 QString RealHedgeHands = QString::number(CurHedgeHands.RealHedgeHands,'f', 1);
 QString netPos = QString::number(CurHedgeHands.netPos,'f', 1);
 QString hedgeDifference = QString::number(CurHedgeHands.hedgeDifference,'f',1);

 QList<QStandardItem *> tList = this->m_strategyGreekModel->findItems(InstrumentName);
 if (tList.count()>0)
 {
  int row = tList.at(0)->row();
  
  //m_strategyGreekModel->beginResetModel();
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 1), sdelta);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 2), sdeltaCash);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 3), sgamma);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 4), stheta);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 5), svega);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 6), sLastPrice);//���׺�Լ
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 7), RealHedgeHands);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 8), QString(instrumentMarketData[inst].UpdateTime));
   //m_strategyGreekModel->endResetModel();
  emit dataChange(m_strategyGreekModel->index(row, 1),m_strategyGreekModel->index(row, 7));
  

  m_strategyGreekModel->item(row,1)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,2)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,3)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,4)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,5)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,6)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,7)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,8)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);


 }
 else
 {
  //m_strategyGreekModel->beginInsertRows();
  m_strategyGreekModel->insertRow(0);
  m_strategyGreekModel->setData(m_strategyGreekModel->index(0, 0), InstrumentName);
  //m_strategyGreekModel->endInsertRows();
 }
 //emit update();
}
void QStrategyThread::PrintLogging(CThostFtdcDepthMarketDataField *pDepthMarketData,optionGreeks callGreeks,optionGreeks putGreeks,optionHedgeHands hedgeHands,optionParam _optionParam){
  static bool isFirst = true;
  //== 
  string s_instId = pDepthMarketData->InstrumentID;
  string date = pDepthMarketData->TradingDay;

  //
  char namec[255];
  wstring namew = this->productName.toStdWString();
  int nLen = WideCharToMultiByte(CP_ACP, 0, namew.c_str(), -1, NULL, 0, NULL, NULL);
  char* pszDst = new char[nLen];
  WideCharToMultiByte(CP_ACP, 0, namew.c_str(), -1, pszDst, nLen, NULL, NULL);
  pszDst[nLen -1] = 0;
  std::string strTemp(pszDst);
  delete [] pszDst;
  string name(strTemp);
  //
  // ���� ���Ǳ��浽���ݿ���
  ofstream output;
  output.open("output/" + name+ "_" +s_instId   + "_" + date +".csv",ios::_Nocreate | ios::ate) ;
  if(output){
   //content
   output << pDepthMarketData->ActionDay << "," << pDepthMarketData->UpdateTime<< ","<< _optionParam.remainTime << "," << pDepthMarketData->LastPrice << "," << _optionParam.callExecPrice << "," << _optionParam.putExecPrice << "," 
    << _optionParam.volability << "," << _optionParam.settlementDate << "," << _optionParam.maturityDate << ","
    << _optionParam.callScale << "," << callGreeks.delta<<","<< hedgeHands.callhedgeHands<< "," 
    <<_optionParam.putScale << "," << putGreeks.delta<<","<< hedgeHands.puthedgeHands  << ","
    << hedgeHands.netPos
    << endl;
  }else{
   // title
    output.open("output/" + name + "_" + s_instId   + "_" + date + ".csv", ios::app);
    output << "ActionDay" << "," << "updateTime" << ","<< "remainTime" << "," << "lastPrice" << "," << "callStrikePrice" << "," << "putStrikePrice" << "," 
    << "vol" << "," <<"settlementDate" << "," << "maturityDate" << ","
    << "���ǹ�ģ" << "," << "����delta" <<","<< "����Ӧ�Գ�����" << "," 
    << "������ģ" << "," << "����delta" <<","<< "����Ӧ�Գ�����"  << ","
    << "�ú�Լ��ͷ��"
    << endl;
  
  }
  output.close();

}

void QStrategyThread::setTDSpi(CtpTraderSpi *spi){
 this->TDSpi_stgy = spi;
}


// ����_valueType��������ֵ
bool QStrategyThread::optionChoose(double underlyingPrice ,time_t _curDateTime,optionGreeks &_CallgreeksValue,optionGreeks &_putGreeksValue){

 double initialPirce = 0;
 if( this->m_initialPrice<= 0){
  initialPirce = underlyingPrice;
 }else{
  initialPirce = this->m_initialPrice;
 }

 //this->m_callExerPrice = initialPirce * this->m_callExecPricePercent;
 //this->m_putExerPrice = initialPirce * this->m_putExecPricePercent;

 this->m_callExerPrice = this->m_callExecPricePercent;
 this->m_putExerPrice =  this->m_putExecPricePercent;
 optionSpace::OptionType optionStyle;
 //ѡ����Ȩ����
 asianPrice asianP;

 europeanPrice euroP;

 bjsmodel ameriPBjs;

 // �޸�deltaΪ�Գ��
 switch (this->optionType)
 {
 case 0:
  if(this->callScale >0){
  optionStyle = optionSpace::OptionType::CALL;
     _CallgreeksValue.price = euroP.price(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _CallgreeksValue.delta = euroP.delta(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _CallgreeksValue.gamma = euroP.gamma(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _CallgreeksValue.vega = euroP.vega(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _CallgreeksValue.theta = euroP.theta(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau,1.0/252);
     _CallgreeksValue.rho = euroP.rho(optionStyle,this->m_callExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
  }
  if(this->putScale >0){

     optionStyle = optionSpace::OptionType::PUT;
     _putGreeksValue.price = euroP.price(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _putGreeksValue.delta = euroP.delta(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _putGreeksValue.gamma = euroP.gamma(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _putGreeksValue.vega = euroP.vega(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
     _putGreeksValue.theta = euroP.theta(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau,1.0/252);
     _putGreeksValue.rho = euroP.rho(optionStyle,this->m_putExerPrice,this->interest,this->dividend,underlyingPrice,this->hedgeVolability,this->m_tau);
 }
  break;
 case 2: // ��ʽ����

  if(this->callScale>0){
     optionStyle = optionSpace::OptionType::CALL;
  _CallgreeksValue.price =  asianP.AsianCurranApprox(optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _CallgreeksValue.delta =  asianP.delta (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _CallgreeksValue.gamma =  asianP.gamma (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _CallgreeksValue.vega =  asianP.vega (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _CallgreeksValue.theta = asianP.theta (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _CallgreeksValue.rho =  asianP.rho (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_callExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  }
  if(this->putScale >0){
  optionStyle = optionSpace::OptionType::PUT;
  _putGreeksValue.price = asianP.AsianCurranApprox(optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _putGreeksValue.delta = asianP.delta (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _putGreeksValue.gamma = asianP.gamma (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _putGreeksValue.vega = asianP.vega (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _putGreeksValue.theta = asianP.theta (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  _putGreeksValue.rho =  asianP.rho (optionStyle,underlyingPrice,this->m_averagePriceAdjust,this->m_putExerPrice,m_t1,m_tau,this->totalAveragePoint,this->m_averagedPointAdjust,this->interest ,this->dividend,this->hedgeVolability);

  }
  break;
 case 1://��ʽ
  if(this->callScale >0){
     

     _CallgreeksValue.price = ameriPBjs.BSAmericanApprox2002( 'c',underlyingPrice,this->m_callExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _CallgreeksValue.delta = ameriPBjs.delta( 'c',underlyingPrice,this->m_callExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _CallgreeksValue.gamma = ameriPBjs.gamma( 'c',underlyingPrice,this->m_callExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _CallgreeksValue.theta = ameriPBjs.theta( 'c',underlyingPrice,this->m_callExerPrice,m_tau,1/252.0,this->interest,this->dividend,this->hedgeVolability);
     _CallgreeksValue.vega = ameriPBjs.vega( 'c',underlyingPrice,this->m_callExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _CallgreeksValue.rho = ameriPBjs.rho( 'c',underlyingPrice,this->m_callExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);

  }
  if(this->putScale >0){
     _putGreeksValue.price = ameriPBjs.BSAmericanApprox2002( 'p',underlyingPrice,this->m_putExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _putGreeksValue.delta = ameriPBjs.delta( 'p',underlyingPrice,this->m_putExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _putGreeksValue.gamma = ameriPBjs.gamma( 'p',underlyingPrice,this->m_putExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _putGreeksValue.theta = ameriPBjs.theta( 'p',underlyingPrice,this->m_putExerPrice,m_tau,1/252.0,this->interest,this->dividend,this->hedgeVolability);
     _putGreeksValue.vega = ameriPBjs.vega( 'p',underlyingPrice,this->m_putExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);
     _putGreeksValue.rho = ameriPBjs.rho( 'p',underlyingPrice,this->m_putExerPrice,m_tau,this->interest,this->dividend,this->hedgeVolability);

 }
  break;
 case 3:
  //m_pOptionCalculate =  &optionEngine::BarrierCalculate;
  break;
 case 4:
  //m_pOptionCalculate =  &optionEngine::Digital;
  break;
 case 5:
  //m_pOptionCalculate =  &optionEngine::LookBacCalculate;
  break;
 case 6:
  if(this->callScale >0){
   _CallgreeksValue.price = 0;

   _CallgreeksValue.delta =  1;

   _CallgreeksValue.gamma = 0;

   _CallgreeksValue.vega = 0;

   _CallgreeksValue.theta = 0;

   _CallgreeksValue.rho = 0;

  }
  if(this->putScale >0){
   _putGreeksValue.price = 0;

   _putGreeksValue.delta =  -1;

   _putGreeksValue.gamma = 0;

   _putGreeksValue.vega = 0;

   _putGreeksValue.theta = 0;

   _putGreeksValue.rho = 0;

  }
  break;
 default:
  ERROR__OUTPUT("OptionKind Error");
  return false;
  break;
 }
 
 

 if(_isnan(_putGreeksValue.delta )){
  _putGreeksValue.reset();
  
 }
 if(_isnan(_CallgreeksValue.delta )){
  _CallgreeksValue.reset();
 }


 return true;
}


//�����껯���ʣ��ʱ�� ����ʱ��
bool QStrategyThread::valideDate(time_t _settlementDate,time_t _todayDate,time_t _maturityDate){
 bool remark = true;
 if((_settlementDate )> _todayDate || _todayDate > (_maturityDate) || (_settlementDate ) > _maturityDate)
 {
  remark = false;
 }
 return remark;
}




double QStrategyThread::getHands(int scaleUnit,double scale,double tickValue){
 double Hands = 0;
 if(this->scaleUnit == 0){//Ԫ 
  //Hands  = (scale *  (this->maturityDate.toTime_t()-this->settlementDate.toTime_t())/ YEAR_SECOND  /this->initialPrice / tickValue ); //�껪
  Hands  = (scale   /this->m_initialPrice / tickValue ); //���껯
 }
 else if(this->scaleUnit == 1)//��
 {
  Hands = (scale / tickValue);
 }
 return Hands;
}

double QStrategyThread::H0(double tau,double lamuda,double vol,double riskA,double gamma,double rate,double underlying){
  double hedge1 ;
  double temp;
  temp = 1.5 * std::exp(-rate * tau) * lamuda * underlying * gamma * gamma / riskA;
  hedge1 = std::pow(temp,1.0/3.0);
  return hedge1;
}

void QStrategyThread::bookSettle(){
 //===================���·��Price==================================
 double  underlyingPrice = 0;
 for (auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
  underlyingPrice += instrumentMarketData[i->instrumentCode].LastPrice * i->ratio;
 }
 //this->timeCalculate("",underlyingPrice); t1��ˢ�� �Լ� T��ˢ��

 this->m_callExerPrice = this->m_callExecPricePercent;
 this->m_putExerPrice =  this->m_putExecPricePercent;
 
 this->m_presentValue = 0;
 optionSpace::OptionType optionStyle;
 europeanPrice euroP;
 asianPrice asianP;
 bjsmodel ameriBjsP;

 // settlePrice Judge
 if(m_bookStatus == 0 ){
  if(underlyingPrice >0){
   m_daySettlePrice = underlyingPrice;
  }

 }
 else{
  m_daySettlePrice = m_deliveryPrice;
 }
 double callDire = -1, putDire = -1;
 // custom sell then we buy
 if(this->callHedgeDirection == 1){
  callDire = 1;
 }
 // cumstom sell then we buy
 if(this->putHedgeDirection == 1){
  putDire = 1;
 }

 string sSettlementDate = this->settlementDate.toString("yyyyMMdd").toStdString();
 string sMaturityDate = this->maturityDate.toString("yyyyMMdd").toStdString();
 string sNextAPointDate = this->nextAveragePoint.toString("yyyyMMdd").toStdString();
 time_t t = time(0);
 char today[64];
 strftime( today, sizeof(today), "%Y%m%d",localtime(&t) ); 
 double tradeDayNums = this->m_workDay[sMaturityDate] - this->m_workDay[today];
 double nextPointNums = this->m_workDay[sNextAPointDate] - this->m_workDay[today];
 this->m_settleTau =   tradeDayNums * DAY_SECOND / YEAR_SECOND_WORKDAY;
 this->m_settleT1 = nextPointNums * DAY_SECOND / YEAR_SECOND_WORKDAY;
 if(string(today) == sSettlementDate && m_tradeValue == 0){
    m_tradeValue = this->m_optionFee * (this->putScale*putDire + this->callScale * callDire);
 }


 switch (this->optionType)
 {
  case 0:
   this->m_settleT1  = 0;
   if(this->callScale >0){
    optionStyle = optionSpace::OptionType::CALL;
    m_presentValue = callDire * euroP.price(optionStyle,this->m_callExerPrice,this->interest,this->dividend,m_daySettlePrice,this->hedgeVolability,m_settleTau);
   }
   if(this->putScale >0){
    optionStyle = optionSpace::OptionType::PUT;
    m_presentValue += putDire * euroP.price(optionStyle,this->m_putExerPrice,this->interest,this->dividend,m_daySettlePrice,this->hedgeVolability,m_settleTau);
   }
   break;
  case 1:
   //american
   if(this->callScale >0){
    optionStyle = optionSpace::OptionType::CALL;
    m_presentValue = callDire * ameriBjsP.BSAmericanApprox2002('c',m_daySettlePrice,this->m_callExerPrice,m_settleTau,this->interest,this->dividend,this->hedgeVolability);
   }
   if(this->putScale >0){
    optionStyle = optionSpace::OptionType::PUT;
    m_presentValue += putDire * ameriBjsP.BSAmericanApprox2002('p',m_daySettlePrice,this->m_putExerPrice,m_settleTau,this->interest,this->dividend,this->hedgeVolability);
   }   
   break;
  
  case 2:
   //asian
   if(this->callScale>0){
    optionStyle = optionSpace::OptionType::CALL;
    m_presentValue =  callDire * asianP.AsianCurranApprox(optionStyle,m_daySettlePrice,this->averagePrice,this->m_callExerPrice,m_settleT1,m_settleTau,this->totalAveragePoint,this->averagedPoint,this->interest ,this->dividend,this->hedgeVolability);
   }
   if(this->putScale > 0){
    optionStyle = optionSpace::OptionType::PUT;
    m_presentValue +=  putDire * asianP.AsianCurranApprox(optionStyle,m_daySettlePrice,this->averagePrice,this->m_putExerPrice,m_settleT1,m_settleTau,this->totalAveragePoint,this->averagedPoint,this->interest ,this->dividend,this->hedgeVolability);
   
   }
   break;

  case 3:
   this->m_settleT1  = 0;
   break;
   //barrier
  case 4:
   this->m_settleT1  = 0;
   break;
   //binary
  case 5:
   this->m_settleT1  = 0;
   //look back
   break;
  case 6:
   // forward
   this->m_settleT1  = 0;
   if(this->callScale>0){
    m_presentValue = callDire*(m_daySettlePrice - m_initialPrice );
   }
   if(this->putScale > 0){
    m_presentValue += putDire * (m_initialPrice - m_daySettlePrice );
   }
   break;;
  default:
   break;
  }

 m_presentValue = m_presentValue  * (this->putScale + this->callScale);
 m_PNL  = m_presentValue - m_tradeValue;
}
void QStrategyThread::settleSaveToIniFile(){
 
    // 20180515��������
 QString saveproductName = QString("output/product/") + this->productName + QString(".ini") ;
 QSettings *configIniWrite = new QSettings(saveproductName, QSettings::IniFormat); 
 configIniWrite->setIniCodec("GB2312");
 configIniWrite->setValue("/OptionParam/PNL",m_PNL);
 configIniWrite->setValue("/OptionParam/optionFee",m_optionFee);
 configIniWrite->setValue("/OptionParam/deliveryPrice",m_deliveryPrice);
 configIniWrite->setValue("/OptionParam/daySettlePrice",m_daySettlePrice);

 configIniWrite->setValue("/OptionParam/tradeValue",m_tradeValue);
 configIniWrite->setValue("/OptionParam/presentValue",m_presentValue);

 delete configIniWrite;

}
void QStrategyThread::settleUpdate(){
 this->m_tradeValue = this->m_presentValue;
 this->settleSaveToIniFile();
}


/// ����ˢ����Ҫ�Ĳ��� �Ѽ����ֵ�� ��ֵ��  ��һ����ֵ��
void QStrategyThread::bookFresh(){

 if(this->m_workDay.size() == 0){
  return;
 }

  //===================���·��Price==================================
 double  underlyingPrice = 0;
 for (auto i = this->m_instrumentList.begin();i!=this->m_instrumentList.end();i++){
  underlyingPrice += instrumentMarketData[i->instrumentCode].LastPrice * i->ratio;
 }

 optionSpace::OptionType optionStyle;


 // settlePrice Judge
 if(m_bookStatus == 0 ){
  if(underlyingPrice >0){
   m_daySettlePrice = underlyingPrice;
  }

 }
 else{
  m_daySettlePrice = m_deliveryPrice;
  
 }
 time_t t = time(0);
 time_t curTimeTic,everySettleTime;
 char today[64],dateTime[64];
 strftime( today, sizeof(today), "%Y%m%d",localtime(&t) ); 
 strftime( dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S",localtime(&t) );
 int iYear,iMonth,iDay,iHour,iMinute,iSecond;
 sscanf(dateTime,"%d-%d-%d %d:%d:%d",&iYear,&iMonth,&iDay,&iHour,&iMinute,&iSecond);
 everySettleTime = dateTime2unix(iYear,iMonth,iDay,this->strategyStartTime.hour(),this->strategyStartTime.minute(),this->strategyStartTime.second());/// ÿ�ս���ʱ���
 curTimeTic = dateTime2unix(iYear,iMonth,iDay,iHour,iMinute,iSecond);/// ��ǰʱ��ת����ʱ���
 
 string snextAverageStartDate = this->nextAveragePoint.toString("yyyyMMdd").toStdString();

 switch (this->optionType)
 {
 case 0:
 case 1:
 case 3:
 case 4:
 break;
 case 2:
  // �������ʱ�� ��Ҫˢ����һ����ֵ��ʱ�� ��ǰ��ֵ����ֵ�������
  if(curTimeTic > everySettleTime){
   // ��ǰʱ�䳬������һ����ֵ�����
   if(curTimeTic > this->nextAveragePoint.toTime_t()){
     this->m_averagedPointAdjust = this->averagedPoint + 1;
     this->m_averagePriceAdjust = (this->averagePrice * this->averagedPoint + underlyingPrice)/double(this->m_averagedPointAdjust );
     if(this->m_averagedPointAdjust < this->totalAveragePoint){
      auto iter  =  this->m_workDay.find(snextAverageStartDate);
      iter++;

       QString nextAveragePointPlusone = QString::fromStdString(iter->first);
       QDateTime nextTradeDate = QDateTime::fromString(nextAveragePointPlusone,"yyyyMMdd");
       nextTradeDate.setTime(QTime(15,0,0));
       this->nextAveragePoint = nextTradeDate;
     }
    QString saveproductName = QString("output/product/") + this->productName + QString(".ini") ;
    QSettings *configIniWrite = new QSettings(saveproductName, QSettings::IniFormat); 
    configIniWrite->setIniCodec("GB2312");
    configIniWrite->setValue("/OptionParam/nextAveragePoint",nextAveragePoint.toString("yyyy-MM-ddThh:mm:ss"));
    configIniWrite->setValue("/OptionParam/averagedPoint",m_averagedPointAdjust);
    configIniWrite->setValue("/OptionParam/averagePrice",m_averagePriceAdjust);
    delete configIniWrite;
   }
  }


 break;
 default:
 break;
 }



  


}

void QStrategyThread::settlePrintToExcel(bool saveToExcel){
   QString msg;
 // print message
 emit strategyMsg("BOOK: " + this->productName + " Inst: "  + this->instrumentCode + "PNL: " +
 QString::number( this->m_PNL,'f',2) + " TradeValue: " + QString::number(this->m_tradeValue,'f',2) + " PV :" + QString::number(this->m_presentValue,'f',2));

 time_t t = time(0);
 char tmp[64];
 strftime( tmp, sizeof(tmp), "%Y%m%d",localtime(&t) ); 
 string today(tmp);
 string callPut = "put";
 double strike = this->m_putExerPrice;
 string optionStyle[7] = {"euro","american","asian","barrier","binary","lookback","forward"};
 if(this->callScale >0){
  callPut = "call";
  strike = this->m_callExerPrice;
 }
 if(this->optionType ==6){
  strike = this->m_initialPrice;
 }

 if(saveToExcel){
  // ���� ���Ǳ��浽���ݿ���
  ofstream output;
  output.open("output/settle/settle_"+today+".csv",ios::_Nocreate | ios::ate) ;
  if(output){
  //content
  output << today << ","  << this->productName.toStdString() << ","<< this->instrumentCode.toStdString() <<"," << this->m_bookStatus<< "," << this->m_PNL << ","<< this->m_tradeValue << "," << this->m_presentValue << ","  <<  optionStyle[this->optionType] 
  << "," << callPut << "," << this->callScale + this->putScale << ","<< this->m_settleTau << "," << this->hedgeVolability << "," << strike << "," << this->m_daySettlePrice << ","
  << this->interest << "," << this->dividend << "," << this->averagePrice << "," <<this->totalAveragePoint  << "," << this->averagedPoint << ","
  << this->m_settleT1   << endl;
  }else{
  // title
  output.open("output/settle/settle_"+today+".csv", ios::app);
  output << "DATE" << ","  << "BOOKID"<<","<< "INST"  << "," << "STATUS" << "," << "PNL" <<","<< "TV" << "," << "PV" << ","  <<  "TYPE" << "," << "CALLPUT" << "," << "SCALE" << ","<< "T" << "," << "HV" << "," << "K" 
  << "," << "SETTLEPRICE" << ","<< "RATE" << "," << "COSTOFCARRY" << "," << "AVERAGEPRICE" << "," << "M"  << "," << "N" << "," <<   "T1"   << endl;
  output << today << ","  << this->productName.toStdString() << ","<< this->instrumentCode.toStdString() <<"," << this->m_bookStatus<< "," << this->m_PNL << ","<< this->m_tradeValue << "," << this->m_presentValue << ","  <<  optionStyle[this->optionType] 
  << "," << callPut << "," << this->callScale + this->putScale << ","<< this->m_settleTau << "," << this->hedgeVolability << "," << strike << "," << this->m_daySettlePrice << ","
  << this->interest << "," << this->dividend << "," << this->averagePrice << "," <<this->totalAveragePoint  << "," << this->averagedPoint << ","
  << this->m_settleT1   << endl;
  
  }
  output.close();
 
 }

}