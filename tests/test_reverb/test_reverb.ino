#include <DaisyDuino.h>


DaisyHardware hw;

size_t num_channels;

static Oscillator osc;
static Oscillator osc2;

float pitchknob;
float pitchknob2;
float pitchknob3;
float pitchknob4;
enum Wave_State {SIN, TRI, SAW, SQUARE};
Wave_State wave_state = TRI;
bool last_button_state = false;
unsigned long prev_millis = 0;

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

  pinMode(0, INPUT_PULLUP);

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
  unsigned long cur_millis = millis();
  if (cur_millis - prev_millis > 50) {
    prev_millis = cur_millis;
    int current_button_state = digitalRead(0);
    if (current_button_state == LOW && last_button_state == HIGH){
      switch (wave_state)
      {
      case SIN:
        wave_state = TRI;
        osc.SetWaveform(osc.WAVE_TRI);
        osc2.SetWaveform(osc.WAVE_TRI);
        break;
      case TRI:
        wave_state = SAW;
        osc.SetWaveform(osc.WAVE_SAW);
        osc2.SetWaveform(osc.WAVE_SAW);
        break;
      case SAW:
        wave_state = SQUARE;
        osc.SetWaveform(osc.WAVE_SQUARE);
        osc2.SetWaveform(osc.WAVE_SQUARE);
        break;
      case SQUARE:
        wave_state = SIN;
        osc.SetWaveform(osc.WAVE_SIN);
        osc2.SetWaveform(osc.WAVE_SIN);
      }
    }
    last_button_state = current_button_state;
  }
  pitchknob = 24.0 + ((analogRead(A0) / 1023.0) * 60.0);
  pitchknob2 = 24.0 + ((analogRead(A1) / 1023.0) * 60.0);
  pitchknob3 = (analogRead(A2) / 1023.0) * 18000.0f;
  pitchknob4 = (analogRead(A3) / 1023.0) * 0.85f;
}