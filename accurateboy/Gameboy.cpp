#include"Gameboy.h"

GameBoy::GameBoy()
{
	
}

GameBoy::~GameBoy()
{
	m_destroy();
}

void GameBoy::run()
{

	std::thread displayThread(&GameBoy::m_displayWorker, this);

	while (Config::GB.System.RomName.empty())	//periodically sleep until rom name specified
		Sleep(10);	

	m_initialise();

	while (!m_shouldStop)
	{

		//if need to reset:
		if (Config::GB.System.reset)
		{
			m_destroy();
			m_initialise();
		}

		m_cpu->step();


	}

	displayThread.join();

	m_destroy();
}

void GameBoy::m_initialise()
{
	if (Config::GB.System.RomName.empty())
		return;

	Logger::getInstance()->msg(LoggerSeverity::Info, "Initialise with new ROM - " + Config::GB.System.RomName);

	std::vector<uint8_t> romData;
	std::ifstream romReadHandle(Config::GB.System.RomName, std::ios::in | std::ios::binary);
	if (!romReadHandle)
		return;

	romReadHandle >> std::noskipws;
	while (!romReadHandle.eof() && romData.size() <= (8 * 1024 * 1024))
	{
		unsigned char curByte;
		romReadHandle.read((char*)&curByte, sizeof(uint8_t));
		romData.push_back((uint8_t)curByte);
	}

	m_interruptManager = std::make_shared<InterruptManager>();
	m_ppu = std::make_shared<PPU>(m_interruptManager);
	m_apu = std::make_shared<APU>();
	m_bus = std::make_shared<Bus>(romData, m_interruptManager, m_ppu,m_apu);
	m_cpu = std::make_shared<CPU>(m_bus, m_interruptManager);

	Config::GB.System.reset = false;
}

void GameBoy::m_destroy()
{
	Logger::getInstance()->msg(LoggerSeverity::Info, "Destroying current emulator instance. .");
	m_cpu.reset();
	m_bus.reset();
	m_interruptManager.reset();
	m_apu.reset();
}

void GameBoy::m_displayWorker()
{
	Logger::getInstance()->msg(LoggerSeverity::Info, "Create display thread..");
	m_display = std::make_shared<Display>(160 * 4, 144 * 4);
	
	while (!m_display->shouldClose())
	{
		//update with new ppu info
		m_display->draw();
	}
	m_shouldStop = true;

	Logger::getInstance()->msg(LoggerSeverity::Info, "Display thread de-init!");
}