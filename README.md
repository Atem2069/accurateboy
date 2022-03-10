# accurateboy

GBEmu rewrite with much higher accuracy, including M-Cycle CPU timing and T-Cycle PPU emulation with a pixel FIFO.

## Blargg Tests
 - [x] cpu_instrs
 - [x] instr_timing
 - [x] mem_timing
 - [x] mem_timing-2
 - [ ] dmg_sound 

## mooneye-gb Suite

### bits
 - [x] mem_oam
 - [x] reg_f
 - [x] unused_hwio-GS

### instr
 - [x] daa

### interrupts
 - [ ] ie_push (R1: not cancelled)

### OAM DMA
 - [x] basic
 - [x] reg_read
 - [x] sources-GS

### PPU
 - [ ] fails basically every test lol. PPU has broken IRQ blocking and timing bugs

### Serial
 - [ ] No serial support implemented

### Timer
 - [x] div_write
 - [ ] rapid_toggle
 - [x] tim00
 - [x] tim00_div_trigger
 - [x] tim01
 - [x] tim01_div_trigger
 - [x] tim10
 - [x] tim10_div_trigger
 - [x] tim11
 - [x] tim11_div_trigger
 - [ ] tima_reload
 - [ ] tima_write_reloading
 - [ ] tma_write_reloading



## PPU Accuracy
 - [x] dmg-acid2
 - [x] pocket.gb (very minor glitches towards the end credits, rest of the demo should be flawless)
 - [x] Prehistorik Man (only game that abuses mid-scanline palette writes, title screen works flawlessly)
