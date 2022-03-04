#pragma once

#include<iostream>
#include<array>

#include"Logger.h"
#include"InterruptManager.h"
#include"dmgRegisters.h"

class PPU
{
public:
	PPU(std::shared_ptr<InterruptManager>& interruptManager);
	~PPU();

	void step();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
private:
	std::shared_ptr<InterruptManager> m_interruptManager;

	std::array<uint8_t, 8192> m_VRAM;
	std::array<uint8_t, 0xA0> m_OAM;
	//io registers
	uint8_t LCDC = {}, STAT = {}, SCX = {}, SCY = {}, WY = {}, WX = {}, LY = 0x90, LYC = {}, BGP = {}, OBP0 = {}, OBP1 = {};

};