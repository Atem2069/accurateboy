#include"CPU.h"

CPU::CPU(std::shared_ptr<Bus>& bus, std::shared_ptr<InterruptManager>& interruptManager)
{
	m_bus = bus;
	m_interruptManager = interruptManager;
	m_initIO();	//init CPU and I/O registers to correct values
	//PC = 0x100;
	//m_bus->write(0xFF50, 1);
}

CPU::~CPU()
{
	//todo
}

void CPU::step()
{
	InterruptType queuedInt = m_interruptManager->getActiveInterrupt(false);
	if (queuedInt != InterruptType::None)
	{
		if (m_interruptManager->getInterruptsEnabled())
		{
			m_bus->tick();
			m_bus->tick();
			m_pushToStack(PC);
			queuedInt = m_interruptManager->getActiveInterrupt(true);	//IF&IE may change in interrupt processing, so check again. if this ends up being 0 the cpu actually jumps to 0
			m_interruptManager->disableInterrupts();
			PC = (uint16_t)queuedInt;
			m_bus->tick();
		}
		m_halted = false;
	}

	m_lastPC = PC;
	if (!m_halted)
		m_executeInstruction();
	else
		m_bus->tick();

	if (m_EIRequested)	//odd hack, essentially only re-enable after an instruction has passed. TODO: EI and then DI overrides the enable
	{
		m_instrSinceEI += 1;
		if (m_instrSinceEI > 1)
		{
			m_EIRequested = false;
			m_interruptManager->enableInterrupts();
		}
	}
}

void CPU::m_executeInstruction()
{
	uint8_t opcode = m_fetch();
	m_lastOpcode = opcode;
	int debug_timesOpcodeSeen = 0;
	if (opcode == 0)
	{
		return;
	}
	if (opcode == 0b0000'1000)
	{
		_storeSPAtAddress(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b0001'0000)
	{
		_STOP(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b0001'1000)
	{
		_JRUnconditional(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1110'0111) == 0b0010'0000)
	{
		_JRConditional(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'0001)
	{
		_loadR16Immediate(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'1001)
	{
		_addHLR16(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'0010)
	{
		_storeAccum(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'1010)
	{
		_loadAccum(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'0011)
	{
		_incR16(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b0000'1011)
	{
		_decR16(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b0000'0100)
	{
		_incR8(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b0000'0101)
	{
		_decR8(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b0000'0110)
	{
		_ldR8Immediate(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b0000'0111)
	{
		_bitwiseOps(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0000) == 0b0100'0000)
	{
		_ldR8(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0000) == 0b1000'0000)
	{
		_ALUOpsRegister(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1110'0111) == 0b11000000)
	{
		_RETConditional(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1110'0000)
	{
		_storeHiImmediate(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1110'1000)
	{
		_addSPImmediate(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1111'0000)
	{
		_loadHiImmediate(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1111'1000)
	{
		_LDHLSPImmediate(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b1100'0001)
	{
		_popR16(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b1100'1001)
	{
		_miscStackOps(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1110'0111) == 0b1100'0010)
	{
		_JPConditional(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1110'0010)
	{
		_storeHi(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1110'1010)
	{
		_storeAccumDirect(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1111'0010)
	{
		_loadHi(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1111'1010)
	{
		_loadAccumDirect(); debug_timesOpcodeSeen++;
	}
	if (((opcode & 0b1100'0111) == 0b1100'0011) && opcode != 0xCB)
	{
		_miscOpsEIDI(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0xCB)
	{
		m_executePrefixedInstruction(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1110'0111) == 0b1100'0100)
	{
		_callConditional(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'1111) == 0b1100'0101)
	{
		_pushR16(); debug_timesOpcodeSeen++;
	}
	if (opcode == 0b1100'1101)
	{
		_callImmediate(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b1100'0110)
	{
		_ALUOpsImmediate(); debug_timesOpcodeSeen++;
	}
	if ((opcode & 0b1100'0111) == 0b1100'0111)
	{
		_reset(); debug_timesOpcodeSeen++;
	}

	if (debug_timesOpcodeSeen > 1)
		std::cout << "fuck " << std::hex << (int)opcode << " " << debug_timesOpcodeSeen << '\n';
}

void CPU::m_executePrefixedInstruction()
{
	uint8_t opcode = m_fetch();
	m_lastOpcode = opcode;

	switch (opcode & 0b11000000)
	{
	case 0b00000000:
		_CBShiftsRotates(); break;
	case 0b01000000:
		_CBGetBitComplement(); break;
	case 0b10000000:
		_CBResetBit(); break;
	case 0b11000000:
		_CBSetBit(); break;
	}
}

void CPU::m_initIO()
{
	AF.reg = {};
	BC.reg = {};
	DE.reg = {};
	HL.reg = {};
	SP.reg = {};
	PC = 0;
	/*
	* Will be re-enabled when emulator is more fleshed out.
	bool disableBootRom = Config::GB.System.useBootRom;
	if (disableBootRom)
	{
		Logger::getInstance()->msg(LoggerSeverity::Info, "Boot ROM was disabled - initializing directly. Custom DMG palettes will not work.");
		m_bus->write(0xFF50, 1);
		bool cartIsCGB = (m_bus->read(CART_COMPAT) == 0xC0 || (m_bus->read(CART_COMPAT) == 0x80 && !Config::GB.System.DmgMode));	//0xC0: CGB only. 0x80: CGB and DMG

		if (cartIsCGB)
		{
			//https://gbdev.io/pandocs/Power_Up_Sequence.html (Hardware registers)
			AF.reg = 0x1180;
			BC.reg = 0x0000;
			DE.reg = 0xFF56;
			HL.reg = 0x000D;
		}
		else
		{
			m_bus->write(REG_KEY0, 0x04);	//enable dmg compatibility mode
			AF.reg = 0x01B0;
			BC.reg = 0x0013;
			DE.reg = 0x00D8;
			HL.reg = 0x014D;

			//init custom palette
			uint8_t paletteData[] = { 0b00001111,0b00001010,0b11101011,0b00100001,0b01100111,0b00100101,0b00000101,0b00011101 };	//dmg palette converted to RGB555(+1)
			m_bus->write(REG_BGPI, 0b10000000);	//set auto increment 
			m_bus->write(REG_OBPI, 0b10000000);
			for (int i = 0; i < sizeof(paletteData) / sizeof(uint8_t); i++)
			{
				m_bus->write(REG_BGPD, paletteData[i]);
				m_bus->write(REG_OBPD, paletteData[i]);
			}
		}
		
		SP.reg = 0xFFFE;
		PC = 0x100;
	}
	*/
}


uint64_t CPU::getCycleCount() { return m_cycleCount; }
bool CPU::getInDoubleSpeedMode() {return m_isInDoubleSpeedMode;}

void CPU::m_set8BitArithmeticFlags(uint8_t opA, uint8_t opB, bool carryIn, bool subtract)
{
	uint8_t carryInVal = (carryIn) ? m_getCarryFlag() : 0;

	if (subtract)
	{
		m_setCarryFlag(opA < (opB + carryInVal));
		m_setHalfCarryFlag(((opA & 0xf) - (opB & 0xf) - (carryInVal & 0xf)) & 0x10);
		m_setSubtractFlag(true);
		m_setZeroFlag((uint8_t)(opA-(opB+carryInVal))==0);
	}
	else
	{
		m_setHalfCarryFlag(((opA & 0x0F) + (opB & 0x0F) + (carryInVal & 0x0F)) > 0x0F);
		m_setCarryFlag(((int)opA + (int)opB + (int)carryInVal) > 0xFF);
		m_setSubtractFlag(false);
		m_setZeroFlag((uint8_t)(opA+opB+carryInVal)==0);
	}

	/*if (subtract)
	{
		opB = ~(opB)+1;
		carryInVal = ~(carryInVal)+1;
	}

	m_setZeroFlag((opA + opB + carryInVal) == 0);
	m_setSubtractFlag(subtract);
	m_setHalfCarryFlag(((opA & 0xF) + (opB & 0xF) + (carryInVal & 0xF)) > 0xF);
	m_setCarryFlag((opA + opB + carryIn) < opA);	//double check this
	//if (subtract)
	//	m_setCarryFlag(!m_getCarryFlag());			//double check this too*/
}

void CPU::m_set8BitLogicalFlags(uint8_t value, bool AND)
{
	m_setZeroFlag(value == 0);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(AND);
	m_setCarryFlag(false);
}

bool CPU::m_getZeroFlag() { return (AF.low & 0b10000000) >> 7; }
void CPU::m_setZeroFlag(bool value)
{
	if (!value)
		AF.low &= 0b01111111;
	else
		AF.low |= 0b10000000;
}

bool CPU::m_getCarryFlag() { return (AF.low & 0b00010000) >> 4; }
void CPU::m_setCarryFlag(bool value)
{
	if (!value)
		AF.low &= 0b11101111;
	else
		AF.low |= 0b00010000;
}

bool CPU::m_getHalfCarryFlag() { return (AF.low & 0b00100000) >> 5; }
void CPU::m_setHalfCarryFlag(bool value)
{
	if (!value)
		AF.low &= 0b11011111;
	else
		AF.low |= 0b00100000;
}

bool CPU::m_getSubtractFlag() { return (AF.low & 0b01000000) >> 6; }
void CPU::m_setSubtractFlag(bool value)
{
	if (!value)
		AF.low &= 0b10111111;
	else
		AF.low |= 0b01000000;
}

uint8_t CPU::m_fetch()
{
	uint8_t val = m_bus->read(PC++);
	return val;
}

void CPU::m_pushToStack(uint16_t val)
{
	uint8_t highByte = (val & 0xFF00) >> 8;
	uint8_t lowByte = (val & 0x00FF);

	m_bus->write(SP.reg - 1, highByte);
	m_bus->write(SP.reg - 2, lowByte);
	SP.reg -= 2;
}

uint16_t CPU::m_popFromStack()
{
	uint8_t lowByte = m_bus->read(SP.reg);
	uint8_t highByte = m_bus->read(SP.reg + 1);
	SP.reg += 2;
	uint16_t val = (((uint16_t)highByte) << 8) | lowByte;
	return val;
}

//register/condition code decoding
uint8_t CPU::getR8(uint8_t id)
{
	switch (id & 0b111)
	{
	case 0:
		return BC.high;
	case 1:
		return BC.low;
	case 2:
		return DE.high;
	case 3:
		return DE.low;
	case 4:
		return HL.high;
	case 5:
		return HL.low;
	case 6:
		return m_bus->read(HL.reg);
	case 7:
		return AF.high;
	}
}

void CPU::setR8(uint8_t id, uint8_t value)
{
	switch (id & 0b111)
	{
	case 0:
		BC.high = value; break;
	case 1:
		BC.low = value; break;
	case 2:
		DE.high = value; break;
	case 3:
		DE.low = value; break;
	case 4:
		HL.high = value; break;
	case 5:
		HL.low = value; break;
	case 6:
		m_bus->write(HL.reg, value); break;
	case 7:
		AF.high = value; break;
	}
}

uint16_t CPU::getR16(uint8_t id, int group)
{
	if (group == 2)
		Logger::getInstance()->msg(LoggerSeverity::Error, "Group 2 decode attempted (not currently supported)");
	switch (id & 0b11)
	{
	case 0:
		return BC.reg;
	case 1:
		return DE.reg;
	case 2:
		if (group == 1 || group==3)
			return HL.reg;
		break;
	case 3:
		if (group == 1)
			return SP.reg;
		if (group == 3)
			return AF.reg;
	}
}

void CPU::setR16(uint8_t id, uint16_t value, int group)
{
	if (group == 2)
		Logger::getInstance()->msg(LoggerSeverity::Error, "Group 2 decode attempted (not currently supported)");
	switch (id & 0b11)
	{
	case 0:
		BC.reg = value; break;
	case 1:
		DE.reg = value; break;
	case 2:
		if (group == 1 || group == 3)
			HL.reg = value;
		break;
	case 3:
		if (group == 1)
			SP.reg = value;
		if (group == 3)
			AF.reg = value;
		break;
	}
}

bool CPU::checkConditionsMet(uint8_t conditionCode)
{
	switch (conditionCode & 0b11)
	{
	case 0:
		return !m_getZeroFlag();
	case 1:
		return m_getZeroFlag();
	case 2:
		return !m_getCarryFlag();
	case 3:
		return m_getCarryFlag();
	}
}

//opcode encodings
void CPU::_storeSPAtAddress()
{
	uint8_t lower = m_fetch();
	uint8_t higher = m_fetch();
	uint16_t addr = ((uint16_t)higher << 8) | lower;
	m_bus->write(addr, SP.low);
	m_bus->write(addr+1, SP.high);
}

void CPU::_STOP()
{
	Logger::getInstance()->msg(LoggerSeverity::Error, "STOP should never be called on DMG.");
}

void CPU::_JRUnconditional()
{
	int8_t offset = (int8_t)m_fetch();
	PC += offset;
	m_bus->tick();
}

void CPU::_JRConditional()
{
	int8_t offset = (int8_t)m_fetch();				//instruction still takes 8 t-cycles if branch not taken
	uint8_t condition = (m_lastOpcode >> 3) & 0b11;
	if (checkConditionsMet(condition))
	{
		PC += offset;
		m_bus->tick();
	}
}

void CPU::_loadR16Immediate()
{
	uint8_t lower = m_fetch();
	uint8_t higher = m_fetch();
	uint16_t res = ((uint16_t)higher << 8) | lower;
	uint8_t regIdx = (m_lastOpcode >> 4) & 0b11;
	setR16(regIdx, res, 1);
}

void CPU::_addHLR16()
{
	uint8_t regIdx = (m_lastOpcode >> 4) & 0b11;
	uint16_t operand = getR16(regIdx, 1);

	m_setSubtractFlag(false);
	m_setHalfCarryFlag(((HL.reg & 0xfff) + (operand & 0xfff)) > 0xfff);
	m_setCarryFlag(((int)HL.reg + (int)operand) > 0xffff);

	HL.reg += operand;

	m_bus->tick();	//internal tick

}

void CPU::_storeAccum()
{
	uint8_t regIndex = (m_lastOpcode >> 4) & 0b11;
	switch (regIndex)
	{
	case 0b00:
		m_bus->write(BC.reg, AF.high); break;
	case 0b01:
		m_bus->write(DE.reg, AF.high); break;
	case 0b10:
		m_bus->write(HL.reg++, AF.high); break;
	case 0b11:
		m_bus->write(HL.reg--, AF.high); break;
	}
}

void CPU::_loadAccum()
{
	uint8_t regIndex = (m_lastOpcode >> 4) & 0b11;
	switch (regIndex)
	{
	case 0b00:
		AF.high = m_bus->read(BC.reg); break;
	case 0b01:
		AF.high = m_bus->read(DE.reg); break;
	case 0b10:
		AF.high = m_bus->read(HL.reg++); break;
	case 0b11:
		AF.high = m_bus->read(HL.reg--); break;
	}
}

void CPU::_incR16()
{
	uint8_t regIndex = (m_lastOpcode >> 4) & 0b11;
	uint16_t regContents = getR16(regIndex, 1);
	setR16(regIndex, regContents + 1, 1);
	m_bus->tick();	//some internal cycle
}

void CPU::_decR16()
{
	uint8_t regIndex = (m_lastOpcode >> 4) & 0b11;
	uint16_t regContents = getR16(regIndex, 1);
	setR16(regIndex, regContents - 1, 1);
	m_bus->tick();	//some internal cycle
}

void CPU::_incR8()
{
	uint8_t regIndex = (m_lastOpcode >> 3) & 0b111;
	uint8_t regContents = getR8(regIndex);

	m_setZeroFlag(regContents == 0xFF);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag((regContents & 0xF) == 0xF);

	setR8(regIndex, regContents + 1);
}

void CPU::_decR8()
{
	uint8_t regIndex = (m_lastOpcode >> 3) & 0b111;
	uint8_t regContents = getR8(regIndex);
	regContents--;

	m_setZeroFlag(!regContents);
	m_setSubtractFlag(true);
	m_setHalfCarryFlag((regContents & 0x0f) == 0x0f);

	setR8(regIndex, regContents);
}

void CPU::_ldR8Immediate()
{
	uint8_t val = m_fetch();
	uint8_t regIndex = (m_lastOpcode >> 3) & 0b111;
	setR8(regIndex, val);
}

void CPU::_bitwiseOps()
{
	uint8_t opEncoding = (m_lastOpcode >> 3) & 0b111;

	uint8_t temp = 0, lastCarry = 0;	//used by shifts/rotates

	switch (opEncoding)
	{
	case 0:	//RLCA
	{
		temp = (AF.high & 0b10000000) >> 7;
		AF.high <<= 1;
		m_setCarryFlag(temp);
		m_setSubtractFlag(false);
		m_setZeroFlag(false);
		m_setHalfCarryFlag(false);
		AF.high |= temp;
	}
		break;
	case 1:	//RRCA
	{
		temp = (AF.high & 0b1);
		AF.high >>= 1;
		m_setCarryFlag(temp);
		m_setSubtractFlag(false);
		m_setZeroFlag(false);
		m_setHalfCarryFlag(false);
		AF.high |= (temp << 7);
	}
		break;
	case 2:	//RLA
	{
		temp = (AF.high & 0b10000000) >> 7;
		AF.high <<= 1;
		lastCarry = m_getCarryFlag();
		AF.high |= lastCarry;
		m_setCarryFlag(temp);
		m_setSubtractFlag(false);
		m_setZeroFlag(false);
		m_setHalfCarryFlag(false);
	}
		break;
	case 3:	//RRA
	{
		temp = AF.high & 0b1;
		AF.high >>= 1;
		lastCarry = (m_getCarryFlag()) ? 0b10000000 : 0b0;
		AF.high |= lastCarry;
		m_setCarryFlag(temp);
		m_setSubtractFlag(false);
		m_setZeroFlag(false);
		m_setHalfCarryFlag(false);
	}
	break;
	case 4:
		_DAA(); break;
	case 5:	//CPL
		m_setSubtractFlag(true);
		m_setHalfCarryFlag(true);
		AF.high = ~AF.high;
		break;
	case 6:	//SCF
		m_setCarryFlag(true);
		m_setSubtractFlag(false);
		m_setHalfCarryFlag(false);
		break;
	case 7:	//CCF
		m_setCarryFlag(!m_getCarryFlag());
		m_setSubtractFlag(false);
		m_setHalfCarryFlag(false);
		break;
	}
}

void CPU::_DAA()
{
	//copypasted because this instruction makes no sense
	uint8_t correction = m_getCarryFlag() ? 0x60 : 0x00;

	if (m_getHalfCarryFlag() || (!m_getSubtractFlag() && ((AF.high & 0xF) > 9)))
		correction |= 0x06;
	if (m_getCarryFlag() || (!m_getSubtractFlag() && (AF.high > 0x99)))
		correction |= 0x60;
	if (m_getSubtractFlag())
		AF.high -= correction;
	else
		AF.high += correction;
	if (((correction << 2) & 0x100) != 0)
		m_setCarryFlag(true);

	m_setHalfCarryFlag(false);
	m_setZeroFlag(!AF.high);

}

void CPU::_halt()
{
	m_halted = true;
}

void CPU::_ldR8()
{
	if (m_lastOpcode == 0b01110110)	//ld where src = dst = 0b110 is a halt, not a ld instruction!!
	{
		_halt();
		return;
	}
	uint8_t srcRegIndex = m_lastOpcode & 0b111;
	uint8_t dstRegIndex = (m_lastOpcode >> 3) & 0b111;

	setR8(dstRegIndex, getR8(srcRegIndex));
}

void CPU::_ALUOpsRegister()
{
	uint8_t operandRegIndex = m_lastOpcode & 0b111;
	uint8_t op = (m_lastOpcode >> 3) & 0b111;
	_performALUOperation(op, getR8(operandRegIndex));
}

void CPU::_performALUOperation(uint8_t op, uint8_t operand)
{
	bool carryIn = false, subtract = false, logical = false, isAND=false;
	uint8_t src = AF.high;
	switch (op)
	{
	case 0:
		AF.high += operand; break;
	case 1:
		AF.high += operand + m_getCarryFlag();
		carryIn = true;
		break;
	case 2:
		AF.high -= operand;
		subtract = true;
		break;
	case 3:
		AF.high -= (operand + m_getCarryFlag());
		subtract = true;
		carryIn = true;
		break;
	case 4:
		AF.high &= operand;
		logical = true;
		isAND = true;
		break;
	case 5:
		AF.high ^= operand;
		logical = true;
		break;
	case 6:
		AF.high |= operand;
		logical = true;
		break;
	case 7:
		subtract = true;
		break;
	}

	if (logical)
		m_set8BitLogicalFlags(AF.high,isAND);
	else
		m_set8BitArithmeticFlags(src, operand, carryIn, subtract);
}

void CPU::_RETConditional()
{
	uint8_t conditionCode = (m_lastOpcode >> 3) & 0b11;
	m_bus->tick();	//internal tick
	if (checkConditionsMet(conditionCode))
	{
		PC = m_popFromStack();
		m_bus->tick();	//internal tick
	}
}

void CPU::_storeHiImmediate()
{
	uint16_t offset = m_fetch();
	m_bus->write(0xFF00 + offset, AF.high);
}

void CPU::_addSPImmediate()
{
	int8_t val = (int8_t)m_fetch();
	uint16_t regInitial = SP.reg;
	SP.reg += val;
	m_setZeroFlag(false);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(((regInitial ^ val ^ (SP.reg & 0xFFFF)) & 0x10) == 0x10);	//black magic 16-bit half carry
	m_setCarryFlag(((regInitial ^ val ^ (SP.reg & 0xFFFF)) & 0x100) == 0x100);
	m_bus->tick();	//2 internal ticks, might be wrong? gbops says two internals (i.e. SP.lower written, then SP.upper)
	m_bus->tick();
}

void CPU::_loadHiImmediate()
{
	uint16_t offset = m_fetch();
	AF.high = m_bus->read(0xFF00 + offset);
}

void CPU::_LDHLSPImmediate()
{
	uint8_t val = m_fetch();
	int8_t offs = (int8_t)val;

	uint16_t SP_temp = SP.reg;
	SP_temp += offs;

	m_setZeroFlag(false);
	m_setSubtractFlag(false);

	m_setHalfCarryFlag(((SP.reg ^ offs ^ (SP_temp & 0xFFFF)) & 0x10) == 0x10);	//black magic 16-bit half carry
	m_setCarryFlag(((SP.reg ^ offs ^ (SP_temp & 0xFFFF)) & 0x100) == 0x100);

	HL.reg = SP_temp;
	m_bus->tick();	//internal tick
}

void CPU::_popR16()
{
	uint8_t reg = (m_lastOpcode >> 4) & 0b11;
	uint16_t val = m_popFromStack();
	if (reg == 3)
		val &= 0b1111111111110000;
	setR16(reg, val, 3);
}

void CPU::_miscStackOps()
{
	uint8_t op = (m_lastOpcode >> 4) & 0b11;
	switch (op)
	{
	case 0b00:
		PC = m_popFromStack();
		m_bus->tick();	//internal
		break;
	case 0b01:
		PC = m_popFromStack();
		m_interruptManager->enableInterrupts();
		m_bus->tick();
		break;
	case 0b10:
		PC = HL.reg;
		break;
	case 0b11:
		SP.reg = HL.reg;
		m_bus->tick();	//internal
		break;
	}
}

void CPU::_JPConditional()
{
	uint8_t low = m_fetch();
	uint8_t high = m_fetch();
	uint16_t addr = ((uint16_t)high << 8) | low;

	uint8_t conditionCode = (m_lastOpcode >> 3) & 0b11;
	if (checkConditionsMet(conditionCode))
	{
		PC = addr;
		m_bus->tick();	//internal tick
	}
}

void CPU::_storeHi()
{
	m_bus->write(0xFF00 + BC.low, AF.high);
}

void CPU::_storeAccumDirect()
{
	uint8_t low = m_fetch();
	uint8_t high = m_fetch();
	uint16_t addr = ((uint16_t)high << 8) | low;
	m_bus->write(addr, AF.high);
}

void CPU::_loadHi()
{
	AF.high = m_bus->read(0xFF00 + BC.low);
}

void CPU::_loadAccumDirect()
{
	uint8_t low = m_fetch();
	uint8_t high = m_fetch();
	uint16_t addr = ((uint16_t)high << 8) | low;
	AF.high = m_bus->read(addr);
}

void CPU::_miscOpsEIDI()
{
	uint8_t op = (m_lastOpcode >> 3) & 0b111;
	if (op == 0)	//JP u16
	{
		uint8_t low = m_fetch();
		uint8_t high = m_fetch();
		uint16_t addr = ((uint16_t)high << 8) | low;
		PC = addr;
		m_bus->tick();
	}
	else if (op == 1)	//CB
	{
		Logger::getInstance()->msg(LoggerSeverity::Error, "CB handler shouldn't be called here");
	}
	else if (op == 6)	//DI
	{
		m_interruptManager->disableInterrupts();
	}
	else if (op == 7)	//EI
	{
		if (!m_EIRequested)
			m_instrSinceEI = 0;
		m_EIRequested = true;
	}
	else
		Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid opcode");
}

void CPU::_callConditional()
{
	uint8_t low = m_fetch();
	uint8_t high = m_fetch();
	uint16_t addr = ((uint16_t)high << 8) | low;
	uint8_t conditionCode = (m_lastOpcode >> 3) & 0b11;
	if (checkConditionsMet(conditionCode))
	{
		m_bus->tick();	//internal branch decision
		m_pushToStack(PC);
		PC = addr;
	}
}

void CPU::_pushR16()
{
	uint8_t regIndex = (m_lastOpcode >> 4) & 0b11;
	m_bus->tick();	//internal
	m_pushToStack(getR16(regIndex, 3));
}

void CPU::_callImmediate()
{
	uint8_t low = m_fetch();
	uint8_t high = m_fetch();
	uint16_t addr = ((uint16_t)high << 8) | low;
	m_bus->tick();	//internal tick
	m_pushToStack(PC);
	PC = addr;
}

void CPU::_ALUOpsImmediate()
{
	uint8_t immVal = m_fetch();
	uint8_t op = (m_lastOpcode >> 3) & 0b111;
	_performALUOperation(op, immVal);
}

void CPU::_reset()
{
	uint16_t resetVector = (m_lastOpcode >> 3) & 0b111;
	resetVector *= 8;
	m_bus->tick();
	m_pushToStack(PC);
	PC = resetVector;
}

void CPU::_CBShiftsRotates()
{
	uint8_t op = (m_lastOpcode >> 3) & 0b111;
	switch (op)
	{
	case 0:
		_RLC(); break;
	case 1:
		_RRC(); break;
	case 2:
		_RL(); break;
	case 3:
		_RR(); break;
	case 4:
		_SLA(); break;
	case 5:
		_SRA(); break;
	case 6:
		_SWAP(); break;
	case 7:
		_SRL(); break;
	}
}

void CPU::_CBGetBitComplement()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t bitIdx = (m_lastOpcode >> 3) & 0b111;

	uint8_t regContents = getR8(regIdx);

	uint8_t bit = (regContents >> bitIdx) & 0b1;
	m_setZeroFlag(!bit);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);

}

void CPU::_CBResetBit()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t bitIdx = (m_lastOpcode >> 3) & 0b111;

	uint8_t regContents = getR8(regIdx);

	uint8_t mask = ~(1 << bitIdx);
	regContents &= mask;
	setR8(regIdx, regContents);
}

void CPU::_CBSetBit()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t bitIdx = (m_lastOpcode >> 3) & 0b111;

	uint8_t regContents = getR8(regIdx);

	uint8_t mask = (1 << bitIdx);
	regContents |= mask;
	setR8(regIdx, regContents);
}


void CPU::_RL()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);

	uint8_t msb = (reg & 0b10000000) >> 7;
	uint8_t lastCarry = (m_getCarryFlag()) ? 0b00000001 : 0b0;
	reg <<= 1;
	reg |= lastCarry;
	m_setZeroFlag(!reg);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}

void CPU::_RLC()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t msb = (reg & 0b10000000) >> 7;
	reg <<= 1;
	reg |= msb;
	m_setCarryFlag(msb);
	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}


void CPU::_RR()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	uint8_t m_lastCarry = (m_getCarryFlag()) ? 0b10000000 : 0b0;
	reg |= m_lastCarry;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}

void CPU::_RRC()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	m_setCarryFlag(lsb);
	reg |= (lsb << 7);
	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}

void CPU::_SLA()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t msb = (reg & 0b10000000) >> 7;
	reg <<= 1;
	m_setZeroFlag(!reg);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}

void CPU::_SRA()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t lsb = (reg & 0b00000001);
	uint8_t msb = (reg & 0b10000000);
	reg >>= 1;
	reg |= msb;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}


void CPU::_SRL()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	setR8(regIdx, reg);
}

void CPU::_SWAP()
{
	uint8_t regIdx = m_lastOpcode & 0b111;
	uint8_t reg = getR8(regIdx);
	uint8_t low = (reg & 0x0F) << 4;
	uint8_t high = (reg & 0xF0) >> 4;

	reg = low | high;

	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
	setR8(regIdx, reg);
}

