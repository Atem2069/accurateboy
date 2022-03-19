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

void PPU::m_checkSTATInterrupt()
{
	uint8_t curPPUMode = STAT & 0b11;

	bool lycEnabled = (STAT >> 6) & 0b1;
	bool oamEnabled = (STAT >> 5) & 0b1;
	bool vblankEnabled = (STAT >> 4) & 0b1;
	bool hblankEnabled = (STAT >> 3) & 0b1;

	bool statCondHigh = false;
	statCondHigh = (lycEnabled && (LY == LYC));
	if (LY == LYC)
		STAT |= 0b00000100;
	else
		STAT &= 0b11111011;
	statCondHigh |= (oamEnabled && (curPPUMode == 2));
	statCondHigh |= (vblankEnabled && (curPPUMode == 1));
	statCondHigh |= (hblankEnabled && (curPPUMode == 0));

	if (statCondHigh && !m_lastStatState)
		m_interruptManager->requestInterrupt(InterruptType::STAT);
	m_lastStatState = statCondHigh;
}

void PPU::m_tickTCycle()
{
	if (!m_getLCDEnabled())
	{
		STAT &= 0b11111100;
		m_totalFrameCycles = 0;
		m_totalLineCycles = 0;
		m_modeCycleDiff = 0;
		LY = 0;
		return;
	}
	uint8_t curPPUMode = STAT & 0b11;

	m_checkSTATInterrupt();

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
	m_VRAMAccessBlocked = false; m_OAMAccessBlocked = false;
	m_modeCycleDiff++;
	m_totalLineCycles++;
	m_totalFrameCycles++;
	if (m_totalLineCycles == 456)	//enter line takes 456 t cycles
	{
		m_modeCycleDiff = 0;
		m_totalLineCycles = 0;
		//reset oam list
		for (int i = 0; i < 10; i++)
			m_spriteBuffer[i] = {};
		LY++;
		if (LY == 144)
		{
			STAT &= 0b11111100;
			STAT |= 0b00000001;	//enter vblank
			m_interruptManager->requestInterrupt(InterruptType::VBlank);

			//copy over scratch buffer to backbuffer
			memcpy(m_backBuffer, m_scratchBuffer, 160 * 144 * sizeof(uint32_t));

		}
		else
		{
			STAT &= 0b11111100;
			STAT |= 0b00000010;	//enter mode 2 again
		}
		m_checkSTATInterrupt();
	}
}

void PPU::m_vblank()	//mode 1
{
	m_VRAMAccessBlocked = false; m_OAMAccessBlocked = false;
	m_modeCycleDiff++;
	m_totalFrameCycles++;

	if (m_modeCycleDiff == 4 && LY == 153)
		LY = 0;

	if (m_modeCycleDiff == 456)
	{
		m_modeCycleDiff = 0;
		LY++;
		if (LY == 1)
		{
			if (m_totalFrameCycles != 70224)
				std::cout << "Timing disrepancy - frame should take 70224 cycles, but took " << m_totalFrameCycles << '\n';
			m_totalFrameCycles = 0;
			m_windowLineCounter = 0;
			LY = 0;				//go back to beginning
			STAT &= 0b11111100;
			STAT |= 0b00000010;	//enter mode 2
			m_checkSTATInterrupt();
		}
	}
}

void PPU::m_OAMSearch()	//mode 2
{
	m_VRAMAccessBlocked = false; m_OAMAccessBlocked = true;
	m_modeCycleDiff++;
	m_totalLineCycles++;
	m_totalFrameCycles++;
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		OAMEntry tempOAMEntry = {};
		tempOAMEntry.y = m_OAM[(m_spritesChecked * 4)];
		tempOAMEntry.x = m_OAM[(m_spritesChecked * 4) + 1];
		tempOAMEntry.tileNumber = m_OAM[(m_spritesChecked * 4) + 2];
		tempOAMEntry.attributes = m_OAM[(m_spritesChecked * 4) + 3];

		//TODO: 8x16 sprites
		int scanlineDiff = LY - (tempOAMEntry.y - 16);
		bool inSpriteBounds = (scanlineDiff < 16 && m_getSpriteSize()) || (scanlineDiff < 8 && !m_getSpriteSize());
		if (LY >= (tempOAMEntry.y - 16) && inSpriteBounds && (tempOAMEntry.y >= 0) && (tempOAMEntry.y <= 160))
			m_spriteBuffer[m_spriteBufferIndex++] = tempOAMEntry;

		m_spritesChecked++;
		if (m_spritesChecked == 40)
		{
			m_modeCycleDiff = 0;
			m_spritesChecked = 0;
			m_spriteBufferIndex = 0;
			STAT &= 0b11111100;
			STAT |= 0b00000011;
			m_fetcherX = 0;
			m_lcdXCoord = 0;
			m_fetcherStage = FetcherStage::FetchTileNumber;
			m_fetcherBeginDelayed = false;
			m_fetchingWindowTiles = false;
			m_checkSTATInterrupt();
		}
	}
}

void PPU::m_LCDTransfer()	//mode 3
{
	m_VRAMAccessBlocked = true; m_OAMAccessBlocked = true;
	m_modeCycleDiff++;
	m_totalLineCycles++;
	m_totalFrameCycles++;
	if (m_lcdXCoord == 0 && !m_fetcherBeginDelayed && m_modeCycleDiff < 6)
		return;
	if (m_lcdXCoord == 0 && !m_fetcherBeginDelayed && m_modeCycleDiff==6)
	{
		m_modeCycleDiff = 0;
		m_fetcherBeginDelayed = true;
	}

	if (!m_spriteFetchInProgress)
	{
		switch (m_fetcherStage)
		{
		case FetcherStage::FetchTileNumber:
			m_fetchTileNumber(); break;
		case FetcherStage::FetchTileDataLow:
			m_fetchTileDataLow(); break;
		case FetcherStage::FetchTileDataHigh:
			m_fetchTileDataHigh(); break;
		case FetcherStage::PushToFIFO:
			m_pushToFIFO(); break;
		}
	}
	else
	{
		switch (m_fetcherStage)
		{
		case FetcherStage::FetchTileNumber:
			m_spriteFetchTileNumber(); break;
		case FetcherStage::FetchTileDataLow:
			m_spriteFetchTileDataLow(); break;
		case FetcherStage::FetchTileDataHigh:
			m_spriteFetchTileDataHigh(); break;
		case FetcherStage::PushToFIFO:
			m_spritePushToFIFO(); break;
		}
	}

	//pop off, push to display
	if (m_backgroundFIFO.size() > 0 && !m_spriteFetchInProgress)
	{
		
		FIFOPixel cur = m_backgroundFIFO.front();
		m_backgroundFIFO.pop_front();

		if (m_lcdXCoord == 0 && m_discardCounter < (SCX % 8) && !m_fetchingWindowTiles)
		{
			m_discardCounter++;
		}
		else if((m_lcdXCoord==0 && m_discardCounter == (SCX % 8)) || m_lcdXCoord>0 || m_fetchingWindowTiles)
		{
			m_discardCounter = 0;

			FIFOPixel spritePixel = {};
			if (!m_spriteFIFO.empty())
			{
				spritePixel = m_spriteFIFO.front();
				m_spriteFIFO.pop_front();
			}

			uint8_t col = (BGP >> (cur.colorID * 2)) & 0b11;
			if (spritePixel.colorID != 0 && (cur.colorID == 0 || spritePixel.hasPriority))
			{
				if (spritePixel.paletteID == 0)
					col = (OBP0 >> (spritePixel.colorID * 2)) & 0b11;
				else
					col = (OBP1 >> (spritePixel.colorID * 2)) & 0b11;
			}

			int pixelCoord = (LY * 160) + m_lcdXCoord;
			uint32_t finalCol = 0;
			
			switch (col)	//need to rewrite it, just good for quick testing
			{
			case 0b00:
				finalCol = 0xFFFFFFFF; break;
			case 0b01:
				finalCol = 0xBFBFBFFF; break;
			case 0b10:
				finalCol = 0x5E5E5EFF; break;
			case 0b11:
				finalCol = 0x000000FF; break;
			}
			if (m_getBackgroundPriority())
				m_scratchBuffer[pixelCoord] = finalCol;
			else
				m_scratchBuffer[pixelCoord] = 0xFFFFFFFF;
			m_lcdXCoord++;
		}

	}

	if (m_getSpritesEnabled() && !m_spriteFetchInProgress)
	{
		for (int i = 0; i < 10; i++)
		{
			int xDiff = m_lcdXCoord - (m_spriteBuffer[i].x - 8);
			if (xDiff >= 0 && xDiff < 8 && m_spriteBuffer[i].x >= 0 && !m_spriteBuffer[i].rendered)
			{
				//m_spriteBuffer[i].rendered = true;
				m_consideredSpriteIndex = i;
				m_modeCycleDiff = 1;
				if (m_fetcherStage != FetcherStage::FetchTileNumber)	//fetcher might have already advanced if it's in another step, so set it back to ensure it gets rendered again
					m_fetcherX--;
				m_fetcherStage = FetcherStage::FetchTileNumber;
				m_spriteFetchInProgress = true;
				i = 100;

			}
		}
	}

	//if we run into window tiles, reset FIFO
	if (m_getWindowEnabled() && LY >= WY && m_lcdXCoord >= ((int)WX - 7) && !m_fetchingWindowTiles)
	{
		m_fetcherStage = FetcherStage::FetchTileNumber;
		m_modeCycleDiff = 0;
		m_fetcherX = 0;
		while (m_backgroundFIFO.size() > 0)
			m_backgroundFIFO.pop_front();
		m_fetchingWindowTiles = true;
	}

	if (m_lcdXCoord == 160)	//enter hblank
	{
		if ((m_totalLineCycles-80) < 172 || (m_totalLineCycles-80) > 289)
			std::cout << m_totalLineCycles-80 << " " << (int)LY << '\n';
		m_lcdXCoord = 0;

		if (m_fetchingWindowTiles)
			m_windowLineCounter++;

		while (m_backgroundFIFO.size() > 0)
			m_backgroundFIFO.pop_front();
		while (m_spriteFIFO.size() > 0)
			m_spriteFIFO.pop_front();
		m_modeCycleDiff = 0;
		STAT &= 0b11111100;
		m_checkSTATInterrupt();
	}
}
//todo:
void PPU::m_fetchTileNumber()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileDataLow;

		uint16_t tileMapAddr = (m_getBackgroundNametable() ? 0x1c00 : 0x1800);
		uint16_t xOffset = (m_fetcherX + (SCX / 8)) & 0x1f;
		uint16_t yOffset = 32 * (((LY + SCY) & 0xFF) / 8);

		if (m_fetchingWindowTiles)
		{
			tileMapAddr = (m_getWindowNametable() ? 0x1c00 : 0x1800);
			xOffset = (m_fetcherX) & 0x1f;
			yOffset = 32 * ((m_windowLineCounter & 0xFF) / 8);
		}

		uint16_t tileMapOffset = (xOffset + yOffset) & 0x3ff;
		m_fetcherX++;

		tileMapAddr += tileMapOffset;

		m_tileNumber = m_VRAM[tileMapAddr];

	}
}

void PPU::m_fetchTileDataLow()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileDataHigh;

		uint16_t tileDataOffset = (m_getTilemap()) ? 0x0000 : 0x1000;
		if (!m_getTilemap())
			tileDataOffset += (int8_t)m_tileNumber * 16;
		else
			tileDataOffset += m_tileNumber * 16;	//*16 because each tile is 16 bytes (2 bytes per row)
		if (!m_fetchingWindowTiles)
			tileDataOffset += (2 * ((LY + SCY) % 8));	//then extract correct row based on ly + scy mod 8
		else
			tileDataOffset += (2 * (m_windowLineCounter % 8));
		
		m_tileDataLow = m_VRAM[tileDataOffset];
	}
}

void PPU::m_fetchTileDataHigh()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::PushToFIFO;

		//same thing really, just + 1!
		uint16_t tileDataOffset = (m_getTilemap()) ? 0x0000 : 0x1000;
		if (!m_getTilemap())
			tileDataOffset += (int8_t)m_tileNumber * 16;
		else
			tileDataOffset += m_tileNumber * 16;	//*16 because each tile is 16 bytes (2 bytes per row)
		if (!m_fetchingWindowTiles)
			tileDataOffset += (2 * ((LY + SCY) % 8));	//then extract correct row based on ly + scy mod 8
		else
			tileDataOffset += (2 * (m_windowLineCounter % 8));

		m_tileDataHigh = m_VRAM[tileDataOffset+1];
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

			m_backgroundFIFO.push_back(tempPixel);
		}
	}
}

void PPU::m_spriteFetchTileNumber()
{
	m_modeCycleDiff = 0;
	m_fetcherStage = FetcherStage::FetchTileDataLow;

	OAMEntry curSprite = m_spriteBuffer[m_consideredSpriteIndex];
	bool yFlip = ((curSprite.attributes >> 6) & 0b1);
	m_tileNumber = curSprite.tileNumber;
	if (m_getSpriteSize())
	{
		uint16_t diff = ((LY + 16) - curSprite.y);
		if (yFlip)
			diff = 15 - diff;
		if (diff >= 8)
			m_tileNumber |= 0x01;
		else
			m_tileNumber &= 0xFE;
	}
}

void PPU::m_spriteFetchTileDataLow()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::FetchTileDataHigh;

		OAMEntry curSprite = m_spriteBuffer[m_consideredSpriteIndex];
		bool yFlip = ((curSprite.attributes >> 6) & 0b1);

		uint16_t tileDataOffset = m_tileNumber * 16;
		uint16_t diff = ((LY + 16) - curSprite.y);
		int flipOffset = m_getSpriteSize() ? 15 : 7;
		if (!yFlip)
			tileDataOffset += (2 * ((diff % 8)));	//then extract correct row based on ly + scy mod 8
		else
			tileDataOffset += (2 * ((flipOffset - diff) % 8));
		m_tileDataLow = m_VRAM[tileDataOffset];
	}
}

void PPU::m_spriteFetchTileDataHigh()
{
	if (m_modeCycleDiff == 2)
	{
		m_modeCycleDiff = 0;
		m_fetcherStage = FetcherStage::PushToFIFO;

		OAMEntry curSprite = m_spriteBuffer[m_consideredSpriteIndex];
		bool yFlip = ((curSprite.attributes >> 6) & 0b1);

		uint16_t tileDataOffset = m_tileNumber * 16;

		uint16_t diff = ((LY + 16) - curSprite.y);
		int flipOffset = m_getSpriteSize() ? 15 : 7;
		if (!yFlip)
			tileDataOffset += (2 * (diff % 8));	//then extract correct row based on ly + scy mod 8
		else
			tileDataOffset += (2 * ((flipOffset - diff) % 8));
		m_tileDataHigh = m_VRAM[tileDataOffset+1];
	}
}

void PPU::m_spritePushToFIFO()
{
	m_modeCycleDiff = 1;
	m_fetcherStage = FetcherStage::FetchTileNumber;
	m_spriteFetchInProgress = false;

	OAMEntry curSprite = m_spriteBuffer[m_consideredSpriteIndex];
	bool xFlip = ((curSprite.attributes >> 5) & 0b1);
	bool oamPriority = true;
	for (int j = 0; j < m_consideredSpriteIndex; j++)
	{
		if (m_spriteBuffer[j].x == curSprite.x)
			oamPriority = false;				//doesn't have priority
	}

;
	FIFOPixel empty = {};
	while (m_spriteFIFO.size() < 8)	//ensure sprite fifo filled with empty pixels
		m_spriteFIFO.push_back(empty);

	int xCutoff = (curSprite.x < 8) ? (8 - curSprite.x) : 0;

	for (int i = xCutoff; i < 8; i++)
	{
		uint8_t shiftOffset = 7 - i;
		if (xFlip)
			shiftOffset = i;
		uint8_t colHigh = (m_tileDataHigh >> shiftOffset) & 0b1;
		uint8_t colLow = (m_tileDataLow >> shiftOffset) & 0b1;
		uint8_t colID = (colHigh << 1) | colLow;

		bool priority = !((curSprite.attributes >> 7) & 0b1);
		int paletteID = ((curSprite.attributes >> 4) & 0b1);
		//hardcoded for now, no sprites yet
		FIFOPixel tempPixel = {};
		if (curSprite.x > 0 && curSprite.x < 168)
		{
			tempPixel.colorID = colID;
			tempPixel.hasPriority = priority;
			tempPixel.paletteID = paletteID;
		}

		if ((m_spriteFIFO[i-xCutoff].colorID == 0 || (!m_spriteFIFO[i-xCutoff].hasPriority && tempPixel.hasPriority && oamPriority)))
			m_spriteFIFO[i-xCutoff] = tempPixel;
	}

	m_spriteBuffer[m_consideredSpriteIndex].rendered = true;
}

uint8_t PPU::read(uint16_t address)
{
	if (address >= 0x8000 && address <= 0x9FFF && !m_VRAMAccessBlocked)
		return m_VRAM[address - 0x8000];
	if (address >= 0xFE00 && address <= 0xFE9F && !m_OAMAccessBlocked)
		return m_OAM[address - 0xFE00];

	switch (address)
	{
	case REG_LCDC:
		return LCDC; break;
	case REG_STAT:
		return STAT | 0b10000000; break;
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

	return 0xFF;
}

void PPU::write(uint16_t address, uint8_t value)
{
	if (address >= 0x8000 && address <= 0x9FFF && !m_VRAMAccessBlocked)
		m_VRAM[address - 0x8000] = value;
	if (address >= 0xFE00 && address <= 0xFE9F && !m_OAMAccessBlocked)
		m_OAM[address - 0xFE00] = value;

	switch (address)
	{
	case REG_LCDC:
		LCDC=value; break;
	case REG_STAT:
		STAT &= 0b00000111; STAT |= (value & 0b11111000); break;
	case REG_SCX:
		SCX=value; break;
	case REG_SCY:
		SCY=value; break;
	case REG_WY:
		WY=value; break;
	case REG_WX:
		WX=value; break;
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