#include <Loom.h>

// Include configuration
const char* json_config = 
#include "config.h"
;

// Set enabled modules
LoomFactory<
	Enable::Internet::LTE,
	Enable::Sensors::Enabled,
	Enable::Radios::Enabled,
	Enable::Actuators::Disabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

void setup() 
{ 
  pinMode(5, OUTPUT);    // Enable control of 3.3V rail
  pinMode(6, OUTPUT);   // Enable control of 5V rail
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail

  Loom.begin_serial();
  Loom.parse_config(json_config);
  Loom.print_config();

  LPrintln("\n ** Setup Complete ** ");
}

void loop() 
{
  if (Loom.LoRa().receive_batch_blocking(5000)) {
    Loom.display_data();
    Loom.log_all(); // Check if this is how it works to publish the Batch
    if(!Loom.GoogleSheets().publish_batch())
      Serial.println("failed to print to Gsheets");
  }
}
