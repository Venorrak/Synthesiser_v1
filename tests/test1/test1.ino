
#include <DaisyDuino.h>


DaisyHardware hw;

size_t num_channels;

static Oscillator osc;
static Oscillator osc2;

float pitchknob;
float pitchknob2;
float pitchknob3;
float pitchknob4;

ReverbSc verb;

void MyCallback(float **in, float **out, size_t size) {
  // Convert Pitchknob MIDI Note Number to frequency
  osc.SetFreq(mtof(pitchknob));
  osc2.SetFreq(mtof(pitchknob2));
  verb.SetLpFreq(pitchknob3);
  verb.SetFeedback(pitchknob4);
  for (size_t i = 0; i < size; i++) {
    float out1, out2;
    float sig = osc.Process();
    float sig2 = osc2.Process();
    verb.Process(sig, sig2, &out1, &out2);
    out[0][i] = out1;
    out[1][i] = out2;
  }
}

void setup() {
  float sample_rate;
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  sample_rate = DAISY.get_samplerate();

  osc.Init(sample_rate);
  osc.SetFreq(440);
  osc.SetAmp(0.5);
  osc.SetWaveform(osc.WAVE_TRI);

  osc2.Init(sample_rate);
  osc2.SetFreq(440);
  osc2.SetAmp(0.5);
  osc2.SetWaveform(osc.WAVE_TRI);

  verb.Init(sample_rate);
  verb.SetFeedback(0.85f);
  verb.SetLpFreq(18000.0f);

  DAISY.begin(MyCallback);
}

void loop() { 
  pitchknob = 24.0 + ((analogRead(A0) / 1023.0) * 60.0);
  pitchknob2 = 24.0 + ((analogRead(A1) / 1023.0) * 60.0);
  pitchknob3 = (analogRead(A2) / 1023.0) * 18000.0f;
  pitchknob4 = (analogRead(A3) / 1023.0) * 0.85f;
}