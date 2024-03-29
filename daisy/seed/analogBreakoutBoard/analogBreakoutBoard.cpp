#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
static Oscillator osc;
GPIO board0;
GPIO board1;
GPIO board2;

// function to read the ADC value for the selected knob
// knobPin is the number of the knob to read (0-7)
float getBoardKnobValue(int knobPin)
{
	for (int i=0; i<3; i++)
	{
		if (knobPin & (1<<i))
			switch (i)
			{
			case 0:
				board0.Write(true);
				break;
			case 1:
				board1.Write(true);
				break;
			case 2:
				board2.Write(true);
				break;
			}
		else
			switch (i)
			{
			case 0:
				board0.Write(false);
				break;
			case 1:
				board1.Write(false);
				break;
			case 2:
				board2.Write(false);
				break;
			}
	}
		
	return hw.adc.GetFloat(0);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig;
	for (size_t i = 0; i < size; i+=2)
	{
		float knobValue = getBoardKnobValue(0);
		hw.PrintLine("My Float: " FLT_FMT(6), FLT_VAR(6, knobValue));
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

	// read the ADC on pin 15
	// Z pin on the breakout board
	AdcChannelConfig adcConfig;
	adcConfig.InitSingle(hw.GetPin(15));
	hw.adc.Init(&adcConfig, 1);
	hw.adc.Start();

	// set up the GPIO pins for the breakout board
	// S0, S1, S2 on the breakout board
	// used to select which knob to read
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
