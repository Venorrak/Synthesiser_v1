#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc;
// Switch button1;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        float knobFrequence = hw.adc.GetFloat(0);
        // float knobAmplitude = fclamp(patch.GetAdcValue(20), 0.0f, 1.0f);

        osc.SetFreq(knobFrequence * 440.f + 440.0f);
        sig = osc.Process();

        // left out
        out[i] = sig;

        // right out
        out[i + 1] = sig;
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();

    AdcChannelConfig adcConfigFrequence;
    adcConfigFrequence.InitSingle(hw.GetPin(21));
    hw.adc.Init(&adcConfigFrequence, 1);

    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

    // start callback
    hw.adc.Start();
    hw.StartAudio(AudioCallback);

    while(1) {}
}
