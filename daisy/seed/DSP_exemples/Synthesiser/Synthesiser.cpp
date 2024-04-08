#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include <cmath>

// Set max delay time to 0.75 of samplerate.
#define MAX_DELAY static_cast<size_t>(48000 * 0.75f)

using namespace daisy;
using namespace daisysp;

static DaisySeed hw;
static Oscillator osc1, osc2, osc3, lfo;
static Adsr env;
static Adsr filterEnv;
static Adsr lfoEnv;
static Metro tick;
static Switch3 toggleWaveFormOsc1;
static Switch3 toggleWaveFormOsc2;
static Switch3 toggleFilter;
static Switch3 toggleOctaveOsc1;
static Switch3 toggleWaveFormLfo;
static Switch3 toggleLfoValAssigned;
// Declare a DelayLine of MAX_DELAY number of floats.
static DelayLine<float, MAX_DELAY> del;
static ATone hpFilter;
static Tone lpFilter;
bool gate;
float sampleRate;

bool isLfoAmp() 
{
    switch (toggleLfoValAssigned.Read())
    {
    case Switch3::POS_DOWN:
    hw.SetLed(true);
        return true;
        break;
    
    default:
        return false;
        break;
    }
}

bool isLfoFreq() 
{
     switch (toggleLfoValAssigned.Read())
    {
    case Switch3::POS_UP:
    hw.SetLed(false);
        return true;
        break;
    
    default:
        return false;
        break;
    }
}

bool isLfoCutOff() 
{
     switch (toggleLfoValAssigned.Read())
    {
    case Switch3::POS_CENTER:
    hw.SetLed(true);

        return true;
        break;
    
    default:
        return false;
        break;
    }
}

float convertValue(int value, float newMin, float newMax) {
    int minValue = 0;
    int maxValue = 65534;
    
    // Calculate the normalized value between 0 and 1
    float normalizedValue = static_cast<float>(value - minValue) / (maxValue - minValue);
    
    // Apply exponential scaling function
    float scaledValue = ((newMax - newMin) * pow(normalizedValue, 2)) + newMin;
    
    return scaledValue;
}

int ftom(float freq) 
{
    // MIDI Value Calculation
    float midiValue = 12 * std::log2(freq / 440.0) + 69;
    
    // Round the MIDI value to the nearest integer
    int midiNote = static_cast<int>(round(midiValue));

    return midiNote;
}

// midiMin & midiMax values ​​that determine the maximum that can reach and the minimum that it can reach
int transposeOctave(float freq, int octaveDifference, int midiMin, int midiMax) 
{
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

void setOscFreq(float freqOsc1, float freqOsc2) 
{
    int midiNote;
    // frequence up to 1 octave for second oscillator
    int tune = static_cast<int>(round(convertValue(hw.adc.Get(9), 0.0f, 12.0f)));
    switch(toggleOctaveOsc1.Read())
    {
        case Switch3::POS_UP:
            // 33 and 108 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc1, -1, 33, 108);
            osc1.SetFreq(mtof(midiNote));
            break;
        case Switch3::POS_CENTER:
            osc1.SetFreq(freqOsc1);
            break;
        case Switch3::POS_DOWN:
            // 33 and 108 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc1, 1, 33, 108); 
            osc1.SetFreq(mtof(midiNote));
            break;
    }
    
    switch(toggleWaveFormOsc2.Read())
    {
        case Switch3::POS_UP:
            // 21 and 96 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc2, -1, 21, 96);
            midiNote += tune;
            osc2.SetFreq(mtof(midiNote));
            break;
        case Switch3::POS_CENTER:
            midiNote = ftom(freqOsc2);
            midiNote += tune;
            osc2.SetFreq(freqOsc2);
            break;
        case Switch3::POS_DOWN:
            // 21 and 96 represent the lowest and the higher he can reach
            midiNote = transposeOctave(freqOsc2, 1, 21, 96);
            midiNote += tune;
            osc2.SetFreq(mtof(midiNote));
            break;
    }
}
// make sure that frequence is not below min freq
float frequencyCheck(float freqOsc, float lfoProcess, float minFreq) 
{
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

void changeWaveForm() 
{
    switch(toggleWaveFormOsc1.Read())
    {
        case Switch3::POS_UP: osc1.SetWaveform(osc1.WAVE_SQUARE); osc3.SetWaveform(osc3.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: osc1.SetWaveform(osc1.WAVE_POLYBLEP_SAW); osc3.SetWaveform(osc3.WAVE_POLYBLEP_SAW); break;
        case Switch3::POS_DOWN: osc1.SetWaveform(osc1.WAVE_TRI); osc3.SetWaveform(osc3.WAVE_TRI); break;
    }

    switch(toggleWaveFormLfo.Read())
    {
        case Switch3::POS_UP: lfo.SetWaveform(lfo.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: lfo.SetWaveform(lfo.WAVE_POLYBLEP_SAW); break;
        case Switch3::POS_DOWN: lfo.SetWaveform(lfo.WAVE_TRI); break;
    }

    switch(toggleWaveFormOsc2.Read())
    {
        case Switch3::POS_UP: osc2.SetWaveform(osc2.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: osc2.SetWaveform(osc2.WAVE_POLYBLEP_SAW); break;
        case Switch3::POS_DOWN: osc2.SetWaveform(osc2.WAVE_TRI); break;
    }
}

void setSubFreq(float freq) 
{
    // 21 and 108 represent the lowest and the higher he can reach
    int midiNote = transposeOctave(freq, 1, 21, 108);
    osc3.SetFreq(mtof(midiNote));
}

bool setDelay() 
{
    int delay = static_cast<int>(round(convertValue(hw.adc.Get(8), 0.0f, 75.0f)));

    if (delay >= 0 && delay <= 9)
    {
        return false;
    } 
    else if (delay >= 10 && delay <= 75)
    {
        float multiplier = (delay / 10.0f) * 0.1f;
        del.SetDelay(sampleRate * multiplier);
    }
    return true;

    // switch (delay)
    // {
    //     case 0 ... 9:
    //         return false;
    //     break;
        
    //     case 10 ... 14:
    //         del.SetDelay(sampleRate * 0.10f);
    //     break;

    //     case 15 ... 19:
    //         del.SetDelay(sampleRate * 0.15f);
    //     break;
        
    //     case 20 ... 24:
    //         del.SetDelay(sampleRate * 0.20f);
    //     break;

    //     case 25 ... 29:
    //         del.SetDelay(sampleRate * 0.25f);
    //     break;

    //     case 30 ... 34:
    //         del.SetDelay(sampleRate * 0.30f);
    //     break;

    //     case 35 ... 39:
    //         del.SetDelay(sampleRate * 0.35f);
    //     break;

    //     case 40 ... 44:
    //         del.SetDelay(sampleRate * 0.40f);
    //     break;

    //     case 45 ... 49:
    //         del.SetDelay(sampleRate * 0.45f);
    //     break;

    //     case 50 ... 54:
    //         del.SetDelay(sampleRate * 0.50f);
    //     break;

    //     case 55 ... 59:
    //         del.SetDelay(sampleRate * 0.55f);
    //     break;

    //     case 60 ... 64:
    //         del.SetDelay(sampleRate * 0.60f);
    //     break;

    //     case 65 ... 69:
    //         del.SetDelay(sampleRate * 0.65f);
    //     break;

    //     case 70 ... 74:
    //         del.SetDelay(sampleRate * 0.70f);
    //     break;

    //     case 75 ... 80:
    //         del.SetDelay(sampleRate * 0.75f);
    //     break;
    // }

    // return true;
}

void setOscAmp(float ampOsc1, float ampOsc2, float lfoProcess) 
{
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
// Update filter with switch 3 positions
void changeFilter(float *sig_out, float lfoProcess) 
{
    float cutOffFreq = convertValue(hw.adc.Get(10), 110.0f, 1046.50f);

    if (isLfoCutOff()) 
    {
        cutOffFreq = ( (cutOffFreq * (lfoProcess * lfoEnv.Process(gate))) * 2);

        if (cutOffFreq < 0) 
        {
            cutOffFreq = -1 * cutOffFreq;
        }
    }

    switch(toggleFilter.Read())
    {
        case Switch3::POS_UP:
            hpFilter.SetFreq(cutOffFreq);
            *sig_out = hpFilter.Process(*sig_out) * filterEnv.Process(gate);
        break;
        case Switch3::POS_CENTER: break;
        case Switch3::POS_DOWN: 
            lpFilter.SetFreq(cutOffFreq);
            *sig_out = lpFilter.Process(*sig_out) * filterEnv.Process(gate);
        break;
    }
}

void AudioCallback(AudioHandle::InputBuffer in, 
                   AudioHandle::OutputBuffer out, 
                   size_t size)
{
    float osc_out, osc2_out, osc3_out, env_out, del_out, sig_out, feedback;
    for (size_t i = 0; i < size; i++)
    {
        if (tick.Process()) 
        {
            gate = !gate;
        }

        //Read potentiometer value
        float ampOsc1 = convertValue(hw.adc.Get(0), 0.0f, 1.0f);
        float freqOsc1 = convertValue(hw.adc.Get(1), 110.0f, 2093.0f);
        float ampOsc2 = convertValue(hw.adc.Get(2), 0.0f, 1.0f);
        float freqOsc2 = convertValue(hw.adc.Get(3), 55.0f, 1046.50f);
        float envAttack = convertValue(hw.adc.Get(4), 1.0f, 5.0f);
        float envDecay = convertValue(hw.adc.Get(5), 1.0f, 5.0f);
        float sustain = convertValue(hw.adc.Get(6), 0.0f, 1.0f);
        float tickFrequency = convertValue(hw.adc.Get(7), 1.0f, 5.0f); 

        //Make sure that frequency is not going below his minimum
        if (isLfoFreq()) 
        {
            freqOsc1 = frequencyCheck(freqOsc1, lfo.Process(), 110.0f);
            freqOsc2 = frequencyCheck(freqOsc2, lfo.Process(), 55.0f);
        }
    
        // LFO mods the oscillator
        
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
        osc_out = osc1.Process() * env_out;
        osc2_out = osc2.Process() * env_out;
        osc3_out = osc3.Process() * env_out;

        del_out = del.Read();

        // Calculate output and feedback
        sig_out  = del_out + osc_out + osc2_out + osc3_out;
        feedback = (del_out * 0.75f) + osc_out + osc2_out + osc3_out;

        // Write to the delay
        del.Write(feedback);
        
        bool isDelay = setDelay();

        if (!isDelay) 
        {
            sig_out = osc_out + osc2_out + osc3_out;
        }

        changeFilter(&sig_out, lfo.Process());

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
    hw.StartLog();

    toggleWaveFormOsc1.Init(seed::D29, seed::D30);
    toggleWaveFormOsc2.Init(seed::D14, seed::D13);
    toggleFilter.Init(seed::D11, seed::D4);
    toggleWaveFormLfo.Init(seed::D5, seed::D6);
    toggleLfoValAssigned.Init(seed::D7, seed::D8);
    toggleOctaveOsc1.Init(seed::D9, seed::D10);

    AdcChannelConfig adcConfig[11];

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
    adcConfig[10].InitSingle(hw.GetPin(28));

    hw.adc.Init(adcConfig, 12);
    hw.adc.Start();
    hw.StartLog();

    // Audio Oscillator
    osc1.Init(sampleRate);
    osc2.Init(sampleRate);
    osc3.Init(sampleRate);
    osc1.SetWaveform(osc1.WAVE_POLYBLEP_SAW);
    osc2.SetWaveform(osc1.WAVE_SQUARE);
    osc3.SetWaveform(osc3.WAVE_POLYBLEP_SAW);

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

    lfoEnv.Init(sampleRate);
    lfoEnv.SetTime(ADSR_SEG_ATTACK, 1.0f);
    lfoEnv.SetTime(ADSR_SEG_DECAY, 1.0f);
    lfoEnv.SetSustainLevel(.50);

    filterEnv.Init(sampleRate);
    filterEnv.SetTime(ADSR_SEG_ATTACK, 1.0f);
    filterEnv.SetTime(ADSR_SEG_DECAY, 1.0f);

    // Set Delay parameters
    del.Init();
    del.SetDelay(sampleRate * 0.50f);

    // Set filter parameter
    hpFilter.Init(sampleRate);
    lpFilter.Init(sampleRate);

    hw.StartAudio(AudioCallback);
    
    for(;;) 
    {
        hw.PrintLine("My Float: " FLT_FMT(6), FLT_VAR(6, lfo.Process()));
    }
}