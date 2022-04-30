#include"Joypad.h"

Joypad::Joypad()
{
	JOYP = 0xCF;
}

Joypad::~Joypad()
{

}

uint8_t Joypad::read(uint16_t address)
{
	return JOYP | 0b11000000;	//there is literally no way any other address could be read
}

void Joypad::write(uint16_t address, uint8_t value)
{
	JOYP &= 0b11001111;
	JOYP |= (value & 0b00110000);
}

void Joypad::updateKeys(JoypadState newState)
{
	bool actionKeys = ((JOYP >> 4) & 0b1);	//if direction buttons check reads 1, then action keys must be selected instead
	JOYP |= 0b11001111;
	if (actionKeys)
	{
		if (newState.A)
			JOYP &= ~0b1;
		if (newState.B)
			JOYP &= ~0b10;
		if (newState.select)
			JOYP &= ~0b100;
		if (newState.start)
			JOYP &= ~0b1000;
	}
	else
	{
		if (newState.right)
			JOYP &= ~0b1;
		if (newState.left)
			JOYP &= ~0b10;
		if (newState.up)
			JOYP &= ~0b100;
		if (newState.down)
			JOYP &= ~0b1000;
	}
}