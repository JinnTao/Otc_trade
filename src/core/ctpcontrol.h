#ifndef    CTPCONTROL_H
#define    CTPCONTROL_H

#include    <QThread>
#include    "StructFunction.h"

#include    <ThostFtdcUserApiStruct.h>
#include    <ThostFtdcMdApi.h>
#include    <ThostFtdcTraderApi.h>
#include    "mdspi.h"
#include    "traderspi.h"
#include    <qtextbrowser.h>
#include    <qstandarditemmodel.h>
class    ctpControl    :    public    QThread
{
    Q_OBJECT

public:
    ctpControl(QObject    *parent    =    0);

    ~ctpControl();

    void    init(AccountParam    ap,QObject*    loggingObject);
    void    run();
    QStandardItemModel    *createQuoteModel();
    void    subscribe_inst_data(TThostFtdcInstrumentIDType    pInstrumentId);
    CtpTraderSpi*    getTradeSpi();



    CThostFtdcMdApi    *pUserApi_md;    //    行情接口
    
    CtpMdSpi        *pUserSpi_md;            //    行情重构

    CThostFtdcTraderApi    *pUserApi_trade;    //    交易接口

    CtpTraderSpi    *pUserSpi_trade;            //    交易重构

    AccountParam    m_accountParam;

    QObject    *m_loggingObject;
    QStandardItemModel    *    m_quote_model;
    
    QTimer*    reconnectionTimer;
public    :
    void    releaseCtpConnect();
    
    vector<QString>    *    getInstrumentListVt();
signals:

    void    connectCtpQuest(CThostFtdcRspInfoField    *curStatus);
    void    ctpcontrolMsg(CTPSTATUS);
    void    ctpMessage(QString);
public    slots:
    void    connectCtpRespond(CThostFtdcRspInfoField    *pRspInfo);
    void    dataChange(const    QModelIndex    &,const    QModelIndex    &);
    void    disconnection();
    void    checkConnectStatus();
};

#endif    //    CTPCONTROL_H
