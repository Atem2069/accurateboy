#pragma once

#include<chrono>
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

class MBC3 : public Cartridge
{
public:
	MBC3(std::vector<uint8_t> m_ROM);
	~MBC3();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
private:
	uint16_t m_bankNumber = 1;
	uint8_t m_ramBankNumber = 0;
	uint8_t m_maxRAMBanks;
	uint8_t m_lastLatchWrite = 0xff;	//writing 0 then 1 toggles latching/unlatching rtc
	bool m_rtcLatched = false;
	bool m_SRAMEnabled = false;
	bool m_shouldSave = false;
	std::string m_saveName;
	std::chrono::steady_clock::time_point m_rtcStart, m_rtcLast;
	std::array<std::array<uint8_t, 16384>, 128> m_ROMBanks;
	std::array<std::array<uint8_t, 8192>, 8> m_RAMBanks;
};

class MBC5 : public Cartridge
{
public:
	MBC5(std::vector<uint8_t> m_ROM);
	~MBC5();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
private:
	uint16_t m_bankNumber = 1;
	uint16_t m_bankNumberHighBit = 0;
	uint8_t m_ramBankNumber = 0;
	uint16_t m_maxROMBanks, m_maxRAMBanks;
	bool m_SRAMEnabled = false;
	bool m_shouldSave = false;
	std::string m_saveName;
	std::array<std::array<uint8_t, 16384>, 512> m_ROMBanks;
	std::array<std::array<uint8_t, 8192>, 8> m_RAMBanks;
};