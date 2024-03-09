#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include <cmath>

using namespace daisy;

DaisySeed hw;
daisysp::Oscillator oscillator, osc2, lfo;

float convertValue(int value, float new_min, float new_max) {
    int min_value = 0;
    int max_value = 65534;
    
    // Calculer la valeur normalisée entre 0 et 1
    float normalized_value = static_cast<float>(value - min_value) / (max_value - min_value);
    
    // Appliquer la fonction de mise à l'échelle exponentielle
    float scaled_value = ((new_max - new_min) * pow(normalized_value, 2)) + new_min;
    
    return scaled_value;
}

void AudioCallback(AudioHandle::InputBuffer in, 
                   AudioHandle::OutputBuffer out, 
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        //Read potentiometer value
        float ampOsc1 = convertValue(hw.adc.Get(0), 0.0f, 1.0f);
        float freqOsc1 = convertValue(hw.adc.Get(1), 0.0f, 1.0f);
        float ampOsc2 = convertValue(hw.adc.Get(2), 0.0f, 1.0f);
        float freqOsc2 = convertValue(hw.adc.Get(3), 0.0f, 1.0f);

        // LFO mods the pitch
        oscillator.SetFreq(440 + lfo.Process() * (440 * freqOsc1) );
        osc2.SetFreq(440 + lfo.Process() * (440 * freqOsc2) );
        oscillator.SetAmp(ampOsc1);
        osc2.SetAmp(ampOsc2);

        // A noticeable sync effect can be heard on high frequencies
        OUT_L[i] = oscillator.Process();
        OUT_R[i] = osc2.Process();
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sampleRate = hw.AudioSampleRate();

    AdcChannelConfig adcConfig[4];

    adcConfig[0].InitSingle(hw.GetPin(20));
    adcConfig[1].InitSingle(hw.GetPin(17));
    adcConfig[2].InitSingle(hw.GetPin(21));
    adcConfig[3].InitSingle(hw.GetPin(18));

    hw.adc.Init(adcConfig, 4);
    hw.adc.Start();

    // Audio Oscillator
    oscillator.Init(sampleRate);
    osc2.Init(sampleRate);
    oscillator.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    osc2.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);

    // LFO
    lfo.Init(sampleRate);
    lfo.SetWaveform(daisysp::Oscillator::WAVE_SIN);
    lfo.SetFreq(0.2f);

    hw.StartAudio(AudioCallback);

    for(;;) {}
}