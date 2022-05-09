#include"gbclient_testing.h"

GBTestClient::GBTestClient()
{

}

GBTestClient::~GBTestClient()
{

}

bool GBTestClient::run()
{
	Config::GB.System.inDebug = true;
	std::vector<std::string> results;
	int failCount = 0;
	for (int i = 0; i < m_numTests; i++)
	{
		std::string resultStr = romPaths[i] + ": ";
		m_initialise(romPaths[i]);
		while (!m_cpu->testing_getBreakpointHit())
			m_cpu->step();
		
		//expected results: b=3,c=5,d=8,e=13,h=21,l=34
		if (m_cpu->testing_getRegister(0) == 3
			&& m_cpu->testing_getRegister(1) == 5
			&& m_cpu->testing_getRegister(2) == 8
			&& m_cpu->testing_getRegister(3) == 13
			&& m_cpu->testing_getRegister(4) == 21
			&& m_cpu->testing_getRegister(5) == 34)
		{
			resultStr += "PASS";
		}
		else
		{
			resultStr += "FAIL";
			failCount++;
		}

		results.push_back(resultStr);

		m_destroy();
	}
	
	for (int i = 0; i < results.size(); i++)
		std::cout << results[i] << '\n';
	if (failCount)
		std::cout << "Passed " << (m_numTests-failCount) << ", failed " << failCount << " tests." << '\n';
	else
		std::cout << "Passed all tests!" << '\n';

	return true;
}

void GBTestClient::m_initialise(std::string path)
{
	std::vector<uint8_t> romData;
	std::ifstream romReadHandle(path, std::ios::in | std::ios::binary);
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
	m_timer = std::make_shared<Timer>(m_interruptManager);
	m_joypad = std::make_shared<Joypad>();
	m_serial = std::make_shared<Serial>(m_interruptManager);
	m_bus = std::make_shared<Bus>(romData, m_interruptManager, m_ppu, m_apu, m_timer, m_joypad,m_serial);
	m_cpu = std::make_shared<CPU>(m_bus, m_interruptManager);

	m_bus->setInTestingMode();
}

void GBTestClient::m_destroy()
{
	m_cpu.reset();
	m_bus.reset();
	m_interruptManager.reset();
	m_apu.reset();
	m_timer.reset();
	m_joypad.reset();
	m_serial.reset();
}