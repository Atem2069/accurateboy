#pragma once
#include<iostream>

const uint16_t REG_KEY0 = 0xFF4C;	//cpu mode register

const uint16_t REG_HDMA1 = 0xFF51;	//dma src (high)
const uint16_t REG_HDMA2 = 0xFF52;	//dma src (low)
const uint16_t REG_HDMA3 = 0xFF53;	//dma dest (high)
const uint16_t REG_HDMA4 = 0xFF54;	//dma dest (low)
const uint16_t REG_HDMA5 = 0xFF55;	//dma length/mode/start

const uint16_t REG_VBK = 0xFF4F;	//vram bank (bit 0)
const uint16_t REG_SVBK = 0xFF70;	//warm bank (bits 0-2)

const uint16_t REG_OPRI = 0xFF6C;	//object priority 
const uint16_t REG_BGPI = 0xFF68;
const uint16_t REG_BGPD = 0xFF69;
const uint16_t REG_OBPI = 0xFF6A;
const uint16_t REG_OBPD = 0xFF6B;

const uint16_t CART_COMPAT = 0x0143;	//cgb compatibility byte