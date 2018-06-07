#include "ctpcontrol.h"
#include <Windows.h>
#include <qmutex.h>
#include <qtextcodec.h>
#include <qtimer.h>
extern QMutex g_mutex;
ctpControl::ctpControl(QObject *parent)
 : QThread(parent)
{
 pUserApi_md = NULL;
 pUserApi_trade = NULL;
 pUserSpi_trade = NULL;
 pUserSpi_md = NULL;

 m_loggingObject = NULL;


 this->reconnectionTimer = new QTimer(this);
 // 连接计时器信号
 connect(reconnectionTimer,SIGNAL(timeout()),this,SLOT(checkConnectStatus()) );
 
// moveToThread(parent);
}

ctpControl::~ctpControl()
{
 releaseCtpConnect();
}



void ctpControl::init(AccountParam ap,QObject *LoggingObject){

 g_mutex.lock();
 this->m_accountParam = ap;
 
 this->m_loggingObject = LoggingObject;

 //--------------初始化行情UserApi，创建行情API实例----------------------------------
 this->pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(".\\MDflow\\");

 this->pUserSpi_md = new CtpMdSpi(pUserApi_md,this->m_quote_model);//创建回调处理类对象MdSpi

 pUserApi_md->RegisterSpi(pUserSpi_md);// 回调对象注入接口类

 pUserApi_md->RegisterFront(this->m_accountParam.m_mdFront);// 注册行情前置地址 
 //pUserApi_md->RegisterFront("tcp://180.168.146.187:10031");// 注册行情前置地址 

 pUserSpi_md->setAccount(this->m_accountParam.m_appId, this->m_accountParam.m_userId, this->m_accountParam.m_passwd);//经纪公司编号，用户名，密码
 //pUserSpi_md->setAccount("9999", "036762", "attention3");//经纪公司编号，用户名，密码

 //pUserSpi_md->setInstId("SR701");//MD所需订阅行情的合约，即策略交易的合约

 //--------------初始化交易UserApi，创建交易API实例----------------------------------
 this->pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(".\\TDflow\\");

 this->pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md,m_loggingObject);//构造函数初始化三个变量

 pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi_trade);// 注册事件类

 pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// 注册公有流

 pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// 注册私有流THOST_TERT_QUICK

 pUserApi_trade->RegisterFront(this->m_accountParam.m_tradeFront);// 注册交易前置地址
 //pUserApi_trade->RegisterFront("tcp://180.168.146.187:10030");// 注册交易前置地址

 pUserSpi_trade->setAccount(this->m_accountParam.m_appId, this->m_accountParam.m_userId, this->m_accountParam.m_passwd);//经纪公司编号，用户名，密码

 //pUserSpi_trade->setAccount("9999", "036762", "attention3");//经纪公司编号，用户名，密码
 //pUserSpi_trade->setInstId("SR701");//策略交易的合约
 QObject::connect(pUserSpi_trade,SIGNAL(IsErrorRspInfo_QT(CThostFtdcRspInfoField*)),this,SLOT(connectCtpRespond(CThostFtdcRspInfoField*)),Qt::UniqueConnection);
 QObject::connect(pUserSpi_md,SIGNAL(dataChange(QModelIndex,QModelIndex)),this,SLOT(dataChange(QModelIndex,QModelIndex)),Qt::QueuedConnection);

 QObject::connect(pUserSpi_trade,SIGNAL(disconnection ()),this,SLOT(disconnection()),Qt::QueuedConnection);
 QObject::connect(pUserSpi_md,SIGNAL(disconnection ()),this,SLOT(disconnection()),Qt::QueuedConnection);

 this->pUserApi_trade->Init(); // 启动行情
 //this->pUserApi_md->Init();
 g_mutex.unlock();
 // this->pUserApi_md->Join();

 //this->pUserApi_trade->Join();

}

void ctpControl::run(){

 exec(); // 进去消息循环

}
void ctpControl::releaseCtpConnect(){
 //释放交易接口对象
 if(this->pUserApi_trade ){
  this->pUserApi_trade->RegisterSpi(NULL);
  this->pUserApi_trade->Release();
  this->pUserApi_trade =NULL;
 }
 if(this->pUserApi_md){
  //this->pUserApi_md->UnSubscribeMarketData();
  this->pUserApi_md->RegisterSpi(NULL);
  this->pUserApi_md->Release();
  this->pUserApi_md = NULL;
 }
 if(pUserSpi_trade){
  //pUserSpi_trade->
  delete this->pUserSpi_trade;
  this->pUserSpi_trade = NULL;
 }
 if(pUserSpi_md){
  //pUserSpi_md->
  delete this->pUserSpi_md;
  this->pUserSpi_md = NULL;
 }
 
 //等待退出
 //this->pUserApi_trade->Join();
 //this->pUserApi_md->Join();
 
}
void ctpControl::connectCtpRespond(CThostFtdcRspInfoField *rsp){
 
 //获取消息 处理后发送给前台
 char errorID[10] ;

 sprintf(errorID,"%d",rsp->ErrorID);

 string error =  "CTP管理器:  " + string(rsp->ErrorMsg) + " 错误ID " + string(errorID);

 QString errorInfor = QString::fromLocal8Bit(error.c_str());

 
 emit connectCtpQuest(rsp);
 if(rsp->ErrorID == 3 || rsp->ErrorID == 11 || rsp->ErrorID  == 12 || rsp->ErrorID == 13 || rsp->ErrorID == 7 || rsp->ErrorID == 21){
  this->releaseCtpConnect();
 }
 if(rsp->ErrorID == 0){
  this->reconnectionTimer->start(60000);//每30秒检测一次网络连接
  emit ctpcontrolMsg(CTPSTATUS::NORMAL);
 }else{
  emit this->ctpMessage(errorInfor);
 }
}
vector<QString> * ctpControl::getInstrumentListVt(){
 return this->pUserSpi_trade->getInstrumentList();//返回合约列表
}
QStandardItemModel * ctpControl::createQuoteModel(){
 
  this->m_quote_model = new QStandardItemModel(0, 14, this); //指定了 父指针不需要 自己维护指针删除

 m_quote_model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("合约代码"));
 m_quote_model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("交易所代码"));
 m_quote_model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("合约名称"));
 m_quote_model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("涨跌幅"));
 m_quote_model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("今开盘"));
 m_quote_model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("最高价"));
 m_quote_model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("最低价"));
 m_quote_model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("最新价"));
 m_quote_model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("昨收盘"));
 m_quote_model->setHeaderData(9, Qt::Horizontal, QString::fromLocal8Bit("成交数量"));
 m_quote_model->setHeaderData(10, Qt::Horizontal, QString::fromLocal8Bit("成交金额"));
 m_quote_model->setHeaderData(11, Qt::Horizontal, QString::fromLocal8Bit("涨停板价"));
 m_quote_model->setHeaderData(12, Qt::Horizontal, QString::fromLocal8Bit("跌停板价"));
 m_quote_model->setHeaderData(13, Qt::Horizontal, QString::fromLocal8Bit("最后修改时间"));
 
 return m_quote_model;
}

void ctpControl::subscribe_inst_data(TThostFtdcInstrumentIDType pInstrumentName){
 if(this->pUserSpi_trade != nullptr){
  this->pUserSpi_trade->subscribe_inst_data(pInstrumentName);
 }
 
}
void ctpControl :: dataChange(const QModelIndex& lefttop, const QModelIndex &bottomRight){
 this->m_quote_model->dataChanged(lefttop,bottomRight);
}

CtpTraderSpi * ctpControl::getTradeSpi(){

 return this->pUserSpi_trade;
}

void ctpControl::disconnection(){
 //网络连接中断
 this->releaseCtpConnect();
 cout << "ctpControl : disconnection "<<endl;
 emit ctpcontrolMsg(CTPSTATUS::DISCONNECT);
 Sleep(2000);
}
// 短线重练有问题 暂不修改
void ctpControl::checkConnectStatus(){
 //if(this->pUserSpi_trade == NULL ||!(this->pUserSpi_trade->getConnectStatus())){
 // this->releaseCtpConnect();

 // int length = sizeof(this->m_accountParam.m_mdFront);
 // if(strcmp(this->m_accountParam.m_mdFront, "") != 0){
 //  this->init(this->m_accountParam,this->m_loggingObject);
 //  emit ctpcontrolMsg(CTPSTATUS::RECONNECT);
 // }

 //}
}