// Title: tone
// Description: Sweeps gentle lowpass filter
// Hardware: Daisy Seed
// Author: Stephen Hensley

#include "DaisyDuino.h"
#include "Arduino.h"
#include <cmath>

DaisyHardware hw;
enum State {UP, CENTER, DOWN};

size_t num_channels;
float ampOsc1;
float ampOsc2;
float freqOsc1;
float freqOsc2;
float envAttack;
float envDecay;
float sustain;
float tickFrequency;
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

static Oscillator osc1, osc2, osc3, lfo;
static Adsr env, filterEnv, lfoEnv;
static Metro tick;
static ATone hpFilter;
static Tone lpFilter;
bool gate;

bool isLfoAmp() {
    switch (readSwitch3(lfoValPinR, lfoValPinL))
    {
    case DOWN:
        return true;
        break;
    
    default:
        return false;
        break;
    }
}

bool isLfoFreq() {
     switch (readSwitch3(lfoValPinR, lfoValPinL))
    {
    case UP:
        return true;
        break;
    
    default:
        return false;
        break;
    }
}

bool isLfoCutOff() {
     switch (readSwitch3(lfoValPinR, lfoValPinL))
    {
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
    float midiValue = 12 * std::log2(freq / 440.0) + 69;
    
    // Round the MIDI value to the nearest integer
    int midiNote = static_cast<int>(round(midiValue));

    return midiNote;
}

// midiMin & midiMax values ​​that determine the maximum that can reach and the minimum that it can reach
int transposeOctave(float freq, int octaveDifference, int midiMin, int midiMax) {
    int midiNote = ftom(freq);

    // If octaveDifference = 1 we go down an octave and if it is -1 we go up an octave
    midiNote = midiNote - (octaveDifference * 12);

    if (midiNote > midiMax) 
    {
        midiNote = midiMax;
    }
    else if (midiNote < midiMin)
    {
        midiNote = midiMin;
    }

    return midiNote;
}

void setOscFreq(float freqOsc1, float freqOsc2) {
    int midiNote;
    // frequence up to 1 octave for second oscillator
    int tune = static_cast<int>(round(convertValue(analogRead(A4), 0, 1023, 0.0, 12.0f)));
    switch(readSwitch3(osc1OctavePinR, osc1OctavePinL))
    {
        case UP:
            // 33 and 108 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc1, -1, 33, 108);
            osc1.SetFreq(mtof(midiNote));
            break;
        case CENTER:
            osc1.SetFreq(freqOsc1);
            break;
        case DOWN:
            // 33 and 108 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc1, 1, 33, 108); 
            osc1.SetFreq(mtof(midiNote));
            break;
    }
    
    switch(readSwitch3(osc2WavePinR, osc2WavePinL))
    {
        case UP:
            // 21 and 96 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc2, -1, 21, 96);
            midiNote += tune;
            osc2.SetFreq(mtof(midiNote));
            break;
        case CENTER:
            midiNote = ftom(freqOsc2);
            midiNote += tune;
            osc2.SetFreq(freqOsc2);
            break;
        case DOWN:
            // 21 and 96 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc2, 1, 21, 96);
            midiNote += tune;
            osc2.SetFreq(mtof(midiNote));
            break;
    }
}

// make sure that frequence is not below min freq
float frequencyCheck(float freqOsc, float lfoProcess, float minFreq) {
    float freq = (( (lfoProcess * lfoEnv.Process(gate)) * freqOsc) * 2);

    if (freq < 0) 
    {
        freq = -1 * freq;
    }

    if (freq < minFreq) 
    {
        freq = minFreq;
    }

    return freq;
}

void setSubFreq(float freq) {
    // 21 and 108 represent the lowest and the higher he can reach
    int midiNote = transposeOctave(freq, 1, 21, 108);
    osc3.SetFreq(mtof(midiNote));
}

int readSwitch3(int rPin, int lPin) {
  bool rState = digitalRead(rPin);
  bool lState = digitalRead(lPin);

  State state;

  if ( (rState == LOW) && (lState == HIGH)) //test for right position
  {
    state = UP;
  }
  else if ( (rState == HIGH) && (lState == HIGH)) //test for center position
  {
    state = CENTER;
  }
  else if ( (rState == HIGH) && (lState == LOW))  //test for left position
  {
    state = DOWN;
  }

  return state;
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

void serialPrintTask() {
    Serial.print("Amp1: ");
    Serial.println(ampOsc1);
    Serial.print("freq1: ");
    Serial.println(freqOsc1);
    Serial.print("Amp2: ");
    Serial.println(ampOsc2);
    Serial.print("freq2: ");
    Serial.println(freqOsc2);
    Serial.print("envAttack: ");
    Serial.println(envAttack);
    Serial.print("envDecay: ");
    Serial.println(envDecay);
    Serial.print("sustain: ");
    Serial.println(sustain);
    Serial.print("tickFrequency: ");
    Serial.println(tickFrequency);
}

float convertValue(int x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void changeWaveForm() {
  switch (readSwitch3(osc1WavePinR, osc1WavePinL)) {
    case UP:
      osc1.SetWaveform(osc1.WAVE_SQUARE);
      osc3.SetWaveform(osc3.WAVE_SQUARE);
      break;
    case CENTER:
      osc1.SetWaveform(osc1.WAVE_POLYBLEP_SAW);
      osc3.SetWaveform(osc3.WAVE_POLYBLEP_SAW);
      break;
    case DOWN:
      osc1.SetWaveform(osc1.WAVE_TRI);
      osc3.SetWaveform(osc3.WAVE_TRI);
      break;
  }

  switch (readSwitch3(osc2WavePinR, osc2WavePinL)) {
    case UP:
      osc2.SetWaveform(osc2.WAVE_SQUARE);
      break;
    case CENTER:
      osc2.SetWaveform(osc2.WAVE_POLYBLEP_SAW);
      break;
    case DOWN:
      osc2.SetWaveform(osc2.WAVE_TRI);
      break;
  }
}

void setOscAmp(float ampOsc1, float ampOsc2, float lfoProcess) {
    if (isLfoAmp()) 
    {
        ampOsc1 = (( (lfoProcess * lfoEnv.Process(gate)) * ampOsc1 ) * 2);
        ampOsc2 = (( (lfoProcess * lfoEnv.Process(gate)) * ampOsc2 ) * 2);

        if (ampOsc1 < 0) 
        {
            ampOsc1 = -1 * ampOsc1;
        }
        
         if (ampOsc2 < 0) 
        {
            ampOsc2 = -1 * ampOsc2;
        }
    }
    osc1.SetAmp(ampOsc1);
    osc2.SetAmp(ampOsc2);
    osc3.SetAmp(ampOsc1);
}

void changeFilter(float *sig_out, float lfoProcess) {
    float cutOffFreq = convertValue(hw.adc.Get(10), 110.0f, 1046.50f);

    if (isLfoCutOff()) 
    {
        cutOffFreq = ( (cutOffFreq * (lfoProcess * lfoEnv.Process(gate))) * 2);

        if (cutOffFreq < 0) 
        {
            cutOffFreq = -1 * cutOffFreq;
        }
    }

    switch(readSwitch3(filterPinR, filterPinL))
    {
        case UP:
            hpFilter.SetFreq(cutOffFreq);
            *sig_out = hpFilter.Process(*sig_out) * filterEnv.Process(gate);
        break;
        case CENTER: break;
        case DOWN: 
            lpFilter.SetFreq(cutOffFreq);
            *sig_out = lpFilter.Process(*sig_out) * filterEnv.Process(gate);
        break;
    }
}

void MyCallback(float **in, float **out, size_t size) {
  float osc1_out, osc2_out, osc3_out, env_out, del_out, sig_out, feedback;
  for (size_t i = 0; i < size; i++) {
    
    // When the metro ticks, trigger the envelope to start.
    if (tick.Process()) {
      gate = !gate;
    }
    
    if (isLfoFreq()) 
    {
        //Make sure that frequency is not going below his minimum
        freqOsc1 = frequencyCheck(freqOsc1, lfo.Process(), 110.0);
        freqOsc2 = frequencyCheck(freqOsc2, lfo.Process(), 55.0);
    }

    changeWaveForm();
    setOscFreq(freqOsc1, freqOsc2);
    setOscAmp(ampOsc1, ampOsc2, lfo.Process());
    setSubFreq(freqOsc1);

    // Update tick frequency
    tick.SetFreq(tickFrequency);

    // Update envelope
    env.SetTime(ADSR_SEG_ATTACK, envAttack);
    env.SetTime(ADSR_SEG_DECAY, envDecay);
    env.SetSustainLevel(sustain);

    lfoEnv.SetTime(ADSR_SEG_ATTACK, envAttack);
    lfoEnv.SetTime(ADSR_SEG_DECAY, envDecay);
    lfoEnv.SetSustainLevel(sustain);

    filterEnv.SetTime(ADSR_SEG_ATTACK, envAttack);
    filterEnv.SetTime(ADSR_SEG_DECAY, envDecay);

    // Use envelope to control the amplitude of the oscillator.
    env_out = env.Process(gate);
    osc1_out = osc1.Process() * env_out;
    osc2_out = osc2.Process() * env_out;
    osc3_out = osc3.Process() * env_out;

    sig_out = osc1_out + osc2_out + osc3_out;

    changeFilter(&sig_out, lfo.Process());

    for (size_t chn = 0; chn < num_channels; chn++) {
      out[chn][i] = sig_out;
    }
  }
}

void setup() {
  float sample_rate;
  Serial.begin();
  setupSwitch3();
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  sample_rate = DAISY.get_samplerate();

  // set parameters for sine oscillator object
  lfo.Init(sample_rate);
  lfo.SetWaveform(Oscillator::WAVE_TRI);
  lfo.SetAmp(1);
  lfo.SetFreq(.4);

  // set parameters for sine oscillator object
  osc1.Init(sample_rate);
  osc1.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
  osc1.SetFreq(100);
  osc1.SetAmp(0.25);

  osc2.Init(sample_rate);
  osc2.SetWaveform(Oscillator::WAVE_SQUARE);
  osc2.SetFreq(100);
  osc2.SetAmp(0.25);

  osc3.Init(sample_rate);
  osc3.SetWaveform(Oscillator::WAVE_TRI);
  osc3.SetFreq(100);
  osc3.SetAmp(0.25);

  // Set up metro to pulse every second
  tick.Init(3.0, sample_rate);

  // Set envelope parameters
  env.Init(sample_rate);
  env.SetTime(ADSR_SEG_ATTACK, 0.1);
  env.SetTime(ADSR_SEG_DECAY, 0.1);
  env.SetSustainLevel(0.25);

  DAISY.begin(MyCallback);
}

void loop() {
  ampOsc1 = convertValue(analogRead(A5), 0, 1023, 0.0, 1.0);
  freqOsc1 = convertValue(analogRead(A2), 0, 1023, 110.0, 2093.0);
  ampOsc2 = convertValue(analogRead(A6), 0, 1023, 0.0, 1.0);
  freqOsc2 = convertValue(analogRead(A3), 0, 1023, 55.0, 1046.50);
  envAttack = convertValue(analogRead(A1), 0, 1023, 1.0, 5.0);
  envDecay = convertValue(analogRead(A7), 0, 1023, 1.0, 5.0);
  sustain = convertValue(analogRead(A8), 0, 1023, 0.0, 1.0);
  tickFrequency = convertValue(analogRead(A9), 0, 1023, 1.0, 5.0);
}
