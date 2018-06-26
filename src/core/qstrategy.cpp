#include "qstrategy.h"
#include <qfile.h>
#include <qsettings.h>
#include <qstring.h>
#include <qdir.h>
#include <qstringlist.h>
#include "StructFunction.h"
#include <qsqlrecord.h>
#include "OracleOperateFun.h"


#define QSTRATEGYCONFIGFILE "input/templateProductParam.ini"
QStrategy::QStrategy(QWidget *parent,QStandardItemModel *pOrderManageModel)
 : QDialog(parent)
{
 ui.setupUi(this);
 //loadTemplate();//载入模版

 edit = false;
 this->m_pOrderManager = new QOrderManager(0,parent,pOrderManageModel);
 m_pOrderManager->moveToThread(&this->m_strategyManagerThread);
 m_strategyManagerThread.start();
 //connect(this,SIGNAL(subscribe_inst_data(QString)),parent,SLOT(subscribe_inst_data(QString))  ); 
 connect(this->ui.onSearchPushButton,SIGNAL(clicked()),this,SLOT(on_searchBook_clicked()));
 connect(this->ui.onSettlePushButton,SIGNAL(clicked()),this,SLOT(onSettleClicked()));
 // 初始化计时器
 connect(&this->m_sumTimer,SIGNAL(timeout()),this,SLOT(strategyStatistic()));
 this->m_sumTimer.start(1000);// 每隔1s检查一次；
 ::readDay("input/workday.txt",this->m_workDay);
 ::readDay("input/calendarDay.txt",this->m_calendarDay);


}
QOrderManager * QStrategy::getOrderManager(){
 return this->m_pOrderManager;
}

QStrategy::~QStrategy()
{

 for(auto iter = this->m_pStrategyList.begin();iter!= this->m_pStrategyList.end();iter++){
  (*iter)->m_status = false;
  (*iter)->exit();
 }
 this->m_strategyManagerThread.quit();
 this->m_strategyManagerThread.wait();

}
void QStrategy::on_save_clicked(){
 // 每个策略的
 QString productName;
 int optionType;
 int scaleUnit ;
 QString callScale ;
 QString putScale;
 QDateTime settlementDate;
 QDateTime maturityDate;
 QDateTime nextAveragePoint;
 QDateTime firstAveragePoint;

 QString initialPrice ;
 QString averagePrice;
 QString callExecPricePercent ; //看涨执行价百分比
 QString putExecPricePercent ;// 看跌执行加百分比
 int callHedgeDirection;
 int putHedgeDirection;
 QString interest;
 QString dividend;
 QString barrier ;

 QTime strategyStartTime;

 QString openNums;
 QString hedgeLevel ;
 QString hedgeVolability ;
 QTime carryNoonTime;
 QTime carryNightTime ;
 int carryNoonDistance ;
 int carryNightDistance;
 int totalAveragePoint;
 int averagedPoint;
 QString instrumentCode ;
 QString instrumentCode2;
 QString instrumentCode3;
 QString instrumentCode4;
 double instrumentRatio;
 double instrumentRatio2;
 double instrumentRatio3;
 double instrumentRatio4;
 double tickValue;
 double tickValue2;
 double tickValue3;
 double tickValue4;
 int hedgeThreshold;
 double tradeCostPercent;
 double riskAversion;
 
 // new parameters at 2018/05/15
 int bookStauts;
 QString PNL;
 QString daySettlePrice;
 QString optionFee;
 QString counterParty;
 QString deliveryPrice; //交割价
 QDateTime paymentDate;   //交付日
 QDateTime deliveryDate;//交割日
 QDateTime terminationDate; //终止日
 QString tradeValue;
 QString presentValue;
 QString returnProfit;


 productName = this->ui.productNameLineEdit->text();
 QString allName = this->ui.productNameLineEdit->text() +QString(".ini");
 QDir *checkDir = new QDir("output/product/");
 QStringList filter;
 filter << "*.ini";
 checkDir->setNameFilters(filter);
 QList<QFileInfo> *fileinfo = new QList<QFileInfo>(checkDir->entryInfoList(filter));
 for (auto iter = fileinfo->begin(); iter != fileinfo->end(); iter++)
 {
  QString curFileName = iter->fileName();
  if(curFileName ==  allName && !edit){
   this->ui.tipsLabel->setText("Existed productName");
   delete fileinfo;
   return;
  }
 }

 settlementDate = this->ui.settlementDateEdit->dateTime();
 maturityDate = this->ui.maturityDateEdit->dateTime();
 nextAveragePoint = this->ui.nextApointDateEdit->dateTime();
 firstAveragePoint = this->ui.firstApointDateEdit->dateTime();
 scaleUnit = this->ui.scaleUnitComboBox->currentIndex();
 callScale = this->ui.callScaleLineEdit->text();
 putScale = this->ui.putScaleLineEdit->text();
 initialPrice = this->ui.initialPriceLineEdit->text();
 callExecPricePercent = this->ui.callExecPricePercentLineEdit->text();
 putExecPricePercent = this->ui.putExecPricePercentLineEdit->text();
 averagePrice = this->ui.averagePriceLineEdit->text();
 interest = this->ui.rateLineEdit->text();
 optionType = this->ui.typeComboBox->currentIndex() ;
 dividend = this->ui.yieldLineEdit->text();
 barrier = this->ui.barrierLineEdit->text();

 callHedgeDirection = this->ui.callTypeComboBox->currentIndex();
 putHedgeDirection = this->ui.putTypeComboBox->currentIndex();

 hedgeVolability =  this->ui.hedgeVolLineEdit->text();
 strategyStartTime = this->ui.startTimeEdit->time();
 openNums = this->ui.openNumsLineBox->text();
 //hedgeLevel = this->ui.hedgeLevelLineBox->text();

 carryNoonTime = this->ui.carryNoonTimeEdit->time();
 carryNoonDistance = this->ui.carryNoonDistanceSpinBox->value();
 carryNightTime = this->ui.carryNightTimeEdit->time();
 carryNightDistance = this->ui.carryNightDistanceSpinBox->value();
 totalAveragePoint = this->ui.totalAveragePoint->value();
 averagedPoint = this->ui.averagedPoint->value();
 
 instrumentCode  = this->ui.instrumentCodeLineEdit->text();
 instrumentCode2 = this->ui.instrumentCodeLineEdit_2->text();
 instrumentCode3 = this->ui.instrumentCodeLineEdit_3->text();
 instrumentCode4 = this->ui.instrumentCodeLineEdit_4->text();
 
 instrumentRatio = this->ui.InstrumentRatioSpinBox->value();
 instrumentRatio2= this->ui.InstrumentRatioSpinBox_2->value();
 instrumentRatio3= this->ui.InstrumentRatioSpinBox_3->value();
 instrumentRatio4= this->ui.InstrumentRatioSpinBox_4->value();
 
 tickValue = this->ui.tickValue->value();
 tickValue2 = this->ui.tickValue_2->value();
 tickValue3 = this->ui.tickValue_3->value();
 tickValue4 = this->ui.tickValue_4->value();

 tradeCostPercent = this->ui.TradeCostDSpinBox->value();
 riskAversion = this->ui.riskAversionDSpinBox->value();
 
 hedgeThreshold = this->ui.hedgeThresHoldSpinBox->value();


 // new parameters
 bookStauts = this->ui.statusComboBox->currentIndex();
 PNL = this->ui.PNLLineEdit->text();
 daySettlePrice = this->ui.settlePriceLineEdit->text();
 optionFee = this->ui.optionFeeLineEdit->text();
 counterParty = this->ui.counterpartyLineEdit->text();
 deliveryPrice = this->ui.deliveryPriceLineEdit->text(); //交割价
 paymentDate = this->ui.paymentDateEdit->dateTime();   //交付日
 tradeValue  = this->ui.TradeValueLineEdit->text();
 presentValue = this->ui.PresentValueEdit->text();
 deliveryDate = this->ui.deliveryDateEdit->dateTime();
 terminationDate = this->ui.terminateDateEdit->dateTime();
 returnProfit = this->ui.returnProfitLineEdit->text();

 QString saveproductName = QString("output/product/") + allName ;
 QSettings *configIniWrite = new QSettings(saveproductName, QSettings::IniFormat); 
 configIniWrite->setIniCodec("GB2312");
 configIniWrite->setValue("/OptionParam/productName",productName);
 configIniWrite->setValue("/OptionParam/optionKind",optionType); // combox是从0开始计算的
 configIniWrite->setValue("/OptionParam/scaleUnit",scaleUnit);
 configIniWrite->setValue("/OptionParam/callScale",callScale);
 configIniWrite->setValue("/OptionParam/putScale",putScale);
 configIniWrite->setValue("/OptionParam/settlementDate",settlementDate.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/maturityDate",maturityDate.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/nextAveragePoint",nextAveragePoint.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/firstAveragePoint",firstAveragePoint.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/initialPrice",initialPrice);
 configIniWrite->setValue("/OptionParam/averagePrice",averagePrice);
 configIniWrite->setValue("/OptionParam/callExecPricePercent",callExecPricePercent);
 configIniWrite->setValue("/OptionParam/putExecPricePercent",putExecPricePercent);
 configIniWrite->setValue("/OptionParam/interest",interest);
 configIniWrite->setValue("/OptionParam/dividend",dividend);
 configIniWrite->setValue("/OptionParam/barrier",barrier);

 configIniWrite->setValue("/OptionParam/callHedgeDirection",callHedgeDirection);
 configIniWrite->setValue("/OptionParam/putHedgeDirection",putHedgeDirection);

 configIniWrite->setValue("/OptionParam/strategyStartTime",strategyStartTime.toString());

 configIniWrite->setValue("/OptionParam/openNums",openNums);
 configIniWrite->setValue("/OptionParam/hedgeLevel",hedgeLevel);
 configIniWrite->setValue("/OptionParam/hedgeVolability",hedgeVolability);
 configIniWrite->setValue("/OptionParam/carryNoonTime",carryNoonTime.toString());
 configIniWrite->setValue("/OptionParam/carryNightTime",carryNightTime.toString());
 configIniWrite->setValue("/OptionParam/carryNoonDistance",carryNoonDistance); 
 configIniWrite->setValue("/OptionParam/carryNightDistance",carryNightDistance); 
 configIniWrite->setValue("/OptionParam/totalAveragePoint",totalAveragePoint);
 configIniWrite->setValue("/OptionParam/averagedPoint",averagedPoint);

 //不同合约
 configIniWrite->setValue("/OptionParam/instrumentCode",instrumentCode); 
 configIniWrite->setValue("/OptionParam/instrumentCode2",instrumentCode2); 
 configIniWrite->setValue("/OptionParam/instrumentCode3",instrumentCode3); 
 configIniWrite->setValue("/OptionParam/instrumentCode4",instrumentCode4); 
 //合约比例
 configIniWrite->setValue("/OptionParam/instrumentRatio",instrumentRatio); 
 configIniWrite->setValue("/OptionParam/instrumentRatio2",instrumentRatio2); 
 configIniWrite->setValue("/OptionParam/instrumentRatio3",instrumentRatio3); 
 configIniWrite->setValue("/OptionParam/instrumentRatio4",instrumentRatio4); 
 //阈值
 configIniWrite->setValue("/OptionParam/hedgeThreahold",hedgeThreshold); 
 //合约价值
 configIniWrite->setValue("/OptionParam/tickValue",tickValue);
 configIniWrite->setValue("/OptionParam/tickValue2",tickValue2);
 configIniWrite->setValue("/OptionParam/tickValue3",tickValue3);
 configIniWrite->setValue("/OptionParam/tickValue4",tickValue4);

 //交易成本 风险厌恶系数
 configIniWrite->setValue("/OptionParam/tradeCostPercent",tradeCostPercent);
 configIniWrite->setValue("/OptionParam/riskAversion",riskAversion);
 // 20180515新增参数
 configIniWrite->setValue("/OptionParam/bookStatus",bookStauts);
 configIniWrite->setValue("/OptionParam/PNL",PNL);
 configIniWrite->setValue("/OptionParam/optionFee",optionFee);
 configIniWrite->setValue("/OptionParam/counterParty",counterParty);
 configIniWrite->setValue("/OptionParam/deliveryPrice",deliveryPrice);
 configIniWrite->setValue("/OptionParam/daySettlePrice",daySettlePrice);
 configIniWrite->setValue("/OptionParam/paymentDate",paymentDate.toString("yyyy-MM-ddThh:mm:ss"));

 configIniWrite->setValue("/OptionParam/tradeValue",tradeValue);
 configIniWrite->setValue("/OptionParam/presentValue",presentValue);
 configIniWrite->setValue("/OptionParam/deliveryDate",deliveryDate.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/terminationDate",terminationDate.toString("yyyy-MM-ddThh:mm:ss"));
 configIniWrite->setValue("/OptionParam/returnProfit",returnProfit);



 delete configIniWrite;
 delete fileinfo;
 this->ui.tipsLabel->setText("save successful");

 //将新建合约加入到策略队列中 非运行状态下 编辑操作 也需要更新到 map中的文档内
 if(!edit){
  QStrategyThread *pThread = new QStrategyThread(m_strategyGreekModel,this->m_pOrderManager,this);
  connect(pThread,SIGNAL(strategyMsg(QString)),this->parent(),SLOT(onStrategyMsg(QString))  ); 
  
  pThread->loadConf(allName);
  this->m_pStrategyList.insert(productName,pThread);
 }else{

  this->m_pStrategyList[productName]->optionType = optionType;
  this->m_pStrategyList[productName]->scaleUnit = scaleUnit ;
  this->m_pStrategyList[productName]->callScale = callScale.toDouble() ;
  this->m_pStrategyList[productName]->putScale = putScale.toDouble();
  this->m_pStrategyList[productName]->settlementDate = settlementDate;
  this->m_pStrategyList[productName]->maturityDate = maturityDate;
  this->m_pStrategyList[productName]->nextAveragePoint = nextAveragePoint;
  this->m_pStrategyList[productName]->firstAveragePoint = firstAveragePoint;
  this->m_pStrategyList[productName]->m_initialPrice  = initialPrice.toDouble();
  this->m_pStrategyList[productName]->averagePrice = averagePrice.toDouble();
  this->m_pStrategyList[productName]->m_putExecPricePercent  = putExecPricePercent.toDouble();
  this->m_pStrategyList[productName]->m_callExecPricePercent  = callExecPricePercent.toDouble();
  this->m_pStrategyList[productName]->interest =interest.toDouble() ;
  this->m_pStrategyList[productName]->dividend = dividend.toDouble();
  this->m_pStrategyList[productName]->barrier  = barrier.toDouble();

  this->m_pStrategyList[productName]->callHedgeDirection  = callHedgeDirection;
  this->m_pStrategyList[productName]->putHedgeDirection  = putHedgeDirection;

  this->m_pStrategyList[productName]->strategyStartTime = strategyStartTime;

  this->m_pStrategyList[productName]->m_openNums = openNums.toInt();
  this->m_pStrategyList[productName]->hedgeLevel  = hedgeLevel;
  this->m_pStrategyList[productName]->hedgeVolability  = hedgeVolability.toDouble();
  this->m_pStrategyList[productName]->carryNoonTime = carryNoonTime;
  this->m_pStrategyList[productName]->carryNightTime  = carryNightTime;
  this->m_pStrategyList[productName]->carryNoonDistance  = carryNoonDistance;
  this->m_pStrategyList[productName]->carryNightDistance = carryNightDistance;
  this->m_pStrategyList[productName]->totalAveragePoint = totalAveragePoint;
  this->m_pStrategyList[productName]->averagedPoint = averagedPoint;
  this->m_pStrategyList[productName]->instrumentCode  = instrumentCode;
  this->m_pStrategyList[productName]->instrumentCode2  = instrumentCode2;
  this->m_pStrategyList[productName]->instrumentCode3  = instrumentCode3;
  this->m_pStrategyList[productName]->instrumentCode4  = instrumentCode4;

  this->m_pStrategyList[productName]->instrumentRatio   = instrumentRatio;
  this->m_pStrategyList[productName]->instrumentRatio2  = instrumentRatio2;
  this->m_pStrategyList[productName]->instrumentRatio3  = instrumentRatio3;
  this->m_pStrategyList[productName]->instrumentRatio4  = instrumentRatio4;

  this->m_pStrategyList[productName]->hedgeThreshold  = hedgeThreshold;

  this->m_pStrategyList[productName]->tickValue = tickValue;
  this->m_pStrategyList[productName]->tickValue2 = tickValue2;
  this->m_pStrategyList[productName]->tickValue3 = tickValue3;
  this->m_pStrategyList[productName]->tickValue4 = tickValue4;

  this->m_pStrategyList[productName]->m_tradeCostPercent = tradeCostPercent;
  this->m_pStrategyList[productName]->m_riskAversion = riskAversion;

  this->m_pStrategyList[productName]->m_bookStatus = bookStauts;
  this->m_pStrategyList[productName]->m_PNL = PNL.toDouble();
  this->m_pStrategyList[productName]->m_daySettlePrice = daySettlePrice.toDouble();
  this->m_pStrategyList[productName]->m_optionFee = optionFee.toDouble();
  this->m_pStrategyList[productName]->m_counterParty = counterParty;
  this->m_pStrategyList[productName]->m_deliveryPrice = deliveryPrice.toDouble();
  this->m_pStrategyList[productName]->m_paymentDate = paymentDate;
  this->m_pStrategyList[productName]->m_tradeValue = tradeValue.toDouble();
  this->m_pStrategyList[productName]->m_presentValue = presentValue.toDouble();
  this->m_pStrategyList[productName]->m_deliveryDate = deliveryDate;
  this->m_pStrategyList[productName]->m_terminationDate = terminationDate;
  this->m_pStrategyList[productName]->m_returnProfit = returnProfit.toDouble();

 }

 freshModel();

 this->accept();
}

void QStrategy::on_cancle_clicked(){
 this->reject();
}
// 重写这个韩素 默认Qstring 空（新建） 如果不为空（修改）则从对应位置载入  在QStrategy中新建一个标志位 来区分是编辑还是新建
void QStrategy::loadTemplate(int row){
 QString templateDir;
 if(row == -1){
  templateDir = "input/templateProductParam.ini";
  this->ui.productNameLineEdit->setDisabled(false);
  edit = false;
 }else{
  QStandardItem* record = this->m_strategy_model->item(row,1);
  QString result = record->text();
  templateDir = QString("output/product/") + result + QString(".ini") ;
  this->ui.productNameLineEdit->setDisabled(true);
  edit = true;
 }
 //读入模板配置文件
 if(QFile::exists(templateDir))
 {

  QSettings *configSetting = new QSettings(templateDir,QSettings::IniFormat);
  configSetting->setIniCodec("GB2312");
 // QString productNamet =QString::fromLocal8Bit(configSetting->value("/OptionParam/productName").toString().toStdString().c_str());
  QString productName =configSetting->value("/OptionParam/productName").toString();
  int optionType = configSetting->value("/OptionParam/optionKind").toInt(); // combox是从0开始计算的
  int scaleUnit = configSetting->value("/OptionParam/scaleUnit").toInt(); 
  QString callScale = configSetting->value("/OptionParam/callScale").toString(); 
  QString putScale = configSetting->value("/OptionParam/putScale").toString(); 
  QDateTime settlementDate = configSetting->value("/OptionParam/settlementDate").toDateTime();
  QDateTime maturityDate = configSetting->value("/OptionParam/maturityDate").toDateTime();
  QDateTime nextAveragePoint = configSetting->value("/OptionParam/nextAveragePoint").toDateTime();
  QDateTime firstAveragePoint = configSetting->value("/OptionParam/firstAveragePoint").toDateTime();
  QString initialPrice = configSetting->value("/OptionParam/initialPrice").toString();
  QString averagePrice = configSetting->value("/OptionParam/averagePrice").toString();
  QString callExecPricePercent = configSetting->value("/OptionParam/callExecPricePercent").toString();
  QString putExecPricePercent = configSetting->value("/OptionParam/putExecPricePercent").toString();
  QString interest = configSetting->value("/OptionParam/interest").toString();
  QString dividend = configSetting->value("/OptionParam/dividend").toString();
  QString barrier = configSetting->value("/OptionParam/barrier").toString();

  int callHedgeDirection = configSetting->value("/OptionParam/callHedgeDirection").toInt();
  int putHedgeDirection = configSetting->value("/OptionParam/putHedgeDirection").toInt();
 
  QTime strategyStartTime = configSetting->value("/OptionParam/strategyStartTime").toTime();

  QString openNums = configSetting->value("/OptionParam/openNums").toString();
  QString hedgeLevel = configSetting->value("/OptionParam/hedgeLevel").toString();
  QString hedgeVolability = configSetting->value("/OptionParam/hedgeVolability").toString();
  QTime carryNoonTime = configSetting->value("/OptionParam/carryNoonTime").toTime();
  QTime carryNightTime = configSetting->value("/OptionParam/carryNightTime").toTime();
  int carryNoonDistance = configSetting->value("/OptionParam/carryNoonDistance").toInt(); 
  int carryNightDistance = configSetting->value("/OptionParam/carryNightDistance").toInt();

  int totalAveragePoint = configSetting->value("/OptionParam/totalAveragePoint").toInt(); 
  int averagedPoint = configSetting->value("/OptionParam/averagedPoint").toInt(); 

  QString instrumentCode = configSetting->value("/OptionParam/instrumentCode").toString(); 
  QString instrumentCode2 = configSetting->value("/OptionParam/instrumentCode2").toString(); 
  QString instrumentCode3 = configSetting->value("/OptionParam/instrumentCode3").toString(); 
  QString instrumentCode4 = configSetting->value("/OptionParam/instrumentCode4").toString(); 

  double instrumentRatio =  configSetting->value("/OptionParam/instrumentRatio").toDouble(); 
  double instrumentRatio2 =  configSetting->value("/OptionParam/instrumentRatio2").toDouble(); 
  double instrumentRatio3 =  configSetting->value("/OptionParam/instrumentRatio3").toDouble(); 
  double instrumentRatio4 =  configSetting->value("/OptionParam/instrumentRatio4").toDouble(); 

  int hedgeThreshold = configSetting->value("/OptionParam/hedgeThreahold").toInt(); 

  double tickValue = configSetting->value("/OptionParam/tickValue").toDouble();
  double tickValue2 = configSetting->value("/OptionParam/tickValue2").toDouble();
  double tickValue3 = configSetting->value("/OptionParam/tickValue3").toDouble();
  double tickValue4 = configSetting->value("/OptionParam/tickValue4").toDouble();

  double tradeCostPercent = configSetting->value("/OptionParam/tradeCostPercent").toDouble();
  double riskAversion = configSetting->value("/OptionParam/riskAversion").toDouble();

  //20180515
  int bookStatus = configSetting->value("/OptionParam/bookStatus").toInt();
  QString PNL = configSetting->value("/OptionParam/PNL").toString();
  QString daySettlePrice = configSetting->value("/OptionParam/daySettlePrice").toString();
  QString optionFee = configSetting->value("/OptionParam/optionFee").toString();
  QString counterParty = configSetting->value("/OptionParam/counterParty").toString();
  QString deliveryPrice = configSetting->value("/OptionParam/deliveryPrice").toString();
  QDateTime paymentDate = configSetting->value("/OptionParam/paymentDate").toDateTime();
  QDateTime deliveryDate = configSetting->value("/OptionParam/deliveryDate").toDateTime();
  QDateTime terminationDate = configSetting->value("/OptionParam/terminationDate").toDateTime();
  QString tradeValue = configSetting->value("/OptionParam/tradeValue").toString();
  QString presentValue = configSetting->value("/OptionParam/presentValue").toString();
  QString returnProfit = configSetting->value("/OptionParam/returnProfit").toString();







  this->ui.productNameLineEdit->setText(productName);
  this->ui.typeComboBox->setCurrentIndex(optionType);
  this->ui.settlementDateEdit->setDateTime(settlementDate);
  this->ui.maturityDateEdit->setDateTime(maturityDate);
  this->ui.nextApointDateEdit->setDateTime(nextAveragePoint);
  this->ui.firstApointDateEdit->setDateTime(firstAveragePoint);
  this->ui.callScaleLineEdit->setText(callScale);
  this->ui.putScaleLineEdit->setText(putScale);
  this->ui.initialPriceLineEdit->setText(initialPrice);
  this->ui.putExecPricePercentLineEdit->setText(putExecPricePercent);
  this->ui.callExecPricePercentLineEdit->setText(callExecPricePercent);
  this->ui.averagePriceLineEdit->setText(averagePrice);
  this->ui.rateLineEdit->setText(interest);
  this->ui.scaleUnitComboBox->setCurrentIndex(scaleUnit);
  this->ui.yieldLineEdit->setText(dividend);
  this->ui.barrierLineEdit->setText(barrier);
 
  this->ui.callTypeComboBox->setCurrentIndex(callHedgeDirection);
  this->ui.putTypeComboBox->setCurrentIndex(putHedgeDirection);

  this->ui.hedgeVolLineEdit->setText(hedgeVolability);
  this->ui.startTimeEdit->setTime(strategyStartTime);
  this->ui.openNumsLineBox->setText(openNums);
//  this->ui.hedgeLevelLineBox->setText(hedgeLevel);

  this->ui.carryNoonTimeEdit->setTime(carryNoonTime);
  this->ui.carryNoonDistanceSpinBox->setRange(0,99999);
  this->ui.carryNoonDistanceSpinBox->setValue(carryNoonDistance);
  this->ui.carryNightTimeEdit->setTime(carryNightTime);
  this->ui.carryNightDistanceSpinBox->setRange(0,99999);
  this->ui.carryNightDistanceSpinBox->setValue(carryNightDistance);
  //

  this->ui.totalAveragePoint->setValue(totalAveragePoint);
  this->ui.averagedPoint->setValue(averagedPoint);

  this->ui.instrumentCodeLineEdit->setText(instrumentCode);
  this->ui.instrumentCodeLineEdit_2->setText(instrumentCode2);
  this->ui.instrumentCodeLineEdit_3->setText(instrumentCode3);
  this->ui.instrumentCodeLineEdit_4->setText(instrumentCode4);

  this->ui.InstrumentRatioSpinBox->setValue(instrumentRatio);
  this->ui.InstrumentRatioSpinBox_2->setValue(instrumentRatio2);
  this->ui.InstrumentRatioSpinBox_3->setValue(instrumentRatio3);
  this->ui.InstrumentRatioSpinBox_4->setValue(instrumentRatio4);

  this->ui.hedgeThresHoldSpinBox->setValue(hedgeThreshold);

  this->ui.tickValue->setValue(tickValue);
  this->ui.tickValue_2->setValue(tickValue2);
  this->ui.tickValue_3->setValue(tickValue3);
  this->ui.tickValue_4->setValue(tickValue4);
  
  this->ui.TradeCostDSpinBox->setValue(tradeCostPercent);
  this->ui.riskAversionDSpinBox->setValue(riskAversion);

  //20180515
  this->ui.statusComboBox->setCurrentIndex(bookStatus);
  this->ui.PNLLineEdit->setText(PNL);
  this->ui.settlePriceLineEdit->setText(daySettlePrice);
  this->ui.optionFeeLineEdit->setText(optionFee);
  this->ui.counterpartyLineEdit->setText(counterParty);
  this->ui.deliveryPriceLineEdit->setText(deliveryPrice);
  this->ui.paymentDateEdit->setDateTime(paymentDate);
  this->ui.TradeValueLineEdit->setText(tradeValue);
  this->ui.PresentValueEdit->setText(presentValue);
  this->ui.deliveryDateEdit->setDateTime(deliveryDate);
  this->ui.terminateDateEdit->setDateTime(terminationDate);
  this->ui.returnProfitLineEdit->setText(returnProfit);

  delete configSetting;
 }else{
  // maybe print logging 
 }

}


QStandardItemModel * QStrategy::createModel(){
 
 this->m_strategy_model = new QStandardItemModel(0, 14, this); //指定了 父指针不需要 自己维护指针删除

 m_strategy_model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("选择"));
 m_strategy_model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("  场外明细  "));
 m_strategy_model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("合约代码"));
 m_strategy_model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("期权种类"));
 m_strategy_model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("发行期"));
 m_strategy_model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("交付日"));
 m_strategy_model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("终止日"));
 m_strategy_model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("到期日"));
 m_strategy_model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("交割日"));
 m_strategy_model->setHeaderData(9, Qt::Horizontal, QString::fromLocal8Bit("对冲波动率"));
 m_strategy_model->setHeaderData(10, Qt::Horizontal, QString::fromLocal8Bit("期权费(单价)"));
 m_strategy_model->setHeaderData(11, Qt::Horizontal, QString::fromLocal8Bit("总规模"));
 m_strategy_model->setHeaderData(12, Qt::Horizontal, QString::fromLocal8Bit("运行状态"));
 m_strategy_model->setHeaderData(13, Qt::Horizontal, QString::fromLocal8Bit("本子状态"));

 return m_strategy_model;
}

void QStrategy::checkProductList(){
 QDir *checkDir = new QDir("output/product/");
 QStringList filter;
 filter << "*.ini";
 checkDir->setNameFilters(filter);
 QList<QFileInfo> *fileinfo = new QList<QFileInfo>(checkDir->entryInfoList(filter));
 for (auto iter = fileinfo->begin(); iter != fileinfo->end(); iter++)
 {
  QString curFileName = iter->fileName();
 
  QStrategyThread *pThread = new  QStrategyThread(this->m_strategyGreekModel,this->m_pOrderManager,this); 
  pThread->m_workDay = this->m_workDay;
  pThread->m_calendarDay = this->m_calendarDay;
  pThread->loadConf(curFileName);
  QString strategyName = curFileName.left(curFileName.length()-4);
  this->m_pStrategyList.insert(strategyName,pThread);
 }
 delete fileinfo;


}

void QStrategy::freshModel(){
 for(auto iter = this->m_pStrategyList.begin();iter!= this->m_pStrategyList.end();iter++){
  QList<QStandardItem *> tList = this->m_strategy_model->findItems((*iter)->productName,Qt::MatchExactly,1);
  QString  sStatus =QString::fromLocal8Bit( (*iter)->m_status == 0?  "停止" : "运行");
  QString bookStatus[3] = {QString::fromLocal8Bit("存续"),QString::fromLocal8Bit("已到期"),QString::fromLocal8Bit("已平仓")};
  QString  sCheckStatus = bookStatus[(*iter)->m_bookStatus];
  string sOptionType = optionKindToStr((*iter)->optionType);
  QString qsOptionType(sOptionType.c_str());
   if (tList.count()>0)
  {
   //更新数据
   int row = tList.at(0)->row();
  
   //m_strategy_model->beginResetModel();
   m_strategy_model->setData(m_strategy_model->index(row, 0),"");
   m_strategy_model->setData(m_strategy_model->index(row, 1), (*iter)->productName);
   m_strategy_model->setData(m_strategy_model->index(row, 2), (*iter)->instrumentCode +QString(" ")+ (*iter)->instrumentCode2 +QString(" ")+ (*iter)->instrumentCode3 +QString(" ")+ (*iter)->instrumentCode4);
   m_strategy_model->setData(m_strategy_model->index(row, 3), qsOptionType);
   m_strategy_model->setData(m_strategy_model->index(row, 4), (*iter)->settlementDate);
   m_strategy_model->setData(m_strategy_model->index(row, 5), (*iter)->m_paymentDate);
   m_strategy_model->setData(m_strategy_model->index(row, 6), (*iter)->m_terminationDate);
   m_strategy_model->setData(m_strategy_model->index(row, 7), (*iter)->maturityDate);
   m_strategy_model->setData(m_strategy_model->index(row, 8), (*iter)->m_deliveryDate);
   m_strategy_model->setData(m_strategy_model->index(row, 9), (*iter)->hedgeVolability);
   m_strategy_model->setData(m_strategy_model->index(row, 10), (*iter)->m_optionFee);
   m_strategy_model->setData(m_strategy_model->index(row, 11), ((*iter)->callScale + (*iter)->putScale));
   m_strategy_model->setData(m_strategy_model->index(row, 12), sStatus);
   m_strategy_model->setData(m_strategy_model->index(row, 13), sCheckStatus);
   //m_strategy_model->endResetModel();
   }
   
  else
  {
   //初始化
   //m_strategy_model->beginInsertRows();
   m_strategy_model->insertRow(0);
   m_strategy_model->setData(m_strategy_model->index(0, 0), 0,Qt::UserRole);
   m_strategy_model->setData(m_strategy_model->index(0, 0), "");
   m_strategy_model->setData(m_strategy_model->index(0, 1), (*iter)->productName);
   m_strategy_model->setData(m_strategy_model->index(0, 2), (*iter)->instrumentCode +QString(" ")+ (*iter)->instrumentCode2 +QString(" ")+ (*iter)->instrumentCode3 +QString(" ")+ (*iter)->instrumentCode4);
   m_strategy_model->setData(m_strategy_model->index(0, 3), qsOptionType);
   m_strategy_model->setData(m_strategy_model->index(0, 4), (*iter)->settlementDate);
   m_strategy_model->setData(m_strategy_model->index(0, 5), (*iter)->m_paymentDate);
   m_strategy_model->setData(m_strategy_model->index(0, 6), (*iter)->m_terminationDate);
   m_strategy_model->setData(m_strategy_model->index(0, 7), (*iter)->maturityDate);
   m_strategy_model->setData(m_strategy_model->index(0, 8), (*iter)->m_deliveryDate);
   m_strategy_model->setData(m_strategy_model->index(0, 9), (*iter)->hedgeVolability);
   m_strategy_model->setData(m_strategy_model->index(0, 10), (*iter)->m_optionFee);
   m_strategy_model->setData(m_strategy_model->index(0, 11), ((*iter)->callScale + (*iter)->putScale));
   m_strategy_model->setData(m_strategy_model->index(0, 12), sStatus);
   m_strategy_model->setData(m_strategy_model->index(0, 13), sCheckStatus);

   //m_strategy_model->endInsertRows();
  }
 }

}



void QStrategy::freshStrategy(){
 for(auto iter = this->m_pStrategyList.begin();iter!= this->m_pStrategyList.end();iter++){
  (*iter)->setTDSpi(this->m_pCtpControl->getTradeSpi());
 }

 //更新订单管理器交易接口
 this->m_pOrderManager->SetTradeSpi(this->m_pCtpControl->getTradeSpi());

}
  //   for(int j=0;j<col;++j)  
  //   {  
  //    QStandardItem* item = m_strategy_model->item(i,j);  
  //    if (item)  
  //    {  
  // // if()
  //  cout<< i<< " "<< j<< " " <<QString::fromLocal8Bit(item->text().toStdString().c_str()).toStdString() << " " << item->data( Qt::UserRole).toBool() << "/t" ;
  //    }  
  //
  //   }  
  //cout << endl;

void QStrategy::onRunClicked(bool allowStatus,int spanTime){
 int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(runTag){
   
   this->m_pStrategyList[strategyName]->m_status = 1;
   this->m_pStrategyList[strategyName]->m_spanTime = spanTime;
   this->m_pStrategyList[strategyName]->start();
  }else{
   //this->m_pStrategyList[strategyName]->m_status = 0;
  }

 }  
 this->m_pOrderManager->setAllowTrade(allowStatus); 
 freshModel();
}

void QStrategy::onStopClicked(){
 int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(runTag){
   
   this->m_pStrategyList[strategyName]->m_status = 0;
  }else{
   //this->m_pStrategyList[strategyName]->m_status = 0;
  }

 }  
 freshModel();
}

void QStrategy::onUpdateClicked(){
 int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(runTag){
    this->m_pStrategyList[strategyName]->settleUpdate();
  }

 }  
 //freshModel();
}

void QStrategy::onFreshTrade(){
    int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(runTag){
    this->m_pStrategyList[strategyName]->bookFresh();
  }

 }  
}
// this settle all book
void QStrategy::onSettleClicked(bool defaultSettle){
 // 单独一个本子结算
 if(defaultSettle){
  QString bookName = this->ui.productNameLineEdit->text();
  this->m_pStrategyList[bookName]->bookSettle();
  this->m_pStrategyList[bookName]->settlePrintToExcel();
  this->ui.PNLLineEdit->setText(QString::number(this->m_pStrategyList[bookName]->m_PNL));
  this->ui.PresentValueEdit->setText(QString::number(this->m_pStrategyList[bookName]->m_presentValue));
 }else{
  int rows = this->m_strategy_model->rowCount();
  int col = this->m_strategy_model->columnCount();
  for (int i=0;i<rows;++i)  
  {  
   QStandardItem* item = m_strategy_model->item(i,0);  
   QStandardItem* itemName = m_strategy_model->item(i,1);  
   bool runTag  = item->data( Qt::UserRole).toBool() ;
   QString strategyName = itemName->text();
   if(runTag){
    this->m_pStrategyList[strategyName]->bookSettle();
    this->m_pStrategyList[strategyName]->settlePrintToExcel();
    this->m_pStrategyList[strategyName]->settleSaveToIniFile();
   }
 
  }

 }  
 //freshModel();
}

void QStrategy::onDelClicked(){


 int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(this->m_pStrategyList.contains(strategyName) && this->m_pStrategyList[strategyName]->m_bookStatus == 0){
   if(runTag == 1){
    m_strategy_model->setData(m_strategy_model->index(i, 0), 0,Qt::UserRole);
   }else{
      m_strategy_model->setData(m_strategy_model->index(i, 0), 1,Qt::UserRole); 
   }

   
  }

 }  

 freshModel();
}

void QStrategy::setCtpControl(ctpControl *p){

 this->m_pCtpControl = p;
 this->m_pOrderManager->SetTradeSpi(p->getTradeSpi());
}

void QStrategy::subscribe_inst_data(QString pInstrumentId){
 TThostFtdcInstrumentIDType instrumentCode;
 strcpy(instrumentCode,pInstrumentId.toStdString().c_str());
 this->m_pCtpControl->subscribe_inst_data(instrumentCode);
}

QStandardItemModel * QStrategy::createStrategyGreekModel(){
 
 this->m_strategyGreekModel = new QStandardItemModel(0, 9, this); //指定了 父指针不需要 自己维护指针删除

 m_strategyGreekModel->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("BookInst"));
 m_strategyGreekModel->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Delta"));
 m_strategyGreekModel->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("ΔCash"));
 m_strategyGreekModel->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("ΓCash"));
 m_strategyGreekModel->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("ΘCash"));
 m_strategyGreekModel->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("νCash"));
 m_strategyGreekModel->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("LastPrice"));
 m_strategyGreekModel->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("应对冲手数"));
 m_strategyGreekModel->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("Time"));





 return m_strategyGreekModel;
}


void QStrategy :: dataChange(QModelIndex lefttop,QModelIndex bottomRight){
 this->m_strategyGreekModel->dataChanged(lefttop,bottomRight);
}

void QStrategy::on_searchBook_clicked(){
 QString bookName = this->ui.productNameLineEdit->text();
 getBookInstrumentList(bookName);
 
 this->ui.instrumentCodeLineEdit->setText(QString(this->m_instumentList[0].c_str()));
 this->ui.instrumentCodeLineEdit_2->setText(QString(this->m_instumentList[1].c_str()));
 this->ui.instrumentCodeLineEdit_3->setText(QString(this->m_instumentList[2].c_str()));
 this->ui.instrumentCodeLineEdit_4->setText(QString(this->m_instumentList[3 ].c_str()));


}

void QStrategy::getBookInstrumentList(QString bookName){
 this->m_instumentList.clear();
 //登陆数据库参数
#ifdef _DEBUG
 string username="otc";
 string password="otc";
 string server="10.192.96.131:1521/testdb";
#else
 string username="cwysp";
 string password="cwysp";
 string server="10.192.80.130:1521/CWYSPDB";

#endif
 //初始化存放句柄的结构体
 struct OCI_hpp *hpp;
 hpp=(struct OCI_hpp *)malloc(sizeof(struct OCI_hpp));

 try{

  //分配各个句柄，连接服务器，连接数据库
  OracleServerConnect(hpp,server,username,password);
  //查询
  //char sqlcommd[512]="Select * from table1 ";
  //OCISelect(hpp,sqlcommd);
  //插入
  //char sqlcommd2[512]="insert into table1(ID,trader_name,cash,currencyid) values(20,'zksjc',55555.4584,'CNY')";
  //OCIInsert(hpp,sqlcommd2);
  //删除
  //char sqlcommd3[512]="delete from table1 where ID='20' ";
  //OCIDelete(hpp,sqlcommd3);
  //更新
  //char sqlcommd4[512]="update table1 set trader_name='kktt' where trader_name='kkkk' ";
  //OCIDelete(hpp,sqlcommd);
  char book[300] ={};
  sprintf(book,this->ui.productNameLineEdit->text().toStdString().c_str());
  char sqlCommand[1024];
  sprintf(sqlCommand,
   "select instT.Instrumentid from otc.t_instrument instT  left join otc.t_book_instrument bookInst on bookInst.Instid = instT.Id left join otc.t_book book on book.id = bookInst.Bookid where book.bookname = '%s'",
   book);
  cout << sqlCommand << endl;
  char (**Result)=(char**)malloc(sizeof(char**)*4);
  for(int i=0;i<4;i++)
  {
   *(Result+i)=(char*)malloc(sizeof(char)*20);
   memset(*(Result+i),0,sizeof(sizeof(char)*20));
  }
 
  
  Result=OCI_LiYong_Select(hpp,sqlCommand,Result);
  char parameter[252] = {};
  QString qsParamter;
  //保存合约名
  this->m_instumentList.push_back(string(*(Result)));
  this->m_instumentList.push_back(string(*(Result+1)));
  this->m_instumentList.push_back(string(*(Result+2)));
  this->m_instumentList.push_back(string(*(Result+3)));
  //optionkind
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_optionKind'",book);
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_optionType = qsParamter.toInt();
  //scaleUnit
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_scaleUnit'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_scaleUnit = qsParamter.toInt();
  //callScale
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_callScale'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_callScale = qsParamter;
  //putScale
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_putScale'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_putScale = qsParamter;
  //settlementDate
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_settlementDate'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_settlementDate  = QDateTime::fromString(qsParamter,"yyyy-MM-ddThh:mm:ss");
  //maturityDate
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_maturityDate'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_maturityDate = QDateTime::fromString(qsParamter,"yyyy-MM-ddThh:mm:ss");
  //NextAveragePoint
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_nextAveragePoint'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_nextAveragePoint =  QDateTime::fromString(qsParamter,"yyyy-MM-ddThh:mm:ss");
  //FirstAveragePoint
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_firstAveragePoint'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_firstAveragePoint =  QDateTime::fromString(qsParamter,"yyyy-MM-ddThh:mm:ss");

  //initialPrice
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_initialPrice'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_initialPrice = qsParamter;
  //averagePrice
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_averagePrice'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_averagePrice = qsParamter;
  //callExecPricePercent
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_callExecPricePercent'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_callExecPricePercent = qsParamter;
  //putExecPricePercent
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_putExecPricePercent'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_putExecPricePercent = qsParamter;
  //dividend
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_dividend'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_dividend = qsParamter;
  //barrier
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_barrier'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_barrier = qsParamter;
  //interest
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_interest'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_interest = qsParamter;
  //callHedgeDirection
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_callHedgeDirection'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_callHedgeDirection = qsParamter.toInt();
  //putHedgeDirection
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_putHedgeDirection'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_putHedgeDirection = qsParamter.toInt();
  //strategyStartTime
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_strategyStartTime'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_strategyStartTime = QTime::fromString(qsParamter,"hh:mm:ss");
  //openNums
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_openNums'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_openNums = qsParamter;
  //hedgeLevel
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_hedgeLevel'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_hedgeLevel = qsParamter;
  //hedgeVolability
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_hedgeVolability'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_hedgeVolability = qsParamter;
  //carryNoonTime
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_carryNoonTime'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_carryNoonTime = QTime::fromString(qsParamter,"hh:mm:ss");
  //carryNightTime
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_carryNightTime'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_carryNightTime = QTime::fromString(qsParamter,"hh:mm:ss");
  //carryNoonDistance
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_carryNoonDistance'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_carryNoonDistance = qsParamter.toInt();
  //carryNightDistance
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_carryNightDistance'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_carryNightDistance = qsParamter.toInt();
  //totalAveragePoint
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_totalAveragePoint'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_totalAveragePoint = qsParamter.toInt();
  //averagedPoint
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_averagedPoint'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_averagedPoint = qsParamter.toInt();
  ////instrumentCode
  //memset(sqlCommand,0,sizeof(sqlCommand));
  //sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentCode'",book);
  //memset(parameter,0,sizeof(parameter));
  //OCI_SearchGetChar(hpp,sqlCommand,parameter);
  //qsParamter = QString(parameter);
  //this->m_instrumentCode = qsParamter;
  ////instrumentCode2
  //memset(sqlCommand,0,sizeof(sqlCommand));
  //sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentCode2'",book);
  //memset(parameter,0,sizeof(parameter));
  //OCI_SearchGetChar(hpp,sqlCommand,parameter);
  //qsParamter = QString(parameter);
  //this->m_instrumentCode2 = qsParamter;
  ////instrumentCode3
  //memset(sqlCommand,0,sizeof(sqlCommand));
  //sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentCode3'",book);
  //memset(parameter,0,sizeof(parameter));
  //OCI_SearchGetChar(hpp,sqlCommand,parameter);
  //qsParamter = QString(parameter);
  //this->m_instrumentCode3 = qsParamter;
  ////instrumentCode4
  //memset(sqlCommand,0,sizeof(sqlCommand));
  //sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentCode4'",book);
  //memset(parameter,0,sizeof(parameter));
  //OCI_SearchGetChar(hpp,sqlCommand,parameter);
  //qsParamter = QString(parameter);
  //this->m_instrumentCode4 = qsParamter;
  //instrumentRatio
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentRatio'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_instrumentRatio = qsParamter.toDouble();
  //instrumentRatio2
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentRatio2'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_instrumentRatio2 = qsParamter.toDouble();
  //instrumentRatio3
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentRatio3'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_instrumentRatio3 = qsParamter.toDouble();
  //instrumentRatio4
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_instrumentRatio4'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_instrumentRatio4 = qsParamter.toDouble();
  //hedgeThreahold
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_hedgeThreahold'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_hedgeThreshold = qsParamter.toInt();
  //tickValue
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_tickValue'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_tickValue = qsParamter.toDouble();
  //tickValue2
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_tickValue2'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_tickValue2 = qsParamter.toDouble();
  //tickValue3
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_tickValue3'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_tickValue3 = qsParamter.toDouble();
  //tickValue4
  memset(sqlCommand,0,sizeof(sqlCommand));
  sprintf(sqlCommand,"select para.PARAVALUE  from otc.T_PARAMETER para where para.PARANAME  = '%s_tickValue4'",book);
  memset(parameter,0,sizeof(parameter));
  OCI_SearchGetChar(hpp,sqlCommand,parameter);
  qsParamter = QString(parameter);
  this->m_tickValue4 = qsParamter.toDouble();
  
  //++++++++++++++++++++++++++++++
  this->ui.typeComboBox->setCurrentIndex(m_optionType);
  this->ui.settlementDateEdit->setDateTime(m_settlementDate);
  this->ui.maturityDateEdit->setDateTime(m_maturityDate);
  this->ui.nextApointDateEdit->setDateTime(m_nextAveragePoint);
  this->ui.firstApointDateEdit->setDateTime(m_firstAveragePoint);
  this->ui.callScaleLineEdit->setText(m_callScale);
  this->ui.putScaleLineEdit->setText(m_putScale);
  this->ui.initialPriceLineEdit->setText(m_initialPrice);
  this->ui.putExecPricePercentLineEdit->setText(m_putExecPricePercent);
  this->ui.callExecPricePercentLineEdit->setText(m_callExecPricePercent);
  this->ui.averagePriceLineEdit->setText(m_averagePrice);
  this->ui.rateLineEdit->setText(m_interest);
  this->ui.scaleUnitComboBox->setCurrentIndex(m_scaleUnit);
  this->ui.yieldLineEdit->setText(m_dividend);
  this->ui.barrierLineEdit->setText(m_barrier);
 
  this->ui.callTypeComboBox->setCurrentIndex(m_callHedgeDirection);
  this->ui.putTypeComboBox->setCurrentIndex(m_putHedgeDirection);

  this->ui.hedgeVolLineEdit->setText(m_hedgeVolability);
  this->ui.startTimeEdit->setTime(m_strategyStartTime);
  this->ui.openNumsLineBox->setText(m_openNums);
//  this->ui.hedgeLevelLineBox->setText(hedgeLevel);

  this->ui.carryNoonTimeEdit->setTime(m_carryNoonTime);
  this->ui.carryNoonDistanceSpinBox->setRange(0,99999);
  this->ui.carryNoonDistanceSpinBox->setValue(m_carryNoonDistance);
  this->ui.carryNightTimeEdit->setTime(m_carryNightTime);
  this->ui.carryNightDistanceSpinBox->setRange(0,99999);
  this->ui.carryNightDistanceSpinBox->setValue(m_carryNightDistance);
  //

  this->ui.totalAveragePoint->setValue(m_totalAveragePoint);
  this->ui.averagedPoint->setValue(m_averagedPoint);


  this->ui.InstrumentRatioSpinBox->setValue(m_instrumentRatio);
  this->ui.InstrumentRatioSpinBox_2->setValue(m_instrumentRatio2);
  this->ui.InstrumentRatioSpinBox_3->setValue(m_instrumentRatio3);
  this->ui.InstrumentRatioSpinBox_4->setValue(m_instrumentRatio4);

  this->ui.hedgeThresHoldSpinBox->setValue(m_hedgeThreshold);

  this->ui.tickValue->setValue(m_tickValue);
  this->ui.tickValue_2->setValue(m_tickValue2);
  this->ui.tickValue_3->setValue(m_tickValue3);
  this->ui.tickValue_4->setValue(m_tickValue4);


 }
 catch(...){
  cout  << "error" << endl;
  this->ui.tipsLabel->setText("Search Error");
  this->m_instumentList.push_back(string(""));
  this->m_instumentList.push_back(string(""));
  this->m_instumentList.push_back(string(""));
  this->m_instumentList.push_back(string(""));
 }

 //断开数据库，断开服务器，释放各个句柄
 OracleServerDetach(hpp);
 free(hpp);
 return ;
}

void QStrategy::strategyStatistic(){
 
 int rows = this->m_strategy_model->rowCount();
 int col = this->m_strategy_model->columnCount();

 cashGreeks totalGreeks;
 for (int i=0;i<rows;++i)  
 {  
  QStandardItem* item = m_strategy_model->item(i,0);  
  QStandardItem* itemName = m_strategy_model->item(i,1);  
  bool runTag  = item->data( Qt::UserRole).toBool() ;
  QString strategyName = itemName->text();
  if(runTag){

   map<QString,cashGreeks>*p = m_pStrategyList[strategyName]->getStrategyCashGreeks();
   for(auto i = p->begin();i!= p->end();i++){
    totalGreeks._deltaCash += i->second._deltaCash;
    totalGreeks._thetaCash += i->second._thetaCash;
    totalGreeks._vegaCash += i->second._vegaCash;
    totalGreeks._rho += i->second._rho;
    totalGreeks._gammaCash += i->second._gammaCash;
   }

     // cout << strategyName.toStdString() << endl;
   //this->m_pStrategyList[strategyName]->m_status = 1;
   //this->m_pStrategyList[strategyName]->m_spanTime = spanTime;
   //this->m_pStrategyList[strategyName]->start();
  }else{
   //this->m_pStrategyList[strategyName]->m_status = 0;
  }

 }  
 //this->m_pOrderManager->setAllowTrade(allowStatus); 
 //freshModel();
 setStrategyGreeksTableData(QString("forAllGreeks"),totalGreeks);
}


void QStrategy::setStrategyGreeksTableData(QString rowName,cashGreeks totalGreeks){
 QString InstrumentName = rowName;

 QString sdelta = QString::number(0,'f', 3);
 QString sdeltaCash = QString::number(totalGreeks._deltaCash,'f', 0);
 QString stheta = QString::number(totalGreeks._thetaCash,'f', 0);
 QString svega = QString::number(totalGreeks._vegaCash,'f', 0);
 QString sgamma = QString::number(totalGreeks._gammaCash,'f', 0);
 QString srho = QString::number(totalGreeks._rho,'f', 3);


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
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 6), 0);//交易合约
  m_strategyGreekModel->setData(m_strategyGreekModel->index(row, 7), 0);
  //m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row, 8), QString(instrumentMarketData[inst].UpdateTime));
  //m_strategyGreekModel->endResetModel();
  emit dataChange(m_strategyGreekModel->index(row, 1),m_strategyGreekModel->index(row, 7));
  

  m_strategyGreekModel->item(row,1)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,2)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,3)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,4)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,5)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,6)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
  m_strategyGreekModel->item(row,7)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
   //  m_pStrategyGreeksModel->item(row,8)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);


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
