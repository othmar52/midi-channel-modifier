 

uint8_t itemRectWidth = 0;
uint8_t itemRectHeight = 0;
uint8_t itemRectOffsetLeft = 0;
uint8_t itemRectOffsetTop = 0;

void setupRendering() {
    drawBypass();
}

void drawBypass() {
  tft.fillRect(0, 0, lcdWidth, lcdHeight, DARKEST_GRAY);
  tft.setCursor(0, 100);
  tft.setTextColor(GRAY);
  tft.setTextSize(2);
  tft.println("      CHANNEL MODIFIER");
  tft.println("   ");
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.println("     DISABLED");
}

void drawReboot() {
  tft.fillRect(0, 0, lcdWidth, lcdHeight, DARKEST_GRAY);
  tft.setCursor(0, 100);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.println("   REBOOT");
}

void loopRendering() {

}

void drawModeLabel() {
  itemRectWidth = 80;
  itemRectHeight = 23;
  itemRectOffsetLeft = (lcdWidth - itemRectWidth)/2;
  itemRectOffsetTop = 2;
  tft.fillRect(itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(itemRectOffsetLeft + 12, itemRectOffsetTop + 3);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println((currentMode == MODE_DUAL_SPLIT) ? "SPLIT" : "LAYER");
}



void drawModifierItem(struct findName item) {
  //tft.fillRect(item.x, 0, lcdWidth/2, lcdHeight, DARKEST_GRAY);
  drawMidiChannel(item);
  drawOctave(item);
  drawTranspose(item);
  drawArrow(item);
}


void drawArrow(struct findName item) {

  tft.setTextColor(WHITE);
  tft.setTextSize(2);

  itemRectWidth = 12;
  itemRectHeight = 14;
  itemRectOffsetLeft = 20;
  itemRectOffsetTop = 145;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft, itemRectOffsetTop);
  if (item.editProp == 1) {
    tft.println(">");
  }
  
  itemRectOffsetLeft = lcdWidth/2 - itemRectWidth - 20;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft, itemRectOffsetTop);
  if (item.editProp == 1) {
    tft.println("<");
  }


  itemRectOffsetTop = 185;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft, itemRectOffsetTop);
  if (item.editProp == 2) {
    tft.println("<");
  }
  itemRectOffsetLeft = 20;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft, itemRectOffsetTop);
  if (item.editProp == 2) {
    tft.println(">");
  }
}

void drawMidiChannel(struct findName item) {
  itemRectWidth = 66;
  itemRectHeight = 43;
  itemRectOffsetLeft = (lcdWidth/2 - itemRectWidth)/2;
  itemRectOffsetTop = 40;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft + ((item.item.midiCh > 9) ? 0 : itemRectWidth*0.3), itemRectOffsetTop);
  tft.setTextColor(WHITE);
  tft.setTextSize(6);
  tft.println(item.item.midiCh);

  
  itemRectWidth = 130;
  itemRectHeight = 23;
  itemRectOffsetLeft = (lcdWidth/2 - itemRectWidth)/2;
  itemRectOffsetTop = 90;
  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, item.item.color);
  tft.setCursor(item.x + itemRectOffsetLeft + 12, itemRectOffsetTop + 3);
  tft.setTextColor(item.item.txtColor );
  tft.setTextSize(2);
  tft.println(item.item.label);
  
}



void drawOctave(struct findName item) {
  itemRectWidth = 45;
  itemRectHeight = 28;
  itemRectOffsetLeft = (lcdWidth/2 - itemRectWidth)/2;
  itemRectOffsetTop = 140;

  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft + ((item.octave == 0) ? itemRectWidth*0.3 : 0), itemRectOffsetTop);
  tft.setTextColor((item.octave == 0) ? GRAY : WHITE);
  tft.setTextSize(4);
  if (item.octave <= 0) {
    tft.println(item.octave);
    return;
  }
  tft.println("+" + String(item.octave));

}




void drawTranspose(struct findName item) {
  itemRectWidth = 68;
  itemRectHeight = 28;
  itemRectOffsetLeft = (lcdWidth/2 - itemRectWidth)/2;
  itemRectOffsetTop = 180;

  uint8_t additionalOffsetLeft = 0;
  if(abs(item.transpose) < 10) {
    additionalOffsetLeft = itemRectWidth*0.2;
  }
  if(item.transpose == 0) {
    additionalOffsetLeft = itemRectWidth*0.37;
  }

  tft.fillRect(item.x + itemRectOffsetLeft, itemRectOffsetTop, itemRectWidth, itemRectHeight, DARKEST_GRAY);
  tft.setCursor(item.x + itemRectOffsetLeft + additionalOffsetLeft, itemRectOffsetTop);
  tft.setTextColor((item.transpose == 0) ? GRAY : WHITE);
  tft.setTextSize(4);
  if (item.transpose <= 0) {
    tft.println(item.transpose);
    return;
  }
  tft.println("+" + String(item.transpose));
}
