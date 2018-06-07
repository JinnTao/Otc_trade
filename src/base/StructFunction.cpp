
#include<fstream>
#include<io.h>
#include<algorithm>
#include    <wchar.h>
#include    <Windows.h>

#include    "StructFunction.h"



//读取历史K线
void    ReadDatas(string    fileName,    vector<History_data>    &history_data_vec)
{
    History_data    stu;
    char    dataline[512];//行数变量
    ifstream    file1(fileName,ios::in);    //以只读方式读入,读取原始数据
    if(!file1){
        cout<<"no    such    file!"<<endl;
        abort();
    }    
    while(file1.getline(dataline,1024,'\n'))//while开始，读取一行1024够大？
    {
        int    len=strlen(dataline);
        char    tmp[20];
        for(int    i=0,j=0,count=0;i<=len;i++){
            if(dataline[i]!=    ','    &&    dataline[i]!='\0'){
                tmp[j++]=dataline[i];
            }
            else{
                tmp[j]='\0';
                count++;
                j=0;
                switch(count){
                case    1:
                    stu.date=tmp;
                    break;    
                case    2:
                    stu.time=tmp;
                    break;
                case    3:
                    stu.buy1price=atof(tmp);
                    break;
                case    4:
                    stu.sell1price=atof(tmp);
                    break;
                case    5:
                    stu.open=atof(tmp);
                    break;
                case    6:
                    stu.high=atof(tmp);
                    break;
                case    7:
                    stu.low=atof(tmp);
                    break;

                    break;
                case    8:
                    stu.close=atof(tmp);
                    break;

                }
            }
        }
        history_data_vec.push_back(stu);
    }

    file1.close();
}


void    readDay(string    fileName,    map<string,int>    &workDay){
    ifstream    file1(fileName,ios::in);    //以只读方式读入,读取原始数据
    char    dataline[512];//行数变量
    string    date;
    int    i    =    1;
    if(!file1){
        cout<<"no    such    file!"<<endl;
        abort();
    }    
    while(file1.getline(dataline,1024,'\n'))//while开始，读取一行1024够大？
    {
        //sscanf_s(dataline,"%s",date);
        date    =    dataline;
        workDay.insert(pair<string,int>(date,i));
        i++;
        //cout    <<    date    <<    endl;

    }
}

int    Store_fileName(string    path,    vector<string>    &FileName)
{
    struct    _finddata_t    fileinfo;        
    string    in_path;        
    string    in_name;        

    //in_path="d:\\future_data";
    in_path    =    path;
    string    curr    =    in_path+"\\*.txt";    
    long    handle;        

    if((handle=_findfirst(curr.c_str(),&fileinfo))==-1L)        
    {        
        cout<<"没有找到匹配文件!"<<endl;

        return    0;        
    }        

    else        
    {
        do
        {
            in_name=in_path    +    "\\"    +fileinfo.name;    
            FileName.push_back(in_name);
        }while(!(_findnext(handle,&fileinfo)));

        _findclose(handle);

        return    0;

    }
}


time_t    strTime2unix(string    timeStamp)        
{        
                struct    tm    tm;        
                memset(&tm,    0,    sizeof(tm));        
                        
                sscanf_s(timeStamp.c_str(),    "%d-%d-%dT%d:%d:%d",            
                                            &tm.tm_year,    &tm.tm_mon,    &tm.tm_mday,        
                                            &tm.tm_hour,    &tm.tm_min,    &tm.tm_sec);        
        
                tm.tm_year    -=    1900;        
                tm.tm_mon--;        
        
                return    mktime(&tm);        
}        

time_t    strTime2unix(string    date,string    time)        
{        
                struct    tm    tm;        
    if(date.length()    <6){
        return    0;
    }
                memset(&tm,    0,    sizeof(tm));        
                //更改date的格式
    tm.tm_year    =    atoi(date.substr(0,4).c_str());
    tm.tm_mon    =    atoi(date.substr(4,2).c_str());
    tm.tm_mday    =    atoi(date.substr(6,2).c_str());


                sscanf_s(time.c_str(),    "%d:%d:%d",                
                                            &tm.tm_hour,    &tm.tm_min,    &tm.tm_sec);        
        
                tm.tm_year    -=    1900;        
                tm.tm_mon--;        
        
                return    mktime(&tm);        
}        

string    dateTime2Str(int    year,int    month,int    mDay,int    hour,    int    minute,int    second){
    char    dateTimeStr[255];

    sprintf_s(dateTimeStr,    "%d-%d-%dT%d:%d:%d",        
        year,month,mDay,
        hour,minute,second);

    return    string(dateTimeStr);

}

time_t    dateTime2unix(int    year,int    month,int    mDay,int    hour,    int    minute,int    second){
        struct    tm    tm;        
                memset(&tm,    0,    sizeof(tm));        
    tm.tm_year    =    year;
    tm.tm_mon    =    month;
    tm.tm_mday    =    mDay;
    tm.tm_hour    =    hour;
    tm.tm_min    =    minute;
    tm.tm_sec    =    second;
        
                tm.tm_year    -=    1900;        
                tm.tm_mon--;        
        
                return    mktime(&tm);        


}


string    optionKindToStr(int    optionKind){
    string    optionKind2String[]    =    {
        "European",
        "American",
        "Asian",
        "Barrier",
        "Digital",
        "LookBack",
        "Forward",
        "",
        "",
        ""
    };
    return    optionKind2String[optionKind];
}

string    wCharToMchar(wstring    sourceString){
    char    namec[255];
    wstring    namew    =    sourceString;
    int    nLen    =    WideCharToMultiByte(CP_ACP,    0,    namew.c_str(),    -1,    NULL,    0,    NULL,    NULL);
    char*    pszDst    =    new    char[nLen];
    WideCharToMultiByte(CP_ACP,    0,    namew.c_str(),    -1,    pszDst,    nLen,    NULL,    NULL);
    pszDst[nLen    -1]    =    0;
    std::string    strTemp(pszDst);
    delete    []    pszDst;
    string    name(strTemp);
    return    name;

}