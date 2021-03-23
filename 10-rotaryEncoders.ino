

// thanks to https://github.com/cosmikwolf/Bounce2mcp
#include <Bounce2mcp.h>

// thanks to https://github.com/maxgerhardt/rotary-encoder-over-mcp23017
#include "lib/RotaryEncOverMCP.h"

// in this setup 2 rotary/push encoders are used
#define NUM_BUTTONS 2

/*
                              MCP23017
                            _____ _____
                           |     U     |
                          -| 8       7 |- -> to Encoder #1 DT
                          -| 9       6 |- -> to Encoder #1 CLK
                          -| 10      5 |- -> to Encoder #1 SW
                          -| 11      4 |- -> to Encoder #2 DT
                          -| 12      3 |- -> to Encoder #2 CLK
                          -| 13      2 |- -> to Encoder #2 SW
                          -| 14      1 |-
                          -| 15      0 |-
     to Arduino UNO 5V <- -| Vdd  INTA |-
    to Arduino UNO GND <- -| Vss  INTB |-
                          -| NC    RST |- -> to Arduino UNO 5V
to Arduino UNO A5(SCL) <- -| SCL    A2 |- -> to Arduino UNO GND
to Arduino UNO A4(SDA) <- -| SDA    A1 |- -> to Arduino UNO GND
                          -| NC     A0 |- -> to Arduino UNO GND
                           |___________|

*/

// pin numbers of MCP23017 for encoders (you can use any of the MCP23017 [0-15] pins for encoders DT, CLK and SW)
#define MCP_PIN_E1_DT 7
#define MCP_PIN_E2_DT 4

#define MCP_PIN_E1_CLK 6
#define MCP_PIN_E2_CLK 3

#define MCP_PIN_E1_SW 5
#define MCP_PIN_E2_SW 2

// Pins, used by buttons
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {
  MCP_PIN_E1_SW,
  MCP_PIN_E2_SW
};

// Flags to distinguish click from turn (length = NUM_BUTTONS)
bool BUTTON_ROTATION[NUM_BUTTONS] = {false, false};

// Helper to track push duration for each encoder (length = NUM_BUTTONS)
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

RotaryEncOverMCP rotaryEncoders[] = { // length = NUM_BUTTONS
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
