#include "daisysp.h"
#include "daisy_seed.h"
#include "daisysp-lgpl.h"
#include <cmath>

// Set max delay time to 0.5 of samplerate.
#define MAX_DELAY static_cast<size_t>(32000 * 0.75f)

#define LEFT (i)
#define RIGHT (i + 1)

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc, osc2;
static Adsr env;
static Metro tick;
static Switch3 toggleWaveForm;
static ReverbSc reverb;
static DelayLine<float, MAX_DELAY> del;
bool gate;

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
        case Switch3::POS_UP: osc.SetWaveform(osc.WAVE_SQUARE); osc2.SetWaveform(osc.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: osc.SetWaveform(osc.WAVE_POLYBLEP_SAW); osc2.SetWaveform(osc.WAVE_POLYBLEP_SAW); break;
        case Switch3::POS_DOWN: osc.SetWaveform(osc.WAVE_TRI); osc2.SetWaveform(osc.WAVE_TRI); break;
    }
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, env_out, feedback, del_out, sig_out;
    for(size_t i = 0; i < size; i += 2)
    {
        if(tick.Process())
        {
            gate = !gate;
        }

        float knobFrequence = hw.adc.GetFloat(1);
        float knobAmplitude = hw.adc.GetFloat(0);
        float knobTickFrequency = hw.adc.Get(2);
        float knobAttack = hw.adc.Get(3);
        float knobDecay = hw.adc.Get(4);
        float knobSustain = hw.adc.Get(5);
        float knobVerbVolume = hw.adc.Get(6);
        float knobVerbFrequence = hw.adc.Get(7);
        float knobDelVolume = hw.adc.Get(8);
        float knobDel = hw.adc.Get(9);
        
        env_out = env.Process(gate);
        osc_out = osc.Process();
        del_out = del.Read();

        osc.SetFreq(24.0 + (fclamp(knobFrequence, 0.0f, 1.0f) * 60) );
        osc.SetAmp(env_out * knobAmplitude);
        del.SetDelay(convertValue(knobDel, 0.0f, 0.75f));
        tick.SetFreq(convertValue(knobTickFrequency, 1.0f, 5.0f));
        env.SetTime(ADSR_SEG_ATTACK, convertValue(knobAttack, 1.0f, 5.0f));
        env.SetTime(ADSR_SEG_DECAY, convertValue(knobDecay, 1.0f, 5.0f));
        env.SetSustainLevel(convertValue(knobSustain, 0.0f, 1.0f));

        sig_out  = (del_out * convertValue(knobDelVolume, 0.0f, 1.0f)) + osc_out;
        feedback = (del_out * 0.50f) + osc_out;

        // Write to the delay
        del.Write(feedback);

        if (convertValue(knobVerbVolume, 0.0f, 1.0f) < 0.05f) 
        {    
            out[LEFT]  = sig_out;
            out[RIGHT] = sig_out;
        }
        else 
        {
            float sig = env_out * sig_out;
            reverb.SetFeedback(convertValue(knobVerbVolume, 0.0f, 1.0f));
            reverb.SetLpFreq(convertValue(knobVerbFrequence, 1000.0f, 36000.0f));
            reverb.Process(sig, sig, &out[LEFT], &out[RIGHT]);  
        }        
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();

    toggleWaveForm.Init(seed::D29, seed::D30);
    
    AdcChannelConfig adcConfig[10];

    adcConfig[0].InitSingle(hw.GetPin(20));
    adcConfig[1].InitSingle(hw.GetPin(21));
    adcConfig[2].InitSingle(hw.GetPin(19));
    adcConfig[3].InitSingle(hw.GetPin(18));
    adcConfig[4].InitSingle(hw.GetPin(17));
    adcConfig[5].InitSingle(hw.GetPin(16));
    adcConfig[6].InitSingle(hw.GetPin(22));
    adcConfig[7].InitSingle(hw.GetPin(23));
    adcConfig[8].InitSingle(hw.GetPin(24));
    adcConfig[9].InitSingle(hw.GetPin(15));
   
    hw.adc.Init(adcConfig, 10);
    hw.adc.Start();
    hw.StartLog();

    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);
    // lfo.Init(20.0f);
    env.Init(sample_rate);

    // Set up metro to pulse x second
    tick.Init(1.0f, sample_rate);

    //Set envelope parameters
    env.SetTime(ADSR_SEG_ATTACK, 1.0f);
    env.SetTime(ADSR_SEG_DECAY, 1.0f);

    env.SetSustainLevel(.50);

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

    // Set parameters for reverb
    reverb.Init(sample_rate);
    reverb.SetFeedback(0.1f);
    reverb.SetLpFreq(18000.0f);

    // start callback
    hw.StartAudio(AudioCallback);

    while(1) 
    {
        changeWaveForm();
        hw.PrintLine("Input 1: %d", sample_rate);      
    }
}