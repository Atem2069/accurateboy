#include"MBC.h"

MBC5::MBC5(std::vector<uint8_t> ROM)
{
	int count = 0;
	for (int i = 0; i < ROM.size() - 16384; i += 16384)
	{
		std::array<uint8_t, 16384> curROMBank;
		std::copy(ROM.begin() + i, ROM.begin() + i + 16384, curROMBank.begin());
		m_ROMBanks[count++] = curROMBank;
	}
	m_ramBankNumber = 0;

	uint8_t cartType = ROM[CART_TYPE];
	if (cartType == 0x1B || cartType == 0x1E)
		m_shouldSave = true;

	m_maxROMBanks = (uint16_t)std::pow(2, (double)ROM[CART_ROMSIZE] + 1);

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

MBC5::~MBC5()
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

uint8_t MBC5::read(uint16_t address)
{

	if (address >= 0x4000 && address < 0x8000)
	{
		int offset = ((int)address) - 0x4000;
		return m_ROMBanks[m_bankNumber][offset];
	}

	if (address >= 0xA000 && address <= 0xBFFF)
	{
		if (m_SRAMEnabled)
			return m_RAMBanks[m_ramBankNumber][address - 0xA000];
		return 0xFF;
	}

	if(address <= 0x3FFF)
		return m_ROMBanks[0][address];
}

void MBC5::write(uint16_t address, uint8_t value)
{
	if (address >= 0x0000 && address <= 0x1fff)
	{
		if (value == 0x0A)
			m_SRAMEnabled = true;
		else if (value == 0x00)
			m_SRAMEnabled = false;	//ignore all other values, only 0a enables and 00 disables
	}

	if (address >= 0x4000 && address <= 0x5fff)
	{
		if (!m_maxRAMBanks)
			return;
		m_ramBankNumber = value % m_maxRAMBanks;
	}


	if (address >= 0x2000 && address <= 0x2FFF)
	{
		m_bankNumber = ((m_bankNumberHighBit << 8) | value) % m_maxROMBanks;
	}

	if (address >= 0x3000 && address <= 0x3FFF)
	{
		m_bankNumberHighBit = value & 0b1;
	}

	if (address >= 0xA000 && address <= 0xBFFF)
	{
		if(m_SRAMEnabled)
			m_RAMBanks[m_ramBankNumber][address - 0xA000] = value;
	}
	//MMUState curState = { m_isInBIOS,m_bankNumber,m_ramBankNumber };
	//Config::getInstance()->setValue<MMUState>("MMUState", curState);
}

