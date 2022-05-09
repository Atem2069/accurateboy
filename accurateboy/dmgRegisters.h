#pragma once
#include<iostream>

const uint16_t REG_JOYPAD = 0xFF00;
const uint16_t REG_SB = 0xFF01;
const uint16_t REG_SC = 0xFF02;
const uint16_t REG_DIV = 0xFF04;	//DIV  timer. incremented at 16384Hz
const uint16_t REG_TIMA = 0xFF05;	//Incremented at some rate according to FF07, and when it overflows triggers an interrupt
const uint16_t REG_TMA = 0xFF06;	//Loaded when TIMA overflows
const uint16_t REG_TAC = 0xFF07;	//Control register - sets if timer is enabled, as well as clock rate
const uint16_t REG_IE = 0xFFFF;
const uint16_t REG_IFLAGS = 0xFF0F;
const uint16_t REG_LCDC = 0xFF40;	//LCD control register
const uint16_t REG_STAT = 0xFF41;	//LCD status register
const uint16_t REG_SCY = 0xFF42;	//Y scroll offset
const uint16_t REG_SCX = 0xFF43;	//X scroll offset
const uint16_t REG_WY = 0xFF4A;		//Window y origin
const uint16_t REG_WX = 0xFF4B;		//Window x origin
const uint16_t REG_LY = 0xFF44;		//Holds current scanline being rendered (critical for game boy bootrom to do anything)
const uint16_t REG_LYC = 0xFF45;	//Holds value that LY is compared against
const uint16_t REG_DMA = 0xFF46;
const uint16_t REG_KEY1 = 0xFF4D;	//Speed switch register (CGB)

//cartridge header constants
static uint16_t CART_TITLE = 0x0134;
static uint16_t CART_TYPE = 0x0147;
static uint16_t CART_ROMSIZE = 0x148;
static uint16_t CART_RAMSIZE = 0x149;
