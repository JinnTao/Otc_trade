#include "logindialog.h"
#include <QTextCodec>
#include <qsettings>
#include <qfile>
#include <qtimer.h>
#include <time.h>
#include <qdatetime.h>
#define CONFIGFILE "input/accountParam.ini"


loginDialog::loginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(QIcon(QStringLiteral(":/Resources/Resources/icon")));
    times = 0;
    ui.setupUi(this);
    ui.pwEdit->setEchoMode(QLineEdit::Password);
    this->loggingTimer = new QTimer(this);
    // 连接计时器信号
    connect(loggingTimer,SIGNAL(timeout()),this,SLOT(timerUpDate()) );
    
    //读入配置文件
    if(QFile::exists(CONFIGFILE))
    {
        QSettings *configSetting = new QSettings(CONFIGFILE,QSettings::IniFormat);
        configSetting->setIniCodec("UTF-8");

        this-> brokerId = configSetting ->value("/Account/brokerID").toString();
        this-> userId = configSetting ->value("/Account/userId").toString();
        this-> userPw = configSetting ->value("/Account/passwd").toString();
        this-> mdAddress = configSetting ->value("/FrontAddress/MDAddress").toString();
        this-> tdAddress = configSetting ->value("/FrontAddress/TDAddress").toString();
        ui.brokerIdEdit->setText(brokerId);
        ui.userEdit->setText(userId);
        ui.pwEdit->setText(userPw);
        ui.mdAddressEdit->setText(mdAddress);
        ui.trAddressEdit->setText(tdAddress);
    
        delete configSetting;
    }else{
        ui.loggingBrowser->append("config file no exist");
    }

}

loginDialog::~loginDialog()
{
    delete loggingTimer;
}


 
void loginDialog::on_loginBtn_clicked(){
    //this->accept();
    //验证
    if(validCheck())
    {
        this->setDisabled(true);
        this->loggingTimer->start(1000);
        this->times = 0;
        this->ui.loggingBrowser->append(QString::fromLocal8Bit("正在登录,请稍后...."));
        QString qsbroker = this->brokerId;
        QString qsUserId = this->userId;
        QString qsuserPw = this->userPw;
        QString qsMd = this->mdAddress;
        QString qsTd = this->tdAddress;
        string sbroker = qsbroker.toStdString().c_str();
        string sUserId = qsUserId.toStdString().c_str();
        string suserPw = qsuserPw.toStdString().c_str();
        string sMd = qsMd.toStdString().c_str();
        string sTd = qsTd.toStdString().c_str();

        strcpy_s( m_accountParam.m_appId, sizeof( TThostFtdcBrokerIDType ), sbroker.c_str() );
        strcpy_s( m_accountParam.m_userId, sizeof( TThostFtdcInvestorIDType ), sUserId.c_str());
        strcpy_s( m_accountParam.m_passwd, sizeof( m_accountParam.m_passwd ), suserPw.c_str());

        // TThostFtdcBrokerNameType 81字节 
        strcpy_s( m_accountParam.m_mdFront, sMd.c_str());
        strcpy_s( m_accountParam.m_tradeFront, sTd.c_str());

        this->m_ctpController->init(m_accountParam,this);

        this->m_ctpController->start();

        //取消注释直接登录
        this->accept();
        
    }else{
        
        
        ui.loggingBrowser->append(tr("Please check input"));
    }
}

bool loginDialog::validCheck(){

    this-> userId = ui.userEdit->text();
    this-> userPw = ui.pwEdit->text();
    this-> brokerId = ui.brokerIdEdit->text();
    this-> mdAddress = ui.mdAddressEdit->text();
    this-> tdAddress = ui.trAddressEdit->text();

    if(!userId.isEmpty() && !userPw.isEmpty() && !brokerId.isEmpty() &&
        !mdAddress.isEmpty() && !tdAddress.isEmpty()){
            
            // do other check
            return true;
    }
    return false;

}
void loginDialog::setCtpControl(ctpControl*manager){

    this->m_ctpController = manager;

}

void loginDialog::logginRespond(CThostFtdcRspInfoField *pRspInfor){
    char errorID[10] ;

    sprintf(errorID,"%d",pRspInfor->ErrorID);

    string error =  "MSG:  " + string(pRspInfor->ErrorMsg) + " ID " + string(errorID);

    QString errorInfor = QString::fromLocal8Bit(error.c_str());

    this->ui.loggingBrowser->append(errorInfor);

    this->loggingTimer->stop(); // 计时器停止
    if(pRspInfor->ErrorID == 0){
        //登录成功的数据保存下来
        if(QFile::exists(CONFIGFILE)){
            QSettings *configWrite = new QSettings(CONFIGFILE,QSettings::IniFormat);
            configWrite ->setValue("/Account/brokerID",brokerId);
            configWrite ->setValue("/Account/userId",userId);
            configWrite ->setValue("/Account/passwd",userPw);
            configWrite ->setValue("/FrontAddress/MDAddress",mdAddress);
            configWrite ->setValue("/FrontAddress/TDAddress",tdAddress);
            
            delete configWrite;
        }
        accept();
    }    
    setDisabled(false);

}

void loginDialog::on_logoutBtn_clicked(){
    this->m_ctpController->releaseCtpConnect();
    close();
}

void loginDialog::timerUpDate(){
    QDateTime time = QDateTime::currentDateTime();
    QString str =QString::fromLocal8Bit("登录计时....")+ time.toString("yyyy-MM-dd hh:mm:ss dddd");

    this->ui.loggingBrowser->append(str);
    times++;

    if(times >120){
        this->loggingTimer->stop();
        this->ui.loggingBrowser->append(QString::fromLocal8Bit("登录超时（最长2min）"));
        this->m_ctpController->releaseCtpConnect();
        setDisabled(false);
    }
}