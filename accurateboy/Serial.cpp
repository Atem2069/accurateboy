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
	//todo
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
		SC = value; break;	//bitmask probably sorts this out anyway
	}
}