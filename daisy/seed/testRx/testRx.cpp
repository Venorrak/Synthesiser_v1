#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
UartHandler uart;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

int main(void)
{
	hw.Init();
	hw.StartLog(false);
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);

	hw.PrintLine("Starting Read-UART-delay-test");

	UartHandler::Config uart_config;
	uart_config.baudrate = 115200;
	uart_config.periph = UartHandler::Config::Peripheral::USART_1;
	uart_config.stopbits = UartHandler::Config::StopBits::BITS_2;
	uart_config.parity = UartHandler::Config::Parity::NONE;
	uart_config.mode = UartHandler::Config::Mode::RX;
	uart_config.wordlength = UartHandler::Config::WordLength::BITS_8;
	uart_config.pin_config.rx = {DSY_GPIOB, 7}; // (USART_1 RX) Daisy pin 15

	uart.Init(uart_config);

	while(1) 
	{
		uint8_t singleByte;
		int ret = uart.PollReceive(&singleByte, 1, 0);
		if (ret == 0) 
		{
			hw.PrintLine("%c", singleByte);
		}

		System::Delay(1);
	}
}
