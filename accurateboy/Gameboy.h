#pragma once
#include"Logger.h"
#include"Bus.h"
#include"CPU.h"
#include"PPU.h"
#include"Display.h"
#include"APU.h"
#include"Timer.h"
#include"Joypad.h"

#include<thread>

class GameBoy
{
public:
	GameBoy();
	~GameBoy();
	void run();
private:

	void m_initialise();
	void m_destroy();

	std::shared_ptr<CPU> m_cpu;
	std::shared_ptr<InterruptManager> m_interruptManager;
	std::shared_ptr<Bus> m_bus;
	std::shared_ptr<PPU> m_ppu;
	std::shared_ptr<APU> m_apu;
	std::shared_ptr<Timer> m_timer;
	std::shared_ptr<Joypad> m_joypad;
	std::shared_ptr<Display> m_display;

	JoypadState m_joyState = {};

	void m_displayWorker();
	bool m_shouldStop = false;
};