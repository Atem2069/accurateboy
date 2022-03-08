#include"Timer.h"

Timer::Timer(std::shared_ptr<InterruptManager>& interruptManager)
{
	m_interruptManager = interruptManager;
}

Timer::~Timer()
{
	
}

void Timer::step()
{
	for (int i = 0; i < 4; i++)
		m_tickTCycle();
}

void Timer::m_tickTCycle()
{
	divCycleDiff++;
	if (divCycleDiff == 256)
	{
		divCycleDiff = 0;
		DIV++;
	}

	bool timerEnabled = (TAC >> 2) & 0b1;
	uint8_t timerMode = (TAC & 0b11);
	if (timerEnabled)
	{
		timerCycleDiff++;
		int cycleThreshold = 0;
		switch (timerMode)
		{
		case 0b00:
			cycleThreshold = 4194304/4096; break;
		case 0b01:
			cycleThreshold = 4194304/262144; break;
		case 0b10:
			cycleThreshold = 4194304/65536; break;
		case 0b11:
			cycleThreshold = 4194304/16384; break;
		}

		while (timerCycleDiff>=cycleThreshold)
		{
			timerCycleDiff -= cycleThreshold;
			TIMA++;
			if (TIMA == 0)
			{
				TIMA = TMA;
				m_interruptManager->requestInterrupt(InterruptType::Timer);
			}
		}

	}
}

uint8_t Timer::read(uint16_t address)
{
	switch (address)
	{
	case REG_DIV:
		return DIV; break;
	case REG_TIMA:
		return TIMA; break;
	case REG_TMA:
		return TMA; break;
	case REG_TAC:
		return TAC; break;
	}
}

void Timer::write(uint16_t address, uint8_t value)
{
	switch (address)
	{
	case REG_DIV:
		DIV = 0; break;
	case REG_TIMA:
		TIMA = value; break;
	case REG_TMA:
		TMA = value; break;
	case REG_TAC:
		TAC = value; break;
	}
}