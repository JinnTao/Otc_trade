#ifndef QSTRATEGYTHREAD_H
#define QSTRATEGYTHREAD_H

#include <QThread>
#include <QDateTime>
#include <traderspi.h>
#include "ctpcontrol.h"
//#include "optionController.h"
#include "qOrderManager.h"
//#include "asianPrice.h"
//#include "bjsmodel.h"
#include "option.h"
#include "structure.h"




struct instrumentDetail
{
    QString instrumentCode;
    QString lastUpdatetick;
    double tickValue;
    double ratio;
    optionHedgeHands hedgeLots;
    bool updateToBook;
    int hedgeThreshold;
    instrumentDetail(){
    //    memset(this, 0, sizeof(instrumentDetail));  
    };
    void reset(){
    //    memset(this, 0, sizeof(instrumentDetail));  
    }
};

class QStrategyThread : public QThread
{
public:
    Q_OBJECT

public:
    //friend class QStrategy;
    QStrategyThread(QStandardItemModel*,QOrderManager*,QObject *parent = 0);
    ~QStrategyThread();

    void loadConf(QString confName);
   
    void run();
    void StrategyOperation(int hedgeDifference,int position,CThostFtdcDepthMarketDataField *pDepthMarketData);
    inline void StrategyOptionCalcultate(CThostFtdcDepthMarketDataField *pDepthMarketData);
    void GetTimeTic(TThostFtdcDateType ActionDay,TThostFtdcTimeType UpdateTime,time_t&curTimeTic,time_t&everyDayStratTime);

    void setModelData(QString instrumentCode,optionGreeks CallOptionGreeks,optionGreeks PutOptionGreeks,optionHedgeHands CurHedgeHands,double ratio,double,QString prodouctName,cashGreeks & greeksCash);
    void PrintLogging(CThostFtdcDepthMarketDataField *pDepthMarketData,optionGreeks callGreeks,optionGreeks putGreeks,optionHedgeHands hedgeHands,optionParam _optionParam);
    void setTDSpi(CtpTraderSpi *spi);
    bool optionChoose(double underlyingPrice ,time_t _curDateTime,optionGreeks &_CallgreeksValue,optionGreeks &_putGreeksValue);
    void carryTime();
    bool valideDate(time_t _settlementDate,time_t _todayDate,time_t _maturityDate);
    double getHands(int scaleUnit,double scale,double tickValue);
    bool timeCalculate(TThostFtdcTimeType,double);

    map<QString,cashGreeks>* getStrategyCashGreeks(){
        return &(this->m_strategyCashGreeksMap);
    };
    void StrategyOperationMutil();
    void testFunc();
    double H0(double tau,double lamuda,double vol,double riskA,double gamma,double rate,double underlying);
    void bookSettle();
    void bookFresh();
    void settleSaveToIniFile();
    void settleUpdate();
    void settlePrintToExcel(bool saveToExcel = true);
    bool settleTimeCalculate(double underlyingPrice);
signals:
    void strategyMsg(QString);
    void subscribe_inst_data(QString pInstrumentId); 
    void dataChange(QModelIndex,QModelIndex);
    void updateToOrderBook(QString strategyName,QString instId,int totalHedgeNum,int hedgeThreshold,int maxOpenNum);

public:
    // 每个策略的
    QString productName;
    int optionType;
    int scaleUnit ;

    int m_canlendarNTotal;
    int m_tradeNTotal;
    int m_canlendarN;
    int m_tradeN;
    int m_tradeT1N;
    int m_calendarT1N;

    double callScale ;
    double putScale;
    QDateTime settlementDate;
    QDateTime maturityDate;
    QDateTime nextAveragePoint;
    QDateTime firstAveragePoint;
    double m_initialPrice ;
    double averagePrice;
    double m_averagePriceAdjust;

    //QString execPricePercent ;
    double m_putExecPricePercent ;
    double m_putExerPrice;
    double m_callExecPricePercent ;      // 原定是由于未知执行价具体值，只知道执行价百分比，目前全部修改为执行价确定值
    double m_callExerPrice;

    double interest;
    double dividend; // 这个表示costofcarry
    double barrier ;
    double tickValue;
    double tickValue2;
    double tickValue3;
    double tickValue4;
    int callHedgeDirection ;
    int putHedgeDirection ;
    double priceVolability;
    QString margin;
    QString openCommission ;
    QTime strategyStartTime;
    QTime settleTime ;
    int m_openNums;
    QString hedgeLevel ;
    double hedgeVolability ;
    QTime carryNoonTime;
    QTime carryNightTime ;

    int carryNoonDistance ;
    int carryNightDistance;
    int totalAveragePoint;
    int averagedPoint;
    int m_averagedPointAdjust;
    QString instrumentCode ;
    QString instrumentCode2;
    QString instrumentCode3;
    QString instrumentCode4;
    QList<instrumentDetail> m_instrumentList;
    bool m_instrumentStatus;
    double instrumentRatio;
    double instrumentRatio2;
    double instrumentRatio3;
    double instrumentRatio4;

    double m_tau;
    double m_t1;

    double m_nextAverageTime;
    double m_firstAverageTime;
    int hedgeThreshold;

    // 状态 true 运行 false 未运行
    bool m_status;
    // 交易时间间隔
    int m_spanTime;

    // ctp control;
    // option manager
//    optionController m_oOptionCalculateEngine;
    // carry 时间Tic
    time_t m_nightTime;
    time_t m_noonTime;
    time_t m_noonCloseTime;

    CtpTraderSpi* TDSpi_stgy;//TD指针
    //策略Model
    QStandardItemModel *m_strategyGreekModel;

    //非交易时间
    int m_iNoTradeHour1;
    int m_iNoTradeHour2;
    //上一个tick更新时间 基于上一个tick和新的tick更新时间变化触发行情
    // 如果上一个tick对比新的数据时间上 无变化 则进行任何交易
    QString m_oldTickUpdate;
    //订单插入次数
    int m_tryInsertOrderTimes;
    //测试次数
    int m_testTimes;
    //策略管理器
    QOrderManager *m_pOrderManager;
    //定价mode
    //optionModel::optionEngine m_optionEngine;
    // deltay
    bool m_isCarry;
    //work Day
    map<string,int> m_workDay;
    map<string,int> m_calendarDay;

    double m_tradeCostPercent;
    double m_riskAversion;

    cashGreeks strategyCash;

    // map Inst - cash
    map<QString,cashGreeks> m_strategyCashGreeksMap;

    // 20180515 add parameters
    int m_bookStatus; //   0 存续 1 到期 2 平仓
    double m_PNL;     // 每日PNL
    double m_daySettlePrice;
    double m_optionFee;
    QString m_counterParty;
    double m_deliveryPrice; //交割价
    QDateTime m_paymentDate;   //交付日
    QDateTime m_deliveryDate;//交割日
    QDateTime m_terminationDate; //终止日
    double m_tradeValue;
    double m_presentValue;
    double m_returnProfit;
    string m_today;
    double m_settleTau;
    double m_settleT1;
};

#endif // QSTRATEGYTHREAD_H
