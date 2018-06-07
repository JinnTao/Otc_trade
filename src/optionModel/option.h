#pragma    once

#include    "europeanPrice.h"
#include    "americanPrice.h"
#include    "asianPrice.h"
#include    "digital.h"
#include    "barrierPrice.h"

using    namespace    std;


class    DLL_API_CLASS        option
//class    option
{
public:
                option();

                ~option(void);
                europeanPrice    europeanPtr;
                americanPrice    americanPtr;
                asianPrice                        asianPtr;
                barrierPrice                barrierPtr;
                americanDigital    americanDigitalPtr;
                europeanDigital    europeanDigitalPtr;

private:
    
};
