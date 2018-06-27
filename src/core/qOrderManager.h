#ifndef QSTRATEGYMANAGER_H
#define QSTRATEGYMANAGER_H

#include <QObject>
#include <qtimer.h>
#include "traderspi.h"
struct strategyOrder
{
 QString InstId;
 int OrderNums;
 int hedgeThreshold;
 int maxOpenNum;
    cashGreeks strategyCashGreeks;
 strategyOrder(){};
 strategyOrder(QString instId,int nums):InstId(instId),OrderNums(nums){}
};
enum  DIRECTION{buy,sell};
enum  OFFSETFLAG{open,close};
enum  HEDGETYPE{FIXTIME,WhalleyWilmott,Zakamouline};
class QOrderManager : public QObject
{
 Q_OBJECT

public:
 QOrderManager(QObject *parent,QWidget *,QStandardItemModel*);
 ~QOrderManager();

 //����TradeSpi
 void SetTradeSpi(CtpTraderSpi *pTradeSpi);
 bool connectCheck();//���Ӽ��
 void sendOrder(QString ,int hedgeDifference,int position,CThostFtdcDepthMarketDataField *pDepthMarketData,bool allowTrade);
 void setModelData(QString instId,int  hedgeDifferenceNums,int netPos,int exposure);
    void setStrategyGreeksTableData(QString rowName,cashGreeks totalGreeks);
 QStandardItemModel * createOrderManageModel();
 bool insertOrder(QString instId,DIRECTION dir, OFFSETFLAG , int num, double price);// �µ��� 
 map<string, CThostFtdcInstrumentField*> getInstMessageMap(){return this->m_instMessage_map_stgy;};
 void setAllowTrade(bool allowTrade);
 void setAversionCofficient(double );
 void setHedgeType(int type);
    void setStrategyGreeksModel(QStandardItemModel *p){ this->m_pStrategyGreeksModel = p;};
 //void set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField*>& instMessage_map_stgy);
public 
slots:
void updateToOrderBook(QString strategyName,QString instId,int Nums,int hedgeThreshold,int maxOpenNum);
 void OrderOperation();
signals:
 void browserMsg(QString);
 void dataChange(QModelIndex,QModelIndex);
private:
 
 QMap<QString,int> m_mInstHedgeNums; //��Լ��ǰӦ�Գ�����
 QMap<QString,int> m_mInstTotalNum;//��Լ��ǰӦ�Գ������ϼ�
 QMap<QString,int> m_mInstThreshold;//��Լ�Գ���ֵ
 QMap<QString,strategyOrder> m_mStrategyOrder; // ���Զ�������
 CtpTraderSpi* m_pTradeSpi;//TDָ��
 QTimer m_hedgeTimer;
 bool m_tradeStatus;
 bool m_tradeTimeStatus;
 bool m_allowTrade;
 double m_aversionCofficient;
 HEDGETYPE m_hedgeType;
 map<string, CThostFtdcInstrumentField*> m_instMessage_map_stgy;//�����Լ��Ϣ��map ��TD���� IN set traderSpi

 //�Գ���� Model
 QStandardItemModel *m_OrderManageModel;

 // �����һ��tick�Ա��µ�����ʱ���� �ޱ仯 ������κν���
 QString m_oldTickUpdate;
    // greeksModel for calculate all book's greeks 
    QStandardItemModel* m_pStrategyGreeksModel;
};

#endif // QSTRATEGYMANAGER_H
