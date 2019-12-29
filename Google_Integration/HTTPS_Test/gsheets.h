#include "Arduino.h"
#ifndef G_SHEETS
#define G_SHEETS

/**
 *
 * @author Mark Huynh
 *
 * @brief 
 */

enum WriteOption{RAW, PARSED};

class GSheets
{
    public:
        //Constructor to create GSheets Object
        GSheets(String clientID, String clientSecret, String refresh);
        GSheets(String oAuthKey, String sheetID);
        
        //Connects client to "sheets.googleapis.com", **MAKE SURE WIFI IS CONNECTED TO BOARD BEFORE**
        void connectToHost();

        //Will write to sheet and cells in a1Notation with option
        void updateSheet(String a1Notation, WriteOption option);

        WiFiClientSecure getClient();
        // void sendTest();
    private:
        String clientID;
        String clientSecret;
        String refresh;
        String oAuthKey;
        String sheetID;

        String headers;
        String body;

        WiFiClientSecure client;

        void getServerResponse();
};

#endif