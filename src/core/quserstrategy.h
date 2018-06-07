#ifndef QUSERSTRATEGY_H
#define QUSERSTRATEGY_H

#include <QThread>
#include <QDateTime>
#include <traderspi.h>
#include "ctpcontrol.h"
#include "qOrderManager.h"
#include "option.h"
struct optionInstrument
{
    QString code;
    double Strike;
    QString tradeDate;
    QString expiryDate;
    double lastPrice;
    double underlyingPrice;
    double askPrice;
    double bidPrice;
    double askVol;
    double bidVol;
    double priceTick; // ��С���۵�λ 1 tick
    string UnderlyingInstrID;// ������Լ����
    
    //optionModel::OptionType ot;
    string optionType ;
    int expiryDayNum; // ����������

    optionInstrument(){
        memset(this, 0, sizeof(optionInstrument));  
    };
    void reset(){
        memset(this, 0, sizeof(optionInstrument));  
    }
};
struct optionPair{
    optionInstrument callOption;
    optionInstrument putOption;

    optionPair(){
        memset(this, 0, sizeof(optionPair));  
    };
    void reset(){
        memset(this, 0, sizeof(optionPair));  
    }
};
class QUserStrategy : public QThread
{
    Q_OBJECT

public:
    QUserStrategy(QOrderManager *,QObject *parent = 0);
    ~QUserStrategy();
    void run();
    bool optionArbitrage(optionInstrument C_K1,optionInstrument C_K2,optionInstrument P_K1,optionInstrument P_K2);
    void optionArbitrageMode1(); //����Լѭ�����
    void optionArbitrageMode2();// ��strikeѭ�����
    void setOptionArbitrageList(vector<QString> ol,QString futureCode){this->m_optionArbitrageList = ol;this->m_futureCode = futureCode;m_futureCodeIndex = futureCode.length();}
    void setStrategyStatus(bool status){this->m_status  = status;if(status)this->start();}
    void printData();
    void updateData();
    void loadConf();
signals:
    void strategyMsg(QString);
    void subscribe_inst_data(QString pInstrumentId); 
    void dataChange(QModelIndex,QModelIndex);
    void hedgeOrder(QString strategyName,QString instId,int hedgeNums);
public slots:
    void onRunClicked();
    void onStopClicked();
private:
    //���Բ���
    optionInstrument C_K1;
    optionInstrument C_K2;
    optionInstrument C_K3;
    optionInstrument C_K4;
    optionInstrument C_K5;

    optionInstrument P_K1;
    optionInstrument P_K2;
    optionInstrument P_K3;
    optionInstrument P_K4;
    optionInstrument P_K5;

    // ״̬ true ���� false δ����
    bool m_status;
    
    CtpTraderSpi* TDSpi_stgy;//TDָ��
    //����Model
    QStandardItemModel *m_strategyGreekModel;

    //��һ��tick����ʱ�� ������һ��tick���µ�tick����ʱ��仯��������
    // �����һ��tick�Ա��µ�����ʱ���� �ޱ仯 ������κν���
    QString m_oldTickUpdate;
    //�����������
    int m_tryInsertOrderTimes;
    //���Դ���
    int m_testTimes;
    //���Թ�����
    QOrderManager *m_pOrderManager;
    //discount
    double discount;
    double riskRate;
    double remainT;
    double expectReturn;
    vector<QString> m_optionArbitrageList;// ��Ȩ��Լ����
    QString m_futureCode;  // ������Լ����
    int m_futureCodeIndex;
    QMap<QString,optionInstrument> m_optionInstrumentList; // ��Ȩ�������ݶ���
    map<string, CThostFtdcInstrumentField*> m_instMessage_map_stgy;//
    map<string,int> m_workDayMap;
    QMap<double,optionPair> m_optionPairList;
    //QMap<QString,vector<QString>> m_option;
};

#endif // QUSERSTRATEGY_H
