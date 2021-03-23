 
#include <avr/wdt.h>

#define MAX_OCTAVE 5
#define MIN_OCTAVE -5
#define MAX_TRANSPOSE 11
#define MIN_TRANSPOSE -11


#define MODE_BYPASS 0
#define MODE_SINGLE 1
#define MODE_DUAL_SPLIT 2
#define MODE_DUAL_LAYER 3

#define DUAL_SPLIT 0
#define DUAL_LAYER 1

#define PROP_MIDI_CHANNEL 0
#define PROP_OCTAVE 1
#define PROP_TRANSPOSE 2

uint8_t currentMode = MODE_BYPASS;
uint8_t lastLayerOrSplit = DUAL_SPLIT;

struct channelItem {
  uint8_t midiCh;
  String label;
  uint16_t color;
  uint16_t txtColor;
};

// TODO find a name for this struct...
struct findName {
  channelItem item;
  int8_t octave;
  int8_t transpose;
  uint16_t x;
  uint8_t editProp;
};


struct channelItem getItemByChannel(uint8_t midiCh) {
  channelItem item {
    0, "   ---", BLACK, WHITE
  };
  switch(midiCh) {
    case 1: item.label =  "JD-Xi  D1"; item.midiCh = 1;  item.color = ORANGE; break;
    case 2: item.label =  "JD-Xi  D2"; item.midiCh = 2;  item.color = ORANGE; break;
    case 3: item.label =  "JD-Xi ana"; item.midiCh = 3;  item.color = BLUE;   break;
    case 4: item.label =  "GEM rp-x";  item.midiCh = 4;  item.color = GRAY;   break;
    case 5: item.label =  " System1";  item.midiCh = 5;  item.color = GREEN;  item.txtColor = BLACK; break;
    case 6: item.label =  "MicroKORG"; item.midiCh = 6;  item.color = 0x35B7; break;
    case 7: item.label =  "  VIRUS  "; item.midiCh = 7;  item.color = RED;    break;
    case 8: item.label =  "   TB3";    item.midiCh = 8;  item.color = GREEN;  item.txtColor = BLACK; break;
    case 9: item.label =  "   CH 9";   item.midiCh = 9;  item.color = BLACK;  break;
    case 10: item.label = "  CH 10";   item.midiCh = 10; item.color = BLACK;  break;
    case 11: item.label = " Blofeld";  item.midiCh = 11; item.color = BROWN;  break;
    case 12: item.label = "iPad  12";  item.midiCh = 12; item.color = PINK;   break;
    case 13: item.label = "iPad  13";  item.midiCh = 13; item.color = PINK;   break;
    case 14: item.label = "iPad  14";  item.midiCh = 14; item.color = PINK;   break;
    case 15: item.label = "iPad  15";  item.midiCh = 15; item.color = PINK;   break;
    case 16: item.label = " Digitakt"; item.midiCh = 16; item.color = YELLOW; item.txtColor = BLACK;  break;
  }
  return item;
}

findName modifier1;
findName modifier2;

void setupDataHandling() {
  modifier1.item = getItemByChannel(0);
  modifier1.x = lcdWidth/4;
  modifier1.x = 0;
  modifier2.item = getItemByChannel(0);
  modifier2.x = lcdWidth/2;
}


void handleEncoderChange(uint8_t eIdx, bool doIncrement) {
  //Serial.println("change");
  //Serial.println(eIdx);
  if (doIncrement) {
    incrementParam(eIdx);
    return;
  }
  decrementParam(eIdx);
}

// TODO consider to have a dynamic split point!?
void handleEncoderPush(uint8_t eIdx, int holdTime) {
  if(holdTime < 800) {
    // short press: control different property
    loopEditPropertyIndex(eIdx);
    return;
  }
  if(holdTime > 4000) {
    // very long press: reboot
    drawReboot();
    delay(1000);
    reboot();
    return;
  }
  // long press: toggle layer/split mode
  currentMode = (lastLayerOrSplit == DUAL_SPLIT) ? MODE_DUAL_LAYER : MODE_DUAL_SPLIT;
  lastLayerOrSplit = (currentMode == MODE_DUAL_SPLIT) ? DUAL_SPLIT : DUAL_LAYER;
  
  if(currentMode > MODE_SINGLE) {
    drawModeLabel();
  }
}


// modify midi channel
void incrementModifierMidiChannel(uint8_t modIndex) {
  uint8_t currentMidiChannel;
  switch(modIndex) {
    case 0: incrementModifierMidiChannel(modifier1); return;
    case 1: incrementModifierMidiChannel(modifier2); return;
  }
}

void incrementModifierMidiChannel(struct findName &modifierItem) {
  uint8_t currentMidiChannel = modifierItem.item.midiCh;
  currentMidiChannel++;
  if(currentMidiChannel > 16) {
    currentMidiChannel = 16;
  }
  modifierItem.item = getItemByChannel(currentMidiChannel);
  checkChangeMode();
  if(currentMode == MODE_BYPASS || modifierItem.item.midiCh == 0) {
    return;
  }
  drawMidiChannel(modifierItem);
  // reset octave and transpose
  modifierItem.octave = 0;
  drawOctave(modifierItem);
  modifierItem.transpose = 0;
  drawTranspose(modifierItem);
}

void decrementModifierMidiChannel(uint8_t modIndex) {
  switch(modIndex) {
    case 0: decrementModifierMidiChannel(modifier1); return;
    case 1: decrementModifierMidiChannel(modifier2); return;
  }
}

void decrementModifierMidiChannel(struct findName &modifierItem) {
  uint8_t currentMidiChannel;
  currentMidiChannel = modifierItem.item.midiCh;
  if(currentMidiChannel > 0) {
    currentMidiChannel--;
  }
  modifierItem.item = getItemByChannel(currentMidiChannel);
  checkChangeMode();
  if(currentMode == MODE_BYPASS || modifierItem.item.midiCh == 0) {
    return;
  }
  drawMidiChannel(modifierItem);
  // reset octave and transpose
  modifierItem.octave = 0;
  drawOctave(modifierItem);
  modifierItem.transpose = 0;
  drawTranspose(modifierItem);
}



// modify octave
void incrementModifierOctave(uint8_t modIndex) {
  switch(modIndex) {
    case 0: incrementModifierOctave(modifier1); return;
    case 1: incrementModifierOctave(modifier2); return;
  }
}
void incrementModifierOctave(struct findName &modifierItem) {
    modifierItem.octave++;
    if(modifierItem.octave > MAX_OCTAVE) {
      modifierItem.octave = MAX_OCTAVE;
    }
    drawOctave(modifierItem);
}

void decrementModifierOctave(uint8_t modIndex) {
  switch(modIndex) {
    case 0: decrementModifierOctave(modifier1); return;
    case 1: decrementModifierOctave(modifier2); return;
  }
}
void decrementModifierOctave(struct findName &modifierItem) {
    modifierItem.octave--;
    if(modifierItem.octave < MIN_OCTAVE) {
      modifierItem.octave = MIN_OCTAVE;
    }
    drawOctave(modifierItem);
}


// modify semitone
void incrementModifierTranspose(uint8_t modIndex) {
  switch(modIndex) {
    case 0: incrementModifierTranspose(modifier1); return;
    case 1: incrementModifierTranspose(modifier2); return;
  }
}
void incrementModifierTranspose(struct findName &modifierItem) {
    modifierItem.transpose++;
    if(modifierItem.transpose > MAX_TRANSPOSE) {
      if(modifierItem.octave < MAX_OCTAVE) {
        incrementModifierOctave(modifierItem);
        modifierItem.transpose = 0;
      } else {
        modifierItem.transpose--;
      }
    }
    drawTranspose(modifierItem);
}

void decrementModifierTranspose(uint8_t modIndex) {
  switch(modIndex) {
    case 0: decrementModifierTranspose(modifier1); return;
    case 1: decrementModifierTranspose(modifier2); return;
  }
}

void decrementModifierTranspose(struct findName &modifierItem) {
    modifierItem.transpose--;
    if(modifierItem.transpose < MIN_TRANSPOSE) {
      decrementModifierOctave(modifierItem);
      modifierItem.transpose = 0;
    }
    drawTranspose(modifierItem);
}



void incrementParam(uint8_t modIndex) {
  switch(modIndex) {
    case 0: incrementParam(modifier1); return;
    case 1: incrementParam(modifier2); return;
  }
}

void incrementParam(struct findName &modifierItem) {
  switch(modifierItem.editProp) {
    case 0: incrementModifierMidiChannel(modifierItem); return;
    case 1: incrementModifierOctave(modifierItem); return;
    case 2: incrementModifierTranspose(modifierItem); return;
  }
}




void decrementParam(uint8_t modIndex) {
  switch(modIndex) {
    case 0: decrementParam(modifier1); return;
    case 1: decrementParam(modifier2); return;
  }
}

void decrementParam(struct findName &modifierItem) {
  switch(modifierItem.editProp) {
    case PROP_MIDI_CHANNEL: decrementModifierMidiChannel(modifierItem); return;
    case PROP_OCTAVE: decrementModifierOctave(modifierItem); return;
    case PROP_TRANSPOSE: decrementModifierTranspose(modifierItem); return;
  }
}


void loopEditPropertyIndex(uint8_t modIndex) {
  switch(modIndex) {
    case 0: loopEditPropertyIndex(modifier1); return;
    case 1: loopEditPropertyIndex(modifier2); return;
  }
}
void loopEditPropertyIndex(struct findName &modifierItem) {
  if(modifierItem.item.midiCh == 0) {
    return;
  }
  modifierItem.editProp++;
  if(modifierItem.editProp > 2) {
    modifierItem.editProp = 0;
  }
  if(modifierItem.item.midiCh > 0) {
    drawArrow(modifierItem);
  }
}


void checkChangeMode() {
  uint8_t modeBefore = currentMode;
  currentMode = MODE_BYPASS;
  bool needLeft = false;
  bool needRight = false;
  if (modifier1.item.midiCh > 0) {
    needLeft = true;
    currentMode = MODE_SINGLE;
    modifier1.x = lcdWidth/4;
    if (modifier2.item.midiCh > 0) {
      needRight = true;
      modifier1.x = 0;
      modifier2.x = lcdWidth/2;
      currentMode = (lastLayerOrSplit == DUAL_SPLIT) ? MODE_DUAL_SPLIT : MODE_DUAL_LAYER;
      lastLayerOrSplit = (currentMode == MODE_DUAL_SPLIT) ? DUAL_SPLIT : DUAL_LAYER;
    }
  }
  if (modeBefore == currentMode) {
    //debug("no mode change");
    return;
  }
  if (currentMode == MODE_BYPASS) {
    //debug("new mode bypass");
    drawBypass();
    modifier1.editProp = PROP_MIDI_CHANNEL;
    modifier2.editProp = PROP_MIDI_CHANNEL;
    return;
  }
  bool needFullRedraw = false;
  
  if (modeBefore == MODE_BYPASS) {
    needFullRedraw = true;
  }
  if (modeBefore > MODE_SINGLE && currentMode == MODE_SINGLE) {
    needFullRedraw = true;
    modifier2.editProp = PROP_MIDI_CHANNEL;
  }
  if (modeBefore == MODE_SINGLE && currentMode > MODE_SINGLE) {
    needFullRedraw = true;
  }
  
  
  if(needFullRedraw == true) {
    tft.fillRect(0, 0, lcdWidth, lcdHeight, DARKEST_GRAY);
    if (currentMode > MODE_SINGLE) {
      drawModeLabel();
    }
  }
  drawModifierItem(modifier1);
  if(currentMode > MODE_SINGLE) {
    drawModifierItem(modifier2);
  }
}


void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}
