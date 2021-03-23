

#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp1;
Adafruit_MCP23017* mcpr = &mcp1;

void setup() {
  mcp1.begin(0);
  setupEncoders();
  setupLcd();
  setupMidi();
  setupDataHandling();
  setupRendering();
}

void loop() {
  loopEncoders();
  loopMidi();
}

void debug(String msg) {
  //Serial.println(msg);
}
