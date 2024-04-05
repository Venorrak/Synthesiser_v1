// Title: tone
// Description: Sweeps gentle lowpass filter
// Hardware: Daisy Seed
// Author: Stephen Hensley

#include "DaisyDuino.h"
#include "Arduino.h"
#include <cmath>

DaisyHardware hw;

size_t num_channels;
float ampOsc1;
float ampOsc2;
float freqOsc1;
float freqOsc2;
float envAttack;
float envDecay;
float sustain;
float tickFrequency;

static Oscillator osc1, osc2, lfo;
static Adsr env;
Metro tick;
bool gate;

float convertValue(int x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void MyCallback(float **in, float **out, size_t size) {
  float osc1_out, osc2_out, osc3_out, env_out, del_out, sig_out, feedback;
  for (size_t i = 0; i < size; i++) {
    
    // When the metro ticks, trigger the envelope to start.
    if (tick.Process()) {
      gate = !gate;
    }
    
    osc1.SetAmp(ampOsc1);
    osc1.SetFreq(freqOsc1);
    osc2.SetAmp(ampOsc2);
    osc2.SetFreq(freqOsc2);

    // Update tick frequency
      tick.SetFreq(tickFrequency);

    // Update envelope
    env.SetTime(ADSR_SEG_ATTACK, envAttack);
    env.SetTime(ADSR_SEG_DECAY, envDecay);
    env.SetSustainLevel(sustain);

    // Use envelope to control the amplitude of the oscillator.
    env_out = env.Process(gate);
    osc1_out = osc1.Process() * env_out;
    osc2_out = osc2.Process() * env_out;

    sig_out = osc1_out + osc2_out;

    for (size_t chn = 0; chn < num_channels; chn++) {
      out[chn][i] = sig_out;
    }
  }
}

void setup() {
  float sample_rate;
  Serial.begin();
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
