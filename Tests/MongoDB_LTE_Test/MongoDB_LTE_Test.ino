#include "arduino_secrets.h"
#include <Loom_Manager.h>
#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Internet/Connectivity/Loom_LTE/Loom_LTE.h>
#include <Internet/Logging/Loom_MongoDB/Loom_MongoDB.h>

Manager manager("Mongo_Test", 0);
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);
Loom_Analog batteryVoltage(manager);
Loom_LTE lte(manager, "hologram", "", "", A5);
Loom_MongoDB mqtt(manager, lte, SECRET_BROKER, SECRET_PORT, DATABASE, BROKER_USER, BROKER_PASS);


void setup() {
  manager.beginSerial();
  hypnos.enable();
  mqtt.loadConfigFromJSON(hypnos.readFile("mqtt_creds.json"));
  manager.initialize();

}

void loop() {
  manager.set_device_name("Mongo_Test");
  manager.set_instance_num(0);
  manager.measure();
  manager.package();
  manager.addData("Test", "TestField", 20);
  manager.display_data();
  mqtt.publish(); 

  delay(30000);

}
