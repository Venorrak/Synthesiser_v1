#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
static Oscillator osc;
GPIO board0;
GPIO board1;
GPIO board2;
int max = 0;
int min = 1000000;

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
		
	return hw.adc.Get(0);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig;
	for (size_t i = 0; i < size; i+=2)
	{
		float knobValue = getBoardKnobValue(0);
	
		osc.SetFreq(knobValue);
		osc.SetAmp(knobValue);
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
	hw.StartLog();

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
	while(1) 
	{
		if (max < getBoardKnobValue(7)) 
		{
			max = getBoardKnobValue(7);
			hw.PrintLine("Max: " FLT_FMT(6), FLT_VAR(6, getBoardKnobValue(7)));
		}
		else if (min > getBoardKnobValue(7)) 
		{
			min = getBoardKnobValue(7);
			hw.PrintLine("Min: " FLT_FMT(6), FLT_VAR(6, getBoardKnobValue(7)));
		}
		System::Delay(1000);
	}
}
