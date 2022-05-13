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
	if (m_timerIrqCycles > 0)
	{
		m_timerIrqCycles++;
		if (m_timerIrqCycles == 4)
			m_timerIrqCycles = 0;
	}
	if (m_timerReloading)
	{
		m_timerReloadCycles++;
		if (m_timerReloadCycles == 4)
		{
			m_timerReloadCycles = 0;
			m_timerIrqCycles = 1;
			m_timerReloading = false;
			TIMA = TMA;
			m_interruptManager->requestInterrupt(InterruptType::Timer);
		}
	}

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
		int m_shiftAmount = m_convertMuxToShiftAmount(timerMode);

		bool shouldTick = ((lastDiv >> m_shiftAmount) & 0b1) && (!((newDiv >> m_shiftAmount) & 0b1));	//falling edge (1 in last tick, 0 now)

		if (shouldTick)
		{
			TIMA++;
			if (TIMA == 0)
			{
				m_timerReloadCycles = 0;
				m_timerReloading = true;
			}
		}

	}
}

void Timer::m_checkTACMuxChange(uint8_t newTAC)
{
	if (!((newTAC >> 2) & 0b1))
		return;
	uint8_t oldTimerMode = (TAC & 0b11);
	uint8_t newTimerMode = (newTAC & 0b11);
	int oldTimerShift = m_convertMuxToShiftAmount(oldTimerMode);
	int newTimerShift = m_convertMuxToShiftAmount(newTimerMode);
	if (!((m_divider >> oldTimerShift) & 0b1) && ((m_divider >> newTimerShift) & 0b1))
	{
		TIMA++;
		if (TIMA == 0)
		{
			m_timerReloadCycles = 0;
			m_timerReloading = true;
		}
	}
}

int Timer::m_convertMuxToShiftAmount(uint8_t mux)
{
	int m_shiftAmount = 0;
	switch (mux)
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
	return m_shiftAmount;
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
		return TAC | 0b11111000; break;
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
		if (!m_timerIrqCycles)	//TIMA is always writable, except for the cycle where TIMA <- TMA.
		{
			TIMA = value;
			m_timerReloading = false;
			m_timerReloadCycles = 0;
		}
		break;
	case REG_TMA:
		TMA = value;
		if (m_timerIrqCycles)	//if TMA written on same m-cycle that the irq is raised (i.e. TIMA is loaded), then TIMA gets loaded with the new value from TMA
			TIMA = TMA;
		break;
	case REG_TAC:
		if (((TAC >> 2) & 0b1) && !((value >> 2) & 0b1))
		{
			m_tickTIMA(m_divider, 0);	//check for falling edge and tick timer (weird behaviour when tac disabled)
			if (m_timerReloading && m_timerReloadCycles == 0)
			{
				m_timerReloadCycles = 0;
				m_timerReloading = false;
				TIMA = TMA;
				m_interruptManager->requestInterrupt(InterruptType::Timer);
			}
		}
		m_checkTACMuxChange(value);
		TAC = value;
		break;
	}
}

uint16_t Timer::getClockDivider()
{
	return m_divider;
}