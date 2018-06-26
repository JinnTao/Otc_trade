#include "traderqtapp.h"
#include <QtWidgets/QApplication>

#include "logindialog.h"
#include <qtextcodec>
#include <windows.h>
#include "ctpcontrol.h"
#include <qstring.h>
#include <qmap.h>
#include <qreadwritelock.h>
#include <qmutex.h>
#include <atomic>
#include <qatomic.h>
#include <qsemaphore.h>
#include <qmutex.h>

//#define ELPP_THREAD_SAFE
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#define LOG_CONFIG_DIR "easyLog.conf"

HANDLE g_hEvent;

QReadWriteLock g_lock;

QMap<QString,CThostFtdcDepthMarketDataField > instrumentMarketData;

QMap<QString,QAtomicInt > gOrderUnfinished;

QMutex g_mutex;                                                    

int requestId=0;

int main(int argc, char *argv[])
{
    int result;
    el::Configurations conf(LOG_CONFIG_DIR);
    el::Loggers::reconfigureAllLoggers(conf);
    LOG(INFO) << "otc trade start ";
    QApplication a(argc, argv);

    TraderQtApp w;

    ctpControl ctpThreadControl(&w);

    loginDialog login(&w);

    w.setQuoteModel(ctpThreadControl.createQuoteModel());
    w.setCtpControl(&ctpThreadControl);
    login.setCtpControl(&ctpThreadControl);

    QObject::connect(&ctpThreadControl,SIGNAL(connectCtpQuest(CThostFtdcRspInfoField*)),&login,SLOT(logginRespond(CThostFtdcRspInfoField*)));
    QObject::connect(&ctpThreadControl,SIGNAL(ctpcontrolMsg(CTPSTATUS)),&w,SLOT(onCtpControlMsg(CTPSTATUS)));
    QObject::connect(&ctpThreadControl,SIGNAL(ctpMessage(QString)),&w,SLOT(onStrategyMsg(QString)));
    if(login.exec() == QDialog::Accepted){
    
        w.show();
        w.initConf();
        result=  a.exec();

    }else{
        result =  0;
    }
    
    ctpThreadControl.quit();
    ctpThreadControl.wait();
    while(true){}

    return result;

}
