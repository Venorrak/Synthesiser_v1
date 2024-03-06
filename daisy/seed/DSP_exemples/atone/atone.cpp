#include "daisysp.h"
#include "daisy_seed.h"
#include "daisysp-lgpl.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static ATone      flt;
static Oscillator osc, lfo;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float saw, freq, output;
    for(size_t i = 0; i < size; i += 2)
    {
        freq = 6000 + (lfo.Process() * 6000);
        saw  = osc.Process();
        // CUT OFF FREQUENCY
        flt.SetFreq(freq);
        output = flt.Process(saw);

        // left out
        out[i] = output;

        // right out
        out[i + 1] = output;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    // initialize Tone object
    flt.Init(sample_rate);

    // set parameters for sine oscillator object
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_TRI);
    lfo.SetAmp(1);
    lfo.SetFreq(.4);

    // set parameters for sine oscillator object
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc.SetFreq(100);
    osc.SetAmp(0.25);


    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {}
}
