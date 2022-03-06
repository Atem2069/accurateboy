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

	std::shared_ptr<InterruptManager> m_interruptManager;

	std::array<uint8_t, 8192> m_VRAM;
	std::array<uint8_t, 0xA0> m_OAM;
	//io registers
	uint8_t LCDC = {}, STAT = {}, SCX = {}, SCY = {}, WY = {}, WX = {}, LY = {}, LYC = {}, BGP = {}, OBP0 = {}, OBP1 = {};

	//lcdc bits
	bool m_getLCDEnabled();
	bool m_getWindowNametable();
	bool m_getWindowEnabled();
	bool m_getTilemap();
	bool m_getBackgroundNametable();
	bool m_getSpriteSize();
	bool m_getSpritesEnabled();
	bool m_getBackgroundPriority();

	std::queue<uint8_t> m_backgroundFIFO;
	std::queue<uint8_t> m_spriteFIFO;
	FetcherStage m_fetcherStage;

	uint32_t m_scratchBuffer[160 * 144];
	uint32_t m_backBuffer[160 * 144];
};