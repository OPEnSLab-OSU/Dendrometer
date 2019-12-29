#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Arduino.h"
#include "gsheets.h"


#define HOST "sheets.googleapis.com"

GSheets::GSheets(String clientID, String clientSecret, String refresh)
{
    this->clientID = clientID;
    this->clientSecret = clientSecret;
    this->refresh = refresh;
}

GSheets::GSheets(String oAuthKey, String sheetID)
{
    this->oAuthKey = oAuthKey;
    this->sheetID = sheetID;
}

//TODO: Throw error if cannot connect to host (Wifi might not be connected)
//TODO: Think about if Serial is not initialized
void GSheets::connectToHost()
{
    this->client.setInsecure();
    if(this->client.connect(HOST, 443))
    {
        Serial.println("Connected to host");
    }
}

WiFiClientSecure GSheets::getClient()
{
    return this->client;
}

void GSheets::updateSheet(String a1Notation, std::vector<std::vector<String>> cells, WriteOption option)
{
    String inputOption;
    switch(option)
    {
        case(0):
            inputOption = "RAW";
            break;
        case(1):
            inputOption = "USER_ENTERED";
            break;
    }
    String url = "/v4/spreadsheets/" + this->sheetID + "/values/" + a1Notation + "?valueInputOption=" + inputOption;
    String oauth = "Bearer " + this->oAuthKey;

    //TODO: Use vectors/templates to pass in and parse to surround with escape quotes

    String test[2][4] = {{"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"}};
    // std::vector<String> test = {"asdffgdsa", "12345", "=2*3", "foo"};

    String value = "[";
    for(int i = 0; i < cells.size(); i++)
    {
        value += "[";
        for(int j = 0; j < cells[i].size(); j++)
        {
            value += "\"" + cells[i][j] + "\",";
        }
        value += "],";
    }
    value += "]";

    Serial.println(value);

    String payload = "{\"range\": \"" + a1Notation + "\",\"majorDimension\": \"ROWS\",\"values\": " + value + "}";
    this->client.print(String("PUT ") + url + " HTTP/1.0\r\n" + "Authorization: " + oauth + "\r\n" + "Accept: */*\r\n" + "Content-Length: " + payload.length() + "\r\n" + "Cache-Control: no-cache\r\n" + "Host: " + HOST + "\r\n\r\n" + payload + "\r\n\r\n");
    Serial.println("Sent");
    this->getServerResponse();
}

void GSheets::getServerResponse()
{
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;
    bool gotResponse = false;

    this->headers = "";
    this->body = "";

    delay(2000);
    while(this->client.available())
    {
        char c = this->client.read();
        if (finishedHeaders) {
            this->body=this->body+c;
            } else {
            if (currentLineIsBlank && c == '\n') {
                finishedHeaders = true;
            }
            else {
                this->headers = this->headers + c;
            }
            }

            if (c == '\n') {
            currentLineIsBlank = true;
            }else if (c != '\r') {
            currentLineIsBlank = false;
            }
    }
  Serial.println("HEAD");
  Serial.println(this->headers);
    Serial.println("BODY");
  Serial.println(this->body);
  delay(10000);
}