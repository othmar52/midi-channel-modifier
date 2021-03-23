
// alternative wiring for debugging
//
// instead of using RX, TX pins of hardware serial use different pins
// so we are able to use the debug monitor of Arduino IDE
//#define USE_SOFTWARE_SERIAL_PIN_12_13


// thanks to https://github.com/FortySevenEffects/arduino_midi_library

#include <MIDI.h>
/*
 * -----------------------------------------------------------
 *                       note numbers
 *  octave |  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
 * -----------------------------------------------------------
 *    -2   |  0   1   2   3   4   5   6   7   8   9   10  11
 *    -1   |  12  13  14  15  16  17  18  19  20  21  22  23
 *     0   |  24  25  26  27  28  29  30  31  32  33  34  35
 *     1   |  36  37  38  39  40  41  42  43  44  45  46  47
 *     2   |  48  49  50  51  52  53  54  55  56  57  58  59
 *     3   |  60  61  62  63  64  65  66  67  68  69  70  71
 *     4   |  72  73  74  75  76  77  78  79  80  81  82  83
 *     5   |  84  85  86  87  88  89  90  91  92  93  94  95
 *     6   |  96  97  98  99  100 101 102 103 104 105 106 107
 *     7   |  108 109 110 111 112 113 114 115 116 117 118 119
 *     8   |  120 121 122 123 124 125 126 127
 */

/*
// Edirol PCR M-30 (32 keys)
uint8_t splitNote = 55;          // first note of upper split (G2) 
int8_t inNoteLowest = 41;        // lowest physical note on keyboard (without octave shift)
int8_t inNoteHighest = 72;       // highest physical note on keyboard (without octave shift)
int8_t splitRootNoteOffset = 12; // notes to pitch in dual split mode (up for left split, down for right split)
*/

// Doepfer LMK (88 keys)
uint8_t splitNote = 60;          // first note of upper split (C3)
int8_t inNoteLowest = 21;        // lowest physical note on keyboard (without octave shift)
int8_t inNoteHighest = 108;      // highest physical note on keyboard (without octave shift)
int8_t splitRootNoteOffset = 24; // notes to pitch in dual split mode (up for left split, down for right split)


#ifdef USE_SOFTWARE_SERIAL_PIN_12_13
#include <SoftwareSerial.h>
SoftwareSerial softSerial(12, 13); // RX, TX
MIDI_CREATE_INSTANCE(SoftwareSerial, softSerial, MIDI);
#else
MIDI_CREATE_DEFAULT_INSTANCE();
#endif



void setupMidi() {
#ifdef USE_SOFTWARE_SERIAL_PIN_12_13
  Serial.begin(115200);
  Serial.println("starting serial with debug monitor");
  Serial.println("MIDI RX/TX pins are 12/13");
#endif
  
  MIDI.begin(MIDI_CHANNEL_OMNI); // Launch MIDI, by default listening to all channels
  MIDI.turnThruOff(); // we have to avoid channel based events beeing sent thru

  // set callbacks for all different event types
  MIDI.setHandleClock(handleMidiEventClock);
  MIDI.setHandleTick(handleMidiEventClock);
  MIDI.setHandleStop(handleMidiEventStop);
  MIDI.setHandleStart(handleMidiEventStart);
  MIDI.setHandleContinue(handleMidiEventContinue);
  MIDI.setHandleNoteOn(handleMidiEventNoteOn);
  MIDI.setHandleNoteOff(handleMidiEventNoteOff);
  MIDI.setHandleAfterTouchPoly(handleMidiEventAfterTouchPoly);
  MIDI.setHandleAfterTouchChannel(handleMidiEventAfterTouchChannel);
  MIDI.setHandleProgramChange(handleMidiEventProgramChange);
  MIDI.setHandleControlChange(handleMidiEventControlChange);
  MIDI.setHandlePitchBend(handleMidiEventPitchBend);
  MIDI.setHandleSystemExclusive(handleMidiEventSysEx);
  MIDI.setHandleTimeCodeQuarterFrame(handleMidiEventTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(handleMidiEventSongPosition);
  MIDI.setHandleSongSelect(handleMidiEventSongSelect);
  MIDI.setHandleTuneRequest(handleMidiEventTuneRequest);
  MIDI.setHandleSystemReset(handleMidiEventSystemReset);

}

void loopMidi() {
  MIDI.read();
}

void modifyChannel(byte &theChannel, struct findName modifierItem) {
    theChannel = modifierItem.item.midiCh;
}
void modifyChannelAndNote(byte &theChannel, byte &theNoteNumber, struct findName modifierItem) {
    modifyChannel(theChannel, modifierItem);
    modifyNote(theNoteNumber, modifierItem);
}
void modifyNote(byte &theNoteNumber, struct findName modifierItem) {
    if (modifierItem.transpose != 0) {
      // only apply transpose if we do not exceed 0-127
      if(theNoteNumber + modifierItem.transpose <= 127 && theNoteNumber + modifierItem.transpose >= 0) {
        theNoteNumber += modifierItem.transpose;
      }
    }
    if (modifierItem.octave != 0) {
      // only apply octave if we do not exceed 0-127
      for(uint8_t i=0; i< abs(modifierItem.octave); i++) {
        if(modifierItem.octave > 0 && theNoteNumber + 12 <= 127) {
          theNoteNumber += 12;
        }
        if(modifierItem.octave < 0 && theNoteNumber - 12 >= 0) {
          theNoteNumber -= 12;
        }
      }
    }
}

void handleGenericMidiEventWithChannelAndPitch(byte inType, byte inPitch, byte inValue2, byte inChannel) {
  byte vanillaChannel = inChannel;
  byte vanillaPitch = inPitch;
  switch(currentMode) {
    case MODE_BYPASS:
      MIDI.send(inType, inPitch, inValue2, inChannel);
      return;
    case MODE_SINGLE:
      modifyChannelAndNote(inChannel, inPitch, modifier1);
      MIDI.send(inType, inPitch, inValue2, inChannel);
      return;
    case MODE_DUAL_LAYER:
      modifyChannelAndNote(inChannel, inPitch, modifier1);
      MIDI.send(inType, inPitch, inValue2, inChannel);
      modifyChannelAndNote(vanillaChannel, vanillaPitch, modifier2);
      MIDI.send(inType, vanillaPitch, inValue2, vanillaChannel);
      return;
    case MODE_DUAL_SPLIT:
      // is left or right split?
      if(inPitch < splitNote) {
        inPitch += splitRootNoteOffset;
        modifyChannelAndNote(inChannel, inPitch, modifier1);
        MIDI.send(inType, inPitch, inValue2, inChannel);
        return;
      }
      inPitch -= splitRootNoteOffset;
      modifyChannelAndNote(inChannel, inPitch, modifier2);
      MIDI.send(inType, inPitch, inValue2, inChannel);
      return;
  }
}


// TODO: does it makes sense to remember all open notes
// to avoid stuck notes on channel change??
void handleMidiEventNoteOn(byte inChannel, byte inPitch, byte inVelocity) {
  handleGenericMidiEventWithChannelAndPitch(midi::NoteOn, inPitch, inVelocity, inChannel);
}
void handleMidiEventNoteOff(byte inChannel, byte inPitch, byte inVelocity) {
  handleGenericMidiEventWithChannelAndPitch(midi::NoteOff, inPitch, inVelocity, inChannel);
}

void handleMidiEventAfterTouchPoly(byte inChannel, byte inNote, byte inValue) {
    //MIDI.sendAfterTouch(inNote, inValue, inChannel);
    handleGenericMidiEventWithChannelAndPitch(midi::AfterTouchPoly, inNote, inValue, inChannel);
}

// TODO: does it really make sense to modify channel of control change messages?
// TODO: does it really make sense to fire twice in DUAL_LAYER mode?
//  consider to limit the channel modification to MOD-Wheel/CC1
void handleMidiEventControlChange(byte inChannel, byte inControlNumber, byte inControlValue) {
  switch(currentMode) {
    case MODE_BYPASS: MIDI.sendControlChange(inControlNumber, inControlValue, inChannel); return;
    case MODE_SINGLE: MIDI.sendControlChange(inControlNumber, inControlValue, modifier1.item.midiCh);return;
    default:
      MIDI.sendControlChange(inControlNumber, inControlValue, modifier1.item.midiCh);
      MIDI.sendControlChange(inControlNumber, inControlValue, modifier2.item.midiCh);
  }
}

// TODO: does it really make sense to modify channel of program change messages?
// TODO: does it really make sense to fire twice in DUAL_LAYER mode?
void handleMidiEventProgramChange(byte inChannel, byte inProgramNumber) {
  switch(currentMode) {
    case MODE_BYPASS: MIDI.sendProgramChange(inProgramNumber, inChannel); return;
    case MODE_SINGLE: MIDI.sendProgramChange(inProgramNumber, modifier1.item.midiCh);return;
    default:
      MIDI.sendProgramChange(inProgramNumber, modifier1.item.midiCh);
      MIDI.sendProgramChange(inProgramNumber, modifier2.item.midiCh);
  }
}

void handleMidiEventAfterTouchChannel(byte inChannel, byte inPressure) {
  //MIDI.sendAfterTouch(inPressure, inChannel);
  switch(currentMode) {
    case MODE_BYPASS: MIDI.sendAfterTouch(inPressure, inChannel); return;
    case MODE_SINGLE: MIDI.sendAfterTouch(inPressure, modifier1.item.midiCh);return;
    default:
      MIDI.sendAfterTouch(inPressure, modifier1.item.midiCh);
      MIDI.sendAfterTouch(inPressure, modifier2.item.midiCh);
  }
}

void handleMidiEventPitchBend(byte inChannel, int inPitchValue) {
  switch(currentMode) {
    case MODE_BYPASS: MIDI.sendPitchBend(inPitchValue, inChannel); return;
    case MODE_SINGLE: MIDI.sendPitchBend(inPitchValue, modifier1.item.midiCh);return;
    default:
      MIDI.sendPitchBend(inPitchValue, modifier1.item.midiCh);
      MIDI.sendPitchBend(inPitchValue, modifier2.item.midiCh);
  }
}

void handleMidiEventSysEx(byte* inData, unsigned inSize)
{
    MIDI.sendSysEx(inSize, inData, true);
}

void handleMidiEventTimeCodeQuarterFrame(byte inData)
{
    MIDI.sendTimeCodeQuarterFrame(inData);
}

void handleMidiEventSongPosition(unsigned inBeats)
{
    MIDI.sendSongPosition(inBeats);
}

void handleMidiEventSongSelect(byte inSongNumber)
{
    MIDI.sendSongSelect(inSongNumber);
}
void handleMidiEventTuneRequest()
{
    MIDI.sendTuneRequest();
}

void handleMidiEventSystemReset() {
  MIDI.sendSystemReset();
}

void handleMidiEventClock() {
  MIDI.sendClock();
}

void handleMidiEventStart() {
  MIDI.sendStart();
}

void handleMidiEventStop() {
  MIDI.sendStop();
}

void handleMidiEventContinue() {
  MIDI.sendContinue();
}
