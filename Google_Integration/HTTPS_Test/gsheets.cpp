#include "gsheets.h"


#define HOST "sheets.googleapis.com"
#define ROOTHOST "googleapis.com"

GSheets::GSheets(String clientID, String clientSecret, String refresh, String sheetID)
{
    this->clientID = clientID;
    this->clientSecret = clientSecret;
    this->refresh = refresh;
    this->sheetID = sheetID;
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
    // this->client.setInsecure();
//    if(this->client.connect(HOST, 443))
//    {
//        Serial.println("Connected to host");
//    }

//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }
//  Serial.println("Connected to the WiFi network");

  client.stop();
  while(!this->client.connect(HOST, 443))
  {
    Serial.println("Connecting to host...");
    client.stop();
    WiFi.end();
    delay(10);
    WiFi.begin("OSU_Access");
//    TODO: If this runs more than 10-20 times, need to reset
//             so put 41-44 only if "Connecting to host..." runs multiple times

// WORKS, but shifts the columns that were being written to into separate columns, need to fix
//        Tried interrupts, but are not supported. Need to implement in different logic

  }
  Serial.println("Connected...");
    
}

WiFiSSLClient GSheets::getClient()
{
    return this->client;
}

void GSheets::updateSheet(String a1Notation, std::vector<std::vector<String>> cells, WriteOption option)
{
    this->connectToHost();
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
    DynamicJsonDocument doc(2048);
    try{
        doc = jsonfyBody();
    }catch(int n)
    {
      Serial.println("CAUGHT");
      this->connectToHost();
      this->client.print(String("PUT ") + url + " HTTP/1.0\r\n" + "Authorization: " + oauth + "\r\n" + "Accept: */*\r\n" + "Content-Length: " + payload.length() + "\r\n" + "Cache-Control: no-cache\r\n" + "Host: " + HOST + "\r\n\r\n" + payload + "\r\n\r\n");
      this->getServerResponse();
//      this->updateSheet(a1Notation, cells, option);
//      FIX THIS RECURSIVE CALL, maybe make a alternate function for error
      doc = jsonfyBody();
//      TODO: This statement assumes that disconnect will only happen once and not twice in a row, need to fix
    }
    int code = doc["error"]["code"];
    if(code == 401)
    {
        this->refreshKey();
        this->connectToHost();
        this->updateSheet(a1Notation, cells, option);
//        FIX RECURSIVE CALL, maybe make a alternate function for error
//        Maybe don't need to bc it is last statement in function?
    }
}

void GSheets::getServerResponse()
{
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;

    this->headers = "";
    this->body = "";

    delay(3000);
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
//   delay(5000);
}

//TODO: look if DynamicJson needs to be deallocated
DynamicJsonDocument GSheets::jsonfyBody()
{
    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, this->body);
    if(err)
    {
        Serial.print("ERR");
        Serial.println(err.c_str());
        throw(20);
    }
    return doc;
}

//Wifi must be connected in order to call (or else will crash)
void GSheets::refreshKey()
{
    this->connectToHost();
    String url = "/token";
    String payload = "refresh_token=" + this->refresh + "&client_id=" + this->clientID + "&client_secret=" + this->clientSecret + "&grant_type=refresh_token";
    this->client.print(String("POST ") + url + " HTTP/1.0\r\n" + "Content-Type: application/x-www-form-urlencoded\r\n" + "Content-Length: " + payload.length() + "\r\n" +"Host: oauth2." + ROOTHOST + "\r\n\r\n" + payload + "\r\n\r\n");
    this->getServerResponse();
    DynamicJsonDocument doc = jsonfyBody();
    String test = doc["access_token"];
    Serial.println("NEWKEY");
    Serial.println(test);
    this->oAuthKey = test;
}
