#pragma once

#include"Logger.h"
#include"InterruptManager.h"
#include"dmgRegisters.h"

class Timer
{
public:
	Timer(std::shared_ptr<InterruptManager>& interruptManager);
	~Timer();

	void step();
	
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	uint16_t getClockDivider();
private:
	void m_tickTIMA(uint16_t lastDiv, uint16_t newDiv);
	void m_checkTACMuxChange(uint8_t newTAC);
	int m_convertMuxToShiftAmount(uint8_t mux);

	std::shared_ptr<InterruptManager> m_interruptManager;

	uint8_t TIMA = 0;
	uint8_t TMA = 0;
	uint8_t TAC = 0;

	bool m_timerReloading = false;
	int m_timerReloadCycles = 0;
	int m_timerIrqCycles = 0;

	uint16_t m_divider = 0;
};
