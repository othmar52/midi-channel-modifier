
#include <Bounce2mcp.h>
#include "lib/RotaryEncOverMCP.h"

#define NUM_BUTTONS 2

/*
      7   6   5   4   3   2   1   0          5V  GND GND GND
      |   |   |   |   |   |   |   |   |   |   |   |   |   |
    ---------------------------------------------------------
    |                                                       |
    Ↄ                       MCP23017                        |
    |                                                       |
    ---------------------------------------------------------
      |   |   |   |   |   |   |   |   |   |   |   |   |   |
      8   9  10  11  12  13  14  15  5V  GND      A5  A4
*/

// pin numbers of MCP23017 for all 2 encoders
#define MCP_PIN_E1_SW 10
#define MCP_PIN_E2_SW 8

#define MCP_PIN_E1_DT 7
#define MCP_PIN_E2_DT 5

#define MCP_PIN_E1_CLK 6
#define MCP_PIN_E2_CLK 4

// Pins, used by buttons
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {
  MCP_PIN_E1_SW,
  MCP_PIN_E2_SW
};

// Flags to distinguish click from turn
bool BUTTON_ROTATION[NUM_BUTTONS] = {false, false};

// Helper to track push duration for each encoder
unsigned long BUTTON_LASTDOWN[NUM_BUTTONS] = {0, 0};

// Button debounce wrappers
BounceMcp * buttons = new BounceMcp[NUM_BUTTONS];


void RotaryEncoderChanged(bool clockwise, int id) {
  buttons[id].update();
  //debug("Encoder " + String(id) + ": "
  //               + (clockwise ? String("clockwise ") : String("counter-clock-wise "))
  //               + (buttons[id].read() ? String("") : String("(pressed)")));
  BUTTON_ROTATION[id] = true;
  handleEncoderChange(id, clockwise);

}

RotaryEncOverMCP rotaryEncoders[] = {
  RotaryEncOverMCP(&mcp1, MCP_PIN_E1_DT, MCP_PIN_E1_CLK, &RotaryEncoderChanged, 0),
  RotaryEncOverMCP(&mcp1, MCP_PIN_E2_DT, MCP_PIN_E2_CLK, &RotaryEncoderChanged, 1)
};

void setupEncoders() {

  // init encoder buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    mcpr->pinMode(BUTTON_PINS[i], INPUT);
    mcpr->pullUp(BUTTON_PINS[i], HIGH);
    buttons[i].attach(*mcpr, BUTTON_PINS[i], 5);
  }

  // init encoders
  for (int i = 0; i < NUM_BUTTONS; i++) {
    rotaryEncoders[i].init();
  }
}

void pollAll() {
  uint16_t gpioAB = mcpr->readGPIOAB();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    rotaryEncoders[i].feedInput(gpioAB);
  }
}

void loopEncoders() {
  pollAll();

  for (int i = 0; i < NUM_BUTTONS; i++)  {
    buttons[i].update();
    // Updating all debouncers
    if ( buttons[i].fell() ) {
      // Clearing the rotation flag
      BUTTON_ROTATION[i] = false;
      BUTTON_LASTDOWN[i] = millis();
    }

    if ( buttons[i].rose() ) {
      // When button was released...
      //debug("Button " + String(i) + ": up (push duration: " + String(millis() - BUTTON_LASTDOWN[i]) + " ms)" );
      handleEncoderPush(i, millis() - BUTTON_LASTDOWN[i]);
    }
  }
}
