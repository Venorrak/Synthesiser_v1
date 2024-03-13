#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
static Oscillator osc;
GPIO board0;
GPIO board1;
GPIO board2;

float getBoardKnobValue(int knobPin)
{
	switch (knobPin)
	{
		case 0:
			board0.Write(false);
			board1.Write(false);
			board2.Write(false);
			break;
		case 1:
			board0.Write(true);
			board1.Write(false);
			board2.Write(false);
			break;
		case 2:
			board0.Write(false);
			board1.Write(true);
			board2.Write(false);
			break;
		case 3:
			board0.Write(true);
			board1.Write(true);
			board2.Write(false);
			break;
		case 4:
			board0.Write(false);
			board1.Write(false);
			board2.Write(true);
			break;
		case 5:
			board0.Write(true);
			board1.Write(false);
			board2.Write(true);
			break;
		case 6:
			board0.Write(false);
			board1.Write(true);
			board2.Write(true);
			break;
		case 7:
			board0.Write(true);
			board1.Write(true);
			board2.Write(true);
			break;
	}
	return hw.adc.GetFloat(0);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig;
	for (size_t i = 0; i < size; i+=2)
	{
		float knobValue = getBoardKnobValue(1);
		osc.SetFreq(24.0 + (fclamp(knobValue, 0.0f, 1.0f) * 60) );
		sig = osc.Process();

		out[0][i] = sig;
    	out[1][i] = sig;
	}
}

int main(void)
{
	float sample_rate;
	hw.Configure();
	hw.Init();
	sample_rate = hw.AudioSampleRate();

	AdcChannelConfig adcConfig;
	adcConfig.InitSingle(hw.GetPin(15));
	hw.adc.Init(&adcConfig, 1);
	hw.adc.Start();

	
	board0.Init(daisy::seed::D1, GPIO::Mode::OUTPUT);
	board1.Init(daisy::seed::D2, GPIO::Mode::OUTPUT);
	board2.Init(daisy::seed::D3, GPIO::Mode::OUTPUT);

	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	osc.Init(sample_rate);
	osc.SetWaveform(osc.WAVE_SAW);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
