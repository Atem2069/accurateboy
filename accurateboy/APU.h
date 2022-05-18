#pragma once

#include<iostream>
#include<format>
#include<SDL.h>
#undef main		//sdl sucks
#include"Logger.h"

struct Channel
{
	uint8_t r[5];
};

struct SampleBuffer
{
	float c1[512];
	float c2[512];
	float c3[512];
	float c4[512];
};

class APU
{
public:
	APU();
	~APU();

	void step(uint16_t newDivider);
	void playSamples();

	void writeIORegister(uint16_t address, uint8_t value);
	uint8_t readIORegister(uint16_t address);

private:

	uint16_t m_clockDivider = 0;

	Channel m_channels[4];
	uint8_t NR50 = 0;
	uint8_t NR51 = 0;
	uint8_t NR52 = 0;

	uint8_t m_waveRAM[32] = {};

	uint8_t m_dutyTable[4] =	//hardware predefined square wave duties
	{	
		0b00000001,				//12.5%
		0b00000011,				//25%
		0b00001111,				//50%
		0b11111100				//75%
	};

	//bitmasks for register reads
	uint8_t m_NR1XMasks[5] = { 0x80,0x3F,0x00,0xFF,0xBF };
	uint8_t m_NR2XMasks[5] = { 0xFF,0x3F,0x00,0xFF,0xBF };
	uint8_t m_NR3XMasks[5] = { 0x7F,0xFF,0x9F,0xFF,0xBF };
	uint8_t m_NR4XMasks[5] = { 0xFF,0xFF,0x00,0x00,0xBF };
	uint8_t m_NR52Mask = 0x70;

	void m_clearRegisters();

	void clockLengthCounters();
	void clockEnvelope();

	//channel 1
	float chan1_getOutput();
	int chan1_freqTimer = 0;
	uint8_t chan1_waveDutyPosition = 0;
	int chan1_amplitude = 0;
	uint8_t chan1_lengthCounter = 0;
	uint8_t chan1_volume = 0;
	bool chan1_envelopeAdd = false;
	uint8_t chan1_envelopePeriod = 0;
	uint8_t chan1_envelopeTimer = 0;
	uint8_t chan1_sweepPeriod = 0;
	uint8_t chan1_sweepTimer = 0;
	uint8_t chan1_sweepShift = 0;
	uint16_t chan1_shadowFrequency = 0;
	bool chan1_sweepNegate = false;

	//channel 2
	float chan2_getOutput();
	int chan2_freqTimer = 0;
	uint8_t chan2_waveDutyPosition = 0;
	int chan2_amplitude = 0;
	uint8_t chan2_lengthCounter = 0;
	uint8_t chan2_volume = 0;
	bool chan2_envelopeAdd = false;
	uint8_t chan2_envelopePeriod = 0;
	uint8_t chan2_envelopeTimer = 0;

	//channel 3
	float chan3_getOutput();
	int chan3_freqTimer = 0;
	uint8_t chan3_samplePosition = 0;
	uint16_t chan3_lengthCounter = 0;
	uint8_t chan3_volume = 0;

	//channel 4
	uint8_t chan4_divisorMapping[8] = { 8,16,32,48,64,80,96,112 };
	float chan4_getOutput();
	int chan4_freqTimer = 0;
	uint8_t chan4_shiftAmount = 0;
	uint8_t chan4_divisorCode = 0;
	uint16_t chan4_LFSR = 0x0;
	bool chan4_widthMode = false;
	uint16_t chan4_lengthCounter = 0;
	uint8_t chan4_volume = 0;
	bool chan4_envelopeAdd = false;
	uint8_t chan4_envelopePeriod = 0;
	uint8_t chan4_envelopeTimer = 0;

	//mixing/sampling
	unsigned long mixer_cycleDiff = 0;
	//float samples[512] = {};
	//float curPlayingSamples[512] = {};
	SampleBuffer samples = {};
	SampleBuffer curPlayingSamples = {};
	int sampleIndex = 0;
	SDL_AudioDeviceID mixer_audioDevice = {};

	bool getChannelEnabledLeft(int idx);
	bool getChannelEnabledRight(int idx);

	//high-pass filter for mixing
	static float capacitor;
	float highPass(float in, bool dacsEnabled);
};