#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Arduino.h"
#include "gsheets.h"

GSheets::GSheets(String key)
{
    this->api_key = key;
}

String GSheets::testfunc()
{
    return api_key;
}