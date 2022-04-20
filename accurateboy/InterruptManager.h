#pragma once

#include"Logger.h"
#include"dmgRegisters.h"

enum class InterruptType
{
	None=0,
	VBlank=0x40,				//VBlank interrupt goes to vector 0x40
	STAT=0x48,					//LCD STAT interrupt
	Timer=0x50,					//generated regularly by interval timer
	Serial=0x58,
	Joypad=0x60
};


class InterruptManager
{
public:
	InterruptManager();
	~InterruptManager();

	void requestInterrupt(InterruptType interrupt);
	InterruptType getActiveInterrupt(bool unsetFlag);

	void enableInterrupts();
	void disableInterrupts();
	bool getInterruptsEnabled();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

private:
	bool interruptsEnabled = false;
	uint8_t IFLAGS = 0, IE = 0;
};