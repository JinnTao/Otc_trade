#ifndef ORDER_TRADERSPI_H_
#define ORDER_TRADERSPI_H_


#include "ThostFtdcTraderApi.h"
#include <vector>
#include <map>
#include "StructFunction.h"
#include "ThostFtdcMdApi.h"
#include <qtextbrowser.h>
#include <qstandarditemmodel.h>
class cashGreeks{
public:
 cashGreeks(){
  _deltaCash = 0;
  _gammaCash = 0;
  _vegaCash = 0;
  _rho = 0;
  _thetaCash = 0;
 }
 ~cashGreeks(){
 
 }
public:   
 // cash
 double _deltaCash;// delta * ��ļ۸�
 // gamma * ��ļ۸�^2 * 100
 double _gammaCash;
 // Theta ������/��
 double _thetaCash;
 // vega * ������ * 100
 double _vegaCash;
 // Rho 1%���޷������ʵı仯�Լ�ֵ������Ӱ��
 double _rho;

};

Q_DECLARE_METATYPE(cashGreeks);





class CtpMdSpi;


class CtpTraderSpi : public QObject ,public CThostFtdcTraderSpi
{
 Q_OBJECT
public:
 CtpTraderSpi(CThostFtdcTraderApi* api, CThostFtdcMdApi* pUserApi_md, CtpMdSpi* MDSpi,QObject *qObject):m_pUserApi_td(api), m_pMDUserApi_td(pUserApi_md), m_MDSpi(MDSpi)
 {  
  m_closeProfit = 0.0;//ƽ��ӯ��
  m_OpenProfit = 0.0;//����ӯ��

  first_inquiry_order = true;//�Ƿ��״β�ѯ����
  first_inquiry_trade = true;//�Ƿ��״β�ѯ�ɽ�
  firs_inquiry_Detail = true;//�Ƿ��״β�ѯ�ֲ���ϸ
  firs_inquiry_TradingAccount = true;//�Ƿ��״β�ѯ�ʽ��˺�
  firs_inquiry_Position = true;//�Ƿ��״β�ѯͶ���ֲ߳�
  first_inquiry_Instrument = true;//�Ƿ��״β�ѯ��Լ
  m_connect_status = false; // �ڳ�����״̬ false
  this ->m_qObejct = qObject;
 };

 ///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
 virtual void OnFrontConnected();

 ///��¼������Ӧ
 virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///Ͷ���߽�����ȷ����Ӧ
 virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�����ѯ��Լ��Ӧ
 virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�����ѯ�ʽ��˻���Ӧ
 virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�����ѯͶ���ֲ߳���Ӧ
 virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///����¼��������Ӧ
 virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///��������������Ӧ
 virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///����Ӧ��
 virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
 virtual void OnFrontDisconnected(int nReason);

 ///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
 virtual void OnHeartBeatWarning(int nTimeLapse);

 ///����֪ͨ
 virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

 ///�ɽ�֪ͨ
 virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

 ///�����ѯ������Ӧ
 virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�����ѯ�ɽ���Ӧ
 virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 ///�����ѯͶ���ֲ߳���ϸ��Ӧ
 virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


public:
 ///�û���¼����
 void ReqUserLogin(TThostFtdcBrokerIDType appId,
  TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd);
 
 ///Ͷ���߽�����ȷ��
 void ReqSettlementInfoConfirm();
 
 ///�����ѯ��Լ
 void ReqQryInstrument(TThostFtdcInstrumentIDType instId);
 
 ///�����ѯ��Լ�����к�Լ
 void ReqQryInstrument_all();

 ///�����ѯ�ʽ��˻�
 void ReqQryTradingAccount();
 
 ///�����ѯͶ���ֲ߳�
 void ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId);

 ///�����ѯͶ���ֲ߳�,���к�Լ
 void ReqQryInvestorPosition_all();
 
 ///����¼������
 void ReqOrderInsert(TThostFtdcInstrumentIDType instId,
  TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
  TThostFtdcPriceType price,   TThostFtdcVolumeType vol);
 
 ///������������
 void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);

 ////������Ӧ
 bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

 //��ӡ������¼
 void PrintOrders();
 
 //��ӡ�ɽ���¼
 void PrintTrades();

 //�����ѯ����
 void ReqQryOrder();

 //�����ѯ�ɽ�
 void ReqQryTrade();

 //�����ѯͶ���ֲ߳���ϸ
 void ReqQryInvestorPositionDetail();

 //�˺�����
 void setAccount(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd);

 //����������׷�������ڱ����ر�����ȳ����ɹ����ٽ���
 void CancelOrder(const string& MDtime, double MDprice,bool allow_open);
 void CancelOrderList();

 //���ý��׵ĺ�Լ����
 void setInstId(string instId);

 //ǿ���˻�ƽ��
 void ForceClose();

 //����ƽ��ӯ��
 double sendCloseProfit();

 //�����˻��ĸ���ӯ�����Կ��ּ��� 
 double sendOpenProfit_account(string instId, double lastPrice);

 //����ֲ�ӯ�����������㣬�������ڣ��븡��ӯ����ͬ��ʵ�ʽ�����Ӧ�ñȽ��٣�
 double sendPositionProfit();

 //��ӡ���к�Լ��Ϣ
 void showInstMessage();

 //��ӡ�ֲ���Ϣm_trade_message_map
 void printTrade_message_map(const char * InstId);

 //���º�Լ�����¼�
 void setLastPrice(string instID, double price);

 //��Լ������Ϣ�ṹ���map��KEY�ĸ�����Ϊ0��ʾû�иú�Լ�Ľ�����Ϣ�����ú�Լû�гֲ�(���ֺ��Ѿ�ƽ�ֵģ�KEY��Ϊ0��
 int send_trade_message_map_KeyNum(string instID);

 //����ĳ��Լ�൥�ֲ���
 int SendHolding_long(string instID);

 //����ĳ��Լ�յ��ֲ���
 int SendHolding_short(string instID);
 // ֱ��ƽ��
 void StraitClose(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcPriceType price,TThostFtdcVolumeType vol);

 void saveOnRtnTrade(CThostFtdcTradeField* curTradeOrder);
 
 void saveInstrumentField(CThostFtdcInstrumentField *intField);

 bool subscribe_inst_data(TThostFtdcInstrumentNameType InstrumentId);
 bool getConnectStatus(){return this->m_connect_status;}
 bool getfirst_inquiry_order(){return first_inquiry_order;}
 vector<QString>* getInstrumentList();
 map<string, CThostFtdcInstrumentField*> getInstMessageMap(){return this->m_instMessage_map;}
 map<string, CThostFtdcInstrumentField*> m_instMessage_map;//�����Լ��Ϣ��map
signals:
 void IsErrorRspInfo_QT(CThostFtdcRspInfoField *pRspInfo); // �����ź�
 void disconnection ();
 // �����ɽ��ź�  ���������ź� ֮��
private:
 
 CThostFtdcTraderApi* m_pUserApi_td;//����APIָ�룬���캯���︳ֵ
 
 CThostFtdcMdApi* m_pMDUserApi_td;//����APIָ�룬���캯���︳ֵ
 
 CtpMdSpi* m_MDSpi;//MDָ�룬���캯���︳ֵ

 TThostFtdcBrokerIDType m_appId;// Ӧ�õ�Ԫ
 TThostFtdcUserIDType m_userId;// Ͷ���ߴ���
 char m_passwd[252];//����


 double m_closeProfit;//ƽ��ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ��ƽ��ӯ��ֵ
 
 double m_OpenProfit;//����ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ�ĸ���ӯ��ֵ
 
 map<string, trade_message*> m_trade_message_map;//��Լ������Ϣ�ṹ���map


 
 TThostFtdcInstrumentIDType m_instId;//��Լ����
 
 string m_Instrument_all;//���к�Լ�������һ��

 vector<QString> m_Instrument_vt; // ���� ��Լ
 
 vector<string> subscribe_inst_vec;//��Ҫ��������ĺ�Լ������������ǰ�гֲֹ��ĺ�Լ

 bool first_inquiry_order;//�Ƿ��״β�ѯ����
 bool first_inquiry_trade;//�Ƿ��״β�ѯ�ɽ�
 bool firs_inquiry_Detail;//�Ƿ��״β�ѯ�ֲ���ϸ
 bool firs_inquiry_TradingAccount;//�Ƿ��״β�ѯ�ʽ��˺�
 bool firs_inquiry_Position;//�Ƿ��״β�ѯͶ���ֲ߳�
 bool first_inquiry_Instrument;//�Ƿ��״β�ѯ��Լ
 
 vector<CThostFtdcOrderField*> orderList;//ί�м�¼��ȫ����Լ
 vector<CThostFtdcOrderField*> pendOrderList;//�ҵ���¼��ȫ����Լ
 vector<CThostFtdcTradeField*> tradeList;//�ɽ���¼��ȫ����Լ
 
 vector<CThostFtdcTradeField*> tradeList_notClosed_account_long;//δƽ�ֵĶ൥�ɽ���¼,�����˻���ȫ����Լ
 vector<CThostFtdcTradeField*> tradeList_notClosed_account_short;//δƽ�ֵĿյ��ɽ���¼,�����˻���ȫ����Լ

 string m_tradeDate;//��ǰ�����գ��ֱֲ��еĿ������ڲ���ȣ�������֣�����ֱ�Ӹ�ֵ

 QObject * m_qObejct;
 CThostFtdcRspInfoField rspError;
 // �ڳ�����״̬
 bool m_connect_status;
 //int (*ccbf_secureApi_LoginTrader)(CThostFtdcTraderApi* ctp_futures_pTraderApi, TThostFtdcBrokerIDType brokeId, TThostFtdcUserIDType userId, char* pChar_passwd, int& ctp_futures_requestId);
};
typedef int (*ccbf_secureApi_LoginTrader)(CThostFtdcTraderApi* ctp_futures_pTraderApi, TThostFtdcBrokerIDType brokeId, TThostFtdcUserIDType userId, char* pChar_passwd, int& ctp_futures_requestId);
#endif