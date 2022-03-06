#include"Bus.h"

Bus::Bus(std::vector<uint8_t> romData, std::shared_ptr<InterruptManager>& interruptManager, std::shared_ptr<PPU>& ppu, std::shared_ptr<APU>& apu)
{
	m_cartridge = std::make_shared<Cartridge>(romData);
	m_interruptManager = interruptManager;
	m_ppu = ppu;
	m_apu = apu;
	m_inBootRom = true;
}

Bus::~Bus()
{

}

uint8_t Bus::read(uint16_t address)
{
	if (m_inBootRom && address <= 0xFF)
		return m_bootRom[address];

	if ((address <= 0x7FFF) || (address >= 0xA000 && address <= 0xBFFF))
		return m_cartridge->read(address);
	if ((address >= 0x8000 && address <= 0x9FFF) || (address >= 0xFE00 && address <= 0xFE9F))
	{
		return m_ppu->read(address);
	}
	if (address >= 0xC000 && address <= 0xFDFF)
	{
		if (address > 0xDFFF)
			address -= 0xE000;
		else
			address -= 0xC000;

		return m_WRAM[address];
	}
	if (address >= 0xFF80 && address <= 0xFFFE)
	{
		return m_HRAM[address - 0xFF80];
	}

	if (address >= 0xFF00 && address <= 0xFF7F)
	{
		//special case: apu
		if (address >= 0xFF10 && address <= 0xFF3F)
			return m_apu->readIORegister(address);
		switch (address)
		{

		case REG_LCDC: case REG_STAT: case REG_SCY: case REG_SCX: case REG_WY: case REG_WX: case REG_LY: case REG_LYC: case 0xFF47: case 0xFF48: case 0xFF49:
			return m_ppu->read(address); break;
		case REG_IE: case REG_IFLAGS:
			return m_interruptManager->read(address); break;
		}
	}


	return 0xFF;
}

void Bus::write(uint16_t address, uint8_t value)
{
	if ((address <= 0x7FFF) || (address >= 0xA000 && address <= 0xBFFF))
		m_cartridge->write(address, value);
	if ((address >= 0x8000 && address <= 0x9FFF) || (address >= 0xFE00 && address <= 0xFE9F))
	{
		m_ppu->write(address, value);
	}
	if (address >= 0xC000 && address <= 0xFDFF)
	{
		if (address > 0xDFFF)
			address -= 0xE000;
		else
			address -= 0xC000;

		m_WRAM[address] = value;
	}
	if (address >= 0xFF80 && address <= 0xFFFE)
	{
		m_HRAM[address - 0xFF80] = value;
		return;
	}

	if ((address >= 0xFF00 && address <= 0xFF7F) || address==0xFFFF)
	{
		if (address >= 0xFF10 && address <= 0xFF3F)
			m_apu->writeIORegister(address, value);
		switch (address)
		{
		case REG_LCDC: case REG_STAT: case REG_SCY: case REG_SCX: case REG_WY: case REG_WX: case REG_LY: case REG_LYC: case 0xFF47: case 0xFF48: case 0xFF49:
			m_ppu->write(address, value); break;
		case REG_IE: case REG_IFLAGS:
			m_interruptManager->write(address, value); break;
		case 0xFF01:
			std::cout << value; break;
		case 0xFF50:		//boot rom lockout
		{
			Logger::getInstance()->msg(LoggerSeverity::Info, "Unmapping boot ROM. .");
			m_inBootRom = false;
			break;
		}
		}
	}


}

void Bus::tick()
{
	//ticks all components
	m_ppu->step();
	m_apu->step();
}