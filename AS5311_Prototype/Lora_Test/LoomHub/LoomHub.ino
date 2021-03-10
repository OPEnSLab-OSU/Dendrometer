// modification of rf95_reliable_datagram_server.pde

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "dendro.h"
#include <Loom.h>

// Include configuration
const char* json_config = 
#include "config.h"
;

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

RH_RF95 driver(RFM95_CS, RFM95_INT);

RHReliableDatagram manager(driver, SERVER_ADDRESS);

// Set enabled modules
LoomFactory<
	Enable::Internet::Ethernet,
	Enable::Sensors::Disabled,
	Enable::Radios::Disabled,
	Enable::Actuators::Disabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

void setup()
{
    Loom.begin_serial(true);
    Loom.parse_config(json_config);
    Loom.print_config();
    // Serial.begin(9600);
    if (!manager.init())
        Serial.println("init failed");
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	bool status = driver.setFrequency(915.0);
	LPrintln( "\tSetting Frequency ", (status) ? "Success" : "Failed" );

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

}

uint8_t data[] = "And hello back to you";
// Dont put this on the stack:

// uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
Dendro_t in_data;

void loop()
{
    if (manager.available())
    {
        // Wait for a message addressed to us from the client
        uint8_t from;
        // var gets modified/replaced by actual payload length
        uint8_t payloadLength = sizeof(in_data.raw);

        if (manager.recvfromAck(in_data.raw, &payloadLength, &from))
        {
            Serial.print("got request from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");

            DynamicJsonDocument out_json(2048);

            JsonObject foo = out_json.to<JsonObject>();

            struct_to_json(in_data, foo);

            serializeJsonPretty(foo, Serial);

            /* RSSI which is the receiver signal strength indicator. This number will range from about -15 to about -100.
            The larger the number (-15 being the highest you'll likely see) the stronger the signal.

            "Minimum reported RSSI seen for successful comms was about -91"

            */
           
            Serial.print("RSSI: ");
            Serial.println(driver.lastRssi(), DEC);

            JsonObject internal_json = Loom.internal_json(true);

            internal_json.set(foo);

            Loom.display_data();

            // // Send a reply back to the originator client
            // if (!manager.sendtoWait(data, sizeof(data), from))
            //     Serial.println("sendtoWait failed");
            if(!Loom.GoogleSheets().publish()) {
                Serial.println("failed to print to Gsheets");
            }
        }
    }
}