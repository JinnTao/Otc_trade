#include    <qsettings.h>
#include    <qreadwritelock.h>
#include    <qsemaphore.h>
#include    <atomic>
#include    <qmutex.h>
#include    "QOrderManager.h"
#include    <wchar.h>
#include    <Windows.h>

extern    QReadWriteLock    g_lock;
extern    QMap<QString,CThostFtdcDepthMarketDataField    >    instrumentMarketData;
extern    QMap<QString,QAtomicInt>    gOrderUnfinished;
extern    QMutex    g_mutex;


QOrderManager::QOrderManager(QObject    *parent,QWidget    *mainApp,QStandardItemModel    *pModel)
    :    QObject(parent)
{
    this->m_tradeStatus    =    false;
    this->m_tradeTimeStatus    =    true;    //假设期初交易时间为真
    //初始化计时器
    connect(&this->m_hedgeTimer,SIGNAL(timeout()),this,SLOT(OrderOperation())    );
    //连接前后台显示
    connect(this,SIGNAL(dataChange(QModelIndex,QModelIndex)),mainApp,SLOT(dataChange(QModelIndex,QModelIndex)),Qt::QueuedConnection);
    connect(this,SIGNAL(browserMsg(QString)),mainApp,SLOT(onStrategyMsg(QString)),Qt::QueuedConnection);
    this->m_hedgeTimer.start(1000);//订单管理器    每隔5s检查一次
    this->m_pTradeSpi    =    NULL;
    this->m_OrderManageModel    =    pModel;
    this->m_allowTrade    =    false;
    this->m_mInstHedgeNums.clear();    
}

QOrderManager::~QOrderManager()
{
    //delete    this->m_OrderManageModel;
}
//对冲数量累加

//    缺陷：如果两个策略A    B    期初运行两个策略后，策略    A    B    报单将保存下来，    如果停止策略A停止    B运行    A的报单将
void    QOrderManager::updateToOrderBook(QString    strategyName,QString    instId,int    hedgeNums,int    hedgeThreshold,int    maxOpenNum){
    //收到策略    交易状态为真
    m_tradeStatus    =    true;
            //    cashGreeks    thisCashGreeks    =    strategyCash.value<cashGreeks>();
    if(this->m_mStrategyOrder.contains(strategyName)){
        this->m_mStrategyOrder[strategyName].InstId    =    instId;
        this->m_mStrategyOrder[strategyName].OrderNums    =    hedgeNums;
        this->m_mStrategyOrder[strategyName].hedgeThreshold    =    hedgeThreshold;
        this->m_mStrategyOrder[strategyName].maxOpenNum    =    maxOpenNum;
                            //    this->m_mStrategyOrder[strategyName].strategyCashGreeks    =    thisCashGreeks;
        if(!this->m_mInstHedgeNums.contains(instId)){
            this->m_mInstHedgeNums.insert(instId,0);//没有此合约加入
        }

    }else{
        this->m_mStrategyOrder.insert(strategyName,strategyOrder(instId,hedgeNums));
    }
    
}
//定期检查OrderBook    如果超过一定数量则对冲
//    策略停止后，发出的报单任然将记录在内，除非重启客户端或者策略再次启动同时发出平仓
void    QOrderManager::OrderOperation(){



    int    longHold    =    0;//多单持仓
    int    shortHold    =    0;//空单持仓
    int    netPos    =    0;    //    净持仓
    int    hedgeDifferenceNums    =    0;//对冲差值
    QString    msg    ;
    int    maxOrderNum    =    0;//所有报单数绝对值
                cashGreeks    totalGreeks;

    //连接检测
    if(!this->connectCheck()){
            msg    =    QString::fromLocal8Bit("订单管理器：    交易前置未连接    正在重连");
        emit    browserMsg(msg);
        return;
    }

    
    
    //撤除所有挂单（该合约未涨停的情况下）
    this->m_pTradeSpi->CancelOrderList();


    //清空报单列表
    for(auto    iter    =    this->m_mInstHedgeNums.begin();iter!=this->m_mInstHedgeNums.end();iter++){
        iter.value()    =    0;
        //重置阈值
        if(!this->m_mInstTotalNum.contains(iter.key())){
            this->m_mInstTotalNum.insert(iter.key(),0);
        }else{
            this->m_mInstTotalNum[iter.key()]    =    0;
        }
        //
        QAtomicInt    orderUnfinished(false);
        if(!gOrderUnfinished.contains(iter.key())){
            gOrderUnfinished.insert(iter.key(),orderUnfinished);
        }
    }

    //合约撮合
    for(auto    iter    =    this->m_mStrategyOrder.begin();iter    !=        this->m_mStrategyOrder.end();iter++){
        this->m_mInstHedgeNums[iter.value().InstId    ]    +=    iter.value().OrderNums    ;    
        //maxOrderNum    +=    std::abs(iter.value().OrderNums);
        this->m_mInstTotalNum[iter.value().InstId    ]    +=    abs(iter.value().OrderNums)    ;    
        this->m_mInstThreshold[iter.value().InstId]    +=    iter.value().hedgeThreshold;//阈值
        maxOrderNum    =    max(iter.value().maxOpenNum,maxOrderNum);


        //    string    name    =    wCharToMchar(    iter.key().toStdWString());
        //    cout    <<    "策略名    ：    "    <<    name    <<    "        合约    "    <<    iter.value().InstId.toStdString()    <<"    对冲数量    "    <<    iter.value().OrderNums        <<    "    多单持仓    "    <<    longHold    <<    "空单持仓    "    <<    shortHold    <<    endl;

    }

    g_lock.lockForRead();
;

    //依次报单
    for(auto    iter    =    this->m_mInstHedgeNums.begin();iter!=this->m_mInstHedgeNums.end();iter++){

        QString    instId    =    iter.key();
        int    hedgeNums    =    iter.value();//当前策略请求报单
        int    exposure    =    0;    //敞口为当期净头寸的

        //    如果合约订单未完成    则忽略
        if(    this->m_allowTrade    &&    gOrderUnfinished[instId].load()){
            msg    =        QString::fromLocal8Bit("订单管理器：合约        ")    +    instId    +
                    QString::fromLocal8Bit("        挂单在队列中(涨跌停版中挂单不撤)        ")    +    
                    QString::fromLocal8Bit("        更新时间        ")        +    QString(instrumentMarketData[instId].UpdateTime    );
            emit    browserMsg(msg);
            continue;
        }


        longHold    =    m_pTradeSpi->SendHolding_long(iter.key().toStdString().c_str());
        shortHold    =    m_pTradeSpi->SendHolding_short(iter.key().toStdString().c_str());
        netPos    =    longHold    -    shortHold;
        hedgeDifferenceNums    =    hedgeNums    -    netPos;//应对冲手数
        //这里不按照设置的阈值对冲    是因为    两个策略    可能交易相同合约，每个策略阈值不同，则对冲时敞口假设为5，阈值为2，3    则出现两个同时满足，以及两个都不满足的情况
        switch    (m_hedgeType)
        {
            case    FIXTIME:
                exposure    =    max(int(this->m_mInstTotalNum[instId]    *    this->m_aversionCofficient),1);    //    最大敞口设置为该合约应对对冲手数        绝对值的之和    的15%    基础考虑为    对冲的头寸越大    敞口以货值计算则    占比也越大
                break;
            case    WhalleyWilmott:
                exposure    =    this->m_mInstThreshold[instId];
                break;
            case    Zakamouline:

                break;
            default:
                break;
        }
        
    
        setModelData(instId,hedgeNums,netPos,exposure);

        //超过敞口    则对冲其风险
        if(abs(hedgeDifferenceNums)    >    exposure)    {
            //    报单数量不能超过买一卖一量的最小值50%
            int    minOrderNum    =    min(instrumentMarketData[instId].AskVolume1,instrumentMarketData[instId].BidVolume1)    *    0.50;
            minOrderNum    =    min(maxOrderNum,minOrderNum);//    不超过最大报单数量

            //    同时不能超过交易所规定合约最大报单数量
            if(m_instMessage_map_stgy.size()    >10){
                minOrderNum    =    min(minOrderNum,    m_instMessage_map_stgy[instId.toStdString().c_str()]->MaxLimitOrderVolume);
            }
            else{
                emit    browserMsg(QString::fromLocal8Bit("合约信息获取不正确，谨慎运行"));
            }
            if(abs(hedgeDifferenceNums)    >    minOrderNum    ){
                //流动性不足的情况下，minOrderNum有可能为0
                hedgeDifferenceNums    =    int(abs(hedgeDifferenceNums)    /    hedgeDifferenceNums)    *    max(minOrderNum,1);
            }
            //收到任意策略信号
            if(this->m_tradeStatus){
                this->sendOrder(instId,hedgeDifferenceNums,netPos,&instrumentMarketData[instId],this->m_allowTrade);
                msg    =        QString::fromLocal8Bit("订单：")    +    instId    +
                    QString::fromLocal8Bit("        应对冲数量    ")    +    QString::number(hedgeNums,'f',1)+
                    QString::fromLocal8Bit("        持仓        ")    +    QString::number(netPos,'f',1)    +    
                    QString::fromLocal8Bit("        报单数    ")        +    QString::number(hedgeDifferenceNums,'f',1)+
                    QString::fromLocal8Bit("        阀值        ")        +    QString::number(exposure,'f',1)    +
                    QString::fromLocal8Bit("        更新时间        ")        +    QString(instrumentMarketData[instId].UpdateTime    );
                emit    browserMsg(msg);
                if(this->m_allowTrade){
                    //    设置该合约订单状态未完成
                    gOrderUnfinished[instId].store(true);
                }

            }

        }
    }
    g_lock.unlock();
    //处理完一个队列后，等待任何策略重新唤醒
    m_tradeStatus    =    false;
    //清空所有列表
    //m_mInstHedgeNums.clear();
}

void    QOrderManager::SetTradeSpi(CtpTraderSpi    *pTradeSpi){
    this->m_pTradeSpi    =    pTradeSpi;
    this->m_instMessage_map_stgy    =    pTradeSpi->getInstMessageMap();
}

bool    QOrderManager::connectCheck(){
    bool    status    =    true;
    if(this->m_pTradeSpi    ==    NULL    ||    !(this->m_pTradeSpi->getConnectStatus())    ||    (this->m_pTradeSpi->getfirst_inquiry_order())    ||    this->m_pTradeSpi->m_instMessage_map.size()    ==    0){
        status    =    false;
    }else{
        int    LongPos        =        this->m_pTradeSpi->SendHolding_long("");
        int    ShortPos    =    this->m_pTradeSpi->SendHolding_short("");
        if(LongPos    ==    -1    &&    ShortPos    ==    -1){
                status    =    false;
        }
    }
    return    status;
}
void    QOrderManager::setAllowTrade(bool    allowTrade){
    this->m_allowTrade    =    allowTrade;
}

//    操作模块    或许应该分成    Strategy类和    Hedge类？    hedgeDifference    为要调整的手数
void    QOrderManager::sendOrder(QString    instrumentCode,int    hedgeDifference,int    position,CThostFtdcDepthMarketDataField    *pDepthMarketData,bool    allowTrade){
    
    if(allowTrade){
        TThostFtdcInstrumentIDType                instId;//合约,合约代码在结构体里已经有了
        TThostFtdcDirectionType                            dir;//方向,'0'买，'1'卖
        TThostFtdcCombOffsetFlagType        kpp;//开平，"0"开，"1"平,"3"平今
        TThostFtdcPriceType                                            price;//价格，0是市价,上期所不支持
        TThostFtdcVolumeType                                        vol;//数量

        double    miniChangeTick    =    m_instMessage_map_stgy[pDepthMarketData->InstrumentID]->PriceTick    *    3;    //    对手盘    最小变动价格    保证成交
        double    BuyPrice,    SellPrice;//    卖出价    买入价
        BuyPrice    =    pDepthMarketData->AskPrice1    +    miniChangeTick;    //买入价在1档价格上调一个miniTick    （高价买入）
        SellPrice    =    pDepthMarketData->BidPrice1    -    miniChangeTick;//卖出价在1档价格下调一个miniTick    （低价卖出）
        //    买入价不能大于当日涨停价格
        if(BuyPrice    >    pDepthMarketData->UpperLimitPrice){
            BuyPrice    =    pDepthMarketData->UpperLimitPrice;
        }
        //    卖出价不能小鱼当日跌体价格
        if(SellPrice    <    pDepthMarketData->LowerLimitPrice){
            SellPrice    =    pDepthMarketData->LowerLimitPrice;
        }
        //ofstream    logging("output/"    +    s_instId    +    "_"    +    date    +    "_logging"    +    ".txt",ios::app);
        strcpy_s(instId,    instrumentCode.toStdString().c_str());
        //    模拟盘    使用1档价格    进行买开卖开    买平卖平
        if(hedgeDifference>0){
            //    买入开仓
            if(position    >=    0){
            
                    dir    =    '0';
                    strcpy_s(kpp,    "0");
                    price    =    BuyPrice;
                    vol    =    hedgeDifference;
                    this->m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            //当前持仓
            else{
                //    >0    则先买如平仓    再买入开仓
                if((hedgeDifference    +    position)    >    0){

                    //买平    position
                    dir    =    '0';
                    strcpy_s(kpp,    "3");
                    price    =    BuyPrice;
                    vol    =    -position;
                    if(vol    >0    ){
                        m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                    }
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                    //买开    hedgeDifference    +    position
                    strcpy_s(kpp,    "0");
                    price    =BuyPrice;
                    vol    =    hedgeDifference    +    position;
                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                }
                else{
                    //买平    hedgeDifference
            
                    dir    =    '0';
                    strcpy_s(kpp,    "3");
                    price    =    BuyPrice;
                    vol    =    hedgeDifference;

                    m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                }
            }
        }
        ///    对冲插值    为    负    要么卖出平仓    要么    卖出开仓
        else    if(hedgeDifference    <    0){
            //    卖出开仓
            if(position    <=    0){
                    dir    =    '1';
                    strcpy_s(kpp,    "0");
                    price    =    SellPrice;
                    vol    =    -hedgeDifference;

                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            //当前持仓
            else{
                //    <0    则先卖出平仓    再卖出开仓
                if((hedgeDifference    +    position)    <    0){
                    //    卖出平
                    dir    =    '1';
                    //strcpy_s(kpp,    "3");
                    price    =    SellPrice;
                    vol    =    position;

                    m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                    //卖出开
            
                    dir    =    '1';
                    strcpy_s(kpp,    "0");
                    price    =    SellPrice;
                    vol    =    -hedgeDifference    -    position;

                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                }
                else{
                    //卖平    hedgeDifference
                    //    卖出平
                    dir    =    '1';
                    strcpy_s(kpp,    "3");
                    price    =    SellPrice    ;
                    vol    =    -hedgeDifference;

                    m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                }
            }
        }
    }
    else{
        emit    browserMsg(QString("OrderManager:    Not    allow    Trade"));
    }

}
//oderPrice=0，以对手盘+3倍tick值开仓
bool    QOrderManager::insertOrder(QString    orderInstId,DIRECTION    orderDir,    OFFSETFLAG    flag,    int    num,    double    orderPrice){

    TThostFtdcInstrumentIDType                instId;//合约,合约代码在结构体里已经有了
    TThostFtdcDirectionType                            dir;//方向,'0'买，'1'卖
    TThostFtdcCombOffsetFlagType        kpp;//开平，"0"开，"1"平,"3"平今
    TThostFtdcPriceType                                            price;//价格，0是市价,上期所不支持
    TThostFtdcVolumeType                                        vol;//数量

    strcpy_s(instId,orderInstId.toStdString().c_str());
    double    miniChangeTick    =    m_instMessage_map_stgy[instId]->PriceTick    *    3;    //    对手盘    最小变动价格    保证成交
    double    BuyPrice,    SellPrice;//    卖出价    买入价
    
    
    if(orderPrice    ==    0){
        BuyPrice    =    instrumentMarketData[orderInstId].UpperLimitPrice    ;    //买入价在1档价格上调一个miniTick    （高价买入）
        SellPrice    =    instrumentMarketData[orderInstId].LowerLimitPrice    ;//卖出价在1档价格下调一个miniTick    （低价卖出）
    }
    else
    {
        BuyPrice    =    orderPrice;
        SellPrice    =    orderPrice;
    }

    int    longHold,shortHold;
    
    //        查询合约持仓
    longHold    =    m_pTradeSpi->SendHolding_long(instId);
    shortHold    =    m_pTradeSpi->SendHolding_short(instId);

    //开仓
    if(flag    ==    OFFSETFLAG::open){
        //买入开仓
        if(    orderDir    ==    DIRECTION::buy){
            dir    =    '0';
            strcpy_s(kpp,    "0");
            price    =    BuyPrice;
            vol    =    num;
            this->m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
        }
        //    卖出开仓
        if(    orderDir    ==    DIRECTION::sell){
            dir    =    '1';
            strcpy_s(kpp,    "0");
            price    =    SellPrice;
            vol    =    num;
            m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            
        }
    }
    if(flag    ==    OFFSETFLAG::close){
    
        //买入平仓
        if(orderDir==DIRECTION::buy){
            dir    =    '0';
            strcpy_s(kpp,    "3");
            price    =    BuyPrice;
            vol    =    num;
            m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
        }
        //卖出平仓
        if(orderDir==DIRECTION::sell){
            dir    =    '1';
            strcpy_s(kpp,    "3");
            price    =    SellPrice    ;
            vol    =    num;
            m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
        }
    }

    return    true;

}


QStandardItemModel    *    QOrderManager::createOrderManageModel(){
    
    //this->m_OrderManageModel    =    new    QStandardItemModel(0,    6);    //指定了    父指针不需要    自己维护指针删除

    //m_OrderManageModel->setHeaderData(0,    Qt::Horizontal,    QString::fromLocal8Bit("交易合约"));
    //m_OrderManageModel->setHeaderData(1,    Qt::Horizontal,    QString::fromLocal8Bit("整体应对冲手数"));
    //m_OrderManageModel->setHeaderData(2,    Qt::Horizontal,    QString::fromLocal8Bit("实际持仓"));
    //m_OrderManageModel->setHeaderData(3,    Qt::Horizontal,    QString::fromLocal8Bit("阈值"));
    //m_OrderManageModel->setHeaderData(4,    Qt::Horizontal,    QString::fromLocal8Bit("当前敞口"));
    //m_OrderManageModel->setHeaderData(5,    Qt::Horizontal,    QString::fromLocal8Bit("最后更新时间"));


    //return    m_OrderManageModel;
    return    NULL;
}




void    QOrderManager::setModelData(QString    instId,int        hedgeNums,int    netPos,int    hedgeLevel)
{
    QString    InstrumentName    =instId;

    int    exposure    =    hedgeNums    -    netPos;

                QString    sHedgeDifferenceNums    =    QString::number(hedgeNums,'f',    1);
                QString    sNetPos    =    QString::number(netPos,'f',    1);
                QString    sHedgeLevel    =    QString::number(hedgeLevel,'f',    1);
                QString    sExposure    =    QString::number(exposure,'f',    1);

    QList<QStandardItem    *>    tList    =    this->m_OrderManageModel->findItems(instId);
                if    (tList.count()>0)
                {
                                int    row    =    tList.at(0)->row();
        
                                //m_OrderManageModel->beginResetModel();
                                m_OrderManageModel->setData(m_OrderManageModel->index(row,    1),    sHedgeDifferenceNums);
        m_OrderManageModel->setData(m_OrderManageModel->index(row,    2),    sNetPos);
        m_OrderManageModel->setData(m_OrderManageModel->index(row,    3),    sHedgeLevel);
        m_OrderManageModel->setData(m_OrderManageModel->index(row,    4),    sExposure);
        m_OrderManageModel->setData(m_OrderManageModel->index(row,    5),    QString(instrumentMarketData[instId].UpdateTime));
                                //m_OrderManageModel->endResetModel();
        //this->m_OrderManageModel->    dataChanged(m_OrderManageModel->index(row,    1),m_OrderManageModel->index(row,    5));
        emit    dataChange(m_OrderManageModel->index(row,    1),m_OrderManageModel->index(row,    5));
                }
                else
                {
                                //m_OrderManageModel->beginInsertRows();
                                m_OrderManageModel->insertRow(0);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    0),    InstrumentName);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    1),    sHedgeDifferenceNums);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    2),    sNetPos);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    3),    sHedgeLevel);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    4),    sExposure);
        m_OrderManageModel->setData(m_OrderManageModel->index(0,    5),    QString(instrumentMarketData[instId].UpdateTime));
                                //m_OrderManageModel->endInsertRows();
                }
    //emit    update();
}

void    QOrderManager::setAversionCofficient(double    aversionCofficient){
    this->m_aversionCofficient    =    aversionCofficient;
}
void    QOrderManager::setHedgeType(int    type){
    this->m_hedgeType    =    HEDGETYPE(type);
}


void    QOrderManager::setStrategyGreeksTableData(QString    rowName,cashGreeks    totalGreeks){
    QString    InstrumentName    =    rowName;

                QString    sdelta    =    QString::number(0,'f',    3);
                QString    sdeltaCash    =    QString::number(totalGreeks._deltaCash,'f',    0);
                QString    stheta    =    QString::number(totalGreeks._thetaCash,'f',    0);
                QString    svega    =    QString::number(totalGreeks._vegaCash,'f',    0);
                QString    sgamma    =    QString::number(totalGreeks._gammaCash,'f',    0);
                QString    srho    =    QString::number(totalGreeks._rho,'f',    3);


                QList<QStandardItem    *>    tList    =    this->m_pStrategyGreeksModel->findItems(InstrumentName);
                if    (tList.count()>0)
                {
                                int    row    =    tList.at(0)->row();
        
                                //m_strategyGreekModel->beginResetModel();
                                m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    1),    sdelta);
                                m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    2),    sdeltaCash);
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    3),    sgamma);
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    4),    stheta);
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    5),    svega);
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    6),    0);//交易合约
                                m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    7),    0);
        //m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    8),    QString(instrumentMarketData[inst].UpdateTime));
                                //m_strategyGreekModel->endResetModel();
        emit    dataChange(m_pStrategyGreeksModel->index(row,    1),m_pStrategyGreeksModel->index(row,    7));
        

                                m_pStrategyGreeksModel->item(row,1)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,2)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,3)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,4)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,5)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,6)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                                m_pStrategyGreeksModel->item(row,7)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);
                        //        m_pStrategyGreeksModel->item(row,8)->setTextAlignment(Qt::Alignment::enum_type::AlignRight);


                }
                else
                {
                                //m_strategyGreekModel->beginInsertRows();
                                m_pStrategyGreeksModel->insertRow(0);
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(0,    0),    InstrumentName);
                                //m_strategyGreekModel->endInsertRows();
                }
    //emit    update();

}




/*

void    QStrategyThread::run(){
    


    QString    msg;
    bool    bFirst    =true;
    while(this->m_status){

        g_mutex.lock();

        if(bFirst){
            msg    =    this->productName    +    QString("    is    subscribe    Inst    ")    +this->instrumentCode;
            QString    productNameDir    =    QString("output/product/")    +    this->productName    +    QString(".ini")    ;
            this->m_oOptionCalculateEngine.loadConf(productNameDir);
            emit    strategyMsg(msg);
            emit    subscribe_inst_data(this->instrumentCode);
            //期初
            QAtomicInt    orderUnfinished(false);
            gOrderUnfinished.insert(this->instrumentCode,orderUnfinished);
        }

        _sleep(1000);;
        g_lock.lockForRead();
        if(instrumentMarketData.contains(this->instrumentCode)){
            QString    LastPrice    =    QString::number(instrumentMarketData[this->instrumentCode].LastPrice,'f',    2);
            msg    =    this->productName    +    QString("    get    data    ")    +LastPrice    +    QString(    "    updateTime        ")    +QString(instrumentMarketData[this->instrumentCode].UpdateTime    );
            
            emit    strategyMsg(msg);

            if(    !gOrderUnfinished[this->instrumentCode].load()){
                StrategyOptionCalcultate(&instrumentMarketData[this->instrumentCode]);
                this->m_tryInsertOrderTimes    =    0;
            }else{
                cerr<<    "等待报单回报中"    <<endl;
                LOG(INFO)    <<    "has    order    unfinished,    cannot    inert    new    order.";
                this->m_tryInsertOrderTimes++;
                if(this->m_tryInsertOrderTimes    >    50)    //    失败过多    开启
                {
                    LOG(INFO)    <<    "wating    too    long,    ignore    exited    order.";
                    gOrderUnfinished[this->instrumentCode].store(false);
                    m_tryInsertOrderTimes    =    0;
                }
            }

        }else{
            
            msg    =    this->productName    +    QString("    don't    get    data");
            emit    strategyMsg(msg);
        }


        bFirst    =    false;
        g_lock.unlock();
        g_mutex.unlock();


    }
    msg    =    this->productName    +    QString("        stopped    ");
    emit    strategyMsg(msg);

}

void    QStrategyThread::StrategyOptionCalcultate(CThostFtdcDepthMarketDataField    *pDepthMarketData){
    if(this->TDSpi_stgy    ==    NULL    ||    !(this->TDSpi_stgy->getConnectStatus())){
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("        交易前置未连接    ");
        emit    strategyMsg(msg);
        return;
    }

    //===================策略开仓时间检测==================================
    time_t    curTimeTic,everyDayStratTime;
    
    optionGreeks    CallOptionGreeks,PutOptionGreeks;
    optionHedgeHands    CurHedgeHands;
    optionParam    _optionParam;
    int    maxOpenNum    =    this->openNums.toInt();
    //对冲阈值
    int    hedge_level    =    this->hedgeLevel.toInt();

    CurHedgeHands.LongPos        =        TDSpi_stgy->SendHolding_long(this->instrumentCode.toStdString().c_str());
    CurHedgeHands.ShortPos    =    TDSpi_stgy->SendHolding_short(this->instrumentCode.toStdString().c_str());

    if(CurHedgeHands.LongPos    ==    -1    &&    CurHedgeHands.ShortPos    ==    -1){
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("        交易前置未连接    ");
        emit    strategyMsg(msg);
        return;
    }


    //    以实际工作时间为准
    this->GetTimeTic(pDepthMarketData->ActionDay,pDepthMarketData->UpdateTime,curTimeTic,everyDayStratTime);
    //    每日启动时间
    if(    curTimeTic    >        everyDayStratTime    &&    curTimeTic    <        everyDayStratTime    +    15){
        //m_allow_open    =    true;
        hedge_level    =    1;
    }
    //carry到下午时间
    if(curTimeTic    >    this->m_noonTime){
        curTimeTic+=this->carryNoonDistance*60;
    }
    //carry到次日早上
    if(curTimeTic    >    this->m_nightTime){
        curTimeTic+=this->carryNightDistance*60;
    }

    this->m_oOptionCalculateEngine.optionCalculate(pDepthMarketData,curTimeTic,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,_optionParam);


    //    获取真实持仓量
    CurHedgeHands.netPos    =    CurHedgeHands.LongPos    -    CurHedgeHands.ShortPos;
    CurHedgeHands.RealHedgeHands    =        (int)(CurHedgeHands.hedgeHands    +    0.5);//四舍五入
    CurHedgeHands.hedgeDifference    =    CurHedgeHands.RealHedgeHands    -    CurHedgeHands.netPos;

    //PrintLogging(pDepthMarketData,    CallOptionGreeks,    PutOptionGreeks,    CurHedgeHands,_optionParam);
    QString    Pdelta    =    QString::number(PutOptionGreeks.delta,'f',    4);
    QString    CDelta    =    QString::number(CallOptionGreeks.delta,'f',    4);
    QString    RealHedgeHands    =    QString::number(CurHedgeHands.RealHedgeHands,'f',    4);
    QString    hedgeDifference    =    QString::number(CurHedgeHands.hedgeDifference,'f',    4);

    PrintLogging(    pDepthMarketData,    CallOptionGreeks,    PutOptionGreeks,    CurHedgeHands,    _optionParam);
    setModelData(CallOptionGreeks,PutOptionGreeks,CurHedgeHands);
    if(this->m_testTimes    !=    0){
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("    将在    ")    +    QString::number(this->m_testTimes,'f',    1)    +    QString::fromLocal8Bit    ("    秒后正式开始交易。");
        emit    strategyMsg(msg);
        this->m_testTimes--;
        return;
    }



    if(abs(CurHedgeHands.hedgeDifference)    <    hedge_level)    return;
    //    档次报单数量不能超过买一卖一量的70%
        int    minOrderNum    =    min(pDepthMarketData->AskVolume1,pDepthMarketData->BidVolume1)    *    0.70;
        if(maxOpenNum    >    minOrderNum){
            maxOpenNum    =minOrderNum;
        }
    //    最大报单数量
    if(maxOpenNum    !=    0    &&    abs(CurHedgeHands.hedgeDifference)    >    maxOpenNum    ){
        CurHedgeHands.hedgeDifference    =    int(abs(CurHedgeHands.hedgeDifference)    /    CurHedgeHands.hedgeDifference)    *maxOpenNum;
    }
    
    //g_order_unfinished    =    true;

    //LOG(INFO)    <<    endl    <<    "    "    <<    this->instrumentCode.toStdString()    <<    "            "    <<    pDepthMarketData->UpdateTime    
    //    <<"    Long    Position    :"    <<    CurHedgeHands.LongPos    
    //    <<"    Short    Position    :"    <<            CurHedgeHands.ShortPos    
    //    <<"    netPos        :"    <<    CurHedgeHands.netPos
    //    <<"    RealHedgeHands    "    <<            CurHedgeHands.RealHedgeHands        
    //    <<"    hedgeDifference    "    <<    CurHedgeHands.hedgeDifference        
    //    <<"    price    "    <<    pDepthMarketData->LastPrice    
    //    <<"    Pdelta    "    <<PutOptionGreeks.delta        
    //    //<<    "    Cdelta    "    <<CallOptionGreeks.delta        
    //    <<"    remainT    "    <<    _optionParam.remainTime
    //    <<"    ElapsedT    "    <<    _optionParam.elapsedTime    ;
    //    ------------------------------------------------操作模块-----------------------------------------
    //    tick时间更新则交易
    if(!strcmp(this->m_oldTickUpdate.toStdString().c_str(),instrumentMarketData[this->instrumentCode].UpdateTime)    ==    0){
        //更新时间
        this->m_oldTickUpdate    =        QString(instrumentMarketData[this->instrumentCode].UpdateTime);//
        //设置订单状态未完成
        gOrderUnfinished[this->instrumentCode].store(true);

        this->StrategyOperation(CurHedgeHands.hedgeDifference,CurHedgeHands.netPos,pDepthMarketData);
        
        
        
        
    }


    

}


//    ===========================建仓时间审查==================================
void    QStrategyThread::GetTimeTic(TThostFtdcDateType    ActionDay,TThostFtdcTimeType    UpdateTime,time_t&curTimeTic,time_t&everyDayStratTime){

    bool    validTime    =    false;
    string    date    =    ActionDay;
    if(date.length()    <    7){
        curTimeTic    =    0    ;
        everyDayStratTime    =    0;
        return    ;
    }
    int    iYear    =    std::atoi(date.substr(0,4).c_str());
    int    iMonth    =    std::atoi(date.substr(4,2).c_str());
    int    iDay    =    std::atoi(date.substr(6,2).c_str());
    int    hour    =    0,minute    =    0,sec    =    0;
    sscanf_s(UpdateTime,"%2d:%2d:%2d",&hour,&minute,&sec);    //    Trading    Current    Time

    curTimeTic    =    dateTime2unix(iYear,iMonth,iDay,hour,minute,sec);///    当前时间转换成时间戳

    everyDayStratTime    =    dateTime2unix(iYear,iMonth,iDay,this->strategyStartTime.hour(),this->strategyStartTime.minute(),this->strategyStartTime.second());///    每日开仓时间戳
    this->m_noonTime    =    dateTime2unix(iYear,iMonth,iDay,this->carryNoonTime.hour(),this->carryNoonTime.minute(),this->carryNoonTime.second());///    noon    Time
    this->m_nightTime    =    dateTime2unix(iYear,iMonth,iDay,this->carryNightTime.hour(),this->carryNightTime.minute(),this->carryNightTime.second());///    Night    Time
    

}



//    操作模块    或许应该分成    Strategy类和    Hedge类？    hedgeDifference    为要调整的手数
void    QStrategyThread::StrategyOperation(int    hedgeDifference,int    position,CThostFtdcDepthMarketDataField    *pDepthMarketData){
        
    TThostFtdcInstrumentIDType                instId;//合约,合约代码在结构体里已经有了
    TThostFtdcDirectionType                            dir;//方向,'0'买，'1'卖
    TThostFtdcCombOffsetFlagType        kpp;//开平，"0"开，"1"平,"3"平今
    TThostFtdcPriceType                                            price;//价格，0是市价,上期所不支持
    TThostFtdcVolumeType                                        vol;//数量
    string    s_instId    =    pDepthMarketData->InstrumentID;
    string    date    =    pDepthMarketData->TradingDay;
//    double    miniChangeTick    =    m_instMessage_map_stgy[pDepthMarketData->InstrumentID]->PriceTick    *    2;    //    两倍的最小变动价格    保证成交

    //ofstream    logging("output/"    +    s_instId    +    "_"    +    date    +    "_logging"    +    ".txt",ios::app);
    strcpy_s(instId,    this->instrumentCode.toStdString().c_str());
    //    模拟盘    使用1档价格    进行买开卖开    买平卖平
    if(hedgeDifference>0){
        //    买入开仓
        if(position    >=    0){
            
                dir    =    '0';
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->AskPrice1;//以1档卖出价买入
                vol    =    hedgeDifference;
                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

        }
        //当前持仓
        else{
            //    >0    则先买如平仓    再买入开仓
            if((hedgeDifference    +    position)    >    0){

                //买平    position
                dir    =    '0';
                strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    -position;
                if(vol    >0    ){
                    TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
                }
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                //买开    hedgeDifference    +    position
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    hedgeDifference    +    position;
                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
            }
            else{
                //买平    hedgeDifference
            
                dir    =    '0';
                strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    hedgeDifference;

                TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
            }
        }
    }
    ///    对冲插值    为    负    要么卖出平仓    要么    卖出开仓
    else    if(hedgeDifference    <    0){
        //    卖出开仓
        if(position    <=    0){
                dir    =    '1';
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->BidPrice1;
                vol    =    -hedgeDifference;

                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

        }
        //当前持仓
        else{
            //    <0    则先卖出平仓    再卖出开仓
            if((hedgeDifference    +    position)    <    0){
                //    卖出平
                dir    =    '1';
                //strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->BidPrice1    ;
                vol    =    position;

                TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                //卖出开
            
                dir    =    '1';
                strcpy_s(kpp,    "0");
                price    =pDepthMarketData->BidPrice1;
                vol    =    -hedgeDifference    -    position;

                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            else{
                //卖平    hedgeDifference
                //    卖出平
                dir    =    '1';
                strcpy_s(kpp,    "3");
                price    =pDepthMarketData->BidPrice1;
                vol    =    -hedgeDifference;

                TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
            }
        }
    }
//    logging.close();

}





*/