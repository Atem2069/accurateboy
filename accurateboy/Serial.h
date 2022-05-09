#pragma once

#include"Logger.h"
#include"dmgRegisters.h"
#include"InterruptManager.h"
#include"Config.h"

//serial stub class - doesn't actually do any real serial comms, just emulates what happens when nothing is connected
class Serial
{
public:
	Serial(std::shared_ptr<InterruptManager>& interruptManager);
	~Serial();

	void step(uint16_t newClockDivider);

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
private:
	std::shared_ptr<InterruptManager> m_interruptManager;
	uint16_t m_lastClockDivider = 0;	//on reset, clock divider is 0
	uint8_t SB = 0, SC = 0;	//SB: serial transfer data. SC: serial transfer control
};