#ifndef MD_SPI_H_
#define MD_SPI_H_

#include "ThostFtdcMdApi.h"
#include <vector>
//#include <qstring.h>
#include <qstandarditemmodel.h>
#include <string.h>
using std::string;
class CtpMdSpi : public QObject,public CThostFtdcMdSpi
{
 Q_OBJECT
public:
 CtpMdSpi(CThostFtdcMdApi* api, QStandardItemModel * quote_model):m_pUserApi_md(api)
 {
  m_charNewIdList_all = NULL;

  m_charNewIdList_holding_md = NULL;
  this->m_quote_model = quote_model;
  
 };
 ///����Ӧ��
 virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
  int nRequestID, bool bIsLast);

 ///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
 ///@param nReason ����ԭ��
 ///        0x1001 �����ʧ��
 ///        0x1002 ����дʧ��
 ///        0x2001 ����������ʱ
 ///        0x2002 ��������ʧ��
 ///        0x2003 �յ�������
 virtual void OnFrontDisconnected(int nReason);

 ///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
 ///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
 virtual void OnHeartBeatWarning(int nTimeLapse);

 ///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
 virtual void OnFrontConnected();

 ///��¼������Ӧ
 virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///��������Ӧ��
 virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///ȡ����������Ӧ��
 virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�������֪ͨ
 virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);



public:
 
 //��¼�ڻ��˻�
 void ReqUserLogin(TThostFtdcBrokerIDType appId,
  TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd);

 //��������
 void SubscribeMarketData(char* instIdList);

 //����ȫ�г���Լ����TD����
 void set_instIdList_all(string instIdList_all);

 //����ȫ�г����飬����Ҫʱ�ŵ��ã�Ҫ����TD����set_instIdList_all()����������ȫ�г��ĺ�Լ��
 //����ʱֻ�趩������ĺ�Լ����
 void SubscribeMarketData_all(); 

 //������Ӧ
 bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

 //�˺�����
 void setAccount(TThostFtdcBrokerIDType appId1, TThostFtdcUserIDType userId1, TThostFtdcPasswordType passwd1);

 //����Ҫ��������ĺ�Լ����
 void setInstId(string instId);

 //���ó�������ǰ�����֣���Ҫ��������ĺ�Լ
 void setInstIdList_holding_md(string instId);
  
 void setModelData(CThostFtdcDepthMarketDataField *pDethMarket);
 
 void printData(CThostFtdcDepthMarketDataField *pDepthMarketData);
signals:
 void dataChange(const QModelIndex&,const QModelIndex&);
 void disconnection ();
private:

 CThostFtdcMdApi* m_pUserApi_md;//����APIָ��

 char* m_charNewIdList_all;//ȫ�г���Լ

 char* m_charNewIdList_holding_md;//��������ǰ�����֣���Ҫ��������ĺ�Լ

 TThostFtdcInstrumentIDType m_instId;//Ҫ��������ĺ�Լ���룬������Ҫ���׵ĺ�Լ
 
 TThostFtdcBrokerIDType m_appId;//���͹�˾����
 TThostFtdcUserIDType m_userId;//�û���
 char m_passwd[252];//����

 QStandardItemModel * m_quote_model;
 QMutex m_mutex;
 
};

typedef int (*ccbf_secureApi_LoginMd)(CThostFtdcMdApi* ctp_futures_pMdApi, TThostFtdcBrokerIDType brokeId, TThostFtdcUserIDType userId, char* pChar_passwd, int& ctp_futures_requestId);

#endif