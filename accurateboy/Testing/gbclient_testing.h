#pragma once

#include"../Logger.h"
#include"../Bus.h"
#include"../CPU.h"
#include"../PPU.h"
#include"../Display.h"
#include"../APU.h"
#include"../Timer.h"
#include"../Joypad.h"

class GBTestClient
{
public:
	GBTestClient();
	~GBTestClient();

	bool run();
private:

	void m_initialise(std::string path);
	void m_destroy();

	std::shared_ptr<CPU> m_cpu;
	std::shared_ptr<InterruptManager> m_interruptManager;
	std::shared_ptr<Bus> m_bus;
	std::shared_ptr<PPU> m_ppu;
	std::shared_ptr<APU> m_apu;
	std::shared_ptr<Timer> m_timer;
	std::shared_ptr<Joypad> m_joypad;
	std::shared_ptr<Serial> m_serial;
	JoypadState m_joyState = {};

	//add rest later, just a few to make the client work
	const int m_numTests = 66;
	std::string romPaths[66] =
	{
		"mooneye/acceptance/add_sp_e_timing.gb",
		"mooneye/acceptance/boot_div-dmgABCmgb.gb",
		"mooneye/acceptance/boot_hwio-dmgABCmgb.gb",
		"mooneye/acceptance/boot_regs-dmgABC.gb",
		"mooneye/acceptance/call_cc_timing.gb",
		"mooneye/acceptance/call_cc_timing2.gb",
		"mooneye/acceptance/call_timing.gb",
		"mooneye/acceptance/call_timing2.gb",
		"mooneye/acceptance/di_timing-GS.gb",
		"mooneye/acceptance/div_timing.gb",
		"mooneye/acceptance/ei_sequence.gb",
		"mooneye/acceptance/ei_timing.gb",
		"mooneye/acceptance/halt_ime0_ei.gb",
		"mooneye/acceptance/halt_ime0_nointr_timing.gb",
		"mooneye/acceptance/halt_ime1_timing.gb",
		"mooneye/acceptance/halt_ime1_timing2-GS.gb",
		"mooneye/acceptance/if_ie_registers.gb",
		"mooneye/acceptance/intr_timing.gb",
		"mooneye/acceptance/jp_cc_timing.gb",
		"mooneye/acceptance/jp_timing.gb",
		"mooneye/acceptance/ld_hl_sp_e_timing.gb",
		"mooneye/acceptance/oam_dma_restart.gb",
		"mooneye/acceptance/oam_dma_start.gb",
		"mooneye/acceptance/oam_dma_timing.gb",
		"mooneye/acceptance/pop_timing.gb",
		"mooneye/acceptance/push_timing.gb",
		"mooneye/acceptance/rapid_di_ei.gb",
		"mooneye/acceptance/ret_cc_timing.gb",
		"mooneye/acceptance/ret_timing.gb",
		"mooneye/acceptance/reti_intr_timing.gb",
		"mooneye/acceptance/reti_timing.gb",
		"mooneye/acceptance/rst_timing.gb",
		"mooneye/acceptance/bits/mem_oam.gb",
		"mooneye/acceptance/bits/reg_f.gb",
		"mooneye/acceptance/bits/unused_hwio-GS.gb",
		"mooneye/acceptance/instr/daa.gb",
		"mooneye/acceptance/interrupts/ie_push.gb",
		"mooneye/acceptance/oam_dma/basic.gb",
		"mooneye/acceptance/oam_dma/reg_read.gb",
		"mooneye/acceptance/oam_dma/sources-GS.gb",
		"mooneye/acceptance/ppu/hblank_ly_scx_timing-GS.gb",
		"mooneye/acceptance/ppu/intr_1_2_timing-GS.gb",
		"mooneye/acceptance/ppu/intr_2_0_timing.gb",
		"mooneye/acceptance/ppu/intr_2_mode0_timing.gb",
		"mooneye/acceptance/ppu/intr_2_mode0_timing_sprites.gb",
		"mooneye/acceptance/ppu/intr_2_mode3_timing.gb",
		"mooneye/acceptance/ppu/intr_2_oam_ok_timing.gb",
		"mooneye/acceptance/ppu/lcdon_timing-GS.gb",
		"mooneye/acceptance/ppu/lcdon_write_timing-GS.gb",
		"mooneye/acceptance/ppu/stat_irq_blocking.gb",
		"mooneye/acceptance/ppu/stat_lyc_onoff.gb",
		"mooneye/acceptance/ppu/vblank_stat_intr-GS.gb",
		"mooneye/acceptance/serial/boot_sclk_align-dmgABCmgb.gb",
		"mooneye/acceptance/timer/div_write.gb",
		"mooneye/acceptance/timer/rapid_toggle.gb",
		"mooneye/acceptance/timer/tim00.gb",
		"mooneye/acceptance/timer/tim00_div_trigger.gb",
		"mooneye/acceptance/timer/tim01.gb",
		"mooneye/acceptance/timer/tim01_div_trigger.gb",
		"mooneye/acceptance/timer/tim10.gb",
		"mooneye/acceptance/timer/tim10_div_trigger.gb",
		"mooneye/acceptance/timer/tim11.gb",
		"mooneye/acceptance/timer/tim11_div_trigger.gb",
		"mooneye/acceptance/timer/tima_reload.gb",
		"mooneye/acceptance/timer/tima_write_reloading.gb",
		"mooneye/acceptance/timer/tma_write_reloading.gb"
	};
};