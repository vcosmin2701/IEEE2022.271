#include <functions.h>

void setup()
{
  Serial.begin(9600); // set serial communication with pc baudrate

  wifi_start();

  server_start();
  server_settings();

  server.onNotFound(notFound);
  server.addHandler(&events);
  server.begin();

  modbus_start();

  server_readCoils();
  server_readHolding();
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    adc_read(); // read battery voltage

    // Send Events to the Web Server with the Sensor Readings
    events.send(String(voltage).c_str(), "voltage", millis());
    lastTime = millis();
  }

  mb.task();
  yield(); // Passes control to other tasks when called.
           // Used in functions that will take awhile to complete.
}

