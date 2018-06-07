#pragma    once
#include<oci.h>
#include<ociap.h>
#include<iostream>
#include<string>
#include<string.h>
#include<stdlib.h>
#include<vector>
using    namespace    std;

    struct    OCI_hpp
    {
        OCIEnv    *envhpp;
        OCIError    *errhpp;
        OCIServer    *servhpp;
        OCISession    *sesshpp;
        OCISvcCtx    *svchpp;
        OCIStmt    *stmthpp;
    };

    struct    T_ACCOUNT_FUND_result
    {
        char    ID[20];
        char    trader_name[20];
        char    cash[20];
        char    Currency[4];
    };


    void    OracleServerConnect(OCI_hpp    *hpp,string    server,string    username,string    password);
    void    OracleServerDetach(OCI_hpp    *hpp);
    void    OCISelect(OCI_hpp    *hpp,char    *sqlcommd);
    void    OCIInsert(OCI_hpp    *hpp,char    *sqlcommd);
    void    OCIDelete(OCI_hpp    *hpp,char    *sqlcommd);
    void    OCIUpdate(OCI_hpp    *hpp,char    *sqlcommd);
    char    **OCI_LiYong_Select(OCI_hpp    *hpp,char    *sqlcommd,char**Result);
    char    *OCI_SearchGetChar(OCI_hpp    *hpp,char    *sqlcommd,char    *Result);
