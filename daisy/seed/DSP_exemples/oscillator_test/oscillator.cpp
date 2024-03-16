#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include <cmath>

// Set max delay time to 0.75 of samplerate.
#define MAX_DELAY static_cast<size_t>(48000 * 0.75f)

using namespace daisy;
using namespace daisysp;

static DaisySeed hw;
static Oscillator oscillator, osc2, lfo;
static Adsr env;
static Metro tick;
static Switch3 toggleWaveForm;
static Switch3 toggleWaveForm2;
// Declare a DelayLine of MAX_DELAY number of floats.
static DelayLine<float, MAX_DELAY> del;
bool gate;
float sampleRate;

int transposeOctave(float freq, int octaveDifference) 
{
    // Calcul de la valeur MIDI
    float midiValue = 12 * std::log2(freq / 440.0) + 69;
    
    // Arrondir la valeur MIDI à l'entier le plus proche
    int midiNote = static_cast<int>(round(midiValue));

    // Si octaveDifference = 1 on descend d'un octave et si c'est -1 on monte d'un octave
    midiNote = midiNote - (octaveDifference * 12);

    if (midiNote > 108) 
    {
        midiNote = 108;
    }
    else if (midiNote < 21)
    {
        midiNote = 21;
    }

    return midiNote;
}

void setOscFreq(float freqOsc1, float freqOsc2) 
{
    int midiNote;
    switch(toggleWaveForm.Read())
    {
        case Switch3::POS_UP:
            hw.SetLed(false);
            midiNote = transposeOctave(freqOsc1, -1); 
            oscillator.SetFreq(mtof(midiNote));
            break;
        case Switch3::POS_CENTER:
            hw.SetLed(true);
            oscillator.SetFreq(freqOsc1);
            break;
        case Switch3::POS_DOWN: 
            hw.SetLed(false);
            midiNote = transposeOctave(freqOsc1, 1); 
            oscillator.SetFreq(mtof(midiNote));
            break;
    }
    
    switch(toggleWaveForm2.Read())
    {
        case Switch3::POS_UP: 
            midiNote = transposeOctave(freqOsc2, -1); 
            osc2.SetFreq(mtof(midiNote));
            break;
        case Switch3::POS_CENTER:
            osc2.SetFreq(freqOsc2);
            break;
        case Switch3::POS_DOWN: 
            midiNote = transposeOctave(freqOsc2, 1);
            osc2.SetFreq(mtof(midiNote));
            break;
    }
}

float frequencyCheck(float freqOsc, float lfoProcess) 
{
    float freq = ((lfoProcess * freqOsc) * 2);

    if (freq < 0) 
    {
        freq = -1 * freq;
    }

    if (freq < 27.5f) 
    {
        freq = 27.5f;
    }

    return freq;
}

float convertValue(int value, float new_min, float new_max) {
    int min_value = 0;
    int max_value = 65534;
    
    // Calculer la valeur normalisée entre 0 et 1
    float normalized_value = static_cast<float>(value - min_value) / (max_value - min_value);
    
    // Appliquer la fonction de mise à l'échelle exponentielle
    float scaled_value = ((new_max - new_min) * pow(normalized_value, 2)) + new_min;
    
    return scaled_value;
}

void changeWaveForm() 
{
    switch(toggleWaveForm.Read())
    {
        case Switch3::POS_UP: oscillator.SetWaveform(oscillator.WAVE_SQUARE); hw.SetLed(false); break;
        case Switch3::POS_CENTER: oscillator.SetWaveform(oscillator.WAVE_POLYBLEP_SAW); hw.SetLed(true); break;
        case Switch3::POS_DOWN: oscillator.SetWaveform(oscillator.WAVE_TRI); hw.SetLed(false); break;
    }

    switch(toggleWaveForm2.Read())
    {
        case Switch3::POS_UP: lfo.SetWaveform(lfo.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: lfo.SetWaveform(lfo.WAVE_POLYBLEP_SAW); break;
        case Switch3::POS_DOWN: lfo.SetWaveform(lfo.WAVE_TRI); break;
    }
}

void AudioCallback(AudioHandle::InputBuffer in, 
                   AudioHandle::OutputBuffer out, 
                   size_t size)
{
    float osc_out, osc2_out, env_out, del_out, sig_out, feedback;
    for (size_t i = 0; i < size; i++)
    {
        if (tick.Process()) 
        {
            gate = !gate;
        }

        //Read potentiometer value
        float ampOsc1 = convertValue(hw.adc.Get(0), 0.0f, 1.0f);
        float freqOsc1 = convertValue(hw.adc.Get(1), 27.5f, 4186.01f);
        float ampOsc2 = convertValue(hw.adc.Get(2), 0.0f, 1.0f);
        float freqOsc2 = convertValue(hw.adc.Get(3), 27.5f, 4186.01f);
        float envAttack = convertValue(hw.adc.Get(4), 1.0f, 5.0f);
        float envDecay = convertValue(hw.adc.Get(5), 1.0f, 5.0f);
        float sustain = convertValue(hw.adc.Get(6), 0.0f, 1.0f);
        float tickFrequency = convertValue(hw.adc.Get(7), 1.0f, 5.0f); 
        float delay = convertValue(hw.adc.Get(8), 0.0f, 0.75f);
        float reverbVolume = convertValue(hw.adc.Get(9), 0.0f, 0.9f);

        //Make sure that frequency is always between 27.5 - 4186.0
        freqOsc1 = frequencyCheck(freqOsc1, lfo.Process());
        freqOsc2 = frequencyCheck(freqOsc2, lfo.Process());

        // LFO mods the oscillator
        // oscillator.SetFreq(freqOsc1);
        // osc2.SetFreq(freqOsc2);
        oscillator.SetAmp(ampOsc1);
        osc2.SetAmp(ampOsc2);
        changeWaveForm();
        setOscFreq(freqOsc1, freqOsc2);
        // Update tick frequency
        tick.SetFreq(tickFrequency);

        // Update envelope
        env.SetTime(ADSR_SEG_ATTACK, envAttack);
        env.SetTime(ADSR_SEG_DECAY, envDecay);
        env.SetSustainLevel(sustain);

        // Use envelope to control the amplitude of the oscillator.
        env_out = env.Process(gate);
        osc_out = oscillator.Process() * env_out;
        osc2_out = osc2.Process() * env_out;

        del_out = del.Read();

          // Calculate output and feedback
        sig_out  = del_out + osc_out + osc2_out;
        feedback = (del_out * 0.75f) + osc_out + osc2_out;

        // Write to the delay
        del.Write(feedback);
        
        del.SetDelay(sampleRate * delay);

        // A noticeable sync effect can be heard on high frequencies
        OUT_L[i] = sig_out;
        OUT_R[i] = sig_out; 
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    sampleRate = hw.AudioSampleRate();

    toggleWaveForm.Init(seed::D29, seed::D30);
    toggleWaveForm2.Init(seed::D14, seed::D13);

    AdcChannelConfig adcConfig[10];

    adcConfig[0].InitSingle(hw.GetPin(20));
    adcConfig[1].InitSingle(hw.GetPin(17));
    adcConfig[2].InitSingle(hw.GetPin(21));
    adcConfig[3].InitSingle(hw.GetPin(18));
    adcConfig[4].InitSingle(hw.GetPin(16));
    adcConfig[5].InitSingle(hw.GetPin(22));
    adcConfig[6].InitSingle(hw.GetPin(23));
    adcConfig[7].InitSingle(hw.GetPin(24));
    adcConfig[8].InitSingle(hw.GetPin(15));
    adcConfig[9].InitSingle(hw.GetPin(19));

    hw.adc.Init(adcConfig, 10);
    hw.adc.Start();

    // Audio Oscillator
    oscillator.Init(sampleRate);
    osc2.Init(sampleRate);
    oscillator.SetWaveform(oscillator.WAVE_POLYBLEP_SAW);
    osc2.SetWaveform(oscillator.WAVE_SQUARE);

    // LFO
    lfo.Init(sampleRate);
    lfo.SetWaveform(lfo.WAVE_SIN);
    lfo.SetFreq(0.2f);

    // Set up metro to pulse every x second
    tick.Init(1.0f, sampleRate);

    // Set envelope parameters
    env.Init(sampleRate);
    env.SetTime(ADSR_SEG_ATTACK, 1.0f);
    env.SetTime(ADSR_SEG_DECAY, 1.0f);
    env.SetSustainLevel(.50);

    // Set Delay parameters
    del.Init();
    del.SetDelay(sampleRate * 0.50f);

    hw.StartAudio(AudioCallback);
    
    for(;;) 
    {
        
    }
}