#include "daisysp.h"
#include "daisy_seed.h"

#define LEFT (i)
#define RIGHT (i + 1)

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc;
static Adsr env;
static Metro tick;
static Switch3 toggleWaveForm;
bool gate;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, env_out;
    for(size_t i = 0; i < size; i += 2)
    {
        if(tick.Process())
        {
            gate = !gate;
        }

        float knobFrequence = hw.adc.GetFloat(1);
        float knobAmplitude = hw.adc.GetFloat(0);
        
        env_out = env.Process(gate);
        osc.SetFreq(24.0 + (fclamp(knobFrequence, 0.0f, 1.0f) * 60) );
        osc.SetAmp(env_out * knobAmplitude);
        osc_out = osc.Process();

        out[LEFT]  = osc_out;
        out[RIGHT] = osc_out;
    }
}

void changeWaveForm() 
{
    switch(toggleWaveForm.Read())
    {
        case Switch3::POS_UP: osc.SetWaveform(osc.WAVE_SQUARE); break;
        case Switch3::POS_CENTER: osc.SetWaveform(osc.WAVE_SAW); break;
        case Switch3::POS_DOWN: osc.SetWaveform(osc.WAVE_TRI); break;
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();

    toggleWaveForm.Init(seed::D29, seed::D30);
    
    AdcChannelConfig adcConfig[5];

    adcConfig[0].InitSingle(hw.GetPin(20));
    adcConfig[1].InitSingle(hw.GetPin(21));
    adcConfig[2].InitSingle(hw.GetPin(19));
    adcConfig[3].InitSingle(hw.GetPin(18));
    adcConfig[4].InitSingle(hw.GetPin(17));
   
    hw.adc.Init(adcConfig, 5);
    hw.adc.Start();
    hw.StartLog();

    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);
    env.Init(sample_rate);

    // Set up metro to pulse x second
    tick.Init(1.2f, sample_rate);

    //Set envelope parameters
    env.SetTime(ADSR_SEG_ATTACK, 1.2f);
    env.SetTime(ADSR_SEG_DECAY, 1.2f);

    env.SetSustainLevel(.50);

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

    // start callback
    hw.StartAudio(AudioCallback);

    while(1) 
    {
        changeWaveForm();
        hw.PrintLine("Input 0: %d", (hw.adc.Get(0)));
    }
}
