#include <MIDI.h>
//Midi library for Arduino
// needs to be added to library folder.
MIDI_CREATE_DEFAULT_INSTANCE();

// -----------------------------------------------------------------------------

void setup()
{
  Serial.begin(31250);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  // RX pin is on pin 15
}

void loop()
{
    if (MIDI.read()) // if a MIDI message is received
    {
        switch(MIDI.getType())      // Get the type of the message we received
        {
          //-------------------------------------------------------//
          //--------MESSAGE------/-------DATA1-------/----DATA2----//
          //        Note On      /     Note number   /   Velocity  //
          //        Note Off     /     Note number   /   Velocity  //
          //Polyphonic Aftertouch/     Note number   /   pressure  //
          //     Control change  / Controller number /     Data    //
          //     Program change  /   Program number  /     ---     //
          //  Channel Aftertouch /     Pressure      /     ---     //
          //      Pitch wheel    /        LSB        /     MSB     //
          //-------------------------------------------------------//
         
          case midi::ProgramChange:
            Serial.println("program change");
            break;
          case midi::NoteOn:
            Serial.print("note on : ");
            Serial.print(MIDI.getData1());
            Serial.print("   velocity : ");
            Serial.println(MIDI.getData2());
          default:
              break;
        }
    }
}