#ifndef    LOGINDIALOG_H
#define    LOGINDIALOG_H

#include    <QDialog>
#include    "ui_logindialog.h"
#include    <StructFunction.h>
#include    <ThostFtdcUserApiStruct.h>
#include    <ThostFtdcMdApi.h>
#include    <ThostFtdcTraderApi.h>
#include    "ctpcontrol.h"
class    loginDialog    :    public    QDialog
{
    Q_OBJECT

public:
    loginDialog(QWidget    *parent    =    0);
    void    setCtpControl(ctpControl    *ctpManage);
    ~loginDialog();
protected:
    bool    validCheck();

private    slots:
        void    on_loginBtn_clicked();
        void    on_logoutBtn_clicked();
        void    logginRespond(CThostFtdcRspInfoField    *pRspInof);
        void    timerUpDate();
private:
    ctpControl    *m_ctpController;
    
    QString    userId;

    QString    userPw    ;

    QString    brokerId;

    QString    mdAddress;

    QString    tdAddress    ;

    Ui::loginDialog    ui;
    
    AccountParam    m_accountParam;    //参数结构
    
    CThostFtdcMdApi    *pUserApi_md;        //    行情接口
    
    CThostFtdcTraderApi    *pUserApi_trade;    //交易接口

    userStatus    loggingStatu;

    QTimer*    loggingTimer;
    int    times;
};

#endif    //    LOGINDIALOG_H
