#include"Gameboy.h"

GameBoy::GameBoy()
{
	m_initialise();
}

GameBoy::~GameBoy()
{
	m_destroy();
}

void GameBoy::run()
{
	while (true)
	{
		m_cpu->step();
	}
}

void GameBoy::m_initialise()
{

	std::vector<uint8_t> romData;
	std::ifstream romReadHandle("Tests\\01-special.gb", std::ios::in | std::ios::binary);
	if (!romReadHandle)
	{
		//Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid file " + name);
		//return false;
		return;
	}
	romReadHandle >> std::noskipws;
	while (!romReadHandle.eof() && romData.size() <= (8 * 1024 * 1024))
	{
		unsigned char curByte;
		romReadHandle.read((char*)&curByte, sizeof(uint8_t));
		romData.push_back((uint8_t)curByte);
	}

	m_interruptManager = std::make_shared<InterruptManager>();
	m_ppu = std::make_shared<PPU>(m_interruptManager);
	m_bus = std::make_shared<Bus>(romData, m_interruptManager, m_ppu);
	m_cpu = std::make_shared<CPU>(m_bus, m_interruptManager);
}

void GameBoy::m_destroy()
{
	m_cpu.reset();
	m_bus.reset();
	m_interruptManager.reset();
}