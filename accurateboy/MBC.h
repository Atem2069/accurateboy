#pragma once

#include"Cartridge.h"
#include"dmgRegisters.h"
#include"Config.h"

class MBC1 : public Cartridge
{
public:
	MBC1(std::vector<uint8_t> m_ROM);
	~MBC1();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
private:
	uint8_t m_bankNumber = 1, m_higherBankBits = 0;
	uint8_t m_ramBankNumber = 0;
	uint8_t m_maxROMBanks = 0, m_maxRAMBanks = 0;
	bool m_RAMBanking = false;
	bool m_SRAMEnabled = false;
	bool m_shouldSave = false;
	std::string m_saveName;
	std::array<std::array<uint8_t, 16384>, 128> m_ROMBanks;
	std::array<std::array<uint8_t, 8192>, 4> m_RAMBanks;
};