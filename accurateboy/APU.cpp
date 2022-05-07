#include"APU.h"

APU::APU()
{
	m_clearRegisters();

	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec desiredSpec = {}, obtainedSpec = {};
	desiredSpec.freq = 96000;
	desiredSpec.format = AUDIO_F32;
	desiredSpec.channels = 1;	//the game boy is stereo but we're just outputting mono channel 2 for now (completely wrong lol)
	desiredSpec.silence = 0;
	desiredSpec.samples = 512;
	mixer_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
	SDL_PauseAudioDevice(mixer_audioDevice, 0);

	mixer_cycleDiff = 0;
}

APU::~APU()
{
	SDL_Quit();
}

void APU::step()
{
	//steps a single t-cycle
	chan2_freqTimer--;	//cycle diff is measured in m-cycles, but frequency timer decrements per t-cycle.
	if (chan2_freqTimer <= 0)
	{
		uint8_t freqLow = m_channels[1].r[3];
		uint8_t freqHigh = m_channels[1].r[4] & 0b00000111;
		uint16_t newFreq = (freqHigh << 8) | freqLow;
		chan2_freqTimer = (2048 - newFreq) * 4;
		chan2_waveDutyPosition++;		//new duty selected when frequency timer is reloaded.
		if (chan2_waveDutyPosition == 8)
			chan2_waveDutyPosition = 0;	//wraps around (only selects bits 0-7)
	}

	chan1_freqTimer--;
	if (chan1_freqTimer <= 0)
	{
		uint8_t freqLow = m_channels[0].r[3];
		uint8_t freqHigh = m_channels[0].r[4] & 0b00000111;
		uint16_t newFreq = (freqHigh << 8) | freqLow;
		chan1_freqTimer = (2048 - newFreq) * 4;
		chan1_waveDutyPosition++;		//new duty selected when frequency timer is reloaded.
		if (chan1_waveDutyPosition == 8)
			chan1_waveDutyPosition = 0;	//wraps around (only selects bits 0-7)
	}

	chan3_freqTimer--;
	if (chan3_freqTimer <= 0)
	{
		uint8_t freqLow = m_channels[2].r[3];
		uint8_t freqHigh = m_channels[2].r[4] & 0b00000111;
		uint16_t newFreq = (freqHigh << 8) | freqLow;
		chan3_freqTimer = (2048 - newFreq) * 2;
		chan3_samplePosition++;
		if (chan3_samplePosition == 32)
			chan3_samplePosition = 0;
	}

	chan4_freqTimer--;
	if (chan4_freqTimer <= 0)
	{
		int divisor = chan4_divisorMapping[chan4_divisorCode];
		chan4_freqTimer = divisor << chan4_shiftAmount;
		int xorRes = (chan4_LFSR & 0b1) ^ ((chan4_LFSR >> 1) & 0b1);
		chan4_LFSR = (chan4_LFSR >> 1) | (xorRes << 14);
		if (chan4_widthMode)
		{
			chan4_LFSR &= 0b1111111110111111;
			chan4_LFSR |= xorRes << 6;
		}
	}

	//frame sequencer: 2048 m-cycles.
	frameSeq_cycleDiff += 1;
	if (frameSeq_cycleDiff >= 8192)
	{
		frameSeq_cycleDiff -= 8192;
		frameSeq_count++;
		frameSeq_clockIsNew = true;
	}

	if (frameSeq_count % 2 == 0 && frameSeq_clockIsNew)	//length counters
		clockLengthCounters();

	if (frameSeq_count % 8 == 7 && frameSeq_clockIsNew)	//envelope function
		clockEnvelope();

	if (frameSeq_count % 4 == 2 && frameSeq_clockIsNew)
	{
		if (chan1_sweepPeriod != 0)
		{
			if (chan1_sweepTimer > 0)
				chan1_sweepTimer--;

			if (chan1_sweepTimer == 0)
			{
				chan1_sweepTimer = chan1_sweepPeriod;
				if (chan1_sweepPeriod == 0)
					chan1_sweepTimer = 8;
				bool chan1_sweepEnabled = true;	//hmm..
				if (chan1_sweepEnabled && chan1_sweepPeriod > 0)
				{
					//calc new frequency
					int oldFreq = ((m_channels[0].r[4] & 0b111) << 8) | m_channels[0].r[3];
					int newFreq = oldFreq >> chan1_sweepShift;
					if (chan1_sweepNegate)
						newFreq = oldFreq - newFreq;
					else
						newFreq = oldFreq + newFreq;

					if (newFreq > 2047)	//overflow: disable channel
						NR52 &= 0b11111110;

					if (newFreq <= 2047 && chan1_sweepShift > 0)
					{
						//chan1_freqTimer = newFreq;
						m_channels[0].r[3] = newFreq & 0b11111111;
						m_channels[0].r[4] &= 0b11111000;
						m_channels[0].r[4] |= ((newFreq >> 8) & 0b111);
						//todo: overflow check again?
					}
				}
			}
		}
	}

	frameSeq_clockIsNew = false;

	//mixing
	mixer_cycleDiff += 96000;
	while (mixer_cycleDiff >= 4194304)
	{
		mixer_cycleDiff -= 4194304;
		float chan1Out = highPass(chan1_getOutput(), (NR52 & 0b1) && (getChannelEnabledLeft(0) || getChannelEnabledRight(0)));
		float chan2Out = highPass(chan2_getOutput(), (NR52 >> 1) & 0b1 && (getChannelEnabledLeft(1) || getChannelEnabledRight(1)));
		float chan3Out = highPass(chan3_getOutput(), (NR52 >> 2) & 0b1 && (getChannelEnabledLeft(2) || getChannelEnabledRight(2)));
		float chan4Out = highPass(chan4_getOutput(), (NR52 >> 3) & 0b1 && (getChannelEnabledLeft(3) || getChannelEnabledRight(3)));
		samples.c1[sampleIndex] = chan1Out;
		samples.c2[sampleIndex] = chan2Out;
		samples.c3[sampleIndex] = chan3Out;
		samples.c4[sampleIndex] = chan4Out;
		sampleIndex += 1;
		if (sampleIndex == 511)
		{
			sampleIndex = 0;
			//memcpy((void*)curPlayingSamples, (void*)samples, 512*sizeof(Sample));
			memcpy((void*)curPlayingSamples.c1, (void*)samples.c1, 512 * 4);
			memcpy((void*)curPlayingSamples.c2, (void*)samples.c2, 512 * 4);
			memcpy((void*)curPlayingSamples.c3, (void*)samples.c3, 512 * 4);
			memcpy((void*)curPlayingSamples.c4, (void*)samples.c4, 512 * 4);
			playSamples();
		}
	}
}

void APU::playSamples()
{
	while (SDL_GetQueuedAudioSize(mixer_audioDevice) > 512*4)
		(void)0;

	//mix samples
	float finalSamples[512] = {};

	if ((NR52 >> 7) & 0b1)	//only mix audio if the apu is actually enabled. still just play silent samples bc the apu always runs for audio sync
	{
		SDL_MixAudioFormat((uint8_t*)finalSamples, (uint8_t*)curPlayingSamples.c1, AUDIO_F32, 512 * 4, SDL_MIX_MAXVOLUME / 32);
		SDL_MixAudioFormat((uint8_t*)finalSamples, (uint8_t*)curPlayingSamples.c2, AUDIO_F32, 512 * 4, SDL_MIX_MAXVOLUME / 32);
		SDL_MixAudioFormat((uint8_t*)finalSamples, (uint8_t*)curPlayingSamples.c3, AUDIO_F32, 512 * 4, SDL_MIX_MAXVOLUME / 32);
		SDL_MixAudioFormat((uint8_t*)finalSamples, (uint8_t*)curPlayingSamples.c4, AUDIO_F32, 512 * 4, SDL_MIX_MAXVOLUME / 32);
	}

	SDL_QueueAudio(mixer_audioDevice, (void*)finalSamples, 512 * 4);
}

void APU::writeIORegister(uint16_t address, uint8_t value)
{
	//this is horrible: check if apu disabled - and only allow writes to wavetable or NR52
	if (!((NR52 >> 7) & 0b1) && address < 0xFF30 && address != 0xFF26)
		return;
	if (address >= 0xFF10 && address <= 0xFF14)
	{
		if (address - 0xFF10 == 0)
		{
			chan1_sweepPeriod = (value >> 4) & 0b111;
			chan1_sweepShift = value & 0b111;
			chan1_sweepNegate = (value >> 3) & 0b1;
		}
		if (address - 0xFF10 == 1)
			chan1_lengthCounter = 64 - (value & 0b00111111);
		if (address - 0xFF10 == 2)
		{
			chan1_volume = ((value >> 4) & 0xF);
			chan1_envelopePeriod = value & 0b111;
			chan1_envelopeTimer = chan1_envelopePeriod;
			chan1_envelopeAdd = ((value >> 3) & 0b1);
		}
		if (address - 0xFF10 == 4)
		{
			if ((value >> 7) & 0b1)
			{
				NR52 |= 0b00000001;	//re-enable channel
				if (chan1_lengthCounter == 0)
					chan1_lengthCounter = 64;
				uint8_t freqLow = m_channels[0].r[3];
				uint8_t freqHigh = m_channels[0].r[4] & 0b00000111;
				uint16_t newFreq = (freqHigh << 8) | freqLow;
				chan1_freqTimer = (2048 - newFreq) * 4;
				chan1_volume = ((m_channels[0].r[2] >> 4) & 0b1111);
				chan1_envelopeTimer = chan1_envelopePeriod;

				if (!(((m_channels[0].r[2]) >> 5) & 0b11111))
					NR52 &= 0b11111110;

			}

		}
		m_channels[0].r[address - 0xFF10] = value;
	}
	if (address >= 0xFF15 && address <= 0xFF19)
	{
		if (address - 0xFF15 == 1)
			chan2_lengthCounter = 64 - (value & 0b00111111);
		if (address - 0xFF15 == 2)
		{
			chan2_volume = ((value >> 4) & 0xF);
			chan2_envelopePeriod = value & 0b111;
			chan2_envelopeTimer = chan2_envelopePeriod;
			chan2_envelopeAdd = ((value >> 3) & 0b1);
		}
		if (address - 0xFF15 == 4)
		{
			if ((value >> 7) & 0b1)
			{
				NR52 |= 0b00000010;	//re-enable channel
				if (chan2_lengthCounter == 0)
					chan2_lengthCounter = 64;
				uint8_t freqLow = m_channels[1].r[3];
				uint8_t freqHigh = m_channels[1].r[4] & 0b00000111;
				uint16_t newFreq = (freqHigh << 8) | freqLow;
				chan2_freqTimer = (2048 - newFreq) * 4;
				chan2_volume = ((m_channels[1].r[2] >> 4) & 0b1111);
				chan2_envelopeTimer = chan2_envelopePeriod;

				if (!(((m_channels[1].r[2]) >> 5) & 0b11111))
					NR52 &= 0b11111101;
			}

		}
		m_channels[1].r[address - 0xFF15] = value;

	}
	if (address >= 0xFF1A && address <= 0xFF1E)
	{
		if (address - 0xFF1A == 1)
			chan3_lengthCounter = 256 - value;	//chan 3 uses full 8-bit value for length load
		if ((address - 0xFF1A) == 0 && ((value >> 7) & 0b1) ==0)	//DAC=0 disables channel in NR52
			NR52 &= 0b11111011;
		if ((address - 0xFF1A) == 2)
		{
			chan3_volume = (value >> 5) & 0b11;
		}
		if ((address - 0xFF1A) == 4 && ((value >> 7) & 0b1))
		{
			NR52 |= 0b00000100;
			if (chan3_lengthCounter == 0)
				chan3_lengthCounter = 256;
			uint8_t freqLow = m_channels[2].r[3];
			uint8_t freqHigh = m_channels[2].r[4] & 0b00000111;
			uint16_t newFreq = (freqHigh << 8) | freqLow;
			chan3_freqTimer = (2048 - newFreq) * 2;
		}
		m_channels[2].r[address - 0xFF1A] = value;
	}
	if (address >= 0xFF1F && address <= 0xFF23)
	{
		if (address - 0xFF1F == 1)
		{
			chan4_lengthCounter = 64 - (value & 0b00111111);
		}
		if (address - 0xFF1F == 2)
		{
			chan4_volume = ((value >> 4) & 0xF);
			chan4_envelopePeriod = value & 0b111;
			chan4_envelopeTimer = chan4_envelopePeriod;
			chan4_envelopeAdd = ((value >> 3) & 0b1);
		}
		if (address - 0xFF1F == 3)
		{
			chan4_shiftAmount = ((value >> 4) & 0xF);
			chan4_divisorCode = value & 0b111;
			chan4_widthMode = ((value >> 3) & 0b1);
		}
		if (address - 0xFF1F == 4 && ((value >> 7) & 0b1))
		{
			NR52 |= 0b00001000;
			chan4_LFSR = 0xFFFF;
			if (chan4_lengthCounter == 0)
				chan4_lengthCounter = 64;
			//load frequency timer
			int divisor = chan4_divisorMapping[chan4_divisorCode];
			chan4_freqTimer = divisor << chan4_shiftAmount;
		}
		m_channels[3].r[address - 0xFF1F] = value;
	}
	if (address == 0xFF24)
		NR50 = value;
	if (address == 0xFF25)
		NR51 = value;
	if (address == 0xFF26)
	{
		NR52 &= 0b01111111;
		NR52 |= (value & 0b10000000);
		if (!((NR52 >> 7) & 0b1))
			m_clearRegisters();
	}
	if (address >= 0xFF30 && address <= 0xFF3F)
		m_waveRAM[address - 0xFF30] = value;
}

uint8_t APU::readIORegister(uint16_t address)
{
	if (address >= 0xFF10 && address <= 0xFF14)
		return m_channels[0].r[address - 0xFF10] | m_NR1XMasks[address - 0xFF10];
	if (address >= 0xFF15 && address <= 0xFF19)
		return m_channels[1].r[address - 0xFF15] | m_NR2XMasks[address - 0xFF15];
	if (address >= 0xFF1A && address <= 0xFF1E)
		return m_channels[2].r[address - 0xFF1A] | m_NR3XMasks[address - 0xFF1A];
	if (address >= 0xFF1F && address <= 0xFF23)
		return m_channels[3].r[address - 0xFF1F] | m_NR4XMasks[address - 0xFF1F];
	if (address == 0xFF24)
		return NR50;
	if (address == 0xFF25)
		return NR51;
	if (address == 0xFF26)
		return NR52 | m_NR52Mask;
	if (address >= 0xFF30 && address <= 0xFF3F)
		return m_waveRAM[address - 0xFF30];
	
	Logger::getInstance()->msg(LoggerSeverity::Warn, std::format("Invalid read from APU IO register. IO port=0x{:x}", (int)address));
	return 0xFF;	//unused registers return ff
}

void APU::m_clearRegisters()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 5; j++)
			m_channels[i].r[j] = {};
	}
	NR50 = 0;
	NR51 = 0;
	NR52 = 0;
}

void APU::clockLengthCounters()
{
	//clock length counters
	bool chan1_lengthEnabled = (m_channels[0].r[4] >> 6) & 0b1;
	if (chan1_lengthEnabled)
	{
		if (chan1_lengthCounter != 0)
			chan1_lengthCounter--;
		if (chan1_lengthCounter == 0)
		{
			NR52 &= 0b11111110;	//clear channel 1 enabled bit
		}
	}

	bool chan2_lengthEnabled = (m_channels[1].r[4] >> 6) & 0b1;
	if (chan2_lengthEnabled)
	{
		if (chan2_lengthCounter != 0)
			chan2_lengthCounter--;
		if (chan2_lengthCounter == 0)
		{
			NR52 &= 0b11111101;	//clear channel 2 enabled bit
		}
	}

	bool chan3_lengthEnabled = (m_channels[2].r[4] >> 6) & 0b1;
	if (chan3_lengthEnabled)
	{
		if (chan3_lengthCounter != 0)
			chan3_lengthCounter--;
		if (chan3_lengthCounter == 0)
		{
			NR52 &= 0b11111011;	//clear chan 3 bit
		}
	}

	bool chan4_lengthEnabled = (m_channels[3].r[4] >> 6) & 0b1;
	if (chan4_lengthEnabled)
	{
		if (chan4_lengthCounter != 0)
			chan4_lengthCounter--;
		if (chan4_lengthCounter == 0)
		{
			NR52 &= 0b11110111;	//clear chan 4 bit
		}
	}
}

void APU::clockEnvelope()
{
	//chan 1:
	bool chan1_enabled = NR52 & 0b1;
	if (chan1_enabled)
	{
		if (chan1_envelopePeriod != 0)
		{
			chan1_envelopeTimer--;
			if (chan1_envelopeTimer == 0)
			{
				chan1_envelopeTimer = chan1_envelopePeriod;
				if (chan1_envelopeAdd && chan1_volume < 15)
					chan1_volume++;
				if (!chan1_envelopeAdd && chan1_volume > 0)
					chan1_volume--;
			}
		}
	}
	//chan 2:
	bool chan2_enabled = (NR52 >> 1) & 0b1;
	if (chan2_enabled)
	{
		if (chan2_envelopePeriod != 0)
		{
			chan2_envelopeTimer--;
			if (chan2_envelopeTimer == 0)
			{
				chan2_envelopeTimer = chan2_envelopePeriod;
				if (chan2_envelopeAdd && chan2_volume < 15)
					chan2_volume++;
				if (!chan2_envelopeAdd && chan2_volume > 0)
					chan2_volume--;
			}
		}
	}
	//chan 4:
	bool chan4_enabled = (NR52 >> 3) & 0b1;
	if (chan4_enabled)
	{
		if (chan4_envelopePeriod != 0)
		{
			chan4_envelopeTimer--;
			if (chan4_envelopeTimer == 0)
			{
				chan4_envelopeTimer = chan4_envelopePeriod;
				if (chan4_envelopeAdd && chan4_volume < 15)
					chan4_volume++;
				if (!chan4_envelopeAdd && chan4_volume > 0)
					chan4_volume--;
			}
		}
	}
}

float APU::chan2_getOutput()
{
	bool chan2_enabled = (NR52 >> 1) & 0b1;
	if (chan2_enabled)
	{
		int chan2_dutyIdx = (m_channels[1].r[1] & 0b11000000) >> 6;
		chan2_amplitude = (m_dutyTable[chan2_dutyIdx] >> (chan2_waveDutyPosition)) & 0b1;
		//set DAC output of channel 2 (amplitude * volume)
		float dac_input = chan2_amplitude * chan2_volume;
		return (dac_input / 7.5) - 1.0;
	}
	return 0.0f;
}

float APU::chan1_getOutput()
{
	bool chan1_enabled = (NR52 & 0b1);
	if (chan1_enabled)
	{
		int chan1_dutyIdx = (m_channels[0].r[1] & 0b11000000) >> 6;
		chan1_amplitude = (m_dutyTable[chan1_dutyIdx] >> (chan1_waveDutyPosition)) & 0b1;
		float dac_input = chan1_amplitude * chan1_volume;
		return (dac_input / 7.5) - 1.0;
	}
	return 0.0f;
}

float APU::chan3_getOutput()
{
	bool chan3_enabled = (NR52 >> 2) & 0b1;
	if (chan3_enabled)
	{
		//get 4-bit sample
		int waveTblIdx = chan3_samplePosition / 2;
		uint8_t sample = 0;
		uint8_t waveTblEntry = m_waveRAM[waveTblIdx];
		if (chan3_samplePosition % 2 == 0)
		{
			sample = (waveTblEntry >> 4) & 0xF;
		}
		else
		{
			sample = waveTblEntry & 0xF;
		}

		switch (chan3_volume)	//no envelope in channel 3!
		{
		case 0b00:
			sample >>= 4; break;
		case 0b01:
			break;
		case 0b10:
			sample >>= 1; break;
		case 0b11:
			sample >>= 2; break;
		}

		float dac_input = sample * 1.f;//hack, use volume
		return (dac_input / 7.5) - 1.0;
	}
	return 0.0f;
}

float APU::chan4_getOutput()
{
	bool chan4_enabled = (NR52 >> 3) & 0b1;
	if (chan4_enabled)
	{
		uint8_t chan4_amplitude = ~chan4_LFSR & 0b1;
		float dac_input = chan4_amplitude * chan4_volume;
		return (dac_input / 7.5) - 1.0;
	}
	return 0.0f;
}

bool APU::getChannelEnabledLeft(int idx)
{
	idx &= 0b11;	//bitmask to ensure index is 0-3
	return (NR51 >> (idx + 4)) & 0b1;
}

bool APU::getChannelEnabledRight(int idx)
{
	idx &= 0b11;
	return (NR51 >> idx) & 0b1;
}

float APU::capacitor = 0.0f;
float APU::highPass(float in, bool dacsEnabled)
{
	float out = 0.0f;
	if (dacsEnabled)
	{
		out = in - capacitor;
		capacitor = in - out * 0.996336;
	}
	return out;
}