#include"PPU.h"

PPU::PPU(std::shared_ptr<InterruptManager>& interruptManager)
{
	m_interruptManager = interruptManager;
}

PPU::~PPU()
{

}

void PPU::step()
{
	//todo
}

uint8_t PPU::read(uint16_t address)
{
	if (address >= 0x8000 && address <= 0x9FFF)
		return m_VRAM[address - 0x8000];
	if (address >= 0xFE00 && address <= 0xFE9F)
		return m_OAM[address - 0xFE00];

	switch (address)
	{
	case REG_LCDC:
		return LCDC; break;
	case REG_STAT:
		return STAT; break;
	case REG_SCX:
		return SCX; break;
	case REG_SCY:
		return SCY; break;
	case REG_WY:
		return WY; break;
	case REG_WX:
		return WX; break;
	case REG_LY:
		return LY; break;
	case REG_LYC:
		return LYC; break;
	case 0xFF47:
		return BGP; break;
	case 0xFF48:
		return OBP0; break;
	case 0xFF49:
		return OBP1; break;
	}

	Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid PPU read");
	return 0xFF;
}

void PPU::write(uint16_t address, uint8_t value)
{
	if (address >= 0x8000 && address <= 0x9FFF)
		m_VRAM[address - 0x8000] = value;
	if (address >= 0xFE00 && address <= 0xFE9F)
		m_OAM[address - 0xFE00] = value;

	switch (address)
	{
	case REG_LCDC:
		LCDC=value; break;
	case REG_STAT:
		STAT=value; break;
	case REG_SCX:
		SCX=value; break;
	case REG_SCY:
		SCY=value; break;
	case REG_WY:
		WY=value; break;
	case REG_WX:
		WX=value; break;
	case REG_LY:
		LY=value; break;
	case REG_LYC:
		LYC=value; break;
	case 0xFF47:
		BGP=value; break;
	case 0xFF48:
		OBP0=value; break;
	case 0xFF49:
		OBP1=value; break;
	}
}

bool PPU::m_getLCDEnabled()
{
	return (LCDC >> 7) & 0b1;
}

bool PPU::m_getWindowNametable()
{
	return (LCDC >> 6) & 0b1;
}

bool PPU::m_getWindowEnabled()
{
	return (LCDC >> 5) & 0b1;
}

bool PPU::m_getTilemap()
{
	return (LCDC >> 4) & 0b1;
}

bool PPU::m_getBackgroundNametable()
{
	return (LCDC >> 3) & 0b1;
}

bool PPU::m_getSpriteSize()
{
	return (LCDC >> 2) & 0b1;
}

bool PPU::m_getSpritesEnabled()
{
	return (LCDC >> 1) & 0b1;
}

bool PPU::m_getBackgroundPriority()
{
	return LCDC & 0b1;
}