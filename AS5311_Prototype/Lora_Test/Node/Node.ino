// modification of rf95_reliable_datagram_client.pde

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "dendro.h"

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define ARDUINOJSON_USE_DOUBLE 1

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup()
{

    Serial.begin(9600);
    if (!manager.init())
        Serial.println("init failed");
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    bool status = driver.setFrequency(915.0);
    Serial.print( "\tSetting Frequency:");
    if(status) {
	    Serial.println("Success");
    } else {
        Serial.println("Failed");
    }


    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 2 to 20 dBm:
     driver.setTxPower(20, false);
    // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
    // transmitter RFO pins and not the PA_BOOST pins
    // then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true.
    // Failure to do that will result in extremely low transmit powers.
    //  driver.setTxPower(14, true);

    // You can optionally require this module to wait until Channel Activity
    // Detection shows no activity on the channel before transmitting by setting
    // the CAD timeout to non-zero:
    //  driver.setCADTimeout(10000);
  manager.setRetries(5);
  manager.setTimeout(400);


// Low frequency, slow and long range. Both TX and RX need to be set
//   driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
}

const char data[] PROGMEM = "{\"type\":\"data\",\"id\":{\"name\":\"Bar\",\"instance\":7},\"contents\":[{\"module\":\"Packet\",\"data\":{\"Number\":3}},{\"module\":\"Analog\",\"data\":{\"Vbat\":5.354444,\"A[0]\":1836,\"A[1]\":1590,\"A[2]\":40,\"A[3]\":1412,\"A[4]\":3775,\"A[5]\":3786}},{\"module\":\"SHT31D\",\"data\":{\"temp\":19.07988,\"humid\":66.68955}},{\"module\":\"AS5311\",\"data\":{\"Serial Value\":114}},{\"module\":\"Displacement (mm)\",\"data\":{\"mm\":-0.007814}},{\"module\":\"Displacement (um)\",\"data\":{\"um\":-7.814408}},{\"module\":\"Difference (mm)\",\"data\":{\"mm\":-0.127961}},{\"module\":\"Difference (um)\",\"data\":{\"um\":-127.9609}},{\"module\":\"Status\",\"data\":{\"Color\":\"Red\"}}],\"timestamp\":{\"date\":\"2021/2/14\",\"time\":\"18:56:2\"}}";
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
    // Deserialize example payload string
    DynamicJsonDocument doc(2048);

    DeserializationError error = deserializeJson(doc, data);

    if (error)
    {
        Serial.print(("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    Dendro_t out_struct;

    JsonObjectConst object = doc.as<JsonObject>();

    json_to_struct(object, out_struct);

    // Send a message to manager_server
    if (manager.sendtoWait(out_struct.raw, (uint8_t)sizeof(out_struct.raw), SERVER_ADDRESS))
    {
        // Now wait for a reply from the server
        uint8_t len = sizeof(buf);
        uint8_t from;


        /* RSSI which is the receiver signal strength indicator. This number will range from about -15 to about -100.
        The larger the number (-15 being the highest you'll likely see) the stronger the signal.

        "Minimum reported RSSI seen for successful comms was about -91"

        */
        
        Serial.print("RSSI: ");
        Serial.println(driver.lastRssi(), DEC);


        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char *)buf);
        }
        else
        {
            Serial.println("No reply, is rf95_reliable_datagram_server running?");
        }
    }
    else
        Serial.println("sendtoWait failed, no ack received");
    delay(500);

    //remember to call sleep on manager/driver
}