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
	for (int i = 0; i < 4; i++)
		m_tickTCycle();
}

void PPU::m_tickTCycle()
{
	if (!m_getLCDEnabled())
		return;
	uint8_t curPPUMode = STAT & 0b11;

	//todo here: STAT interrupts

	switch (curPPUMode)
	{
	case 0:
		m_hblank(); break;
	case 1:
		m_vblank(); break;
	case 2:
		m_OAMSearch(); break;
	case 3:
		m_LCDTransfer(); break;
	}
}

void PPU::m_hblank()	//mode 0
{
	m_modeCycleDiff++;
	m_totalLineCycles++;
	if (m_totalLineCycles == 456)	//enter line takes 456 t cycles
	{
		m_modeCycleDiff = 0;
		m_totalLineCycles = 0;
		LY++;
		if (LY == 144)
		{
			STAT &= 0b11111100;
			STAT |= 0b00000001;	//enter vblank
			m_interruptManager->requestInterrupt(InterruptType::VBlank);
		}
		else
		{
			STAT &= 0b11111100;
			STAT |= 0b00000011;	//enter mode 2 again
		}
	}
}

void PPU::m_vblank()	//mode 1
{
	m_modeCycleDiff++;
	if (m_modeCycleDiff == 456)
	{
		m_modeCycleDiff = 0;
		LY++;
		if (LY == 154)
		{
			LY = 0;				//go back to beginning
			STAT &= 0b11111100;
			STAT |= 0b00000011;	//enter mode 2
		}
	}
}

void PPU::m_OAMSearch()	//mode 2
{
	m_modeCycleDiff++;
	m_totalLineCycles++;
	if (m_modeCycleDiff == 80)
	{
		m_modeCycleDiff = 0;
		STAT &= 0b11111100;
		STAT |= 0b00000011;	//enter mode 3
	}
}

void PPU::m_LCDTransfer()	//mode 3
{
	m_modeCycleDiff++;
	m_totalLineCycles++;

	//we're not actually rendering yet, but mode 3 has variable cycle timing. rn we'll just say 172 (but will be fixed when doing actual fifo)
	if (m_modeCycleDiff == 172)
	{
		m_modeCycleDiff = 0;
		STAT &= 0b11111100;	//enter mode 0
	}
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