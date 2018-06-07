#ifndef QSTRATEGY_H
#define QSTRATEGY_H

#include <QWidget>
#include <qdialog.h>
#include "ui_qstrategy.h"
#include <qstandarditemmodel.h>
#include "qstrategythread.h"
#include <qlist.h>
#include <qvector.h>
#include <qmap.h>
#include <qthread.h>
#include "ctpcontrol.h"
#include "qOrderManager.h"
class QStrategy : public QDialog
{
    Q_OBJECT

public:
    QMap<QString,QStrategyThread*> m_pStrategyList;
    QStrategy(QWidget *parent,QStandardItemModel *m_OrderManageModel);
    ~QStrategy();
    void loadTemplate(int row);
    void checkProductList();
    void freshModel();
    void freshStrategy();
    QStandardItemModel * createModel();
    void setCtpControl(ctpControl*);
    QStandardItemModel * createStrategyGreekModel();
    QStandardItemModel * getStrategyGreekModel(){return m_strategyGreekModel;};
    QOrderManager * getOrderManager();
    void onRunClicked(bool allowTrade,int spanTime);
    void onStopClicked();
    void onUpdateClicked();
    void onFreshTrade();

    void onDelClicked();
    void setStrategyGreeksTableData(QString rowName,cashGreeks totalGreeks);
    void getBookInstrumentList(QString bookName);
    public slots:
        void on_save_clicked();
        void on_cancle_clicked();
        void on_searchBook_clicked();
        void onSettleClicked(bool defaultSettle = true);
        // 通过tradepp 绑定 下面函数接受信号
        void strategyStatistic();
        void subscribe_inst_data(QString pInstrumentId);
        void dataChange(QModelIndex lefttop,QModelIndex bottomRight);  
private:

    Ui::QStrategy ui;
    ctpControl *m_pCtpControl;
    QStandardItemModel * m_strategy_model;
    //edit 标志位
    bool edit;
    //策略Model
    QStandardItemModel *m_strategyGreekModel;
    QThread m_strategyManagerThread;
    QOrderManager *m_pOrderManager;
    vector<string> m_instumentList;

    QString m_productName ;


    int m_optionType; // combox是从0开始计算的
    int m_scaleUnit ; 
    QString m_callScale; 
    QString m_putScale ; 
    QDateTime m_settlementDate;
    QDateTime m_maturityDate;
    QDateTime m_nextAveragePoint;
    QDateTime m_firstAveragePoint;
    QString m_initialPrice;
    QString m_averagePrice;
    QString m_callExecPricePercent;
    QString m_putExecPricePercent;
    QString m_interest;
    QString m_dividend;
    QString m_barrier;

    int m_callHedgeDirection;
    int m_putHedgeDirection ;
    
    QTime m_strategyStartTime;

    QString m_openNums;
    QString m_hedgeLevel ;
    QString m_hedgeVolability ;
    QTime m_carryNoonTime ;
    QTime m_carryNightTime;
    int m_carryNoonDistance; 
    int m_carryNightDistance;

    int m_totalAveragePoint ; 
    int m_averagedPoint ; 

    QString m_instrumentCode ; 
    QString m_instrumentCode2 ; 
    QString m_instrumentCode3 ; 
    QString m_instrumentCode4 ; 

    double m_instrumentRatio ;; 
    double m_instrumentRatio2; 
    double m_instrumentRatio3; 
    double m_instrumentRatio4; 

    int m_hedgeThreshold; 

    double m_tickValue;
    double m_tickValue2;;
    double m_tickValue3;
    double m_tickValue4;

    //Qtimer for sum all strategy cash greeks 
    QTimer m_sumTimer;

    //Status
    int m_bookStauts;
    double m_PNL;
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
    //work Day
    map<string,int> m_workDay;
    map<string,int> m_calendarDay;

};

#endif // QSTRATEGY_H
