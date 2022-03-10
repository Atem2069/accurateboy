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
	uint16_t m_lastDivider = m_divider;
	m_divider++;

	m_tickTIMA(m_lastDivider, m_divider);
}

void Timer::m_tickTIMA(uint16_t lastDiv, uint16_t newDiv)
{
	bool timerEnabled = (TAC >> 2) & 0b1;
	uint8_t timerMode = (TAC & 0b11);
	if (timerEnabled)
	{
		int m_shiftAmount = 0;
		switch (timerMode)	//the shift is -1 bc it's edge triggered
		{
		case 0b00:
			m_shiftAmount = 9; break;
		case 0b01:
			m_shiftAmount = 3; break;
		case 0b10:
			m_shiftAmount = 5; break;
		case 0b11:
			m_shiftAmount = 7; break;
		}

		bool shouldTick = ((lastDiv >> m_shiftAmount) & 0b1) && (!((newDiv >> m_shiftAmount) & 0b1));	//falling edge (1 in last tick, 0 now)

		if (shouldTick)
		{
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
		return (m_divider >> 8) & 0xFF; break;
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
		m_tickTIMA(m_divider, 0);
		m_divider = 0; break;
	case REG_TIMA:
		TIMA = value; break;
	case REG_TMA:
		TMA = value; break;
	case REG_TAC:
		TAC = value; break;
	}
}