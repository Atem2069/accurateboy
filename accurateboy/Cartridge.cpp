#include"Cartridge.h"

Cartridge::Cartridge(std::vector<uint8_t> ROM)
{
	//todo: check cartridge type
	std::copy(ROM.begin(), ROM.begin() + 32768, m_ROM.begin());
}

Cartridge::~Cartridge()
{

}

uint8_t Cartridge::read(uint16_t address)
{
	if (address <= 0x7FFF)
		return m_ROM[address];

	Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid read from SRAM on unmapped cartridge");
	return 0xFF;
}

void Cartridge::write(uint16_t address, uint8_t value)
{
	Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid write to unmapped cartridge, ignoring.");
}