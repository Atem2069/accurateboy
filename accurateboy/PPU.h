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
	PushToFIFO
};

struct FIFOPixel
{
	uint8_t colorID;
	bool hasPriority;
	int paletteID;	//not really used yet
};

class PPU
{
public:
	PPU(std::shared_ptr<InterruptManager>& interruptManager);
	~PPU();

	void step();

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	uint32_t* getDisplayBuffer();
private:
	void m_tickTCycle();
	void m_hblank();
	void m_vblank();
	void m_OAMSearch();
	void m_LCDTransfer();
	int m_modeCycleDiff = 0;	//holds cycle difference (reset on mode switch)
	int m_totalLineCycles = 0;	//keeps track of how many cycles passed
	int m_totalFrameCycles = 0;	//debug

	std::shared_ptr<InterruptManager> m_interruptManager;

	std::array<uint8_t, 8192> m_VRAM;
	std::array<uint8_t, 0xA0> m_OAM;
	//io registers
	uint8_t LCDC = {}, STAT = {}, SCX = {}, SCY = {}, WY = {}, WX = {}, LY = {}, LYC = {}, BGP = {}, OBP0 = {}, OBP1 = {};

	//stat interrupt
	bool m_lastStatState = false;

	//lcdc bits
	bool m_getLCDEnabled();
	bool m_getWindowNametable();
	bool m_getWindowEnabled();
	bool m_getTilemap();
	bool m_getBackgroundNametable();
	bool m_getSpriteSize();
	bool m_getSpritesEnabled();
	bool m_getBackgroundPriority();

	std::queue<FIFOPixel> m_backgroundFIFO;
	std::queue<FIFOPixel> m_spriteFIFO;
	FetcherStage m_fetcherStage;
	int m_fetcherX = 0;
	int m_lcdXCoord = 0;
	uint8_t m_tileNumber = 0;
	uint8_t m_tileDataLow = 0;
	uint8_t m_tileDataHigh = 0;
	bool m_fetcherBeginDelayed = false;
	int m_discardCounter = 0;

	void m_fetchTileNumber();
	void m_fetchTileDataLow();
	void m_fetchTileDataHigh();
	void m_pushToFIFO();

	uint32_t m_scratchBuffer[160 * 144];
	uint32_t m_backBuffer[160 * 144];
};