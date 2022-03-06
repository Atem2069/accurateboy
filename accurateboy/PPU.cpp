#include"PPU.h"

PPU::PPU(std::shared_ptr<InterruptManager>& interruptManager)
{
	m_interruptManager = interruptManager;
	//simple test
	for (int i = 0; i < (160 * 144); i++)
		m_backBuffer[i] = i;
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
			STAT |= 0b00000010;	//enter mode 2 again
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
			STAT |= 0b00000010;	//enter mode 2
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
		m_fetcherX = 0;									//resetting fifo as we enter mode 3
		m_lcdXCoord = 0;
		m_fetcherStage = FetcherStage::FetchTileNumber;
		m_fetcherBeginDelayed = false;
	}
}

void PPU::m_LCDTransfer()	//mode 3
{
	m_modeCycleDiff++;
	m_totalLineCycles++;
	if (m_lcdXCoord == 0 && !m_fetcherBeginDelayed && m_modeCycleDiff < 6)
		return;
	if (m_lcdXCoord == 0 && !m_fetcherBeginDelayed && m_modeCycleDiff==6)
	{
		m_modeCycleDiff = 0;
		m_fetcherBeginDelayed = true;
	}

	switch (m_fetcherStage)
	{
	case FetcherStage::FetchTileNumber:
		m_fetchTileNumber(); break;
	case FetcherStage::FetchTileDataHigh:
		m_fetchTileDataHigh(); break;
	case FetcherStage::FetchTileDataLow:
		m_fetchTileDataLow(); break;
	case FetcherStage::PushToFIFO:
		m_pushToFIFO(); break;
	}

	//pop off, push to display
	if (m_backgroundFIFO.size() > 0)
	{
		FIFOPixel cur = m_backgroundFIFO.front();
		m_backgroundFIFO.pop();
		m_lcdXCoord++;
		if (m_lcdXCoord == 160)	//enter hblank
		{
			while (m_backgroundFIFO.size() > 0)
				m_backgroundFIFO.pop();
			m_modeCycleDiff = 0;
			STAT &= 0b11111100;
		}
	}

	/*m_modeCycleDiff++;
	m_totalLineCycles++;

	//we're not actually rendering yet, but mode 3 has variable cycle timing. rn we'll just say 172 (but will be fixed when doing actual fifo)
	if (m_modeCycleDiff == 172)
	{
		m_modeCycleDiff = 0;
		STAT &= 0b11111100;	//enter mode 0
	}*/
}
//todo:
void PPU::m_fetchTileNumber()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileDataLow;

		//assume always fetching bg for now, fix later
		int xOffset = SCX / 8;
		int yOffset = 32 * (((LY + SCY) & 0xFF) / 8);
		int tileMapOffset = (m_fetcherX + xOffset + yOffset) % 1024;
		m_fetcherX++;

		if (m_getBackgroundNametable())
			tileMapOffset += 0x1C00;
		else
			tileMapOffset += 0x1800;

		m_tileNumber = m_VRAM[tileMapOffset];

	}
}

void PPU::m_fetchTileDataLow()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileDataHigh;

		//grab low tile number
	}
}

void PPU::m_fetchTileDataHigh()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::PushToFIFO;

		//grab high tile number
	}
}

void PPU::m_pushToFIFO()
{
	if (m_backgroundFIFO.size()==0)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileNumber;
		for (int i = 0; i < 8; i++)
		{
			uint8_t colHigh = (m_tileDataHigh >> (7 - i)) & 0b1;
			uint8_t colLow = (m_tileDataLow >> (7 - i)) & 0b1;
			uint8_t colID = (colHigh << 1) | colLow;

			//hardcoded for now, no sprites yet
			FIFOPixel tempPixel = {};
			tempPixel.colorID = colID;
			tempPixel.hasPriority = true;
			tempPixel.paletteID = 0;

			m_backgroundFIFO.push(tempPixel);
		}
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

uint32_t* PPU::getDisplayBuffer()
{
	return m_backBuffer;
}