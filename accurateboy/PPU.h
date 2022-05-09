#pragma once

#include<iostream>
#include<array>
#include<queue>

#include"Logger.h"
#include"InterruptManager.h"
#include"dmgRegisters.h"

enum class FetcherStage
{
	FetchTileNumber,
	FetchTileDataLow,
	FetchTileDataHigh,
	PushToFIFO,
	SpriteFetchTileNumber,
	SpriteFetchTileDataLow,
	SpriteFetchTileDataHigh,
	SpritePushToFIFO
};

struct FIFOPixel
{
	uint8_t colorID;
	bool hasPriority;
	int paletteID;	//not really used yet
};

struct OAMEntry
{
	uint8_t x;
	uint8_t y;
	uint8_t tileNumber;
	uint8_t attributes;
	bool rendered;
};

class PPU
{
public:
	PPU(std::shared_ptr<InterruptManager>& interruptManager);
	~PPU();

	void step();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	void DMAForceWrite(uint16_t address, uint8_t value);
	uint32_t* getDisplayBuffer();
private:
	void m_checkSTATInterrupt();
	void m_hblank();
	void m_vblank();
	void m_buggedOAMSearch();
	void m_OAMSearch();
	void m_LCDTransfer();
	int m_modeCycleDiff = 0;	//holds cycle difference (reset on mode switch)
	int m_totalLineCycles = 0;	//keeps track of how many cycles passed
	int m_totalFrameCycles = 0;	//debug

	std::shared_ptr<InterruptManager> m_interruptManager;

	std::array<uint8_t, 8192> m_VRAM;
	std::array<uint8_t, 0xA0> m_OAM;
	bool m_VRAMReadAccessBlocked = false, m_OAMReadAccessBlocked = false;
	bool m_VRAMWriteAccessBlocked = false, m_OAMWriteAccessBlocked = false;
	//io registers
	uint8_t LCDC = {}, STAT = {}, SCX = {}, SCY = {}, WY = {}, WX = {}, LY = {}, LYC = {}, BGP = {}, OBP0 = {}, OBP1 = {};

	//check if in "mode 0" from LCD enable
	bool m_buggyMode2 = false;
	bool m_islcdOnLine = false;	//maybe could optimise this out?^^

	//stat interrupt
	bool m_lastStatState = false;
	bool m_lyDelay = false;

	//stat mode switching
	bool m_latchingNewMode = false;
	uint8_t m_newMode = 0;

	//lcdc bits
	bool m_getLCDEnabled();
	bool m_getWindowNametable();
	bool m_getWindowEnabled();
	bool m_getTilemap();
	bool m_getBackgroundNametable();
	bool m_getSpriteSize();
	bool m_getSpritesEnabled();
	bool m_getBackgroundPriority();

	std::deque<FIFOPixel> m_backgroundFIFO;
	std::deque<FIFOPixel> m_spriteFIFO;
	FetcherStage m_fetcherStage;
	int m_fetcherX = 0;
	int m_lcdXCoord = 0;
	uint8_t m_tileNumber = 0, m_tileDataLow = 0, m_tileDataHigh = 0;
	uint8_t m_spriteTileNumber = 0, m_spriteTileDataLow = 0, m_spriteTileDataHigh = 0;
	bool m_fetcherBeginDelayed = false;
	bool m_fetchingWindowTiles = false;
	uint8_t m_windowLineCounter = 0;
	int m_discardCounter = 0;
	int m_pixelsToDiscard = 0;
	int m_xScroll = 0;

	bool m_spriteFetchInProgress = false;
	bool m_lastPushSucceeded = false;
	bool m_wasSpriteFetch = false;;
	int m_spritePenaltyCycles = 0;

	void m_fetchTileNumber();
	void m_fetchTileDataLow();
	void m_fetchTileDataHigh();
	void m_pushToFIFO();

	void m_spriteFetchTileNumber();
	void m_spriteFetchTileDataLow();
	void m_spriteFetchTileDataHigh();
	void m_spritePushToFIFO();
	OAMEntry m_spriteBuffer[10];
	int m_spriteBufferIndex = 0;
	int m_spritesChecked = 0;
	int m_consideredSpriteIndex = 0;

	uint32_t m_scratchBuffer[160 * 144];
	uint32_t m_backBuffer[160 * 144];
};