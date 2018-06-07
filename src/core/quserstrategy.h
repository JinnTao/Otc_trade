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
    double priceTick; // 最小报价单位 1 tick
    string UnderlyingInstrID;// 基础合约代码
    
    //optionModel::OptionType ot;
    string optionType ;
    int expiryDayNum; // 到期日天数

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
    void optionArbitrageMode1(); //按合约循环检测
    void optionArbitrageMode2();// 按strike循环检测
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
    //策略参数
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

    // 状态 true 运行 false 未运行
    bool m_status;
    
    CtpTraderSpi* TDSpi_stgy;//TD指针
    //策略Model
    QStandardItemModel *m_strategyGreekModel;

    //上一个tick更新时间 基于上一个tick和新的tick更新时间变化触发行情
    // 如果上一个tick对比新的数据时间上 无变化 则进行任何交易
    QString m_oldTickUpdate;
    //订单插入次数
    int m_tryInsertOrderTimes;
    //测试次数
    int m_testTimes;
    //策略管理器
    QOrderManager *m_pOrderManager;
    //discount
    double discount;
    double riskRate;
    double remainT;
    double expectReturn;
    vector<QString> m_optionArbitrageList;// 期权合约队列
    QString m_futureCode;  // 基础合约代码
    int m_futureCodeIndex;
    QMap<QString,optionInstrument> m_optionInstrumentList; // 期权具体内容队列
    map<string, CThostFtdcInstrumentField*> m_instMessage_map_stgy;//
    map<string,int> m_workDayMap;
    QMap<double,optionPair> m_optionPairList;
    //QMap<QString,vector<QString>> m_option;
};

#endif // QUSERSTRATEGY_H
