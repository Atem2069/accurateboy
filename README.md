# accurateboy

GBEmu rewrite with much higher accuracy, including M-Cycle CPU timing and T-Cycle PPU emulation with a pixel FIFO.

### Blargg Tests
 - [x] cpu_instrs
 - [x] instr_timing
 - [x] mem_timing
 - [x] mem_timing-2
 - [ ] dmg_sound 

### PPU Accuracy
 - [x] dmg-acid2
 - [x] pocket.gb (very minor glitches towards the end credits, rest of the demo should be flawless)
 - [x] Prehistorik Man (only game that abuses mid-scanline palette writes, title screen works flawlessly)
