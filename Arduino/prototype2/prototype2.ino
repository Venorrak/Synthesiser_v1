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

enum State {
  UP,
  CENTER,
  DOWN 
};
State switchOctOsc1 = CENTER;
State switchOctOsc2 = CENTER;
State switchWaveOsc1 = CENTER;
State switchWaveOsc2 = CENTER;
State switchFilter = CENTER;
State switchLfoWave = CENTER;
State switchLfoValue = CENTER;

unsigned long previousMillisPot = 0;
unsigned long previousMillisSwitch = 0;
unsigned long previousMillisWave = 0;
float notes[10];
float osc1_out[10];
float osc2_out[10];
float osc3_out[10];
int osc1WavePinU = 7;
int osc1WavePinD = 6;
int osc2WavePinU = 9;
int osc2WavePinD = 8;
int filterPinU = 4;
int filterPinD = 5;
int lfoWavePinU = 0;
int lfoWavePinD = 1;
int lfoValPinU = 3;
int lfoValPinD = 2;
int osc1OctPinU = 13;
int osc1OctPinD = 12;
int osc2OctPinU = 11;
int osc2OctPinD = 10;
int sourcePinU = 26;
int sourcePinD = 27;
int tune;
float ampOsc1;
float ampOsc2;
float ampOsc3;
float ampLfo;
float freqLfo;
int delayOsc;
float cutOffFreq;
float sample_rate;

DaisyHardware hw;
Oscillator osc1[10];
Oscillator osc2[10];
Oscillator osc3[10];
Oscillator lfo;
static Metro tick;
static ATone hpFilter;
static Tone lpFilter;
static DelayLine<float, MAX_DELAY> del;

int ftom(float freq) {
  // MIDI Value Calculation
  float midiValue = 12 * log2(freq / 440.0) + 69;

  // Round the MIDI value to the nearest integer
  int midiNote = int((round(midiValue)));

  return midiNote;
}

int transposeOctave(float freq, int octaveDifference) {
  int midiNote = ftom(freq);

  // If octaveDifference = 1 we go down an octave and if it is -1 we go up an octave
  midiNote = midiNote - (octaveDifference * 12);
  return midiNote;
}

float convertValue(int x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setupSwitch() {
  pinMode(osc1WavePinU, INPUT_PULLUP);
  pinMode(osc1WavePinD, INPUT_PULLUP);
  pinMode(osc2WavePinU, INPUT_PULLUP);
  pinMode(osc2WavePinD, INPUT_PULLUP);
  pinMode(filterPinU, INPUT_PULLUP);
  pinMode(filterPinD, INPUT_PULLUP);
  pinMode(lfoWavePinU, INPUT_PULLUP);
  pinMode(lfoWavePinD, INPUT_PULLUP);
  pinMode(lfoValPinU, INPUT_PULLUP);
  pinMode(lfoValPinD, INPUT_PULLUP);
  pinMode(osc1OctPinU, INPUT_PULLUP);
  pinMode(osc1OctPinD, INPUT_PULLUP);
  pinMode(osc2OctPinU, INPUT_PULLUP);
  pinMode(osc2OctPinD, INPUT_PULLUP);
  pinMode(sourcePinU, INPUT_PULLUP);
  pinMode(sourcePinD, INPUT_PULLUP);
}

void readPotentiometer(unsigned long currentMillis) {
  if (currentMillis - previousMillisPot > 200) {
    previousMillisPot = currentMillis;
    ampOsc1 = convertValue(analogRead(A0), 0, 1023, 0.0, 1.0);
    ampOsc2 = convertValue(analogRead(A1), 0, 1023, 0.0, 1.0);
    ampOsc3 = convertValue(analogRead(A11), 0, 1023, 0.0, 1.0);
    freqLfo = convertValue(analogRead(A4), 0, 1023, 0.0, 15.0f);
    delayOsc = int(convertValue(analogRead(A5), 0, 1023, 1.0, 80.0));
    tune = int(convertValue(analogRead(A10), 0, 1023, 0, 12));
    ampLfo = convertValue(analogRead(A6), 0, 1023, 0.0f, 1.0f);
    cutOffFreq = convertValue(analogRead(A9), 0, 1023, 0.0f, 2093.0f);
  }
}

State readSwitch3(int rPin, int lPin) {
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

void readSwitch(unsigned long currentMillis){
  if (currentMillis - previousMillisSwitch > 400) {
    previousMillisSwitch = currentMillis;
    switchOctOsc1 = readSwitch3(osc1OctPinU, osc1OctPinD);
    switchOctOsc2 = readSwitch3(osc2OctPinU, osc2OctPinD);
    switchWaveOsc1 = readSwitch3(osc1WavePinU, osc1WavePinD);
    switchWaveOsc2 = readSwitch3(osc2WavePinU, osc2WavePinD);
    switchFilter = readSwitch3(filterPinU, filterPinD);
    switchLfoWave = readSwitch3(lfoWavePinU, lfoWavePinD);
    switchLfoValue = readSwitch3(lfoValPinU, lfoValPinD);
  }
}

float getSubFreq(float freq) {
  // 21 and 108 represent the lowest and the higher he can reach
  int midiNote = transposeOctave(freq, 1);

  return mtof(midiNote);
}

float frequencyLfo(float freqOsc){
  float lfoDo;
  lfoDo = lfo.Process() + 1;
  float freq = lfoDo * freqOsc;
  return freq;
}

void setOscFreq(float freq, int index){
  int midiNote;
  float freqOsc2;
  int tuneMidiNote;
  if(switchLfoValue == UP){
    freq = frequencyLfo(freq);
  }
  switch(switchOctOsc2){
    case UP:
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
      midiNote = transposeOctave(freq, 1);
      tuneMidiNote = midiNote + tune;
      freqOsc2 = mtof(tuneMidiNote);
      break;
  }

  osc1[index].SetFreq(freq);
  osc2[index].SetFreq(freqOsc2);
  osc3[index].SetFreq(getSubFreq(freq));
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

void oscillatorsOut() {
  for (int i = 0; i < 10; i++) {
    osc1_out[i] = (notes[i] == 0) ? 0 : osc1[i].Process();
    osc2_out[i] = (notes[i] == 0) ? 0 : osc2[i].Process();
    osc3_out[i] = (notes[i] == 0) ? 0 : osc3[i].Process();
  }
}

void setEachOscAmp(float amp1, float amp2, float amp3){
  for (int i = 0; i < 10; i++) {
    if (notes[i] != 0) {
      osc1[i].SetAmp(amp1);
      osc2[i].SetAmp(amp2);
      osc3[i].SetAmp(amp3);
    }
  }
}

void setOscAmp(){
  float newAmpOsc1 = ampOsc1;
  float newAmpOsc2 = ampOsc2;
  float newAmpOsc3 = ampOsc3;

  if(switchLfoValue == DOWN){
    newAmpOsc1 = frequencyLfo(ampOsc1);
    newAmpOsc2 = frequencyLfo(ampOsc2);
    newAmpOsc3 = frequencyLfo(ampOsc3);
  }

  setEachOscAmp(newAmpOsc1, newAmpOsc2, newAmpOsc3);
}

void setOscParams(unsigned long currentMillis){
  if (currentMillis - previousMillisWave > 200) {
    Wave waveOsc1;
    Wave waveOsc2;
    Wave waveLfo;
    switch(switchWaveOsc1){
      case UP:
        waveOsc1 = WAVE_TRI;
        break;
      case CENTER:
        waveOsc1 = WAVE_POLYBLEP_SAW;
        break;
      case DOWN:
        waveOsc1 = WAVE_SQUARE;
        break;
    }
    switch(switchWaveOsc2){
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
    switch(switchLfoWave){
      case UP:
        waveLfo = WAVE_SQUARE;
        break;
      case CENTER:
        waveLfo = WAVE_POLYBLEP_SAW;
        break;
      case DOWN:
        waveLfo = WAVE_TRI;
        break;
    }
    for (int i = 0; i < 10; i++) {
      osc1[i].SetWaveform(waveOsc1);
      osc2[i].SetWaveform(waveOsc2);
      osc3[i].SetWaveform(waveOsc1);
    }
    lfo.SetWaveform(waveLfo);
    lfo.SetFreq(freqLfo);
    lfo.SetAmp(ampLfo);
  }
}

bool setDelay() {
  switch (delayOsc) {
    case 0 ... 3:
      return false;
      break;

    case 4 ... 14:
      del.SetDelay(sample_rate * 0.05f);
      break;

    case 15 ... 19:
      del.SetDelay(sample_rate * 0.10f);
      break;

    case 20 ... 24:
      del.SetDelay(sample_rate * 0.15f);
      break;

    case 25 ... 29:
      del.SetDelay(sample_rate * 0.20f);
      break;

    case 30 ... 34:
      del.SetDelay(sample_rate * 0.25f);
      break;

    case 35 ... 39:
      del.SetDelay(sample_rate * 0.30f);
      break;

    case 40 ... 44:
      del.SetDelay(sample_rate * 0.35f);
      break;

    case 45 ... 49:
      del.SetDelay(sample_rate * 0.40f);
      break;

    case 50 ... 54:
      del.SetDelay(sample_rate * 0.45f);
      break;

    case 55 ... 59:
      del.SetDelay(sample_rate * 0.50f);
      break;

    case 60 ... 64:
      del.SetDelay(sample_rate * 0.55f);
      break;

    case 65 ... 69:
      del.SetDelay(sample_rate * 0.60f);
      break;

    case 70 ... 74:
      del.SetDelay(sample_rate * 0.65f);
      break;

    case 75 ... 80:
      del.SetDelay(sample_rate * 0.70f);
      break;
  }

  return true;
}

float changeFilter(float sig_out){
  float output;
  float cutOffFreqNow;
  cutOffFreqNow = cutOffFreq;
  if (switchLfoValue == CENTER){
    cutOffFreqNow = frequencyLfo(cutOffFreq);
  }
  switch(switchFilter){
    case UP:
      hpFilter.SetFreq(cutOffFreqNow);
      output = hpFilter.Process(sig_out);
      break;
    case CENTER:
      output = sig_out;
      break;
    case DOWN:
      lpFilter.SetFreq(cutOffFreqNow);
      output = lpFilter.Process(sig_out);
      break;
  }
  return output;
}

void MyCallback(float **in, float **out, size_t size){
  float sig_out;
  float output;
  float feedback;
  float del_out;

  

  for (size_t i = 0; i < size; i++){
    oscillatorsOut();

    del_out = del.Read();
    feedback = (del_out * 0.75f) + osc1Out() + osc2Out() + osc3Out();
    del.Write(feedback);
    bool isDelay = setDelay();
    if (isDelay) {
      sig_out = del_out + osc1Out() + osc2Out() + osc3Out();
    } else {
      sig_out = osc1Out() + osc2Out() + osc3Out();
    }

    output = changeFilter(sig_out);

    OUT_L[i] = output;
    OUT_R[i] = output;
  }
  
}

void setup() {
  setupSwitch();
  Serial.begin(31250);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  for (int i = 0; i < 10; i++) {
    osc1[i].Init(sample_rate);
    osc1[i].SetFreq(440);
    osc1[i].SetAmp(1);
    osc1[i].SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc2[i].Init(sample_rate);
    osc2[i].SetFreq(440);
    osc2[i].SetAmp(1);
    osc2[i].SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc3[i].Init(sample_rate);
    osc3[i].SetFreq(440);
    osc3[i].SetAmp(1);
    osc3[i].SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
  }

  lfo.Init(sample_rate);
  hpFilter.Init(sample_rate);
  lpFilter.Init(sample_rate);

  lfo.SetFreq(0.2);
  lfo.SetFreq(lfo.WAVE_TRI);

  del.Init();
  del.SetDelay(sample_rate * 0.75f);

  DAISY.begin(MyCallback);
}

void loop() {
  unsigned long currentMillis = millis();
  readPotentiometer(currentMillis);
  readSwitch(currentMillis);
  if (MIDI.read())  // if a MIDI message is received
  {
    switch (MIDI.getType())  // Get the type of the message we received
    {

      case midi::NoteOn:
        Serial.print("note on : ");
        Serial.println(MIDI.getData1());

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
  setOscParams(currentMillis);
}
