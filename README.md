# accurateboy

Highly accurate Game Boy emulator, with a T-Cycle accurate Pixel FIFO implementation.
![Pocket demo](img/pocketdemo.png)

# Testing

## Blargg Tests
| Name | Pass? |
| -----|------ |
| cpu_instrs | :heavy_check_mark: |
| instr_timing | :heavy_check_mark: |
| mem_timing |:heavy_check_mark: |
| mem_timing-2 | :heavy_check_mark: |
| dmg_sound | :x: |

## mooneye-gb Suite

### Acceptance (Excluding Super Game Boy/Game Boy Pocket boot tests)
| Name | Pass? |
| -----|------ |
| add_sp_e_timing | :heavy_check_mark: |
| boot_div-DMGABCmgb | :heavy_check_mark: |
| boot_hwio-dmgABCmgb | :heavy_check_mark: |
| boot_regs-dmgABC | :heavy_check_mark: |
| call_cc_timing | :heavy_check_mark: |
| call_cc_timing2 | :heavy_check_mark: |
| call_timing | :heavy_check_mark: |
| call_timing2 | :heavy_check_mark: |
| di_timing-GS | :heavy_check_mark: |
| div_timing | :heavy_check_mark: |
| ei_sequence | :heavy_check_mark: |
| ei_timing | :heavy_check_mark: |
| halt_ime0_ei | :heavy_check_mark: |
| halt_ime0_nointr_timing | :heavy_check_mark: |
| halt_ime1_timing | :heavy_check_mark: |
| halt_ime1_timing2-GS | :heavy_check_mark: |
| if_ie_registers | :heavy_check_mark: |
| intr_timing | :heavy_check_mark: |
| jp_cc_timing | :heavy_check_mark: |
| jp_timing | :heavy_check_mark: |
| ld_hl_sp_e_timing | :heavy_check_mark: |
| oam_dma_restart | :heavy_check_mark: |
| oam_dma_start | :heavy_check_mark: |
| oam_dma_timing | :heavy_check_mark: |
| pop_timing | :heavy_check_mark: |
| push_timing | :heavy_check_mark: |
| rapid_di_ei | :heavy_check_mark: |
| ret_cc_timing | :heavy_check_mark: |
| ret_timing | :heavy_check_mark: |
| reti_intr_timing | :heavy_check_mark: |
| reti_timing | :heavy_check_mark: |
| rst_timing | :heavy_check_mark: |

### bits
| Name | Pass? |
| -----|------ |
| mem_oam | :heavy_check_mark: |
| reg_f | :heavy_check_mark: |
| unused_hwio-GS | :heavy_check_mark: |

### instr
| Name | Pass? |
| -----|------ |
| daa | :heavy_check_mark: |

### interrupts
| Name | Pass? |
| -----|------ |
| ie_push | :heavy_check_mark: |

### OAM DMA
| Name | Pass? |
| -----|------ |
| basic | :heavy_check_mark: |
| reg_read | :heavy_check_mark: |
| sources-GS | :heavy_check_mark: |

### PPU
| Name | Pass? |
| -----|------ |
| hblank_ly_scx_timing-GS | :heavy_check_mark: |
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
| rapid_toggle | :heavy_check_mark: |
| tim00 | :heavy_check_mark: |
| tim00_div_trigger | :heavy_check_mark: |
| tim01 | :heavy_check_mark: |
| tim01_div_trigger | :heavy_check_mark: |
| tim10 | :heavy_check_mark: |
| tim10_div_trigger | :heavy_check_mark: |
|  tim11 | :heavy_check_mark: |
| tim11_div_trigger | :heavy_check_mark: |
| tima_reload | :heavy_check_mark: |
| tima_write_reloading | :heavy_check_mark: |
| tma_write_reloading | :heavy_check_mark: |



## PPU Accuracy
 - [x] dmg-acid2
 - [x] pocket.gb (very minor glitches towards the end credits, rest of the demo should be flawless)
 - [x] Prehistorik Man (only game that abuses mid-scanline palette writes, title screen works flawlessly)
