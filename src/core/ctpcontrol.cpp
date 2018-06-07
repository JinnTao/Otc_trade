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
 // ���Ӽ�ʱ���ź�
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

 //--------------��ʼ������UserApi����������APIʵ��----------------------------------
 this->pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(".\\MDflow\\");

 this->pUserSpi_md = new CtpMdSpi(pUserApi_md,this->m_quote_model);//�����ص����������MdSpi

 pUserApi_md->RegisterSpi(pUserSpi_md);// �ص�����ע��ӿ���

 pUserApi_md->RegisterFront(this->m_accountParam.m_mdFront);// ע������ǰ�õ�ַ 
 //pUserApi_md->RegisterFront("tcp://180.168.146.187:10031");// ע������ǰ�õ�ַ 

 pUserSpi_md->setAccount(this->m_accountParam.m_appId, this->m_accountParam.m_userId, this->m_accountParam.m_passwd);//���͹�˾��ţ��û���������
 //pUserSpi_md->setAccount("9999", "036762", "attention3");//���͹�˾��ţ��û���������

 //pUserSpi_md->setInstId("SR701");//MD���趩������ĺ�Լ�������Խ��׵ĺ�Լ

 //--------------��ʼ������UserApi����������APIʵ��----------------------------------
 this->pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(".\\TDflow\\");

 this->pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md,m_loggingObject);//���캯����ʼ����������

 pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi_trade);// ע���¼���

 pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// ע�ṫ����

 pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// ע��˽����THOST_TERT_QUICK

 pUserApi_trade->RegisterFront(this->m_accountParam.m_tradeFront);// ע�ύ��ǰ�õ�ַ
 //pUserApi_trade->RegisterFront("tcp://180.168.146.187:10030");// ע�ύ��ǰ�õ�ַ

 pUserSpi_trade->setAccount(this->m_accountParam.m_appId, this->m_accountParam.m_userId, this->m_accountParam.m_passwd);//���͹�˾��ţ��û���������

 //pUserSpi_trade->setAccount("9999", "036762", "attention3");//���͹�˾��ţ��û���������
 //pUserSpi_trade->setInstId("SR701");//���Խ��׵ĺ�Լ
 QObject::connect(pUserSpi_trade,SIGNAL(IsErrorRspInfo_QT(CThostFtdcRspInfoField*)),this,SLOT(connectCtpRespond(CThostFtdcRspInfoField*)),Qt::UniqueConnection);
 QObject::connect(pUserSpi_md,SIGNAL(dataChange(QModelIndex,QModelIndex)),this,SLOT(dataChange(QModelIndex,QModelIndex)),Qt::QueuedConnection);

 QObject::connect(pUserSpi_trade,SIGNAL(disconnection ()),this,SLOT(disconnection()),Qt::QueuedConnection);
 QObject::connect(pUserSpi_md,SIGNAL(disconnection ()),this,SLOT(disconnection()),Qt::QueuedConnection);

 this->pUserApi_trade->Init(); // ��������
 //this->pUserApi_md->Init();
 g_mutex.unlock();
 // this->pUserApi_md->Join();

 //this->pUserApi_trade->Join();

}

void ctpControl::run(){

 exec(); // ��ȥ��Ϣѭ��

}
void ctpControl::releaseCtpConnect(){
 //�ͷŽ��׽ӿڶ���
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
 
 //�ȴ��˳�
 //this->pUserApi_trade->Join();
 //this->pUserApi_md->Join();
 
}
void ctpControl::connectCtpRespond(CThostFtdcRspInfoField *rsp){
 
 //��ȡ��Ϣ ������͸�ǰ̨
 char errorID[10] ;

 sprintf(errorID,"%d",rsp->ErrorID);

 string error =  "CTP������:  " + string(rsp->ErrorMsg) + " ����ID " + string(errorID);

 QString errorInfor = QString::fromLocal8Bit(error.c_str());

 
 emit connectCtpQuest(rsp);
 if(rsp->ErrorID == 3 || rsp->ErrorID == 11 || rsp->ErrorID  == 12 || rsp->ErrorID == 13 || rsp->ErrorID == 7 || rsp->ErrorID == 21){
  this->releaseCtpConnect();
 }
 if(rsp->ErrorID == 0){
  this->reconnectionTimer->start(60000);//ÿ30����һ����������
  emit ctpcontrolMsg(CTPSTATUS::NORMAL);
 }else{
  emit this->ctpMessage(errorInfor);
 }
}
vector<QString> * ctpControl::getInstrumentListVt(){
 return this->pUserSpi_trade->getInstrumentList();//���غ�Լ�б�
}
QStandardItemModel * ctpControl::createQuoteModel(){
 
  this->m_quote_model = new QStandardItemModel(0, 14, this); //ָ���� ��ָ�벻��Ҫ �Լ�ά��ָ��ɾ��

 m_quote_model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("��Լ����"));
 m_quote_model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("����������"));
 m_quote_model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("��Լ����"));
 m_quote_model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("�ǵ���"));
 m_quote_model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("����"));
 m_quote_model->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("��߼�"));
 m_quote_model->setHeaderData(6, Qt::Horizontal, QString::fromLocal8Bit("��ͼ�"));
 m_quote_model->setHeaderData(7, Qt::Horizontal, QString::fromLocal8Bit("���¼�"));
 m_quote_model->setHeaderData(8, Qt::Horizontal, QString::fromLocal8Bit("������"));
 m_quote_model->setHeaderData(9, Qt::Horizontal, QString::fromLocal8Bit("�ɽ�����"));
 m_quote_model->setHeaderData(10, Qt::Horizontal, QString::fromLocal8Bit("�ɽ����"));
 m_quote_model->setHeaderData(11, Qt::Horizontal, QString::fromLocal8Bit("��ͣ���"));
 m_quote_model->setHeaderData(12, Qt::Horizontal, QString::fromLocal8Bit("��ͣ���"));
 m_quote_model->setHeaderData(13, Qt::Horizontal, QString::fromLocal8Bit("����޸�ʱ��"));
 
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
 //���������ж�
 this->releaseCtpConnect();
 cout << "ctpControl : disconnection "<<endl;
 emit ctpcontrolMsg(CTPSTATUS::DISCONNECT);
 Sleep(2000);
}
// �������������� �ݲ��޸�
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