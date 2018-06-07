#ifndef _STRUCTFUNCTION_H
#define _STRUCTFUNCTION_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <time.h>
#include "ThostFtdcUserApiStruct.h"
using namespace std;

#define YEAR_SECOND 31536000.0

#define YEAR_SECOND_WORKDAY 21772800.0

#define DAY_SECOND 86400.0

#define HOUR_SECOND 3600.0

//��Ȩ����
enum optionKind{European,Asian,American,Barrier,Lookback,Digital};

//��¼״̬
enum userStatus{LoggingFailed,LoggingSuccess,UserOrPwError,UserTimeOut};
//ctp״̬
enum CTPSTATUS{NORMAL,RECONNECT,DISCONNECT};

//������Ϣ�Ľṹ��
struct trade_message
{
    trade_message()
    {
        instId = "";
        lastPrice = 0.0;
        PreSettlementPrice = 0.0;
        holding_long = 0;
        holding_short = 0;
        TodayPosition_long = 0;
        YdPosition_long = 0;
        TodayPosition_short = 0;
        YdPosition_short = 0;

        closeProfit_long = 0.0;
        OpenProfit_long = 0.0;
        closeProfit_short = 0.0;
        OpenProfit_short = 0.0;
    }


    string instId;//��Լ����
    double lastPrice;//���¼ۣ�ʱ�̱����Լ�����¼ۣ�ƽ����
    double PreSettlementPrice;//�ϴν���ۣ��Ը�ҹ����ʱ��Ҫ�ã���������
    int holding_long;//�൥�ֲ���
    int holding_short;//�յ��ֲ���

    int TodayPosition_long;//�൥���ճֲ�
    int YdPosition_long;//�൥���ճֲ�

    int TodayPosition_short;//�յ����ճֲ�
    int YdPosition_short;//�յ����ճֲ�

    double closeProfit_long;//�൥ƽ��ӯ��
    double OpenProfit_long;//�൥����ӯ��

    double closeProfit_short;//�յ�ƽ��ӯ��
    double OpenProfit_short;//�յ�����ӯ��

};




struct FutureData//����ṹ�嶨��
{
    string date;
    string time;
    double buy1price;
    int buy1vol;
    double new1;
    double sell1price;
    int sell1vol;
    double vol;
    double openinterest;//�ֲ���

};



//��ʷK��
struct History_data
{
    string date;
    string time;
    double buy1price;//��һ
    double sell1price;//��һ
    double open;
    double high;
    double low;
    double close;

};

//��Ȩ����ֵ
struct  optionGreeks
{
    double price;
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;
    optionGreeks(){
        memset(this, 0, sizeof(optionGreeks));  
    };
    void reset(){
        memset(this, 0, sizeof(optionGreeks));  
    }
};

struct optionHedgeHands
{
    int LongPos; 
    int ShortPos;
    int netPos;//��ͷ��
    double callhedgeHands;
    double puthedgeHands;
    double hedgeHands;
    int RealHedgeHands;//ȡ֤��ͷ��
    int hedgeDifference;
    time_t DateTime;
    optionHedgeHands(){
        memset(this, 0, sizeof(optionHedgeHands));  
    };
    void reset(){
        memset(this, 0, sizeof(optionHedgeHands));  
    };
};


struct optionParam
{
    string settlementDate;
    string maturityDate;
    string timeToNextAveraged;
    optionKind _optionKind;
    double initialPirce;
    double callExecPrice;
    double putExecPrice;
    int callScale;
    int putScale;
    double volability;
    double elapsedTime;
    double remainTime;
    optionParam(){
        memset(this,0,sizeof(optionParam));
    };
    void reset(){
        memset(this,0,sizeof(optionParam));
    }

};

//�����ȡ����Ϣ�Ľṹ��
struct AccountParam
{
    TThostFtdcBrokerIDType    m_appId;//���͹�˾����
    TThostFtdcInvestorIDType    m_userId;//�û���
    char    m_passwd[252];//����

    char m_mdFront[50];//�����������ַ
    char m_tradeFront[50];//���׷�������ַ

    //string m_read_contract;//��Լ����
    AccountParam(){
        memset(this,0,sizeof(AccountParam));
    }
};




//��ȡ��ʷK��
void ReadDatas(string fileName, vector<History_data> &history_data_vec);

//�����ļ�
int Store_fileName(string path, vector<string> &FileName);

//�ַ�תʱ���
time_t strTime2unix(string timeStamp) ;
time_t strTime2unix(string date,string time)  ;
// ��׼����ת��Ϊʱ���
time_t dateTime2unix(int year,int month,int mDay,int hour, int minute,int second);

//��׼����ת���ַ���
string dateTime2Str(int year,int month,int mDay,int hour, int minute,int second);


//����ת��
string optionKindToStr(int optionKind);

string wCharToMchar(wstring);


void readDay(string fileName, map<string,int> &workDay);

#endif