
#include <fstream>
#include <string>
#include <sstream>

#include "traderspi.h"
#include "mdspi.h"
#include "windows.h"
#include <qdatetime.h>
#include <qreadwritelock.h>
#pragma warning(disable : 4996)

extern QReadWriteLock g_lock;
extern QMap<QString,CThostFtdcDepthMarketDataField> instrumentMarketData;
extern int requestId;  
extern HANDLE g_hEvent;
//#define _DEBUG


void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
 int nRequestID, bool bIsLast)
{
 IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
 cerr<<" CtpMdSpi | OnFrontDisconnected..." 
  << " reason=" << nReason << endl;
 emit disconnect();
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
 cerr<<" ��Ӧ | ������ʱ����..." 
  << " TimerLapse = " << nTimeLapse << endl;
}

void CtpMdSpi::OnFrontConnected()
{
 cerr<<"MD ���ӽ���ǰ��OnFrontConnected()...�ɹ�"<<endl;
 
 //��¼�ڻ��˺�
 ReqUserLogin(m_appId, m_userId, m_passwd);
 

 SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType appId,
 TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd)
{
 CThostFtdcReqUserLoginField req;
 memset(&req, 0, sizeof(req));
 strcpy(req.BrokerID, appId);
 strcpy(req.UserID, userId);
 strcpy(req.Password, passwd);
#ifdef _DEBUG

 int ret = m_pUserApi_md->ReqUserLogin(&req, ++requestId);
 //int ret = ccbf_MdFuncInterface(m_pUserApi_md,appId,userId,passwd,requestId);
 cerr<<"MD ���� | ���͵�¼..."<<((ret == 0) ? "�ɹ�" :"ʧ��") << endl; 

#else
 HINSTANCE hInst = LoadLibrary(TEXT("dll_FBI_Release_Win32.dll"));
 DWORD errorID = GetLastError();
 ccbf_secureApi_LoginMd ccbf_MdFuncInterface = (ccbf_secureApi_LoginMd)GetProcAddress(hInst,"ccbf_secureApi_LoginMd_After_CTP_OnConnected");
 if(!ccbf_MdFuncInterface){
  cerr << " DLL Interface Error" << endl;
 }else{
  //int ret = m_pUserApi_md->ReqUserLogin(&req, ++requestId);
  int ret = ccbf_MdFuncInterface(m_pUserApi_md,appId,userId,passwd,requestId);
  cerr<<"MD ���� | ���͵�¼..."<<((ret == 0) ? "�ɹ�" :"ʧ��") << endl; 
 }

#endif
 SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
 if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
 {

  cerr<<"����ģ���¼�ɹ�"<<endl; 

  //cerr<<"---"<<"�������Ϊ0��ʾ�ɹ���"<<pRspInfo->ErrorID<<",������Ϣ:"<<pRspInfo->ErrorMsg<<endl;

  //SubscribeMarketData_all();//����ȫ�г�����

  SubscribeMarketData(m_instId);//���Ľ��׺�Լ������

  //���ĳֲֺ�Լ������
  if(m_charNewIdList_holding_md)
  { 
   cerr<<"m_charNewIdList_holding_md��С:"<<strlen(m_charNewIdList_holding_md)<<","<<m_charNewIdList_holding_md<<endl;

   cerr<<"�гֲ֣��������飺"<<endl;
   SubscribeMarketData(m_charNewIdList_holding_md);//����Ϊ6��/��,���û�гֲ֣��Ͳ�Ҫ����

   delete []m_charNewIdList_holding_md;//������ɣ��ͷ��ڴ�
  }
  else
   cerr<<"��ǰû�гֲ�"<<endl;


  //����������Ĭ�Ͻ�ֹ�����Ǹ��õķ��ϰ��
  cerr<<endl<<endl<<endl<<"����Ĭ�Ͻ�ֹ���֣����������ף�������ָ��(������:yxkc, ��ֹ����:jzkc)��"<<endl;
  

 }
 if(bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
 vector<char*> list;

 char *token = strtok(instIdList, ",");

 while( token != NULL ){

  list.push_back(token); 

  token = strtok(NULL, ",");

 }

 unsigned int len = list.size();

 char** pInstId = new char* [len];  

 for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i]; // vector list ת�� char **

 int ret=m_pUserApi_md->SubscribeMarketData(pInstId, len);

 cerr<<" ���� | �������鶩��... "<<((ret == 0) ? "�ɹ�" : "ʧ��")<< endl;
 

 SetEvent(g_hEvent);
}



//����ȫ�г�����
void CtpMdSpi::SubscribeMarketData_all()
{
 SubscribeMarketData(m_charNewIdList_all);
 delete []m_charNewIdList_all;
}




void CtpMdSpi::OnRspSubMarketData(
 CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
 cerr<<" ��Ӧ |  ���鶩��...�ɹ�"<<endl;

 if(bIsLast)  SetEvent(g_hEvent);
}



void CtpMdSpi::OnRspUnSubMarketData(
 CThostFtdcSpecificInstrumentField *pSpecificInstrument,
 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
 cerr<<" ��Ӧ |  ����ȡ������...�ɹ�"<<endl;
 if(bIsLast)  SetEvent(g_hEvent);
}



void CtpMdSpi::OnRtnDepthMarketData(
 CThostFtdcDepthMarketDataField *pDepthMarketData)
{
 if(pDepthMarketData){
  g_lock.lockForWrite();
 
  QString instrumentKey(pDepthMarketData->InstrumentID);
  if(!instrumentMarketData.contains(instrumentKey)){
   instrumentMarketData.insert(instrumentKey,*pDepthMarketData);
  }else{
   instrumentMarketData[instrumentKey] = *pDepthMarketData;
  }
  setModelData(pDepthMarketData);// �����޸��źŵ�ui UI��ʾ����
  //printData(pDepthMarketData);// ���ݱ���csv������
  g_lock.unlock();
 }
 
}





bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{ 
 bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
 if (ret){
  cerr<<" ��Ӧ | "<<pRspInfo->ErrorMsg<<endl;
 }
 return ret;
}






void CtpMdSpi::setAccount(TThostFtdcBrokerIDType appId1, TThostFtdcUserIDType userId1, TThostFtdcPasswordType passwd1)
{
 strcpy(m_appId, appId1);
 strcpy(m_userId, userId1);
 strcpy(m_passwd, passwd1);
}





//���ý��׵ĺ�Լ����
void CtpMdSpi::setInstId(string instId)
{
 strcpy(m_instId, instId.c_str());
}





void CtpMdSpi::setInstIdList_holding_md(string instId)
{
 //strcpy(m_instIdList_holding_md, instId.c_str());
 int sizeInstId = instId.size();

 m_charNewIdList_holding_md = new char[sizeInstId+1];

 memset(m_charNewIdList_holding_md,0,sizeof(char)*(sizeInstId+1));

 strcpy(m_charNewIdList_holding_md, instId.c_str());

 /*strcpy(m_instIdList_all, instIdList_all.c_str());*/

 cerr<<"�гֲֵĺ�Լ:"<<strlen(m_charNewIdList_holding_md)<<","<<sizeof(m_charNewIdList_holding_md)<<","<<_msize(m_charNewIdList_holding_md)<<endl<<m_charNewIdList_holding_md<<endl;

}



//����ȫ�г���Լ����TD����
void CtpMdSpi::set_instIdList_all(string instIdList_all)
{ 
 int sizeIdList_all = instIdList_all.size();

 m_charNewIdList_all = new char[sizeIdList_all+1];

 memset(m_charNewIdList_all,0,sizeof(char)*(sizeIdList_all+1));

 strcpy(m_charNewIdList_all, instIdList_all.c_str());

 /*strcpy(m_instIdList_all, instIdList_all.c_str());*/

 if(!m_charNewIdList_all)//��strlenʱm_charNewIdList_all����Ϊ��
  cerr<<"�յ���ȫ�г���Լ:"<<strlen(m_charNewIdList_all)<<","<<sizeof(m_charNewIdList_all)<<","<<_msize(m_charNewIdList_all)<<endl<<m_charNewIdList_all<<endl;
  
}


void CtpMdSpi::setModelData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
 QString InstrumentID = QString::fromLocal8Bit(pDepthMarketData->InstrumentID);
 QString ExchangeID = QString::fromLocal8Bit(pDepthMarketData->ExchangeID);
 QString InstrumentName = QString::fromLocal8Bit(pDepthMarketData->InstrumentID);
 double dailyPNL = 0;
 QString OpenPrice = QString::number(pDepthMarketData->OpenPrice,'f', 2);
 QString HighestPrice = QString::number(pDepthMarketData->HighestPrice,'f', 2);
 QString LowestPrice = QString::number(pDepthMarketData->LowestPrice,'f', 2);
 QString LastPrice = QString::number(pDepthMarketData->LastPrice,'f', 2);
 QString PreClosePrice = QString::number(pDepthMarketData->PreClosePrice,'f', 2);
 QString Volume = QString::number(pDepthMarketData->Volume);
 QString Turnover = QString::number(pDepthMarketData->Turnover,'f',2);
 QString UpperLimitPrice = QString::number(pDepthMarketData->UpperLimitPrice,'f', 2);
 QString LowerLimitPrice = QString::number(pDepthMarketData->LowerLimitPrice,'f', 2);
 QString UpdateTime = QString::fromLocal8Bit(pDepthMarketData->UpdateTime);

 if (pDepthMarketData->PreClosePrice>0)
  dailyPNL = (pDepthMarketData->LastPrice/pDepthMarketData->PreClosePrice-1)*100;
 QList<QStandardItem *> tList = this->m_quote_model->findItems(InstrumentID);
 if (tList.count()>0)
 {
  int row = tList.at(0)->row();
  
  //m_quote_model->beginResetModel();
  m_quote_model->setData(m_quote_model->index(row, 3), dailyPNL);
  m_quote_model->setData(m_quote_model->index(row, 4), OpenPrice);
  m_quote_model->setData(m_quote_model->index(row, 5), HighestPrice);
  m_quote_model->setData(m_quote_model->index(row, 6), LowestPrice);
  m_quote_model->setData(m_quote_model->index(row, 7), LastPrice);
  m_quote_model->setData(m_quote_model->index(row, 8), PreClosePrice);
  m_quote_model->setData(m_quote_model->index(row, 9), Volume);
  m_quote_model->setData(m_quote_model->index(row,10), Turnover);
  m_quote_model->setData(m_quote_model->index(row,11), UpperLimitPrice);
  m_quote_model->setData(m_quote_model->index(row,12), LowerLimitPrice);
  m_quote_model->setData(m_quote_model->index(row,13), UpdateTime);
  //m_quote_model->endResetModel();
  // ��ֹδ֪����
  if(m_mutex.tryLock()){
   //emit dataChange(m_quote_model->index(row, 3),m_quote_model->index(row, 13));//ȡӦ�ã�
   m_mutex.unlock();
  }
 // cout << row << "  "<< endl;
 }
 else
 {
  //m_quote_model->beginInsertRows();
  m_quote_model->insertRow(0);
  m_quote_model->setData(m_quote_model->index(0, 0), InstrumentID);
  m_quote_model->setData(m_quote_model->index(0, 1), ExchangeID);
  m_quote_model->setData(m_quote_model->index(0, 2), InstrumentName);
  m_quote_model->setData(m_quote_model->index(0, 3), dailyPNL);
  m_quote_model->setData(m_quote_model->index(0, 4), OpenPrice);
  m_quote_model->setData(m_quote_model->index(0, 5), HighestPrice);
  m_quote_model->setData(m_quote_model->index(0, 6), LowestPrice);
  m_quote_model->setData(m_quote_model->index(0, 7), LastPrice);
  m_quote_model->setData(m_quote_model->index(0, 8), PreClosePrice);
  m_quote_model->setData(m_quote_model->index(0, 9), Volume);
  m_quote_model->setData(m_quote_model->index(0,10), Turnover);
  m_quote_model->setData(m_quote_model->index(0,11), UpperLimitPrice);
  m_quote_model->setData(m_quote_model->index(0,12), LowerLimitPrice);
  m_quote_model->setData(m_quote_model->index(0,13), UpdateTime);
  //m_quote_model->endInsertRows();
 }
 //emit update();
}


void CtpMdSpi::printData(CThostFtdcDepthMarketDataField *pDepthMarketData){
 int length = instrumentMarketData.size();
 QDateTime time = QDateTime::currentDateTime();
 QString timeStr = time.toString("yyyyMMdd");
 QMap<QString,CThostFtdcDepthMarketDataField>::const_iterator i;
 ofstream output;
 string outputDir = "output/" + timeStr.toStdString() + ".csv";
 output.open(outputDir,ios::_Nocreate | ios::ate) ;
 QString curTimeStr = time.toString("yyyy/MM/dd hh:mm:ss zzz");
 if(output){
  for( i = instrumentMarketData.constBegin() ; i != instrumentMarketData.constEnd();i++){
   output << curTimeStr.toStdString() << "," << i->UpdateTime << "," << i->InstrumentID << "," <<i->LastPrice << ","<< i->AskPrice1 << "," << i->AskVolume1<< "," << i->BidPrice1 << "," << i->BidVolume1 << endl;
  }
 }else{
  // title
  output.open(outputDir, ios::app);
  output << "reportTime" << "," <<"updateTime" << "," << "InstrumentId" << ","<< "lastPrice" << "," << "AskPrice1" << ","  << "AskVol1" << ","<< "BidPrice1"  << "," << "BidVol" << endl;
 }
 output.close();

 //output.open("output/dataNew.csv",ios::_Nocreate | ios::ate) ;
 //if(output){
 // //content
 // output << pDepthMarketData->InstrumentID<< "," <<pDepthMarketData->ActionDay << "," << pDepthMarketData->UpdateTime<< ","<< pDepthMarketData->PreClosePrice << "," << pDepthMarketData->LastPrice << "," << pDepthMarketData->ClosePrice << endl;
 //}else{
 // // title
 // output.open("output/dataNew.csv", ios::app);
 // output << "InstrumentId" << "," <<"ActionDay" << "," << "updateTime" << ","<< "yesterDayClosePrice" << "," << "lastPrice" << "," << "todayClosePrice" << endl;
 // 
 //}
 //output.close();
}