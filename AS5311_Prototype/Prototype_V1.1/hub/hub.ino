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

void setup() {
  
  Loom.begin_serial();
  Loom.parse_config(json_config);
  Loom.print_config();

  LPrintln("\n ** Setup Complete ** ");
  LPrintln("\n Looping... Please ignore recieve failure messages");
}

void loop() 
{	
  if (Loom.LoRa().receive_batch_blocking(5000)) {
    Loom.display_data();
	  if(!Loom.GoogleSheets().publish_batch())
      Serial.println("failed to print to Gsheets");
  }
}
