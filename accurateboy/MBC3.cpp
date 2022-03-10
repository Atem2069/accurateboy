#include"MBC.h"

MBC3::MBC3(std::vector<uint8_t> ROM)
{
	int count = 0;
	for (int i = 0; i < ROM.size() - 16384; i += 16384)
	{
		std::array<uint8_t, 16384> curROMBank;
		std::copy(ROM.begin() + i, ROM.begin() + i + 16384, curROMBank.begin());
		m_ROMBanks[count++] = curROMBank;
	}

	m_rtcStart = std::chrono::high_resolution_clock::now();

	uint8_t cartType = ROM[CART_TYPE];
	if (cartType == 0x10 || cartType==0x13)
		m_shouldSave = true;

	//need lookup table for RAM banks..
	uint8_t ramBanks[6] = { 0, 1, 1, 4, 16, 8 };
	m_maxRAMBanks = ramBanks[ROM[CART_RAMSIZE] % 6];

	if (m_shouldSave)
	{
		//load save file into RAM bank 0 (only if cart has battery)
		m_saveName = Config::GB.System.RomName;
		if (m_saveName[m_saveName.size() - 1] == 'b')
			m_saveName.resize(m_saveName.size() - 2);
		else
			m_saveName.resize(m_saveName.size() - 3);
		m_saveName += "sav";
		std::ifstream ramReadHandle(m_saveName, std::ios::in | std::ios::binary);
		if (!ramReadHandle)
		{
			Logger::getInstance()->msg(LoggerSeverity::Info, "No save file exists for current ROM - file will be created upon unload!");
		}
		else
		{
			ramReadHandle >> std::noskipws;
			int i = 0;
			int bank = 0;
			while (!ramReadHandle.eof())
			{
				unsigned char curByte;
				ramReadHandle.read((char*)&curByte, sizeof(uint8_t));
				m_RAMBanks[bank][i] = (uint8_t)curByte;
				i++;
				if (i == 8192)
				{
					i = 0;
					bank += 1;
				}
			}

			ramReadHandle.close();
		}
	}
}

MBC3::~MBC3()
{
	if (m_shouldSave)
	{
		Logger::getInstance()->msg(LoggerSeverity::Info, "Saving game memory..");
		std::ofstream ramWriteHandle(m_saveName, std::ios::out | std::ios::binary);
		for (int i = 0; i < m_maxRAMBanks; i++)
			ramWriteHandle.write((const char*)m_RAMBanks[i].data(), m_RAMBanks[i].size());
		ramWriteHandle.close();
	}
}

uint8_t MBC3::read(uint16_t address)
{

	if (address >= 0x4000 && address < 0x8000 && m_bankNumber)
	{
		int offset = ((int)address) - 0x4000;
		return m_ROMBanks[m_bankNumber][offset];
	}

	if (address >= 0xA000 && address <= 0xBFFF && m_ramBankNumber >= 0x08)
	{
		if(!m_rtcLatched)
			m_rtcLast = std::chrono::high_resolution_clock::now();	//only update rtc if not latched
		double timeDiff = std::chrono::duration<double, std::milli>(m_rtcLast - m_rtcStart).count() / 1000.0;	//time diff in seconds
		if (m_ramBankNumber == 8)
			return (uint8_t)(std::fmod(timeDiff, 60));
		else if (m_ramBankNumber == 9)
			return (uint8_t)(std::fmod(timeDiff/60, 60));
		return 0;	//hour,days is redundant.

	}

	if (address >= 0x4000 && address < 0x8000 && m_bankNumber == 0)
		return m_ROMBanks[1][(int)address - 0x4000];

	if (address >= 0xA000 && address <= 0xBFFF && m_ramBankNumber < 0x08)
	{
		if(m_SRAMEnabled)
			return m_RAMBanks[m_ramBankNumber][address - 0xA000];
		return 0xFF;
	}

	if(address <= 0x3FFF)
		return m_ROMBanks[0][address];
}

void MBC3::write(uint16_t address, uint8_t value)
{
	if (address >= 0x0000 && address <= 0x1fff)
	{
		value &= 0x0F;
		if (value == 0x0A)
			m_SRAMEnabled = true;
		else
			m_SRAMEnabled = false;
		return;
	}

	if (address >= 0x4000 && address <= 0x5fff)
	{
		m_ramBankNumber = value;
		return;
	}

	if (address >= 0x2000 && address <= 0x3FFF)
	{
		m_bankNumber =  value;
		return;
	}

	if (address >= 0x6000 && address <= 0x7fff)
	{
		if (m_lastLatchWrite == 0 && value == 1)
			m_rtcLatched = !m_rtcLatched;
		m_lastLatchWrite = value;
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF && m_ramBankNumber < 0x08)
	{
		if(m_SRAMEnabled)
			m_RAMBanks[m_ramBankNumber][address - 0xA000] = value;
	}

	//m_memory[address] = value;

	//MMUState curState = { m_isInBIOS,m_bankNumber,m_ramBankNumber };
	//Config::getInstance()->setValue<MMUState>("MMUState", curState);
}
