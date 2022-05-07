#include"MBC.h"

MBC1::MBC1(std::vector<uint8_t> ROM)
{
	int count = 0;
	for (int i = 0; i < ROM.size() - 16384; i += 16384)
	{
		std::array<uint8_t, 16384> curROMBank;
		std::copy(ROM.begin() + i, ROM.begin() + i + 16384, curROMBank.begin());
		m_ROMBanks[count++] = curROMBank;
	}


	uint8_t cartType = ROM[CART_TYPE];
	if (cartType == 3 && !Config::GB.System.inDebug)
		m_shouldSave = true;

	m_maxROMBanks = (uint8_t)std::pow(2, (double)ROM[CART_ROMSIZE] + 1);	
	
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

	m_bankNumber = 1;
}

MBC1::~MBC1()
{
	if (m_shouldSave)
	{
		Logger::getInstance()->msg(LoggerSeverity::Info, "Saving game memory..");
		std::ofstream ramWriteHandle(m_saveName, std::ios::out | std::ios::binary);
		for(int i = 0; i < m_maxRAMBanks; i++)
			ramWriteHandle.write((const char*)m_RAMBanks[i].data(), m_RAMBanks[i].size());
		ramWriteHandle.close();
	}
}

uint8_t MBC1::read(uint16_t address)
{

	if (address >= 0x4000 && address < 0x8000)
	{
		int offset = ((int)address) - 0x4000;
		return m_ROMBanks[m_bankNumber][offset];
	}

	if (address >= 0xa000 && address <= 0xbfff)
	{
		if (!m_SRAMEnabled)
			return 0xFF;
		int offset = ((int)address - 0xa000);
		return m_RAMBanks[m_ramBankNumber][offset];
	}
	
	//if (address >= 0x4000 && address < 0x8000 && m_bankNumber == 0)
	//	return m_ROMBanks[1][(int)address - 0x4000];
	if(address <= 0x3fff)
		return m_ROMBanks[0][address];

}

void MBC1::write(uint16_t address, uint8_t value)
{

	if (address >= 0x0000 && address <= 0x1fff)
	{
		if ((value & 0x0f) == 0x0A)
			m_SRAMEnabled = true;
		else
			m_SRAMEnabled = false;
		return;
	}

	if (address >= 0x4000 && address <= 0x5fff)
	{
		if (!m_RAMBanking)
			m_higherBankBits = (value & 0b00000011);
		else if(m_maxRAMBanks)
			m_ramBankNumber = (value & 0b00000011) % m_maxRAMBanks;
		return;
	}

	if (address >= 0x6000 && address <= 0x7fff)
	{
		if ((value & 0b1) == 0)
		{
			m_RAMBanking = false;
			m_ramBankNumber = 0;
		}
		if ((value & 0b1) == 1)
		{
			m_higherBankBits = 0;
			m_RAMBanking = true;
			m_bankNumber &= 0b00011111;
		}

		return;
	}

	if (address >= 0x2000 && address <= 0x3FFF)
	{
		value = value & 0b00011111;
		if (!value)
			value = 1;
		m_bankNumber = ((m_higherBankBits << 5) | value) % m_maxROMBanks;
		return;
	}

	if (address >= 0xa000 && address <= 0xbfff)
	{
		if (!m_SRAMEnabled)
			return;
		int offset = ((int)address - 0xa000);
		m_RAMBanks[m_ramBankNumber][offset] = value;
	}

	//MMUState curState = { m_isInBIOS,m_bankNumber,m_ramBankNumber };
	//Config::getInstance()->setValue<MMUState>("MMUState", curState);
}
