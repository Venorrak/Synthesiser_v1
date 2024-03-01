#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc;
Switch buttonWaveForm;
uint8_t waveform = osc.WAVE_SQUARE;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        float knobFrequence = hw.adc.GetFloat(1);
        float knobAmplitude = hw.adc.GetFloat(0);

        osc.SetFreq(knobFrequence * 440.f + 440.0f);
        osc.SetAmp(knobAmplitude);

        sig = osc.Process();

        // left out
        out[i] = sig;

        // right out
        out[i + 1] = sig;
    }
}

void changeWaveForm() 
{
    switch (waveform)
    {
    case osc.WAVE_SQUARE:
        waveform = osc.WAVE_SAW;
        break;

    case osc.WAVE_SAW:
        waveform = osc.WAVE_TRI;
        break;

    case osc.WAVE_TRI:
        waveform = osc.WAVE_SQUARE;
        break;
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();

    buttonWaveForm.Init(hw.GetPin(22), 1000);

    AdcChannelConfig adcConfig[2];

    adcConfig[0].InitSingle(hw.GetPin(20));
    adcConfig[1].InitSingle(hw.GetPin(21));
   
    hw.adc.Init(adcConfig, 2);
    hw.adc.Start();

    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);

    // Set parameters for oscillator

    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

    // start callback
    hw.StartAudio(AudioCallback);

    while(1) 
    {
        buttonWaveForm.Debounce();

        if (buttonWaveForm.RisingEdge()) 
        {
           changeWaveForm();
        }

    }
}
