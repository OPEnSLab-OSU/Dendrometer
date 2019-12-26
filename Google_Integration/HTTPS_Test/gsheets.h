#include "Arduino.h"
#ifndef G_SHEETS
#define G_SHEETS

class GSheets
{
    public:
        GSheets(String key);
        String testfunc();
    private:
        String api_key;

};

#endif