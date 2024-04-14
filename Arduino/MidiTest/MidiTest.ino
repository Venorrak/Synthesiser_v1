#include <MIDI.h>
#include "DaisyDuino.h"
#include <cmath>
#include "Arduino.h"
//Midi library for Arduino
// needs to be added to library folder.
MIDI_CREATE_DEFAULT_INSTANCE();

#define MAX_DELAY static_cast<size_t>(48000 * 0.75f)

enum Wave {
  WAVE_SIN,
  WAVE_TRI,
  WAVE_SAW,
  WAVE_RAMP,
  WAVE_SQUARE,
  WAVE_POLYBLEP_TRI,
  WAVE_POLYBLEP_SAW,
  WAVE_POLYBLEP_SQUARE,
  WAVE_LAST,
};

enum State { UP,
             CENTER,
             DOWN };
// -----------------------------------------------------------------------------
float notes[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
float osc1_out[10];
float osc2_out[10];
float osc3_out[10];
float sample_rate;
bool gate;
unsigned long previousMillis = 0;
float ampOsc1;
float ampOsc2;
float envAttack;
float envDecay;
float sustain;
float tickFrequency;
float cutOffFreq;
int osc1WavePinR = 27;
int osc1WavePinL = 26;
int osc2WavePinR = 1;
int osc2WavePinL = 0;
int filterPinR = 3;
int filterPinL = 2;
int lfoWavePinR = 5;
int lfoWavePinL = 4;
int lfoValPinR = 7;
int lfoValPinL = 6;
int osc1OctavePinR = 9;
int osc1OctavePinL = 8;
bool rState = false;
bool lState = false;
int tune;
int delay_osc;
float env_out;
float ampLfo;
float lfoFreq;

DaisyHardware hw;
Oscillator osc1[10];
Oscillator osc2[10];
Oscillator osc3[10];
Oscillator lfo;
static Adsr env, filterEnv, lfoEnv;
static Metro tick;
static ATone hpFilter;
static Tone lpFilter;
static DelayLine<float, MAX_DELAY> del;

void cutOffFrequency() {
  cutOffFreq = convertValue(analogRead(A6), 0, 1023, 0.0f, 2093.0f);

  if (isLfoCutOff()) {
    cutOffFreq = ((cutOffFreq * lfo.Process()) * 2);

    if (cutOffFreq < 0) {
      cutOffFreq = -1 * cutOffFreq;
    }
  }
}

void readPotentiometer(unsigned long currentMillis) {
  if (currentMillis - previousMillis < 200) {
    return;
  }
  previousMillis = currentMillis;
  ampOsc1 = convertValue(analogRead(A5), 0, 1023, 0.0, 1.0);
  ampOsc2 = convertValue(analogRead(A6), 0, 1023, 0.0, 1.0);
  envAttack = convertValue(analogRead(A1), 0, 1023, 1.0, 5.0);
  envDecay = convertValue(analogRead(A7), 0, 1023, 1.0, 5.0);
  sustain = convertValue(analogRead(A8), 0, 1023, 0.0, 1.0);
  tickFrequency = convertValue(analogRead(A9), 0, 1023, 1.0, 5.0);
  tune = int(convertValue(analogRead(A4), 0, 1023, 0, 12));
  ampLfo = convertValue(analogRead(A2), 0, 1023, 0.0f, 1.0f);
  lfoFreq = convertValue(analogRead(A3), 0, 1023, 0.0f, 1.0f);
  cutOffFrequency();
  delay_osc = int(convertValue(analogRead(A0), 0, 1023, 0.0f, 80.0f));
}

bool isLfoAmp() {
  switch (readSwitch3(lfoValPinR, lfoValPinL)) {
    case DOWN:
      return true;
      break;

    default:
      return false;
      break;
  }
}

bool isLfoFreq() {
  switch (readSwitch3(lfoValPinR, lfoValPinL)) {
    case UP:
      Serial.println("UP LFO OSC");
      return true;
      break;

    default:
      return false;
      break;
  }
}

bool isLfoCutOff() {
  switch (readSwitch3(lfoValPinR, lfoValPinL)) {
    case CENTER:
      return true;
      break;

    default:
      return false;
      break;
  }
}

int ftom(float freq) {
  // MIDI Value Calculation
  float midiValue = 12 * log2(freq / 440.0) + 69;

  // Round the MIDI value to the nearest integer
  int midiNote = int((round(midiValue)));

  return midiNote;
}

// midiMin & midiMax values ​​that determine the maximum that can reach and the minimum that it can reach
int transposeOctave(float freq, int octaveDifference) {
  int midiNote = ftom(freq);

  // If octaveDifference = 1 we go down an octave and if it is -1 we go up an octave
  midiNote = midiNote - (octaveDifference * 12);
  return midiNote;
}

float getSubFreq(float freq) {
  // 21 and 108 represent the lowest and the higher he can reach
  int midiNote = transposeOctave(freq, 1);

  return mtof(midiNote);
}

void setOscAmp() {
  float newAmpOsc1 = ampOsc1 * env_out;
  float newAmpOsc2 = ampOsc2 * env_out;
  float newAmpLfo = ampLfo * env_out;

  if (isLfoAmp()) {
    newAmpLfo = ampLfo;
    newAmpOsc1 = ((lfo.Process() * ampOsc1) * 2);
    newAmpOsc2 = ((lfo.Process() * ampOsc2) * 2);
    if (newAmpOsc1 < 0) {
      newAmpOsc1 = -1 * newAmpOsc1;
    }

    if (newAmpOsc2 < 0) {
      newAmpOsc2 = -1 * newAmpOsc2;
    }
  }

  for (int i = 0; i < 10; i++) {
    if (notes[i] != 0) {
      osc1[i].SetAmp(newAmpOsc1);
      osc2[i].SetAmp(newAmpOsc2);
      osc3[i].SetAmp(newAmpOsc1);
    }
  }
}

float convertValue(int x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setupSwitch3() {
  pinMode(osc1WavePinR, INPUT_PULLUP);
  pinMode(osc1WavePinL, INPUT_PULLUP);
  pinMode(osc2WavePinR, INPUT_PULLUP);
  pinMode(osc2WavePinL, INPUT_PULLUP);
  pinMode(filterPinR, INPUT_PULLUP);
  pinMode(filterPinL, INPUT_PULLUP);
  pinMode(lfoWavePinR, INPUT_PULLUP);
  pinMode(lfoWavePinL, INPUT_PULLUP);
  pinMode(lfoValPinR, INPUT_PULLUP);
  pinMode(lfoValPinL, INPUT_PULLUP);
  pinMode(osc1OctavePinR, INPUT_PULLUP);
  pinMode(osc1OctavePinL, INPUT_PULLUP);
}

int readSwitch3(int rPin, int lPin) {
  bool rState = digitalRead(rPin);
  bool lState = digitalRead(lPin);

  State state;

  if ((rState == LOW) && (lState == HIGH))  //test for right position
  {
    state = UP;
  } else if ((rState == HIGH) && (lState == HIGH))  //test for center position
  {
    state = CENTER;
  } else if ((rState == HIGH) && (lState == LOW))  //test for left position
  {
    state = DOWN;
  }

  return state;
}

void changeWaveForm() {
  Wave waveOsc1;
  Wave waveOsc2;

  switch (readSwitch3(osc1WavePinR, osc1WavePinL)) {
    case UP:
      waveOsc1 = WAVE_SQUARE;
      break;
    case CENTER:
      waveOsc1 = WAVE_POLYBLEP_SAW;
      break;
    case DOWN:
      waveOsc1 = WAVE_TRI;
      break;
  }

  switch (readSwitch3(lfoWavePinR, lfoWavePinL)) {
    case UP:
      lfo.SetWaveform(lfo.WAVE_SQUARE);
      break;
    case CENTER:
      lfo.SetWaveform(lfo.WAVE_POLYBLEP_SAW);
      break;
    case DOWN:
      lfo.SetWaveform(lfo.WAVE_TRI);
      break;
  }

  switch (readSwitch3(osc2WavePinR, osc2WavePinL)) {
    case UP:
      waveOsc2 = WAVE_SQUARE;
      break;
    case CENTER:
      waveOsc2 = WAVE_POLYBLEP_SAW;
      break;
    case DOWN:
      waveOsc2 = WAVE_TRI;
      break;
  }

  for (int i = 0; i < 10; i++) {
    osc1[i].SetWaveform(waveOsc1);
    osc2[i].SetWaveform(waveOsc2);
    osc3[i].SetWaveform(waveOsc1);
  }
}

float frequencyLfo(float freqOsc) {
  float freq = ((lfo.Process() * freqOsc) * 2);

  if (freq < 0) {
    freq = -1 * freq;
  }

  return freq;
}

void setOscFreq(float freq, int index) {
  int midiNote;
  int tuneMidiNote;
  float freqOsc1;
  float freqOsc2;

  if (isLfoFreq()) {
    freq = frequencyLfo(freq);
  }

  // frequence up to 1 octave for second oscillator
  switch (readSwitch3(osc1OctavePinR, osc1OctavePinL)) {
    case UP:
      // 33 and 108 represent the lowest and the higher he can reach
      midiNote = transposeOctave(freq, -1);
      freqOsc1 = mtof(midiNote);
      break;
    case CENTER:
      freqOsc1 = freq;
      break;
    case DOWN:
      // 33 and 108 represent the lowest and the higher he can reach
      midiNote = transposeOctave(freq, 1);
      freqOsc1 = mtof(midiNote);
      break;
  }

  switch (readSwitch3(osc2WavePinR, osc2WavePinL)) {
    case UP:
      // 21 and 96 represent the lowest and the higher he can reach
      midiNote = transposeOctave(freq, -1);
      tuneMidiNote = midiNote + tune;
      freqOsc2 = mtof(tuneMidiNote);
      break;
    case CENTER:
      midiNote = ftom(freq);
      tuneMidiNote = midiNote + tune;
      freqOsc2 = mtof(tuneMidiNote);
      break;
    case DOWN:
      // 21 and 96 represent the lowest and the higher he can reach
      midiNote = transposeOctave(freq, 1);
      tuneMidiNote = midiNote + tune;
      freqOsc2 = mtof(tuneMidiNote);
      break;
  }
  osc1[index].SetFreq(freqOsc1);
  osc2[index].SetFreq(freqOsc2);
  osc3[index].SetFreq(getSubFreq(freqOsc1));
}

// Update filter with switch 3 positions
float changeFilter(float sig_out) {
  float output;
  switch (readSwitch3(filterPinR, filterPinL)) {
    case UP:
      hpFilter.SetFreq(cutOffFreq);
      // *sig_out = hpFilter.Process(*sig_out) * filterEnv.Process(gate);
      output = hpFilter.Process(sig_out);
      break;
    case CENTER: output = sig_out; break;
    case DOWN:
      lpFilter.SetFreq(cutOffFreq);
      // *sig_out = lpFilter.Process(*sig_out) * filterEnv.Process(gate);
      output = lpFilter.Process(sig_out);
      break;
  }
  return output;
}

bool setDelay() {
  switch (delay_osc) {
    case 0 ... 9:
      return false;
      break;

    case 10 ... 14:
      del.SetDelay(sample_rate * 0.10f);
      break;

    case 15 ... 19:
      del.SetDelay(sample_rate * 0.15f);
      break;

    case 20 ... 24:
      del.SetDelay(sample_rate * 0.20f);
      break;

    case 25 ... 29:
      del.SetDelay(sample_rate * 0.25f);
      break;

    case 30 ... 34:
      del.SetDelay(sample_rate * 0.30f);
      break;

    case 35 ... 39:
      del.SetDelay(sample_rate * 0.35f);
      break;

    case 40 ... 44:
      del.SetDelay(sample_rate * 0.40f);
      break;

    case 45 ... 49:
      del.SetDelay(sample_rate * 0.45f);
      break;

    case 50 ... 54:
      del.SetDelay(sample_rate * 0.50f);
      break;

    case 55 ... 59:
      del.SetDelay(sample_rate * 0.55f);
      break;

    case 60 ... 64:
      del.SetDelay(sample_rate * 0.60f);
      break;

    case 65 ... 69:
      del.SetDelay(sample_rate * 0.65f);
      break;

    case 70 ... 74:
      del.SetDelay(sample_rate * 0.70f);
      break;

    case 75 ... 80:
      del.SetDelay(sample_rate * 0.75f);
      break;
  }

  return true;
}

void oscillatorsOut() {
  osc1_out[0] = (notes[0] == 0) ? 0 : osc1[0].Process();
  osc1_out[1] = (notes[1] == 0) ? 0 : osc1[1].Process();
  osc1_out[2] = (notes[2] == 0) ? 0 : osc1[2].Process();
  osc1_out[3] = (notes[3] == 0) ? 0 : osc1[3].Process();
  osc1_out[4] = (notes[4] == 0) ? 0 : osc1[4].Process();
  osc1_out[5] = (notes[5] == 0) ? 0 : osc1[5].Process();
  osc1_out[6] = (notes[6] == 0) ? 0 : osc1[6].Process();
  osc1_out[7] = (notes[7] == 0) ? 0 : osc1[7].Process();
  osc1_out[8] = (notes[8] == 0) ? 0 : osc1[8].Process();
  osc1_out[9] = (notes[9] == 0) ? 0 : osc1[9].Process();
  osc2_out[0] = (notes[0] == 0) ? 0 : osc2[0].Process();
  osc2_out[1] = (notes[1] == 0) ? 0 : osc2[1].Process();
  osc2_out[2] = (notes[2] == 0) ? 0 : osc2[2].Process();
  osc2_out[3] = (notes[3] == 0) ? 0 : osc2[3].Process();
  osc2_out[4] = (notes[4] == 0) ? 0 : osc2[4].Process();
  osc2_out[5] = (notes[5] == 0) ? 0 : osc2[5].Process();
  osc2_out[6] = (notes[6] == 0) ? 0 : osc2[6].Process();
  osc2_out[7] = (notes[7] == 0) ? 0 : osc2[7].Process();
  osc2_out[8] = (notes[8] == 0) ? 0 : osc2[8].Process();
  osc2_out[9] = (notes[9] == 0) ? 0 : osc2[9].Process();
  osc3_out[0] = (notes[0] == 0) ? 0 : osc3[0].Process();
  osc3_out[1] = (notes[1] == 0) ? 0 : osc3[1].Process();
  osc3_out[2] = (notes[2] == 0) ? 0 : osc3[2].Process();
  osc3_out[3] = (notes[3] == 0) ? 0 : osc3[3].Process();
  osc3_out[4] = (notes[4] == 0) ? 0 : osc3[4].Process();
  osc3_out[5] = (notes[5] == 0) ? 0 : osc3[5].Process();
  osc3_out[6] = (notes[6] == 0) ? 0 : osc3[6].Process();
  osc3_out[7] = (notes[7] == 0) ? 0 : osc3[7].Process();
  osc3_out[8] = (notes[8] == 0) ? 0 : osc3[8].Process();
  osc3_out[9] = (notes[9] == 0) ? 0 : osc3[9].Process();
}

float osc1Out() {
  return osc1_out[0] + osc1_out[1] + osc1_out[2] + osc1_out[3] + osc1_out[4] + osc1_out[5] + osc1_out[6] + osc1_out[7] + osc1_out[8] + osc1_out[9];
}

float osc2Out() {
  return osc2_out[0] + osc2_out[1] + osc2_out[2] + osc2_out[3] + osc2_out[4] + osc2_out[5] + osc2_out[6] + osc2_out[7] + osc2_out[8] + osc2_out[9];
}

float osc3Out() {
  return osc3_out[0] + osc3_out[1] + osc3_out[2] + osc3_out[3] + osc3_out[4] + osc3_out[5] + osc3_out[6] + osc3_out[7] + osc3_out[8] + osc3_out[9];
}

void MyCallback(float **in, float **out, size_t size) {
  float del_out, sig_out, feedback, output;
  for (size_t i = 0; i < size; i++) {

    // When the metro ticks, trigger the envelope to start.
    if (tick.Process()) {
      gate = !gate;
    }

    // Update tick frequency
    tick.SetFreq(tickFrequency);

    // // Update envelope
    env.SetTime(ADSR_SEG_ATTACK, envAttack);
    env.SetTime(ADSR_SEG_DECAY, envDecay);
    env.SetSustainLevel(sustain);

    env_out = env.Process(gate);

    oscillatorsOut();

    del_out = del.Read();

    feedback = (del_out * 0.75f) + osc1Out() + osc2Out() + osc3Out();

    del.Write(feedback);

    bool isDelay = setDelay();

    if (!isDelay) {
      sig_out = osc1Out() + osc2Out() + osc3Out();
    } else {
      sig_out = del_out + osc1Out() + osc2Out() + osc3Out();
    }

    output = changeFilter(sig_out);

    OUT_L[i] = output;
    OUT_R[i] = output;
  }
}

void setup() {
  setupSwitch3();
  Serial.begin(31250);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  // RX pin is on pin 15

  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  for (int i = 0; i < 10; i++) {
    osc1[i].Init(sample_rate);
    osc1[i].SetFreq(440);
    osc1[i].SetAmp(1);
    osc1[i].SetWaveform(Oscillator::WAVE_TRI);
    osc2[i].Init(sample_rate);
    osc2[i].SetFreq(440);
    osc2[i].SetAmp(1);
    osc2[i].SetWaveform(Oscillator::WAVE_TRI);
    osc3[i].Init(sample_rate);
    osc3[i].SetFreq(440);
    osc3[i].SetAmp(1);
    osc3[i].SetWaveform(Oscillator::WAVE_TRI);
  }
  lfo.Init(sample_rate);
  tick.Init(1.0, sample_rate);
  env.Init(sample_rate);
  hpFilter.Init(sample_rate);
  lpFilter.Init(sample_rate);

  env.SetTime(ADSR_SEG_ATTACK, 0.1);
  env.SetTime(ADSR_SEG_DECAY, 0.1);
  env.SetSustainLevel(0.25);

  lfo.SetFreq(0.2);
  lfo.SetFreq(lfo.WAVE_TRI);

  del.Init();
  del.SetDelay(sample_rate * 0.75f);

  DAISY.begin(MyCallback);
}

void loop() {
  unsigned long currentMillis = millis();
  readPotentiometer(currentMillis);
  if (MIDI.read())  // if a MIDI message is received
  {
    switch (MIDI.getType())  // Get the type of the message we received
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
        Serial.println(MIDI.getData1());
        // Serial.print("   velocity : ");
        // Serial.println(MIDI.getData2());

        if (notes[0] == 0) {
          notes[0] = mtof(MIDI.getData1());
        } else if (notes[1] == 0) {
          notes[1] = mtof(MIDI.getData1());
        } else if (notes[2] == 0) {
          notes[2] = mtof(MIDI.getData1());
        } else if (notes[3] == 0) {
          notes[3] = mtof(MIDI.getData1());
        } else if (notes[4] == 0) {
          notes[4] = mtof(MIDI.getData1());
        } else if (notes[5] == 0) {
          notes[5] = mtof(MIDI.getData1());
        } else if (notes[6] == 0) {
          notes[6] = mtof(MIDI.getData1());
        } else if (notes[7] == 0) {
          notes[7] = mtof(MIDI.getData1());
        } else if (notes[8] == 0) {
          notes[8] = mtof(MIDI.getData1());
        } else if (notes[9] == 0) {
          notes[9] = mtof(MIDI.getData1());
        }
        break;
      case midi::NoteOff:
        for (int i = 0; i < sizeof(notes); i++) {
          if (notes[i] == mtof(MIDI.getData1())) {
            Serial.println("Trouve!");
            notes[i] = 0;
          }
        }
        break;
    }
  }
  for (int i = 0; i < 10; i++) {
    if (notes[i] != 0) {
      setOscFreq(notes[i], i);
    }
  }
  setOscAmp();
  changeWaveForm();
  lfo.SetFreq(lfoFreq);
}
