#pragma once

#include<iostream>
#include<array>
#include"Logger.h"
#include"Cartridge.h"

class Cartridge
{
public:
	Cartridge() {};
	Cartridge(std::vector<uint8_t> ROM);
	~Cartridge();

	virtual uint8_t read(uint16_t address);
	virtual void write(uint16_t address, uint8_t value);

private:
	std::array<uint8_t, 32768> m_ROM;
};