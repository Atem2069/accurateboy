#include"Serial.h"

Serial::Serial(std::shared_ptr<InterruptManager>& interruptManager)
{
	m_interruptManager = interruptManager;
}

Serial::~Serial()
{

}

void Serial::step(uint16_t newClockDivider)
{
	bool transferEnabled = (SC >> 7) & 0b1;
	if (transferEnabled)
	{
		if (!(SC & 0b1))
			return;

		//8192 hz = bit 8
		uint16_t m_lastDividerBit = ((m_lastClockDivider >> 8) & 0b1);
		uint16_t m_newDividerBit = ((newClockDivider >> 8) & 0b1);
		if (m_lastDividerBit && !m_newDividerBit)	//falling edge detected - do shift
		{
			SB <<= 1;	//right shift and shift in a 1
			SB |= 0b1;
			m_shiftCounter++;
			if (m_shiftCounter == 8)
			{
				m_shiftCounter = 0;
				SC &= 0b01111111;
				m_interruptManager->requestInterrupt(InterruptType::Serial);
			}
		}
	}
	m_lastClockDivider = newClockDivider;
}

uint8_t Serial::read(uint16_t address)
{
	switch (address)
	{
	case REG_SB:
		return SB; break;
	case REG_SC:
		return SC | 0b01111110;	//mask off unused bits (in DMG, bit 1 is unused. CGB it is writeable and controls transfer speed)
	}
}

void Serial::write(uint16_t address, uint8_t value)
{
	switch (address)
	{
	case REG_SB:
		if (Config::GB.System.serial)
			std::cout << value;
		SB = value; break;
	case REG_SC:
		if (((SC >> 7) & 0b1) && !((value >> 7) & 0b1))
			m_shiftCounter = 0;
		SC = value;
		break;	//bitmask probably sorts this out anyway
	}
}