#pragma    once
#define    _XLL_RELEASE
#ifdef    _XLL_RELEASE
    #define    DLL_API_CLASS
#else
    #ifdef        DLL_API
    #define    DLL_API_CLASS    __declspec(dllexport)        
    #else
    #define    DLL_API_CLASS    _declspec(dllimport)
    #endif
#endif

#include<algorithm>

namespace    optionSpace{
enum    OptionType    {CALL,PUT,CallUpIn,CallUpOut,CallDownIn,CallDownOut,PutUpIn,PutUpOut,PutDownIn,PutDownOut};

enum    Greeks    {PRICE,DELTA,GAMMA,VEGA,THETA,RHO};

enum    PayoutType{AtExpiry,Immediate};

enum    underlyingType{Stock,Future};
}
class    DLL_API_CLASS    structure
{
public:
                structure(void);
                ~structure(void);
};

