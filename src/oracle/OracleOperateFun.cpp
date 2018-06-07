#include"OracleOperateFun.h"

void    OracleServerConnect(OCI_hpp    *hpp,string    server,string    username,string    password)
{
    //����OCI���������价�����
    sword    SwResult=OCIEnvCreate(&hpp->envhpp,(ub4)OCI_DEFAULT,NULL,NULL,NULL,NULL,0,NULL);
    if    (SwResult!=OCI_SUCCESS    &&    SwResult!=OCI_SUCCESS_WITH_INFO)
    {
        throw("Oracle    environment    initialization    error!");
    }
    printf("Oracle    environment    initialization    success!\n");
    //���������
    OCIHandleAlloc((const    void    *)hpp->envhpp,(dvoid    **)&hpp->errhpp,(const    ub4)OCI_HTYPE_ERROR,(const    size_t)0,(dvoid    **)0);
    //������������
    OCIHandleAlloc((const    void    *)hpp->envhpp,(dvoid    **)&hpp->servhpp,(const    ub4)OCI_HTYPE_SERVER,(const    size_t)0,(dvoid    **)0);
    //���������
    OCIHandleAlloc((const    void    *)hpp->envhpp,(dvoid    **)&hpp->svchpp,(const    ub4)OCI_HTYPE_SVCCTX,(const    size_t)0,(dvoid    **)0);
    //����Ự���
    OCIHandleAlloc((const    void    *)hpp->envhpp,(dvoid    **)&hpp->sesshpp,(const    ub4)OCI_HTYPE_SESSION,(const    size_t)0,(dvoid    **)0);
    //������ʽ���
    if(OCIHandleAlloc((const    void    *)hpp->envhpp,(dvoid    **)&hpp->stmthpp,(const    ub4)OCI_HTYPE_STMT,(const    size_t)0,(dvoid    **)0)!=OCI_SUCCESS)
    {
        printf("Creat    STMT    error!");
    }
    printf("Creat    STMT    success!\n");
    //�������ݿ������
    sword    OCIServerConnect=OCIServerAttach(hpp->servhpp,hpp->errhpp,(const    OraText    *)server.c_str(),(sb4)strlen(server.c_str()),(ub4)OCI_DEFAULT);
    if(OCIServerConnect!=OCI_SUCCESS)
    {

        printf("Oracle    Server    attach    error!");
    }
    printf("Oracle    Server    attach    success!\n");
    //��������������õ���������
    OCIAttrSet((dvoid    *)hpp->svchpp,(ub4)OCI_HTYPE_SVCCTX,(dvoid    *)hpp->servhpp,(ub4)0,(ub4)OCI_ATTR_SERVER,hpp->errhpp);
    //���û������������õ��Ự�����
    OCIAttrSet((dvoid    *)hpp->sesshpp,(ub4)OCI_HTYPE_SESSION,(dvoid    *)username.c_str(),(ub4)strlen(username.c_str()),(ub4)OCI_ATTR_USERNAME,hpp->errhpp);
    OCIAttrSet((dvoid    *)hpp->sesshpp,(ub4)OCI_HTYPE_SESSION,(dvoid    *)password.c_str(),(ub4)strlen(password.c_str()),(ub4)OCI_ATTR_PASSWORD,hpp->errhpp);
    //�����Ự����
    sword    OCISessionConnect=OCISessionBegin(hpp->svchpp,hpp->errhpp,hpp->sesshpp,(ub4)OCI_CRED_RDBMS,(ub4)OCI_DEFAULT);
    if(OCISessionConnect!=OCI_SUCCESS)
    {
        throw("User    Session    error!");
    }
    printf("User    Session    success!\n");
    //���Ự������õ���������
    OCIAttrSet((dvoid    *)hpp->svchpp,(ub4)OCI_HTYPE_SVCCTX,(dvoid    *)hpp->sesshpp,(ub4)0,(ub4)OCI_ATTR_SESSION,hpp->errhpp);
}

void    OracleServerDetach(OCI_hpp    *hpp)
{
    //�Ͽ��Ự����
    OCISessionEnd(hpp->svchpp,hpp->errhpp,hpp->sesshpp,(ub4)OCI_DEFAULT);
    //�Ͽ�������
    OCIServerDetach(hpp->servhpp,hpp->errhpp,(ub4)OCI_DEFAULT);
    //�ͷž����Դ
    OCIHandleFree(hpp->stmthpp,(const    ub4)OCI_HTYPE_STMT);
    OCIHandleFree(hpp->sesshpp,(const    ub4)OCI_HTYPE_SESSION);
    OCIHandleFree(hpp->svchpp,(const    ub4)OCI_HTYPE_SVCCTX);
    OCIHandleFree(hpp->servhpp,(const    ub4)OCI_HTYPE_SERVER);
    OCIHandleFree(hpp->errhpp,(const    ub4)OCI_HTYPE_ERROR);
    OCIHandleFree(hpp->envhpp,(const    ub4)OCI_HTYPE_ENV);
}

void    OCISelect(OCI_hpp    *hpp,char    *sqlcommd)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    printf("Creat    prepare    success!\n");

    //��ʼ����������ָ��
    OCIDefine    *bhpp[10];
    //��ʼ����ѯ�����
    struct    T_ACCOUNT_FUND_result    *QueryResultPoint;
    QueryResultPoint=(struct    T_ACCOUNT_FUND_result    *)malloc(sizeof(struct    T_ACCOUNT_FUND_result));
    //��ʼ������ָʾ��
    char    isnull[10];
    //��ʼ�����������鳤��ָʾ��
    ub2    datalen[10];
    //�����������
    OCIDefineByPos(hpp->stmthpp,&bhpp[0],hpp->errhpp,(ub4)1,(dvoid    *)&QueryResultPoint->ID,sizeof(QueryResultPoint->ID),SQLT_INT,(dvoid    *)&isnull[0],(ub2    *)&datalen[0],NULL,OCI_DEFAULT);
    OCIDefineByPos(hpp->stmthpp,&bhpp[1],hpp->errhpp,(ub4)2,(dvoid    *)&QueryResultPoint->trader_name,sizeof(QueryResultPoint->trader_name),SQLT_STR,(dvoid    *)&isnull[1],(ub2    *)&datalen[1],NULL,OCI_DEFAULT);
    OCIDefineByPos(hpp->stmthpp,&bhpp[2],hpp->errhpp,(ub4)3,(dvoid    *)&QueryResultPoint->cash,sizeof(QueryResultPoint->cash),SQLT_FLT,(dvoid    *)&isnull[2],(ub2    *)&datalen[2],NULL,OCI_DEFAULT);
    //OCIDefineByPos(hpp->stmthpp,&bhpp[3],hpp->errhpp,(ub4)(4),(dvoid    *)&QueryResultPoint->Currency,sizeof(QueryResultPoint->Currency),SQLT_STR,(dvoid    *)&isnull[3],(ub2    *)&datalen[3],NULL,OCI_DEFAULT);

    //��ȡSQL�������
    ub2    StmtType;
    OCIAttrGet((const    void    *)hpp->errhpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&StmtType,NULL,(ub4)OCI_ATTR_STMT_TYPE,hpp->errhpp);
    //ִ��SQL���
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)0,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
    
    int    k=0;
    do
    {
        if    (k==0)    
        {
            k++;
            continue;
        }
        printf("%d    ",QueryResultPoint->ID);
        printf("%s    ",QueryResultPoint->trader_name);
        printf("%f\n",QueryResultPoint->cash);
        k++;
    }
    while(OCIStmtFetch2(hpp->stmthpp,hpp->errhpp,(ub4)1,(ub2)OCI_FETCH_NEXT,(sb4)1,OCI_DEFAULT)!=OCI_NO_DATA);
    //��ȡ��¼����
    int    RowsFecthed;
    OCIAttrGet((const    void    *)hpp->stmthpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&RowsFecthed,(ub4*)sizeof(RowsFecthed),(ub4)OCI_ATTR_ROW_COUNT,hpp->errhpp);
    //�ͷŲ�ѯ�����
    free(QueryResultPoint);
}

void    OCIInsert(OCI_hpp    *hpp,char    *sqlcommd)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    printf("Creat    prepare    success!\n");
    //ִ�в������
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)1,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
}

void    OCIDelete(OCI_hpp    *hpp,char    *sqlcommd)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    printf("Creat    prepare    success!\n");
    //ִ��ɾ�����
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)1,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
}

void    OCIUpdate(OCI_hpp    *hpp,char    *sqlcommd)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    printf("Creat    prepare    success!\n");
    //ִ�и������
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)1,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
}

char    **OCI_LiYong_Select(OCI_hpp    *hpp,char    *sqlcommd,char    **Result)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    //printf("Creat    prepare    success!\n");

    //��ʼ����������ָ��
    OCIDefine    *bhpp;
    //��ʼ����ѯ�����
    struct    T_ACCOUNT_FUND_result    *QueryResultPoint;
    QueryResultPoint=(struct    T_ACCOUNT_FUND_result    *)malloc(sizeof(struct    T_ACCOUNT_FUND_result));
    //��ʼ������ָʾ��
    char    isnull[10];
    //��ʼ�����������鳤��ָʾ��
    ub2    datalen[10];
    //�����������
    //OCIDefineByPos(hpp->stmthpp,&bhpp,hpp->errhpp,(ub4)1,(dvoid    *)&QueryResultPoint->ID,sizeof(QueryResultPoint->ID),SQLT_STR,(dvoid    *)&isnull[0],(ub2    *)&datalen[0],NULL,OCI_DEFAULT);
    OCIDefineByPos(hpp->stmthpp,&bhpp,hpp->errhpp,(ub4)1,(dvoid    *)&QueryResultPoint->trader_name,sizeof(QueryResultPoint->trader_name),SQLT_STR,(dvoid    *)&isnull[1],(ub2    *)&datalen[1],NULL,OCI_DEFAULT);
    //OCIDefineByPos(hpp->stmthpp,&bhpp[2],hpp->errhpp,(ub4)3,(dvoid    *)&QueryResultPoint->cash,sizeof(QueryResultPoint->cash),SQLT_FLT,(dvoid    *)&isnull[2],(ub2    *)&datalen[2],NULL,OCI_DEFAULT);
    //OCIDefineByPos(hpp->stmthpp,&bhpp[3],hpp->errhpp,(ub4)(4),(dvoid    *)&QueryResultPoint->Currency,sizeof(QueryResultPoint->Currency),SQLT_STR,(dvoid    *)&isnull[3],(ub2    *)&datalen[3],NULL,OCI_DEFAULT);

    //��ȡSQL�������
    ub2    StmtType;
    OCIAttrGet((const    void    *)hpp->errhpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&StmtType,NULL,(ub4)OCI_ATTR_STMT_TYPE,hpp->errhpp);
    //ִ��SQL���
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)0,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
    
    int    k=0;
    do
    {
        if    (k==0)    
        {
            k++;
            continue;
        }
        for(int    j=0;j<20;j++)
        {
            *(*(Result+k-1)+j)=*((QueryResultPoint->trader_name)+j);
        }
        //printf("%s    ",*(Result+k-1));
        
        k++;
    }
    while(OCIStmtFetch2(hpp->stmthpp,hpp->errhpp,(ub4)1,(ub2)OCI_FETCH_NEXT,(sb4)1,OCI_DEFAULT)!=OCI_NO_DATA);
    //��ȡ��¼����
    int    RowsFecthed;
    OCIAttrGet((const    void    *)hpp->stmthpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&RowsFecthed,(ub4*)sizeof(RowsFecthed),(ub4)OCI_ATTR_ROW_COUNT,hpp->errhpp);
    //�ͷŲ�ѯ�����
    free(QueryResultPoint);

    return    Result;
}

char    *OCI_SearchGetChar(OCI_hpp    *hpp,char    *sqlcommd,char    *Result)
{
    //����sql��䣬�����飬��sql�������ʽ�����
    sword    OCIStmtPre=OCIStmtPrepare(hpp->stmthpp,hpp->errhpp,(const    OraText*)sqlcommd,(ub4)strlen((char    *)sqlcommd),(ub4)OCI_NTV_SYNTAX,(ub4)OCI_DEFAULT);
    if(OCIStmtPre!=OCI_SUCCESS)
    {
        throw("Creat    prepare    error");
    }
    printf("Creat    prepare    success!\n");

    //��ʼ����������ָ��
    OCIDefine    *bhpp;    
    //��ʼ����ѯ�����
    struct    T_ACCOUNT_FUND_result    *QueryResultPoint;
    QueryResultPoint=(struct    T_ACCOUNT_FUND_result    *)malloc(sizeof(struct    T_ACCOUNT_FUND_result));
    memset(QueryResultPoint,0,sizeof(T_ACCOUNT_FUND_result));
    //��ʼ������ָʾ��
    char    isnull[10];
    //��ʼ�����������鳤��ָʾ��
    ub2    datalen[10];
    //�����������
    OCIDefineByPos(hpp->stmthpp,&bhpp,hpp->errhpp,(ub4)1,(dvoid    *)&QueryResultPoint->ID,sizeof(QueryResultPoint->ID),SQLT_STR,(dvoid    *)&isnull[1],(ub2    *)&datalen[1],NULL,OCI_DEFAULT);

    //��ȡSQL�������
    ub2    StmtType;
    OCIAttrGet((const    void    *)hpp->errhpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&StmtType,NULL,(ub4)OCI_ATTR_STMT_TYPE,hpp->errhpp);
    //ִ��SQL���
    OCIStmtExecute(hpp->svchpp,hpp->stmthpp,hpp->errhpp,(ub4)0,(ub4)0,(const    OCISnapshot    *)NULL,(OCISnapshot    *)NULL,OCI_DEFAULT);
    
    int    k=0;
    do
    {
        if    (k==0)    
        {
            k++;
            continue;
        }
        for(int    j=0;j<64;j++)
        {
            *(Result+j)=*(QueryResultPoint->ID+j);
        }
        k++;
    }
    while(OCIStmtFetch2(hpp->stmthpp,hpp->errhpp,(ub4)1,(ub2)OCI_FETCH_NEXT,(sb4)1,OCI_DEFAULT)!=OCI_NO_DATA);
    //��ȡ��¼����
    int    RowsFecthed;
    OCIAttrGet((const    void    *)hpp->stmthpp,(ub4)OCI_HTYPE_STMT,(dvoid    *)&RowsFecthed,(ub4*)sizeof(RowsFecthed),(ub4)OCI_ATTR_ROW_COUNT,hpp->errhpp);
    //�ͷŲ�ѯ�����
    free(QueryResultPoint);

    return    Result;
}