#pragma once

#include<iostream>


struct PPUConfig
{
	bool debugOverride;
	bool background;
	bool window;
	bool sprites;
};

struct SystemConfig
{
	std::string RomName;
	bool useBootRom;
	bool reset;
	bool pause;
	bool DmgMode;
};

struct DisplayConfig
{
	int displayScale;
	bool resize;
	bool colorCorrect;
	bool frameBlend;
};

struct GBConfig
{
	PPUConfig PPU;
	SystemConfig System;
	DisplayConfig Display;
};

class Config
{
public:
	static GBConfig GB;
};