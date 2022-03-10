#include "InterruptManager.h"

InterruptManager::InterruptManager()
{

}

InterruptManager::~InterruptManager()
{

}

void InterruptManager::requestInterrupt(InterruptType interrupt)
{
	switch (interrupt)
	{
	case InterruptType::VBlank:
		IFLAGS |= 0b00000001;
		break;
	case InterruptType::STAT:
		IFLAGS |= 0b00000010;
		break;
	case InterruptType::Timer:
		IFLAGS |= 0b00000100;
		break;
	case InterruptType::Joypad:
		IFLAGS |= 0b00010000;
		break;
	}

}

InterruptType InterruptManager::getActiveInterrupt()
{
	//uint8_t intFlags = m_bus->read(REG_IFLAGS);
	//uint8_t enabledInterrupts = m_bus->read(REG_IE);

	uint8_t tempIntFlags = IFLAGS;

	uint8_t activeInterrupts = IFLAGS & IE;
	InterruptType chosenInterruptType = InterruptType::None;
	if ((activeInterrupts & 0b1))	//todo bad code rewrite
	{
		tempIntFlags &= 0b11111110;
		chosenInterruptType = InterruptType::VBlank;
	}
	else if ((activeInterrupts >> 1) & 0b1)
	{
		tempIntFlags &= 0b11111101;
		chosenInterruptType = InterruptType::STAT;
	}
	else if ((activeInterrupts >> 2) & 0b1)
	{
		tempIntFlags &= 0b11111011;
		chosenInterruptType = InterruptType::Timer;
	}
	else if ((activeInterrupts >> 4) & 0b1)
	{
		tempIntFlags &= 0b11101111;
		chosenInterruptType = InterruptType::Joypad;
	}

	if (interruptsEnabled)				//only turn down flags if interrupts enabled
		IFLAGS = tempIntFlags;

	return chosenInterruptType;
}

void InterruptManager::enableInterrupts()
{
	interruptsEnabled = true;
}

void InterruptManager::disableInterrupts()
{
	interruptsEnabled = false;
}

bool InterruptManager::getInterruptsEnabled()
{
	return interruptsEnabled;
}

uint8_t InterruptManager::read(uint16_t address)
{
	switch (address)
	{
	case REG_IE:
		return IE; break;
	case REG_IFLAGS:
		return IFLAGS | 0b11100000; break;
	}

	Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid interrupt IO register read");
	return 0xFF;
}

void InterruptManager::write(uint16_t address, uint8_t value)
{
	switch (address)
	{
	case REG_IE:
		IE = value; break;
	case REG_IFLAGS:
		IFLAGS = value; break;
	}
}