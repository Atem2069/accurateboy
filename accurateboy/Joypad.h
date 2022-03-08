#pragma once

#include"Logger.h"
#include"dmgRegisters.h"

struct JoypadState
{
	bool A;
	bool B;
	bool up;
	bool down;
	bool left;
	bool right;
	bool start;
	bool select;
};

class Joypad
{
public:
	Joypad();
	~Joypad();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

	void updateKeys(JoypadState newState);

private:
	uint8_t JOYP;
};