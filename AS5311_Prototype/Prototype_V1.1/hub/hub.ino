#include <Loom.h>

// Include configuration
const char* json_config = 
#include "config.h"
;

// Set enabled modules
LoomFactory<
	Enable::Internet::Ethernet,
	Enable::Sensors::Enabled,
	Enable::Radios::Enabled,
	Enable::Actuators::Disabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };



void setup() 
{ 

	pinMode(5, OUTPUT);
	digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
	pinMode(6, OUTPUT);
	digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

	Loom.begin_serial();
	Loom.parse_config(json_config);
	Loom.print_config();

	LPrintln("\n ** Setup Complete ** ");
	LPrintln("\n Looping... Please ignore recieve failure messages");
}

void loop() 
{	
	if (Loom.LoRa().receive_blocking(10000)) {
		Loom.display_data();
		Lprintln(Loom.LoRa().get_rssi())
		if(!Loom.GoogleSheets().publish()) {
                Serial.println("failed to print to Gsheets");
            }
	}
}
