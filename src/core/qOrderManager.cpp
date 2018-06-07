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
    this->m_tradeTimeStatus    =    true;    //�����ڳ�����ʱ��Ϊ��
    //��ʼ����ʱ��
    connect(&this->m_hedgeTimer,SIGNAL(timeout()),this,SLOT(OrderOperation())    );
    //����ǰ��̨��ʾ
    connect(this,SIGNAL(dataChange(QModelIndex,QModelIndex)),mainApp,SLOT(dataChange(QModelIndex,QModelIndex)),Qt::QueuedConnection);
    connect(this,SIGNAL(browserMsg(QString)),mainApp,SLOT(onStrategyMsg(QString)),Qt::QueuedConnection);
    this->m_hedgeTimer.start(1000);//����������    ÿ��5s���һ��
    this->m_pTradeSpi    =    NULL;
    this->m_OrderManageModel    =    pModel;
    this->m_allowTrade    =    false;
    this->m_mInstHedgeNums.clear();    
}

QOrderManager::~QOrderManager()
{
    //delete    this->m_OrderManageModel;
}
//�Գ������ۼ�

//    ȱ�ݣ������������A    B    �ڳ������������Ժ󣬲���    A    B    ����������������    ���ֹͣ����Aֹͣ    B����    A�ı�����
void    QOrderManager::updateToOrderBook(QString    strategyName,QString    instId,int    hedgeNums,int    hedgeThreshold,int    maxOpenNum){
    //�յ�����    ����״̬Ϊ��
    m_tradeStatus    =    true;
            //    cashGreeks    thisCashGreeks    =    strategyCash.value<cashGreeks>();
    if(this->m_mStrategyOrder.contains(strategyName)){
        this->m_mStrategyOrder[strategyName].InstId    =    instId;
        this->m_mStrategyOrder[strategyName].OrderNums    =    hedgeNums;
        this->m_mStrategyOrder[strategyName].hedgeThreshold    =    hedgeThreshold;
        this->m_mStrategyOrder[strategyName].maxOpenNum    =    maxOpenNum;
                            //    this->m_mStrategyOrder[strategyName].strategyCashGreeks    =    thisCashGreeks;
        if(!this->m_mInstHedgeNums.contains(instId)){
            this->m_mInstHedgeNums.insert(instId,0);//û�д˺�Լ����
        }

    }else{
        this->m_mStrategyOrder.insert(strategyName,strategyOrder(instId,hedgeNums));
    }
    
}
//���ڼ��OrderBook    �������һ��������Գ�
//    ����ֹͣ�󣬷����ı�����Ȼ����¼���ڣ����������ͻ��˻��߲����ٴ�����ͬʱ����ƽ��
void    QOrderManager::OrderOperation(){



    int    longHold    =    0;//�൥�ֲ�
    int    shortHold    =    0;//�յ��ֲ�
    int    netPos    =    0;    //    ���ֲ�
    int    hedgeDifferenceNums    =    0;//�Գ��ֵ
    QString    msg    ;
    int    maxOrderNum    =    0;//���б���������ֵ
                cashGreeks    totalGreeks;

    //���Ӽ��
    if(!this->connectCheck()){
            msg    =    QString::fromLocal8Bit("������������    ����ǰ��δ����    ��������");
        emit    browserMsg(msg);
        return;
    }

    
    
    //�������йҵ����ú�Լδ��ͣ������£�
    this->m_pTradeSpi->CancelOrderList();


    //��ձ����б�
    for(auto    iter    =    this->m_mInstHedgeNums.begin();iter!=this->m_mInstHedgeNums.end();iter++){
        iter.value()    =    0;
        //������ֵ
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

    //��Լ���
    for(auto    iter    =    this->m_mStrategyOrder.begin();iter    !=        this->m_mStrategyOrder.end();iter++){
        this->m_mInstHedgeNums[iter.value().InstId    ]    +=    iter.value().OrderNums    ;    
        //maxOrderNum    +=    std::abs(iter.value().OrderNums);
        this->m_mInstTotalNum[iter.value().InstId    ]    +=    abs(iter.value().OrderNums)    ;    
        this->m_mInstThreshold[iter.value().InstId]    +=    iter.value().hedgeThreshold;//��ֵ
        maxOrderNum    =    max(iter.value().maxOpenNum,maxOrderNum);


        //    string    name    =    wCharToMchar(    iter.key().toStdWString());
        //    cout    <<    "������    ��    "    <<    name    <<    "        ��Լ    "    <<    iter.value().InstId.toStdString()    <<"    �Գ�����    "    <<    iter.value().OrderNums        <<    "    �൥�ֲ�    "    <<    longHold    <<    "�յ��ֲ�    "    <<    shortHold    <<    endl;

    }

    g_lock.lockForRead();
;

    //���α���
    for(auto    iter    =    this->m_mInstHedgeNums.begin();iter!=this->m_mInstHedgeNums.end();iter++){

        QString    instId    =    iter.key();
        int    hedgeNums    =    iter.value();//��ǰ�������󱨵�
        int    exposure    =    0;    //����Ϊ���ھ�ͷ���

        //    �����Լ����δ���    �����
        if(    this->m_allowTrade    &&    gOrderUnfinished[instId].load()){
            msg    =        QString::fromLocal8Bit("��������������Լ        ")    +    instId    +
                    QString::fromLocal8Bit("        �ҵ��ڶ�����(�ǵ�ͣ���йҵ�����)        ")    +    
                    QString::fromLocal8Bit("        ����ʱ��        ")        +    QString(instrumentMarketData[instId].UpdateTime    );
            emit    browserMsg(msg);
            continue;
        }


        longHold    =    m_pTradeSpi->SendHolding_long(iter.key().toStdString().c_str());
        shortHold    =    m_pTradeSpi->SendHolding_short(iter.key().toStdString().c_str());
        netPos    =    longHold    -    shortHold;
        hedgeDifferenceNums    =    hedgeNums    -    netPos;//Ӧ�Գ�����
        //���ﲻ�������õ���ֵ�Գ�    ����Ϊ    ��������    ���ܽ�����ͬ��Լ��ÿ��������ֵ��ͬ����Գ�ʱ���ڼ���Ϊ5����ֵΪ2��3    ���������ͬʱ���㣬�Լ�����������������
        switch    (m_hedgeType)
        {
            case    FIXTIME:
                exposure    =    max(int(this->m_mInstTotalNum[instId]    *    this->m_aversionCofficient),1);    //    ��󳨿�����Ϊ�ú�ԼӦ�ԶԳ�����        ����ֵ��֮��    ��15%    ��������Ϊ    �Գ��ͷ��Խ��    �����Ի�ֵ������    ռ��ҲԽ��
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

        //��������    ��Գ������
        if(abs(hedgeDifferenceNums)    >    exposure)    {
            //    �����������ܳ�����һ��һ������Сֵ50%
            int    minOrderNum    =    min(instrumentMarketData[instId].AskVolume1,instrumentMarketData[instId].BidVolume1)    *    0.50;
            minOrderNum    =    min(maxOrderNum,minOrderNum);//    ��������󱨵�����

            //    ͬʱ���ܳ����������涨��Լ��󱨵�����
            if(m_instMessage_map_stgy.size()    >10){
                minOrderNum    =    min(minOrderNum,    m_instMessage_map_stgy[instId.toStdString().c_str()]->MaxLimitOrderVolume);
            }
            else{
                emit    browserMsg(QString::fromLocal8Bit("��Լ��Ϣ��ȡ����ȷ����������"));
            }
            if(abs(hedgeDifferenceNums)    >    minOrderNum    ){
                //�����Բ��������£�minOrderNum�п���Ϊ0
                hedgeDifferenceNums    =    int(abs(hedgeDifferenceNums)    /    hedgeDifferenceNums)    *    max(minOrderNum,1);
            }
            //�յ���������ź�
            if(this->m_tradeStatus){
                this->sendOrder(instId,hedgeDifferenceNums,netPos,&instrumentMarketData[instId],this->m_allowTrade);
                msg    =        QString::fromLocal8Bit("������")    +    instId    +
                    QString::fromLocal8Bit("        Ӧ�Գ�����    ")    +    QString::number(hedgeNums,'f',1)+
                    QString::fromLocal8Bit("        �ֲ�        ")    +    QString::number(netPos,'f',1)    +    
                    QString::fromLocal8Bit("        ������    ")        +    QString::number(hedgeDifferenceNums,'f',1)+
                    QString::fromLocal8Bit("        ��ֵ        ")        +    QString::number(exposure,'f',1)    +
                    QString::fromLocal8Bit("        ����ʱ��        ")        +    QString(instrumentMarketData[instId].UpdateTime    );
                emit    browserMsg(msg);
                if(this->m_allowTrade){
                    //    ���øú�Լ����״̬δ���
                    gOrderUnfinished[instId].store(true);
                }

            }

        }
    }
    g_lock.unlock();
    //������һ�����к󣬵ȴ��κβ������»���
    m_tradeStatus    =    false;
    //��������б�
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

//    ����ģ��    ����Ӧ�÷ֳ�    Strategy���    Hedge�ࣿ    hedgeDifference    ΪҪ����������
void    QOrderManager::sendOrder(QString    instrumentCode,int    hedgeDifference,int    position,CThostFtdcDepthMarketDataField    *pDepthMarketData,bool    allowTrade){
    
    if(allowTrade){
        TThostFtdcInstrumentIDType                instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
        TThostFtdcDirectionType                            dir;//����,'0'��'1'��
        TThostFtdcCombOffsetFlagType        kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
        TThostFtdcPriceType                                            price;//�۸�0���м�,��������֧��
        TThostFtdcVolumeType                                        vol;//����

        double    miniChangeTick    =    m_instMessage_map_stgy[pDepthMarketData->InstrumentID]->PriceTick    *    3;    //    ������    ��С�䶯�۸�    ��֤�ɽ�
        double    BuyPrice,    SellPrice;//    ������    �����
        BuyPrice    =    pDepthMarketData->AskPrice1    +    miniChangeTick;    //�������1���۸��ϵ�һ��miniTick    ���߼����룩
        SellPrice    =    pDepthMarketData->BidPrice1    -    miniChangeTick;//��������1���۸��µ�һ��miniTick    ���ͼ�������
        //    ����۲��ܴ��ڵ�����ͣ�۸�
        if(BuyPrice    >    pDepthMarketData->UpperLimitPrice){
            BuyPrice    =    pDepthMarketData->UpperLimitPrice;
        }
        //    �����۲���С�㵱�յ���۸�
        if(SellPrice    <    pDepthMarketData->LowerLimitPrice){
            SellPrice    =    pDepthMarketData->LowerLimitPrice;
        }
        //ofstream    logging("output/"    +    s_instId    +    "_"    +    date    +    "_logging"    +    ".txt",ios::app);
        strcpy_s(instId,    instrumentCode.toStdString().c_str());
        //    ģ����    ʹ��1���۸�    ����������    ��ƽ��ƽ
        if(hedgeDifference>0){
            //    ���뿪��
            if(position    >=    0){
            
                    dir    =    '0';
                    strcpy_s(kpp,    "0");
                    price    =    BuyPrice;
                    vol    =    hedgeDifference;
                    this->m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            //��ǰ�ֲ�
            else{
                //    >0    ��������ƽ��    �����뿪��
                if((hedgeDifference    +    position)    >    0){

                    //��ƽ    position
                    dir    =    '0';
                    strcpy_s(kpp,    "3");
                    price    =    BuyPrice;
                    vol    =    -position;
                    if(vol    >0    ){
                        m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                    }
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                    //��    hedgeDifference    +    position
                    strcpy_s(kpp,    "0");
                    price    =BuyPrice;
                    vol    =    hedgeDifference    +    position;
                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                }
                else{
                    //��ƽ    hedgeDifference
            
                    dir    =    '0';
                    strcpy_s(kpp,    "3");
                    price    =    BuyPrice;
                    vol    =    hedgeDifference;

                    m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                }
            }
        }
        ///    �Գ��ֵ    Ϊ    ��    Ҫô����ƽ��    Ҫô    ��������
        else    if(hedgeDifference    <    0){
            //    ��������
            if(position    <=    0){
                    dir    =    '1';
                    strcpy_s(kpp,    "0");
                    price    =    SellPrice;
                    vol    =    -hedgeDifference;

                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            //��ǰ�ֲ�
            else{
                //    <0    ��������ƽ��    ����������
                if((hedgeDifference    +    position)    <    0){
                    //    ����ƽ
                    dir    =    '1';
                    //strcpy_s(kpp,    "3");
                    price    =    SellPrice;
                    vol    =    position;

                    m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                    //������
            
                    dir    =    '1';
                    strcpy_s(kpp,    "0");
                    price    =    SellPrice;
                    vol    =    -hedgeDifference    -    position;

                    m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
                //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                }
                else{
                    //��ƽ    hedgeDifference
                    //    ����ƽ
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
//oderPrice=0���Զ�����+3��tickֵ����
bool    QOrderManager::insertOrder(QString    orderInstId,DIRECTION    orderDir,    OFFSETFLAG    flag,    int    num,    double    orderPrice){

    TThostFtdcInstrumentIDType                instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
    TThostFtdcDirectionType                            dir;//����,'0'��'1'��
    TThostFtdcCombOffsetFlagType        kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    TThostFtdcPriceType                                            price;//�۸�0���м�,��������֧��
    TThostFtdcVolumeType                                        vol;//����

    strcpy_s(instId,orderInstId.toStdString().c_str());
    double    miniChangeTick    =    m_instMessage_map_stgy[instId]->PriceTick    *    3;    //    ������    ��С�䶯�۸�    ��֤�ɽ�
    double    BuyPrice,    SellPrice;//    ������    �����
    
    
    if(orderPrice    ==    0){
        BuyPrice    =    instrumentMarketData[orderInstId].UpperLimitPrice    ;    //�������1���۸��ϵ�һ��miniTick    ���߼����룩
        SellPrice    =    instrumentMarketData[orderInstId].LowerLimitPrice    ;//��������1���۸��µ�һ��miniTick    ���ͼ�������
    }
    else
    {
        BuyPrice    =    orderPrice;
        SellPrice    =    orderPrice;
    }

    int    longHold,shortHold;
    
    //        ��ѯ��Լ�ֲ�
    longHold    =    m_pTradeSpi->SendHolding_long(instId);
    shortHold    =    m_pTradeSpi->SendHolding_short(instId);

    //����
    if(flag    ==    OFFSETFLAG::open){
        //���뿪��
        if(    orderDir    ==    DIRECTION::buy){
            dir    =    '0';
            strcpy_s(kpp,    "0");
            price    =    BuyPrice;
            vol    =    num;
            this->m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
        }
        //    ��������
        if(    orderDir    ==    DIRECTION::sell){
            dir    =    '1';
            strcpy_s(kpp,    "0");
            price    =    SellPrice;
            vol    =    num;
            m_pTradeSpi->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            
        }
    }
    if(flag    ==    OFFSETFLAG::close){
    
        //����ƽ��
        if(orderDir==DIRECTION::buy){
            dir    =    '0';
            strcpy_s(kpp,    "3");
            price    =    BuyPrice;
            vol    =    num;
            m_pTradeSpi->StraitClose(instId,    dir,    price,    vol);
        }
        //����ƽ��
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
    
    //this->m_OrderManageModel    =    new    QStandardItemModel(0,    6);    //ָ����    ��ָ�벻��Ҫ    �Լ�ά��ָ��ɾ��

    //m_OrderManageModel->setHeaderData(0,    Qt::Horizontal,    QString::fromLocal8Bit("���׺�Լ"));
    //m_OrderManageModel->setHeaderData(1,    Qt::Horizontal,    QString::fromLocal8Bit("����Ӧ�Գ�����"));
    //m_OrderManageModel->setHeaderData(2,    Qt::Horizontal,    QString::fromLocal8Bit("ʵ�ʳֲ�"));
    //m_OrderManageModel->setHeaderData(3,    Qt::Horizontal,    QString::fromLocal8Bit("��ֵ"));
    //m_OrderManageModel->setHeaderData(4,    Qt::Horizontal,    QString::fromLocal8Bit("��ǰ����"));
    //m_OrderManageModel->setHeaderData(5,    Qt::Horizontal,    QString::fromLocal8Bit("������ʱ��"));


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
        m_pStrategyGreeksModel->setData(m_pStrategyGreeksModel->index(row,    6),    0);//���׺�Լ
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
            //�ڳ�
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
                cerr<<    "�ȴ������ر���"    <<endl;
                LOG(INFO)    <<    "has    order    unfinished,    cannot    inert    new    order.";
                this->m_tryInsertOrderTimes++;
                if(this->m_tryInsertOrderTimes    >    50)    //    ʧ�ܹ���    ����
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
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("        ����ǰ��δ����    ");
        emit    strategyMsg(msg);
        return;
    }

    //===================���Կ���ʱ����==================================
    time_t    curTimeTic,everyDayStratTime;
    
    optionGreeks    CallOptionGreeks,PutOptionGreeks;
    optionHedgeHands    CurHedgeHands;
    optionParam    _optionParam;
    int    maxOpenNum    =    this->openNums.toInt();
    //�Գ���ֵ
    int    hedge_level    =    this->hedgeLevel.toInt();

    CurHedgeHands.LongPos        =        TDSpi_stgy->SendHolding_long(this->instrumentCode.toStdString().c_str());
    CurHedgeHands.ShortPos    =    TDSpi_stgy->SendHolding_short(this->instrumentCode.toStdString().c_str());

    if(CurHedgeHands.LongPos    ==    -1    &&    CurHedgeHands.ShortPos    ==    -1){
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("        ����ǰ��δ����    ");
        emit    strategyMsg(msg);
        return;
    }


    //    ��ʵ�ʹ���ʱ��Ϊ׼
    this->GetTimeTic(pDepthMarketData->ActionDay,pDepthMarketData->UpdateTime,curTimeTic,everyDayStratTime);
    //    ÿ������ʱ��
    if(    curTimeTic    >        everyDayStratTime    &&    curTimeTic    <        everyDayStratTime    +    15){
        //m_allow_open    =    true;
        hedge_level    =    1;
    }
    //carry������ʱ��
    if(curTimeTic    >    this->m_noonTime){
        curTimeTic+=this->carryNoonDistance*60;
    }
    //carry����������
    if(curTimeTic    >    this->m_nightTime){
        curTimeTic+=this->carryNightDistance*60;
    }

    this->m_oOptionCalculateEngine.optionCalculate(pDepthMarketData,curTimeTic,CallOptionGreeks,PutOptionGreeks,CurHedgeHands,_optionParam);


    //    ��ȡ��ʵ�ֲ���
    CurHedgeHands.netPos    =    CurHedgeHands.LongPos    -    CurHedgeHands.ShortPos;
    CurHedgeHands.RealHedgeHands    =        (int)(CurHedgeHands.hedgeHands    +    0.5);//��������
    CurHedgeHands.hedgeDifference    =    CurHedgeHands.RealHedgeHands    -    CurHedgeHands.netPos;

    //PrintLogging(pDepthMarketData,    CallOptionGreeks,    PutOptionGreeks,    CurHedgeHands,_optionParam);
    QString    Pdelta    =    QString::number(PutOptionGreeks.delta,'f',    4);
    QString    CDelta    =    QString::number(CallOptionGreeks.delta,'f',    4);
    QString    RealHedgeHands    =    QString::number(CurHedgeHands.RealHedgeHands,'f',    4);
    QString    hedgeDifference    =    QString::number(CurHedgeHands.hedgeDifference,'f',    4);

    PrintLogging(    pDepthMarketData,    CallOptionGreeks,    PutOptionGreeks,    CurHedgeHands,    _optionParam);
    setModelData(CallOptionGreeks,PutOptionGreeks,CurHedgeHands);
    if(this->m_testTimes    !=    0){
        QString    msg    =    this->productName    +    QString::fromLocal8Bit("    ����    ")    +    QString::number(this->m_testTimes,'f',    1)    +    QString::fromLocal8Bit    ("    �����ʽ��ʼ���ס�");
        emit    strategyMsg(msg);
        this->m_testTimes--;
        return;
    }



    if(abs(CurHedgeHands.hedgeDifference)    <    hedge_level)    return;
    //    ���α����������ܳ�����һ��һ����70%
        int    minOrderNum    =    min(pDepthMarketData->AskVolume1,pDepthMarketData->BidVolume1)    *    0.70;
        if(maxOpenNum    >    minOrderNum){
            maxOpenNum    =minOrderNum;
        }
    //    ��󱨵�����
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
    //    ------------------------------------------------����ģ��-----------------------------------------
    //    tickʱ���������
    if(!strcmp(this->m_oldTickUpdate.toStdString().c_str(),instrumentMarketData[this->instrumentCode].UpdateTime)    ==    0){
        //����ʱ��
        this->m_oldTickUpdate    =        QString(instrumentMarketData[this->instrumentCode].UpdateTime);//
        //���ö���״̬δ���
        gOrderUnfinished[this->instrumentCode].store(true);

        this->StrategyOperation(CurHedgeHands.hedgeDifference,CurHedgeHands.netPos,pDepthMarketData);
        
        
        
        
    }


    

}


//    ===========================����ʱ�����==================================
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

    curTimeTic    =    dateTime2unix(iYear,iMonth,iDay,hour,minute,sec);///    ��ǰʱ��ת����ʱ���

    everyDayStratTime    =    dateTime2unix(iYear,iMonth,iDay,this->strategyStartTime.hour(),this->strategyStartTime.minute(),this->strategyStartTime.second());///    ÿ�տ���ʱ���
    this->m_noonTime    =    dateTime2unix(iYear,iMonth,iDay,this->carryNoonTime.hour(),this->carryNoonTime.minute(),this->carryNoonTime.second());///    noon    Time
    this->m_nightTime    =    dateTime2unix(iYear,iMonth,iDay,this->carryNightTime.hour(),this->carryNightTime.minute(),this->carryNightTime.second());///    Night    Time
    

}



//    ����ģ��    ����Ӧ�÷ֳ�    Strategy���    Hedge�ࣿ    hedgeDifference    ΪҪ����������
void    QStrategyThread::StrategyOperation(int    hedgeDifference,int    position,CThostFtdcDepthMarketDataField    *pDepthMarketData){
        
    TThostFtdcInstrumentIDType                instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
    TThostFtdcDirectionType                            dir;//����,'0'��'1'��
    TThostFtdcCombOffsetFlagType        kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    TThostFtdcPriceType                                            price;//�۸�0���м�,��������֧��
    TThostFtdcVolumeType                                        vol;//����
    string    s_instId    =    pDepthMarketData->InstrumentID;
    string    date    =    pDepthMarketData->TradingDay;
//    double    miniChangeTick    =    m_instMessage_map_stgy[pDepthMarketData->InstrumentID]->PriceTick    *    2;    //    ��������С�䶯�۸�    ��֤�ɽ�

    //ofstream    logging("output/"    +    s_instId    +    "_"    +    date    +    "_logging"    +    ".txt",ios::app);
    strcpy_s(instId,    this->instrumentCode.toStdString().c_str());
    //    ģ����    ʹ��1���۸�    ����������    ��ƽ��ƽ
    if(hedgeDifference>0){
        //    ���뿪��
        if(position    >=    0){
            
                dir    =    '0';
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->AskPrice1;//��1������������
                vol    =    hedgeDifference;
                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

        }
        //��ǰ�ֲ�
        else{
            //    >0    ��������ƽ��    �����뿪��
            if((hedgeDifference    +    position)    >    0){

                //��ƽ    position
                dir    =    '0';
                strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    -position;
                if(vol    >0    ){
                    TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
                }
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
                //��    hedgeDifference    +    position
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    hedgeDifference    +    position;
                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
            }
            else{
                //��ƽ    hedgeDifference
            
                dir    =    '0';
                strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->AskPrice1;
                vol    =    hedgeDifference;

                TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;
            }
        }
    }
    ///    �Գ��ֵ    Ϊ    ��    Ҫô����ƽ��    Ҫô    ��������
    else    if(hedgeDifference    <    0){
        //    ��������
        if(position    <=    0){
                dir    =    '1';
                strcpy_s(kpp,    "0");
                price    =    pDepthMarketData->BidPrice1;
                vol    =    -hedgeDifference;

                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

        }
        //��ǰ�ֲ�
        else{
            //    <0    ��������ƽ��    ����������
            if((hedgeDifference    +    position)    <    0){
                //    ����ƽ
                dir    =    '1';
                //strcpy_s(kpp,    "3");
                price    =    pDepthMarketData->BidPrice1    ;
                vol    =    position;

                TDSpi_stgy->StraitClose(instId,    dir,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

                //������
            
                dir    =    '1';
                strcpy_s(kpp,    "0");
                price    =pDepthMarketData->BidPrice1;
                vol    =    -hedgeDifference    -    position;

                TDSpi_stgy->ReqOrderInsert(instId,    dir,    kpp,    price,    vol);
            //    logging    <<    instId    <<    "        dir:"    <<    dir    <<    "    kp:"    <<    kpp    <<    "    p:    "    <<    price    <<    "        v:"    <<    vol    <<    endl;

            }
            else{
                //��ƽ    hedgeDifference
                //    ����ƽ
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