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

//期权种类
enum optionKind{European,Asian,American,Barrier,Lookback,Digital};

//登录状态
enum userStatus{LoggingFailed,LoggingSuccess,UserOrPwError,UserTimeOut};
//ctp状态
enum CTPSTATUS{NORMAL,RECONNECT,DISCONNECT};

//交易信息的结构体
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


    string instId;//合约代码
    double lastPrice;//最新价，时刻保存合约的最新价，平仓用
    double PreSettlementPrice;//上次结算价，对隔夜仓有时候要用，快期有用
    int holding_long;//多单持仓量
    int holding_short;//空单持仓量

    int TodayPosition_long;//多单今日持仓
    int YdPosition_long;//多单上日持仓

    int TodayPosition_short;//空单今日持仓
    int YdPosition_short;//空单上日持仓

    double closeProfit_long;//多单平仓盈亏
    double OpenProfit_long;//多单浮动盈亏

    double closeProfit_short;//空单平仓盈亏
    double OpenProfit_short;//空单浮动盈亏

};




struct FutureData//行情结构体定义
{
    string date;
    string time;
    double buy1price;
    int buy1vol;
    double new1;
    double sell1price;
    int sell1vol;
    double vol;
    double openinterest;//持仓量

};



//历史K线
struct History_data
{
    string date;
    string time;
    double buy1price;//买一
    double sell1price;//卖一
    double open;
    double high;
    double low;
    double close;

};

//期权风险值
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
    int netPos;//净头寸
    double callhedgeHands;
    double puthedgeHands;
    double hedgeHands;
    int RealHedgeHands;//取证后头寸
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

//保存读取的信息的结构体
struct AccountParam
{
    TThostFtdcBrokerIDType    m_appId;//经纪公司代码
    TThostFtdcInvestorIDType    m_userId;//用户名
    char    m_passwd[252];//密码

    char m_mdFront[50];//行情服务器地址
    char m_tradeFront[50];//交易服务器地址

    //string m_read_contract;//合约代码
    AccountParam(){
        memset(this,0,sizeof(AccountParam));
    }
};




//读取历史K线
void ReadDatas(string fileName, vector<History_data> &history_data_vec);

//保存文件
int Store_fileName(string path, vector<string> &FileName);

//字符转时间戳
time_t strTime2unix(string timeStamp) ;
time_t strTime2unix(string date,string time)  ;
// 标准日期转化为时间戳
time_t dateTime2unix(int year,int month,int mDay,int hour, int minute,int second);

//标准日期转化字符串
string dateTime2Str(int year,int month,int mDay,int hour, int minute,int second);


//类型转换
string optionKindToStr(int optionKind);

string wCharToMchar(wstring);


void readDay(string fileName, map<string,int> &workDay);

#endif