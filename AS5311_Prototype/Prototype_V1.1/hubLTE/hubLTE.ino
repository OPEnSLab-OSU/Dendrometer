#include <Loom.h>

// Include configuration
const char* json_config =
#include "config.h"
;

// In Tools menu, set:
// Internet  > LTE
// Sensors   > Enabled
// Radios    > Enabled
// Actuators > Disabled
// Max       > Disabled


using namespace Loom;

Loom::Manager Feather{};

void setup()
{
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail
  
  Feather.begin_serial();
  Feather.parse_config(json_config);
  Feather.print_config();

  LPrintln("\n ** Setup Complete ** ");
}

void loop()
{
  if (getLoRa(Feather).receive_batch_blocking(5000)) {

    Feather.display_data();
    getSD(Feather).log();

    getGoogleSheets(Feather).print_config();
    getGoogleSheets(Feather).publish_batch();
  }
}
