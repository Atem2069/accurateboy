# accurateboy

GBEmu rewrite with much higher accuracy, including M-Cycle CPU timing and T-Cycle PPU emulation with a pixel FIFO.

## Blargg Tests
| Name | Pass? |
| -----|------ |
| cpu_instrs | :heavy_check_mark: |
| instr_timing | :heavy_check_mark: |
| mem_timing |:heavy_check_mark: |
| mem_timing-2 | :heavy_check_mark: |
| dmg_sound | :x: |

## mooneye-gb Suite

### bits
| Name | Pass? |
| -----|------ |
| mem_oam | :heavy_check_mark: |
| reg_f | :heavy_check_mark: |
| unused_hwio-GS | :heavy_check_mark: |

### instr
 - [x] daa

### interrupts
 - [ ] ie_push (R1: not cancelled)

### OAM DMA
| Name | Pass? |
| -----|------ |
| basic | :heavy_check_mark: |
| reg_read | :heavy_check_mark: |
| sources-GS | :heavy_check_mark: |

### PPU
| Name | Pass? |
| -----|------ |
| hblank_ly_scx_timing-GS | :x: |
| intr_1_2_timing-GS | :heavy_check_mark: |
| intr_2_0_timing | :heavy_check_mark: |
| intr_2_mode0_timing | :heavy_check_mark: |
| intr_2_mode0_timing_sprites | :x: |
| intr_2_mode3_timing | :heavy_check_mark: |
| intr_2_oam_ok_timing | :heavy_check_mark: |
| lcdon_timing-GS | :x: |
| lcdon_write_timing-GS | :x: |
| stat_irq_blocking | :heavy_check_mark: |
| stat_lyc_onoff | :heavy_check_mark: |
| vblank_stat_intr-GS | :x: |


### Serial
 - [ ] No serial support implemented

### Timer
| Name | Pass? |
| -----|------ |
| div_write | :heavy_check_mark: |
| rapid_toggle | :x: |
| tim00 | :heavy_check_mark: |
| tim00_div_trigger | :heavy_check_mark: |
| tim01 | :heavy_check_mark: |
| tim01_div_trigger | :heavy_check_mark: |
| tim10 | :heavy_check_mark: |
| tim10_div_trigger | :heavy_check_mark: |
|  tim11 | :heavy_check_mark: |
| tim11_div_trigger | :heavy_check_mark: |
| tima_reload | :x: |
| tima_write_reloading | :x: |
| tma_write_reloading | :x: |



## PPU Accuracy
 - [x] dmg-acid2
 - [x] pocket.gb (very minor glitches towards the end credits, rest of the demo should be flawless)
 - [x] Prehistorik Man (only game that abuses mid-scanline palette writes, title screen works flawlessly)
