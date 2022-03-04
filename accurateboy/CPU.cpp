#include"CPU.h"

CPU::CPU(std::shared_ptr<Bus>& bus, std::shared_ptr<InterruptManager>& interruptManager)
{
	m_bus = bus;
	m_interruptManager = interruptManager;
	m_initIO();	//init CPU and I/O registers to correct values
}

CPU::~CPU()
{
	//todo
}

void CPU::step()
{
	m_lastPC = PC;
	if (!m_halted)
		m_executeInstruction();
	else
		m_bus->tick();

	InterruptType queuedInt = m_interruptManager->getActiveInterrupt();
	if (queuedInt != InterruptType::None)
	{
		if (m_interruptManager->getInterruptsEnabled())
		{
			m_interruptManager->disableInterrupts();
			m_pushToStack(PC);
			PC = (uint16_t)queuedInt;
		}
		m_halted = false;
	}

}

void CPU::m_executeInstruction()
{
	uint8_t opcode = m_fetch();

	switch (opcode)
	{
	case 0x0: m_cycleCount++; break;
	case 0x1: _loadImmPairRegister(BC); break;
	case 0x2: _storeRegisterAtPairRegister(BC, AF.high); break;
	case 0x3: _incrementPairRegister(BC); break;
	case 0x4: _incrementRegister(BC.high); break;
	case 0x5: _decrementRegister(BC.high); break;
	case 0x6: _loadImmRegister(BC.high); break;
	case 0x7: _RLCA(); break;
	case 0x8: _storePairRegisterAtAddress(SP); break;
	case 0x9: _addPairRegisters(HL, BC); break;
	case 0xA: _loadDirectFromPairRegister(AF.high, BC); break;
	case 0xB:  _decrementPairRegister(BC); break;
	case 0xC: _incrementRegister(BC.low); break;
	case 0xD: _decrementRegister(BC.low); break;
	case 0xE: _loadImmRegister(BC.low); break;
	case 0xF: _RRCA(); break;
	case 0x10: _stop(); break;
	case 0x11: _loadImmPairRegister(DE); break;
	case 0x12: _storeRegisterAtPairRegister(DE, AF.high); break;
	case 0x13: _incrementPairRegister(DE); break;
	case 0x14: _incrementRegister(DE.high); break;
	case 0x15: _decrementRegister(DE.high); break;
	case 0x16: _loadImmRegister(DE.high); break;
	case 0x17: _RLA(); break;
	case 0x18: _jumpRelative(); break;
	case 0x19: _addPairRegisters(HL, DE); break;
	case 0x1A: _loadDirectFromPairRegister(AF.high, DE); break;
	case 0x1B: _decrementPairRegister(DE); break;
	case 0x1C: _incrementRegister(DE.low); break;
	case 0x1D: _decrementRegister(DE.low); break;
	case 0x1E: _loadImmRegister(DE.low); break;
	case 0x1F: _RRA(); break;
	case 0x20: _jumpRelativeIfZeroNotSet(); break;
	case 0x21: _loadImmPairRegister(HL); break;
	case 0x22: _storeRegisterAtPairRegisterInc(HL, AF.high); break;
	case 0x23: _incrementPairRegister(HL); break;
	case 0x24: _incrementRegister(HL.high); break;
	case 0x25: _decrementRegister(HL.high); break;
	case 0x26: _loadImmRegister(HL.high); break;
	case 0x27: _adjustBCD(); break;
	case 0x28: _jumpRelativeIfZeroSet(); break;
	case 0x29: _addPairRegisters(HL, HL); break;
	case 0x2A: _loadDirectFromPairRegisterInc(AF.high, HL); break;
	case 0x2B: _decrementPairRegister(HL); break;
	case 0x2C: _incrementRegister(HL.low); break;
	case 0x2D: _decrementRegister(HL.low); break;
	case 0x2E: _loadImmRegister(HL.low); break;
	case 0x2F: _complement(); break;
	case 0x30: _jumpRelativeIfCarryNotSet(); break;
	case 0x31: _loadImmPairRegister(SP); break;
	case 0x32: _storeRegisterAtPairRegisterDec(HL, AF.high); break;
	case 0x33: _incrementPairRegister(SP); break;
	case 0x34: _incrementPairAddress(HL); break;
	case 0x35: _decrementPairAddress(HL); break;
	case 0x36: _storeOperandAtPairAddress(HL); break;
	case 0x37: _setCarryFlag(); break;
	case 0x38: _jumpRelativeIfCarrySet(); break;
	case 0x39: _addPairRegisters(HL, SP); break;
	case 0x3A: _loadDirectFromPairRegisterDec(AF.high, HL); break;
	case 0x3B: _decrementPairRegister(SP); break;
	case 0x3C: _incrementRegister(AF.high); break;
	case 0x3D: _decrementRegister(AF.high); break;
	case 0x3E: _loadImmRegister(AF.high); break;
	case 0x3F: _flipCarryFlag(); break;
	case 0x40: _loadImmFromRegister(BC.high, BC.high); break;
	case 0x41: _loadImmFromRegister(BC.high, BC.low); break;
	case 0x42: _loadImmFromRegister(BC.high, DE.high); break;
	case 0x43: _loadImmFromRegister(BC.high, DE.low); break;
	case 0x44: _loadImmFromRegister(BC.high, HL.high); break;
	case 0x45: _loadImmFromRegister(BC.high, HL.low); break;
	case 0x46: _loadDirectFromPairRegister(BC.high, HL); break;
	case 0x47: _loadImmFromRegister(BC.high, AF.high); break;
	case 0x48: _loadImmFromRegister(BC.low, BC.high); break;
	case 0x49: _loadImmFromRegister(BC.low, BC.low); break;
	case 0x4A: _loadImmFromRegister(BC.low, DE.high); break;
	case 0x4B: _loadImmFromRegister(BC.low, DE.low); break;
	case 0x4C: _loadImmFromRegister(BC.low, HL.high); break;
	case 0x4D: _loadImmFromRegister(BC.low, HL.low); break;
	case 0x4E: _loadDirectFromPairRegister(BC.low, HL); break;
	case 0x4F: _loadImmFromRegister(BC.low, AF.high); break;
	case 0x50: _loadImmFromRegister(DE.high, BC.high); break;
	case 0x51: _loadImmFromRegister(DE.high, BC.low); break;
	case 0x52: _loadImmFromRegister(DE.high, DE.high); break;
	case 0x53: _loadImmFromRegister(DE.high, DE.low); break;
	case 0x54: _loadImmFromRegister(DE.high, HL.high); break;
	case 0x55: _loadImmFromRegister(DE.high, HL.low); break;
	case 0x56: _loadDirectFromPairRegister(DE.high, HL); break;
	case 0x57: _loadImmFromRegister(DE.high, AF.high); break;
	case 0x58: _loadImmFromRegister(DE.low, BC.high); break;
	case 0x59: _loadImmFromRegister(DE.low, BC.low); break;
	case 0x5A: _loadImmFromRegister(DE.low, DE.high); break;
	case 0x5B: _loadImmFromRegister(DE.low, DE.low); break;
	case 0x5C: _loadImmFromRegister(DE.low, HL.high); break;
	case 0x5D: _loadImmFromRegister(DE.low, HL.low); break;
	case 0x5E: _loadDirectFromPairRegister(DE.low, HL); break;
	case 0x5F: _loadImmFromRegister(DE.low, AF.high); break;
	case 0x60: _loadImmFromRegister(HL.high, BC.high); break;
	case 0x61: _loadImmFromRegister(HL.high, BC.low); break;
	case 0x62: _loadImmFromRegister(HL.high, DE.high); break;
	case 0x63: _loadImmFromRegister(HL.high, DE.low); break;
	case 0x64: _loadImmFromRegister(HL.high, HL.high); break;
	case 0x65: _loadImmFromRegister(HL.high, HL.low); break;
	case 0x66: _loadDirectFromPairRegister(HL.high, HL); break;
	case 0x67: _loadImmFromRegister(HL.high, AF.high); break;
	case 0x68: _loadImmFromRegister(HL.low, BC.high); break;
	case 0x69: _loadImmFromRegister(HL.low, BC.low); break;
	case 0x6A: _loadImmFromRegister(HL.low, DE.high); break;
	case 0x6B: _loadImmFromRegister(HL.low, DE.low); break;
	case 0x6C: _loadImmFromRegister(HL.low, HL.high); break;
	case 0x6D: _loadImmFromRegister(HL.low, HL.low); break;
	case 0x6E: _loadDirectFromPairRegister(HL.low, HL); break;
	case 0x6F: _loadImmFromRegister(HL.low, AF.high); break;
	case 0x70: _storeRegisterAtPairRegister(HL, BC.high); break;
	case 0x71: _storeRegisterAtPairRegister(HL, BC.low); break;
	case 0x72: _storeRegisterAtPairRegister(HL, DE.high); break;
	case 0x73: _storeRegisterAtPairRegister(HL, DE.low); break;
	case 0x74: _storeRegisterAtPairRegister(HL, HL.high); break;
	case 0x75: _storeRegisterAtPairRegister(HL, HL.low); break;
	case 0x76: _halt(); break;
	case 0x77: _storeRegisterAtPairRegister(HL, AF.high); break;
	case 0x78: _loadImmFromRegister(AF.high, BC.high); break;
	case 0x79: _loadImmFromRegister(AF.high, BC.low); break;
	case 0x7A: _loadImmFromRegister(AF.high, DE.high); break;
	case 0x7B: _loadImmFromRegister(AF.high, DE.low); break;
	case 0x7C: _loadImmFromRegister(AF.high, HL.high); break;
	case 0x7D: _loadImmFromRegister(AF.high, HL.low); break;
	case 0x7E: _loadDirectFromPairRegister(AF.high, HL); break;
	case 0x7F: _loadImmFromRegister(AF.high, AF.high); break;
	case 0x80: _addRegisters(AF.high, BC.high); break;
	case 0x81: _addRegisters(AF.high, BC.low); break;
	case 0x82: _addRegisters(AF.high, DE.high); break;
	case 0x83: _addRegisters(AF.high, DE.low); break;
	case 0x84: _addRegisters(AF.high, HL.high); break;
	case 0x85: _addRegisters(AF.high, HL.low); break;
	case 0x86: _addPairAddress(AF.high, HL); break;
	case 0x87: _addRegisters(AF.high, AF.high); break;
	case 0x88: _addRegistersCarry(AF.high, BC.high); break;
	case 0x89: _addRegistersCarry(AF.high, BC.low); break;
	case 0x8A: _addRegistersCarry(AF.high, DE.high); break;
	case 0x8B: _addRegistersCarry(AF.high, DE.low); break;
	case 0x8C: _addRegistersCarry(AF.high, HL.high); break;
	case 0x8D: _addRegistersCarry(AF.high, HL.low); break;
	case 0x8E: _addPairAddressCarry(AF.high, HL); break;
	case 0x8F: _addRegistersCarry(AF.high, AF.high); break;
	case 0x90: _subRegister(BC.high); break;
	case 0x91: _subRegister(BC.low); break;
	case 0x92: _subRegister(DE.high); break;
	case 0x93: _subRegister(DE.low); break;
	case 0x94: _subRegister(HL.high); break;
	case 0x95: _subRegister(HL.low); break;
	case 0x96: _subPairAddress(HL); break;
	case 0x97: _subRegister(AF.high); break;
	case 0x98: _subRegisterCarry(BC.high); break;
	case 0x99: _subRegisterCarry(BC.low); break;
	case 0x9A: _subRegisterCarry(DE.high); break;
	case 0x9B: _subRegisterCarry(DE.low); break;
	case 0x9C: _subRegisterCarry(HL.high); break;
	case 0x9D: _subRegisterCarry(HL.low); break;
	case 0x9E: _subPairAddressCarry(HL); break;
	case 0x9F: _subRegisterCarry(AF.high); break;
	case 0xA0: _andRegister(BC.high); break;
	case 0xA1: _andRegister(BC.low); break;
	case 0xA2: _andRegister(DE.high); break;
	case 0xA3: _andRegister(DE.low); break;
	case 0xA4: _andRegister(HL.high); break;
	case 0xA5: _andRegister(HL.low); break;
	case 0xA6: _andPairAddress(HL); break;
	case 0xA7: _andRegister(AF.high); break;
	case 0xA8: _xorRegister(BC.high); break;
	case 0xA9: _xorRegister(BC.low); break;
	case 0xAA: _xorRegister(DE.high); break;
	case 0xAB: _xorRegister(DE.low); break;
	case 0xAC: _xorRegister(HL.high); break;
	case 0xAD: _xorRegister(HL.low); break;
	case 0xAE: _xorPairAddress(HL); break;
	case 0xAF: _xorRegister(AF.high); break;
	case 0xB0: _orRegister(BC.high); break;
	case 0xB1: _orRegister(BC.low); break;
	case 0xB2: _orRegister(DE.high); break;
	case 0xB3: _orRegister(DE.low); break;
	case 0xB4: _orRegister(HL.high); break;
	case 0xB5: _orRegister(HL.low); break;
	case 0xB6: _orPairAddress(HL); break;
	case 0xB7: _orRegister(AF.high); break;
	case 0xB8: _compareRegister(BC.high); break;
	case 0xB9: _compareRegister(BC.low); break;
	case 0xBA: _compareRegister(DE.high); break;
	case 0xBB: _compareRegister(DE.low); break;
	case 0xBC: _compareRegister(HL.high); break;
	case 0xBD: _compareRegister(HL.low); break;
	case 0xBE: _comparePairAddress(HL); break;
	case 0xBF: _compareRegister(AF.high); break;
	case 0xC0: _returnIfZeroNotSet(); break;
	case 0xC1: _popToPairRegister(BC); break;
	case 0xC2: _jumpAbsoluteIfZeroNotSet(); break;
	case 0xC3: _jumpAbsolute(); break;
	case 0xC4: _callIfZeroNotSet(); break;
	case 0xC5: _pushPairRegister(BC); break;
	case 0xC6: _addValue(AF.high); break;
	case 0xC7: _resetToVector(0); break;
	case 0xC8: _returnIfZeroSet(); break;
	case 0xC9: _return(); break;
	case 0xCA: _jumpAbsoluteIfZeroSet(); break;
	case 0xCB: m_executePrefixedInstruction(); break;	//Special 16-bit prefixed instruction decoder called instead
	case 0xCC: _callIfZeroSet(); break;
	case 0xCD: _call(); break;
	case 0xCE: _addValueCarry(AF.high); break;
	case 0xCF: _resetToVector(1); break;
	case 0xD0: _returnIfCarryNotSet(); break;
	case 0xD1: _popToPairRegister(DE); break;
	case 0xD2: _jumpAbsoluteIfCarryNotSet(); break;
//	case 0xD3: break;									Invalid opcode decoding D3
	case 0xD4: _callIfCarryNotSet(); break;
	case 0xD5: _pushPairRegister(DE); break;
	case 0xD6: _subValue(); break;
	case 0xD7: _resetToVector(2); break;
	case 0xD8: _returnIfCarrySet(); break;
	case 0xD9: _returnFromInterrupt(); break;
	case 0xDA: _jumpAbsoluteIfCarrySet(); break;
//	case 0xDB: break;                                   Invalid opcode decoding DB
	case 0xDC: _callIfCarrySet(); break;
//	case 0xDD: break;                                   Invalid opcode decoding DD
	case 0xDE: _subValueCarry(); break;
	case 0xDF: _resetToVector(3); break;
	case 0xE0: _storeRegisterInHRAMImm(AF.high); break;
	case 0xE1: _popToPairRegister(HL); break;
	case 0xE2: _storeRegisterInHRAM(BC.low, AF.high); break;
//	case 0xE3: break;                                   Invalid opcode decoding E3
//	case 0xE4: break;                                   Invalid opcode decoding E4
	case 0xE5: _pushPairRegister(HL); break;
	case 0xE6: _andValue(); break;
	case 0xE7: _resetToVector(4); break;
	case 0xE8: _addSignedValueToPairRegister(SP); break;
	case 0xE9: PC = HL.reg; m_cycleCount++;  break;	//lazy implementation but instruction is 1-cycle anyway
	case 0xEA: _storeRegisterAtAddress(AF.high); break;
//	case 0xEB: break;                                   Invalid opcode decoding EB
//	case 0xEC: break;                                   Invalid opcode decoding EC
//	case 0xED: break;                                   Invalid opcode decoding ED
	case 0xEE: _xorValue(); break;
	case 0xEF: _resetToVector(5); break;
	case 0xF0: _loadFromHRAMImm(AF.high); break;
	case 0xF1: _popToPairRegister(AF); AF.low &= 0b11110000; break;	//Lower 4 bits of F are hard-wired to low.
	case 0xF2: _loadFromHRAM(AF.high, BC.low); break;
	case 0xF3: _disableInterrupts(); break;
//	case 0xF4: break;                                   Invalid opcode decoding F4
	case 0xF5: _pushPairRegister(AF); break;
	case 0xF6: _orValue(); break;
	case 0xF7: _resetToVector(6); break;
	case 0xF8: _loadHLStackIdx(); break;
	case 0xF9: SP.reg = HL.reg; m_cycleCount += 2; break;	
	case 0xFA: _loadRegisterFromAddress(AF.high); break;
	case 0xFB: _enableInterrupts(); break;
//	case 0xFC: break;                                   Invalid opcode decoding FC
//	case 0xFD: break;                                   Invalid opcode decoding FD
	case 0xFE: _compareValue(); break;
	case 0xFF: _resetToVector(7); break;
	default: Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid opcode " + std::to_string(opcode));
	}
}

void CPU::m_executePrefixedInstruction()
{
	uint8_t opcode = m_fetch();

	switch (opcode)
	{
	case 0x0: _RLC(BC.high); break;
	case 0x1: _RLC(BC.low); break;
	case 0x2: _RLC(DE.high); break;
	case 0x3: _RLC(DE.low); break;
	case 0x4: _RLC(HL.high); break;
	case 0x5: _RLC(HL.low); break;
	case 0x6: _RLC(HL); break;
	case 0x7: _RLC(AF.high); break;
	case 0x8: _RRC(BC.high); break;
	case 0x9: _RRC(BC.low); break;
	case 0xA: _RRC(DE.high); break;
	case 0xB: _RRC(DE.low); break;
	case 0xC: _RRC(HL.high); break;
	case 0xD: _RRC(HL.low); break;
	case 0xE: _RRC(HL); break;
	case 0xF: _RRC(AF.high); break;
	case 0x10: _RL(BC.high); break;
	case 0x11: _RL(BC.low); break;
	case 0x12: _RL(DE.high); break;
	case 0x13: _RL(DE.low); break;
	case 0x14: _RL(HL.high); break;
	case 0x15: _RL(HL.low); break;
	case 0x16: _RL(HL); break;
	case 0x17: _RL(AF.high); break;
	case 0x18: _RR(BC.high); break;
	case 0x19: _RR(BC.low); break;
	case 0x1A: _RR(DE.high); break;
	case 0x1B: _RR(DE.low); break;
	case 0x1C: _RR(HL.high); break;
	case 0x1D: _RR(HL.low); break;
	case 0x1E: _RR(HL); break;
	case 0x1F: _RR(AF.high); break;
	case 0x20: _SLA(BC.high); break;
	case 0x21: _SLA(BC.low); break;
	case 0x22: _SLA(DE.high); break;
	case 0x23: _SLA(DE.low); break;
	case 0x24: _SLA(HL.high); break;
	case 0x25: _SLA(HL.low); break;
	case 0x26: _SLA(HL); break;
	case 0x27: _SLA(AF.high); break;
	case 0x28: _SRA(BC.high); break;
	case 0x29: _SRA(BC.low); break;
	case 0x2A: _SRA(DE.high); break;
	case 0x2B: _SRA(DE.low); break;
	case 0x2C: _SRA(HL.high); break;
	case 0x2D: _SRA(HL.low); break;
	case 0x2E: _SRA(HL); break;
	case 0x2F: _SRA(AF.high); break;
	case 0x30: _SWAP(BC.high); break;
	case 0x31: _SWAP(BC.low); break;
	case 0x32: _SWAP(DE.high); break;
	case 0x33: _SWAP(DE.low); break;
	case 0x34: _SWAP(HL.high); break;
	case 0x35: _SWAP(HL.low); break;
	case 0x36: _SWAP(HL); break;
	case 0x37: _SWAP(AF.high); break;
	case 0x38: _SRL(BC.high); break;
	case 0x39: _SRL(BC.low); break;
	case 0x3A: _SRL(DE.high); break;
	case 0x3B: _SRL(DE.low); break;
	case 0x3C: _SRL(HL.high); break;
	case 0x3D: _SRL(HL.low); break;
	case 0x3E: _SRL(HL); break;
	case 0x3F: _SRL(AF.high); break;
	case 0x40: _BIT(0, BC.high); break;
	case 0x41: _BIT(0, BC.low); break;
	case 0x42: _BIT(0, DE.high); break;
	case 0x43: _BIT(0, DE.low); break;
	case 0x44: _BIT(0, HL.high); break;
	case 0x45: _BIT(0, HL.low); break;
	case 0x46: _BIT(0, HL); break;
	case 0x47: _BIT(0, AF.high); break;
	case 0x48: _BIT(1, BC.high); break;
	case 0x49: _BIT(1, BC.low); break;
	case 0x4A: _BIT(1,DE.high); break;
	case 0x4B: _BIT(1,DE.low); break;
	case 0x4C: _BIT(1,HL.high); break;
	case 0x4D: _BIT(1,HL.low); break;
	case 0x4E: _BIT(1, HL); break;
	case 0x4F: _BIT(1, AF.high); break;
	case 0x50: _BIT(2, BC.high); break;
	case 0x51: _BIT(2, BC.low); break;
	case 0x52: _BIT(2, DE.high); break;
	case 0x53: _BIT(2, DE.low); break;
	case 0x54: _BIT(2, HL.high); break;
	case 0x55: _BIT(2, HL.low); break;
	case 0x56: _BIT(2, HL); break;
	case 0x57: _BIT(2, AF.high); break;
	case 0x58: _BIT(3, BC.high); break;
	case 0x59: _BIT(3, BC.low); break;
	case 0x5A: _BIT(3, DE.high); break;
	case 0x5B: _BIT(3, DE.low); break;
	case 0x5C: _BIT(3, HL.high); break;
	case 0x5D: _BIT(3, HL.low); break;
	case 0x5E: _BIT(3, HL); break;
	case 0x5F: _BIT(3, AF.high); break;
	case 0x60: _BIT(4, BC.high); break;
	case 0x61: _BIT(4, BC.low); break;
	case 0x62: _BIT(4, DE.high); break;
	case 0x63: _BIT(4, DE.low); break;
	case 0x64: _BIT(4, HL.high); break;
	case 0x65: _BIT(4, HL.low); break;
	case 0x66: _BIT(4, HL); break;
	case 0x67: _BIT(4, AF.high); break;
	case 0x68: _BIT(5, BC.high); break;
	case 0x69: _BIT(5, BC.low); break;
	case 0x6A: _BIT(5, DE.high); break;
	case 0x6B: _BIT(5, DE.low); break;
	case 0x6C: _BIT(5, HL.high); break;
	case 0x6D: _BIT(5, HL.low); break;
	case 0x6E: _BIT(5, HL); break;
	case 0x6F: _BIT(5, AF.high); break;
	case 0x70: _BIT(6, BC.high); break;
	case 0x71: _BIT(6, BC.low); break;
	case 0x72: _BIT(6, DE.high); break;
	case 0x73: _BIT(6, DE.low); break;
	case 0x74: _BIT(6, HL.high); break;
	case 0x75: _BIT(6, HL.low); break;
	case 0x76: _BIT(6, HL); break;
	case 0x77: _BIT(6, AF.high); break;
	case 0x78: _BIT(7, BC.high); break;
	case 0x79: _BIT(7, BC.low); break;
	case 0x7A: _BIT(7, DE.high); break;
	case 0x7B: _BIT(7, DE.low); break;
	case 0x7C: _BIT(7, HL.high); break;
	case 0x7D: _BIT(7, HL.low); break;
	case 0x7E: _BIT(7, HL); break;
	case 0x7F: _BIT(7, AF.high); break;
	case 0x80: _RES(0, BC.high); break;
	case 0x81: _RES(0, BC.low); break;
	case 0x82: _RES(0, DE.high); break;
	case 0x83: _RES(0, DE.low); break;
	case 0x84: _RES(0, HL.high); break;
	case 0x85: _RES(0, HL.low); break;
	case 0x86: _RES(0, HL); break;
	case 0x87: _RES(0, AF.high); break;
	case 0x88: _RES(1, BC.high); break;
	case 0x89: _RES(1, BC.low); break;
	case 0x8A: _RES(1, DE.high); break;
	case 0x8B: _RES(1, DE.low); break;
	case 0x8C: _RES(1, HL.high); break;
	case 0x8D: _RES(1, HL.low); break;
	case 0x8E: _RES(1, HL); break;
	case 0x8F: _RES(1, AF.high); break;
	case 0x90: _RES(2, BC.high); break;
	case 0x91: _RES(2, BC.low); break;
	case 0x92: _RES(2, DE.high); break;
	case 0x93: _RES(2, DE.low); break;
	case 0x94: _RES(2, HL.high); break;
	case 0x95: _RES(2, HL.low); break;
	case 0x96: _RES(2, HL); break;
	case 0x97: _RES(2, AF.high); break;
	case 0x98: _RES(3, BC.high); break;
	case 0x99: _RES(3, BC.low); break;
	case 0x9A: _RES(3, DE.high); break;
	case 0x9B: _RES(3, DE.low); break;
	case 0x9C: _RES(3, HL.high); break;
	case 0x9D: _RES(3, HL.low); break;
	case 0x9E: _RES(3, HL); break;
	case 0x9F: _RES(3, AF.high); break;
	case 0xA0: _RES(4, BC.high); break;
	case 0xA1: _RES(4, BC.low); break;
	case 0xA2: _RES(4, DE.high); break;
	case 0xA3: _RES(4, DE.low); break;
	case 0xA4: _RES(4, HL.high); break;
	case 0xA5: _RES(4, HL.low); break;
	case 0xA6: _RES(4, HL); break;
	case 0xA7: _RES(4, AF.high); break;
	case 0xA8: _RES(5, BC.high); break;
	case 0xA9: _RES(5, BC.low); break;
	case 0xAA: _RES(5, DE.high); break;
	case 0xAB: _RES(5, DE.low); break;
	case 0xAC: _RES(5, HL.high); break;
	case 0xAD: _RES(5, HL.low); break;
	case 0xAE: _RES(5, HL); break;
	case 0xAF: _RES(5, AF.high); break;
	case 0xB0: _RES(6, BC.high); break;
	case 0xB1: _RES(6, BC.low); break;
	case 0xB2: _RES(6, DE.high); break;
	case 0xB3: _RES(6, DE.low); break;
	case 0xB4: _RES(6, HL.high); break;
	case 0xB5: _RES(6, HL.low); break;
	case 0xB6: _RES(6, HL); break;
	case 0xB7: _RES(6, AF.high); break;
	case 0xB8: _RES(7, BC.high); break;
	case 0xB9: _RES(7, BC.low); break;
	case 0xBA: _RES(7, DE.high); break;
	case 0xBB: _RES(7, DE.low); break;
	case 0xBC: _RES(7, HL.high); break;
	case 0xBD: _RES(7, HL.low); break;
	case 0xBE: _RES(7, HL); break;
	case 0xBF: _RES(7, AF.high); break;
	case 0xC0: _SET(0, BC.high); break;
	case 0xC1: _SET(0, BC.low); break;
	case 0xC2: _SET(0, DE.high); break;
	case 0xC3: _SET(0, DE.low); break;
	case 0xC4: _SET(0, HL.high); break;
	case 0xC5: _SET(0, HL.low); break;
	case 0xC6: _SET(0, HL); break;
	case 0xC7: _SET(0, AF.high); break;
	case 0xC8: _SET(1, BC.high); break;
	case 0xC9: _SET(1, BC.low); break;
	case 0xCA: _SET(1, DE.high); break;
	case 0xCB: _SET(1, DE.low); break;
	case 0xCC: _SET(1, HL.high); break;
	case 0xCD: _SET(1, HL.low); break;
	case 0xCE: _SET(1, HL); break;
	case 0xCF: _SET(1, AF.high); break;
	case 0xD0: _SET(2, BC.high); break;
	case 0xD1: _SET(2, BC.low); break;
	case 0xD2: _SET(2, DE.high); break;
	case 0xD3: _SET(2, DE.low); break;
	case 0xD4: _SET(2, HL.high); break;
	case 0xD5: _SET(2, HL.low); break;
	case 0xD6: _SET(2, HL); break;
	case 0xD7: _SET(2, AF.high); break;
	case 0xD8: _SET(3, BC.high); break;
	case 0xD9: _SET(3, BC.low); break;
	case 0xDA: _SET(3, DE.high); break;
	case 0xDB: _SET(3, DE.low); break;
	case 0xDC: _SET(3, HL.high); break;
	case 0xDD: _SET(3, HL.low); break;
	case 0xDE: _SET(3, HL); break;
	case 0xDF: _SET(3, AF.high); break;
	case 0xE0: _SET(4, BC.high); break;
	case 0xE1: _SET(4, BC.low); break;
	case 0xE2: _SET(4, DE.high); break;
	case 0xE3: _SET(4, DE.low); break;
	case 0xE4: _SET(4, HL.high); break;
	case 0xE5: _SET(4, HL.low); break;
	case 0xE6: _SET(4, HL); break;
	case 0xE7: _SET(4, AF.high); break;
	case 0xE8: _SET(5, BC.high); break;
	case 0xE9: _SET(5, BC.low); break;
	case 0xEA: _SET(5, DE.high); break;
	case 0xEB: _SET(5, DE.low); break;
	case 0xEC: _SET(5, HL.high); break;
	case 0xED: _SET(5, HL.low); break;
	case 0xEE: _SET(5, HL); break;
	case 0xEF: _SET(5, AF.high); break;
	case 0xF0: _SET(6, BC.high); break;
	case 0xF1: _SET(6, BC.low); break;
	case 0xF2: _SET(6, DE.high); break;
	case 0xF3: _SET(6, DE.low); break;
	case 0xF4: _SET(6,HL.high); break;
	case 0xF5: _SET(6, HL.low); break;
	case 0xF6: _SET(6, HL); break;
	case 0xF7: _SET(6, AF.high); break;
	case 0xF8: _SET(7, BC.high); break;
	case 0xF9: _SET(7, BC.low); break;
	case 0xFA: _SET(7, DE.high); break;
	case 0xFB: _SET(7, DE.low); break;
	case 0xFC: _SET(7, HL.high); break;
	case 0xFD: _SET(7, HL.low); break;
	case 0xFE: _SET(7, HL); break;
	case 0xFF: _SET(7, AF.high); break;
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
	m_bus->tick();
	return val;
}

void CPU::m_pushToStack(uint16_t val)
{
	uint8_t highByte = (val & 0xFF00) >> 8;
	uint8_t lowByte = (val & 0x00FF);

	m_bus->write(SP.reg - 1, highByte);
	m_bus->tick();
	m_bus->write(SP.reg - 2, lowByte);
	m_bus->tick();
	SP.reg -= 2;
}

uint16_t CPU::m_popFromStack()
{
	uint8_t lowByte = m_bus->read(SP.reg);
	m_bus->tick();
	uint8_t highByte = m_bus->read(SP.reg + 1);
	m_bus->tick();
	SP.reg += 2;
	uint16_t val = (((uint16_t)highByte) << 8) | lowByte;
	return val;
}

void CPU::_loadImmPairRegister(Register& reg)
{
	reg.low = m_fetch();
	reg.high = m_fetch();
}

void CPU::_loadImmRegister(uint8_t& reg)
{
	reg = m_fetch();
}

void CPU::_loadImmFromRegister(uint8_t& regA, uint8_t& regB)
{
	regA = regB;
}

void CPU::_loadDirectFromPairRegister(uint8_t& regA, Register& regB)
{
	regA = m_bus->read(regB.reg);
	m_bus->tick();
}

void CPU::_loadDirectFromPairRegisterInc(uint8_t& regA, Register& regB)
{
	regA = m_bus->read(regB.reg);
	m_bus->tick();
	regB.reg++;
}

void CPU::_loadDirectFromPairRegisterDec(uint8_t& regA, Register& regB)
{
	regA = m_bus->read(regB.reg);
	m_bus->tick();
	regB.reg--;
}

void CPU::_storeRegisterAtPairRegister(Register& regA, uint8_t& regB)
{
	m_bus->write(regA.reg, regB);
	m_bus->tick();
}

void CPU::_storeRegisterAtPairRegisterInc(Register& regA, uint8_t& regB)
{
	m_bus->write(regA.reg, regB);
	m_bus->tick();
	regA.reg++;
}

void CPU::_storeRegisterAtPairRegisterDec(Register& regA, uint8_t& regB)
{
	m_bus->write(regA.reg, regB);
	m_bus->tick();
	regA.reg--;
}

void CPU::_storePairRegisterAtAddress(Register& reg)
{
	uint8_t memLow = m_fetch();
	uint8_t memHigh = m_fetch();
	uint16_t addr = ((uint16_t)memHigh << 8) | memLow;

	m_bus->write(addr, reg.low);
	m_bus->tick();
	m_bus->write(addr + 1, reg.high);
	m_bus->tick();
}

void CPU::_storeRegisterAtAddress(uint8_t& reg)
{
	uint8_t memLow = m_fetch();
	uint8_t memHigh = m_fetch();
	uint16_t addr = ((uint16_t)memHigh << 8) | memLow;
	m_bus->write(addr, reg);
	m_bus->tick();
}

void CPU::_loadRegisterFromAddress(uint8_t& reg)
{
	uint8_t memLow = m_fetch();
	uint8_t memHigh = m_fetch();
	uint16_t addr = ((uint16_t)memHigh << 8) | memLow;
	reg = m_bus->read(addr);
	m_bus->tick();
}

void CPU::_storeOperandAtPairAddress(Register& reg)
{
	m_bus->write(reg.reg, m_fetch());	//fetch ticks
	m_bus->tick();						//then tick again = 3 ticks total (fetch,fetch,write)
}

void CPU::_storeRegisterInHRAM(uint8_t& regDst, uint8_t& regSrc)
{
	m_bus->write(0xFF00 + regDst, regSrc);	//destination register contains index from 0xFF00 to write to HRAM
	m_bus->tick();
}

void CPU::_loadFromHRAM(uint8_t& regDst, uint8_t& regSrcIdx)
{
	regDst = m_bus->read(0xFF00 + regSrcIdx);
	m_bus->tick();
}

void CPU::_storeRegisterInHRAMImm(uint8_t& reg)
{
	uint16_t addr = (uint16_t)m_fetch();
	m_bus->write(0xFF00 + addr, reg);
	m_bus->tick();
}

void CPU::_loadFromHRAMImm(uint8_t& reg)
{
	uint16_t addr = (uint16_t)m_fetch();
	reg = m_bus->read(0xFF00 + addr);
	m_bus->tick();
}

void CPU::_incrementPairRegister(Register& reg)
{
	reg.reg++;
	m_bus->tick();
}

void CPU::_incrementPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	 
	m_setHalfCarryFlag(((val & 0x0F) + (1 & 0x0F)) > 0x0F);
	val += 1;
	m_setZeroFlag(!val);
	m_setSubtractFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_incrementRegister(uint8_t& reg)
{
	m_setHalfCarryFlag(((reg & 0x0F) + (1 & 0x0F)) > 0x0F);
	reg += 1;
	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
}
void CPU::_decrementPairRegister(Register& reg)
{
	reg.reg--;
	m_bus->tick();
}

void CPU::_decrementPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	val--;
	m_setHalfCarryFlag((val & 0x0f) == 0x0f);
	m_setZeroFlag(!val);
	m_setSubtractFlag(true);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_decrementRegister(uint8_t& reg)
{
	reg--;
	m_setHalfCarryFlag((reg & 0x0f) == 0x0f);
	m_setZeroFlag(!reg);
	m_setSubtractFlag(true);
}

void CPU::_addPairRegisters(Register& regA, Register& regB)
{
	m_setHalfCarryFlag(((regA.reg & 0xfff) + (regB.reg & 0xfff)) > 0xfff);
	m_setCarryFlag(((int)regA.reg + (int)regB.reg) > 0xffff);
	m_setSubtractFlag(false);
	regA.reg += regB.reg;
	m_bus->tick();	//not sure about this.
}

void CPU::_addSignedValueToPairRegister(Register& reg)
{
	int8_t val = (int8_t)m_fetch();
	uint16_t regInitial = reg.reg;
	reg.reg += val;
	m_setZeroFlag(false);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(((regInitial ^ val ^ (reg.reg & 0xFFFF)) & 0x10) == 0x10);	//black magic 16-bit half carry
	m_setCarryFlag(((regInitial ^ val ^ (reg.reg & 0xFFFF)) & 0x100) == 0x100);
	m_bus->tick();	//2 internal ticks, might be wrong? gbops says two internals (i.e. SP.lower written, then SP.upper)
	m_bus->tick();
}

void CPU::_jumpRelative()
{
	int8_t disp = (int8_t)m_fetch();	//cast unsigned int value to signed int value as displacement is signed/relative
	PC += disp;
	m_bus->tick();
}

void CPU::_jumpRelativeIfZeroNotSet()	
{
	if (!m_getZeroFlag())
		_jumpRelative();				//if flag isn't set then 3-cycle relative jump takes place (same operation), otherwise cycles += 2
	else
	{
		PC++;
		m_bus->tick();	//if branch not taken, then it's 8 t cycles
	}
}

void CPU::_jumpRelativeIfZeroSet()
{
	if (m_getZeroFlag())
		_jumpRelative();
	else
	{
		PC++;
		m_bus->tick();
	}
}

void CPU::_jumpRelativeIfCarryNotSet()
{
	if (!m_getCarryFlag())
		_jumpRelative();
	else
	{
		PC++;
		m_bus->tick();
	}
}

void CPU::_jumpRelativeIfCarrySet()
{
	if (m_getCarryFlag())
		_jumpRelative();
	else
	{
		PC++;
		m_bus->tick();
	}
}

void CPU::_jumpAbsolute()
{
	uint8_t byteLow = m_fetch();
	uint8_t byteHigh = m_fetch();
	uint16_t addr = ((uint16_t)byteHigh << 8) | byteLow;
	PC = addr;
	m_bus->tick();
}

void CPU::_jumpAbsoluteIfZeroNotSet()
{
	if (!m_getZeroFlag())
		_jumpAbsolute();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_jumpAbsoluteIfZeroSet()
{
	if (m_getZeroFlag())
		_jumpAbsolute();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_jumpAbsoluteIfCarryNotSet()
{
	if (!m_getCarryFlag())
		_jumpAbsolute();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_jumpAbsoluteIfCarrySet()
{
	if (m_getCarryFlag())
		_jumpAbsolute();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_call()
{
	uint8_t addrLow = m_fetch();
	uint8_t addrHigh = m_fetch();
	uint16_t addr = (((uint16_t)addrHigh) << 8) | (uint16_t)addrLow;

	m_pushToStack(PC);
	PC = addr;
	m_bus->tick();

}

void CPU::_callIfZeroNotSet()
{
	if (!m_getZeroFlag())
		_call();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_callIfZeroSet()
{
	if (m_getZeroFlag())
		_call();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_callIfCarryNotSet()
{
	if (!m_getCarryFlag())
		_call();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_callIfCarrySet()
{
	if (m_getCarryFlag())
		_call();
	else
	{
		PC += 2;
		m_bus->tick();
		m_bus->tick();
	}
}

void CPU::_return()
{
	uint16_t returnAddress = m_popFromStack();
	PC = returnAddress;
	m_bus->tick();
}

void CPU::_returnIfZeroNotSet()
{
	if (!m_getZeroFlag())
		_return();
	m_bus->tick();
}

void CPU::_returnIfZeroSet()
{
	if (m_getZeroFlag())
		_return();
	m_bus->tick();
}

void CPU::_returnIfCarryNotSet()
{
	if (!m_getCarryFlag())
		_return();
	m_bus->tick();
}

void CPU::_returnIfCarrySet()
{
	if (m_getCarryFlag())
		_return();
	m_bus->tick();
}

void CPU::_returnFromInterrupt()
{
	m_interruptManager->enableInterrupts();
	uint16_t returnAddress = m_popFromStack();
	PC = returnAddress;
	m_bus->tick();
}

void CPU::_setCarryFlag()
{
	m_setCarryFlag(true);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_flipCarryFlag()
{
	m_setCarryFlag(!m_getCarryFlag());
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}



void CPU::_addRegisters(uint8_t& regA, uint8_t& regB)
{
	m_setHalfCarryFlag(((regA & 0x0F) + (regB & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)regA + (int)regB) > 0xFF);
	m_setSubtractFlag(false);

	regA += regB;
	m_setZeroFlag(regA == 0);

}

void CPU::_addPairAddress(uint8_t& regA, Register& regB)
{
	uint8_t val = m_bus->read(regB.reg);
	m_bus->tick();

	m_setHalfCarryFlag(((regA & 0x0F) + (val & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)regA + (int)val) > 0xFF);
	m_setSubtractFlag(false);

	regA += val;
	m_setZeroFlag(regA == 0);

}

void CPU::_addRegistersCarry(uint8_t& regA, uint8_t& regB)
{
	uint8_t lastCarryFlag = m_getCarryFlag();	//save carry flag
	m_setHalfCarryFlag(((regA & 0x0F) + (regB & 0x0F) + (lastCarryFlag & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)regA + (int)regB + (int)lastCarryFlag) > 0xFF);
	m_setSubtractFlag(false);

	regA += regB + lastCarryFlag;
	m_setZeroFlag(regA == 0);
}

void CPU::_addPairAddressCarry(uint8_t& regA, Register& regB)
{
	uint8_t val = m_bus->read(regB.reg);
	m_bus->tick();

	uint8_t lastCarryFlag = m_getCarryFlag();
	m_setHalfCarryFlag(((regA & 0x0F) + (val & 0x0F) + (lastCarryFlag & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)regA + (int)val + (int)lastCarryFlag) > 0xFF);
	m_setSubtractFlag(false);

	regA += val + lastCarryFlag;
	m_setZeroFlag(regA == 0);
}

void CPU::_addValue(uint8_t& reg)
{
	uint8_t val = m_fetch();

	m_setHalfCarryFlag(((reg & 0x0F) + (val & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)reg + (int)val) > 0xFF);
	m_setSubtractFlag(false);

	reg += val;
	m_setZeroFlag(reg == 0);
	
}

void CPU::_addValueCarry(uint8_t& reg)
{
	uint8_t val = m_fetch();
	uint8_t lastCarryFlag = m_getCarryFlag();

	m_setHalfCarryFlag(((reg & 0x0F) + (val & 0x0F) + (lastCarryFlag & 0x0F)) > 0x0F);
	m_setCarryFlag(((int)reg + (int)val + (int)lastCarryFlag) > 0xFF);
	m_setSubtractFlag(false);

	reg += val + lastCarryFlag;
	m_setZeroFlag(reg == 0);

}

void CPU::_subRegister(uint8_t& reg)
{
	m_setCarryFlag(AF.high < reg);
	m_setHalfCarryFlag(((AF.high & 0xf) - (reg & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= reg;
	m_setZeroFlag(AF.high == 0);

}

void CPU::_subRegisterCarry(uint8_t& reg)
{
	uint8_t lastCarryFlag = m_getCarryFlag();
	m_setCarryFlag(AF.high < (reg+lastCarryFlag));
	m_setHalfCarryFlag(((AF.high & 0xf) - (reg & 0xf) - (lastCarryFlag & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= (reg + lastCarryFlag);
	m_setZeroFlag(AF.high == 0);

}

void CPU::_subPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	m_setCarryFlag(AF.high < val);
	m_setHalfCarryFlag(((AF.high & 0xf) - (val & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= val;
	m_setZeroFlag(AF.high == 0);

}

void CPU::_subPairAddressCarry(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t lastCarryFlag = m_getCarryFlag();
	m_setCarryFlag(AF.high < (val+lastCarryFlag));
	m_setHalfCarryFlag(((AF.high & 0xf) - (val & 0xf) - (lastCarryFlag & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= (val + lastCarryFlag);
	m_setZeroFlag(!AF.high);

}

void CPU::_subValue()
{
	uint8_t val = m_fetch();
	m_setCarryFlag(AF.high < val);
	m_setHalfCarryFlag(((AF.high & 0xf) - (val & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= val;
	m_setZeroFlag(AF.high == 0);

}

void CPU::_subValueCarry()
{
	uint8_t val = m_fetch();
	uint8_t lastCarryFlag = m_getCarryFlag();
	m_setCarryFlag(AF.high < (val+lastCarryFlag));
	m_setHalfCarryFlag(((AF.high & 0xf) - (val & 0xf) - (lastCarryFlag & 0xf)) & 0x10);
	m_setSubtractFlag(true);

	AF.high -= (val + lastCarryFlag);
	m_setZeroFlag(AF.high == 0);

}

void CPU::_andRegister(uint8_t& reg)
{
	AF.high &= reg;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);
	m_setCarryFlag(false);
}

void CPU::_andPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	AF.high &= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);
	m_setCarryFlag(false);
}

void CPU::_andValue()
{
	uint8_t val = m_fetch();
	AF.high &= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);
	m_setCarryFlag(false);
}

void CPU::_xorRegister(uint8_t& reg)
{
	AF.high ^= reg;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_xorPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	AF.high ^= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_xorValue()
{
	uint8_t val = m_fetch();
	AF.high ^= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_orRegister(uint8_t& reg)
{
	AF.high |= reg;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_orPairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	AF.high |= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_orValue()
{
	uint8_t val = m_fetch();
	AF.high |= val;
	m_setZeroFlag(!AF.high);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_compareRegister(uint8_t& reg)
{
	m_setHalfCarryFlag((AF.high & 0xF) - (reg & 0xF) & 0x10);
	m_setCarryFlag(AF.high < reg);
	m_setZeroFlag(AF.high - reg == 0);
	m_setSubtractFlag(true);
}

void CPU::_comparePairAddress(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	m_setHalfCarryFlag((AF.high & 0xF) - (val & 0xF) & 0x10);
	m_setCarryFlag(AF.high < val);
	m_setZeroFlag(AF.high - val == 0);
	m_setSubtractFlag(true);
}

void CPU::_compareValue()
{
	uint8_t val = m_fetch();
	m_setHalfCarryFlag((AF.high & 0xF) - (val & 0xF) & 0x10);
	m_setCarryFlag(AF.high < val);
	m_setZeroFlag(AF.high - val == 0);
	m_setSubtractFlag(true);
}

void CPU::_pushPairRegister(Register& reg)
{
	m_bus->tick();	//internal
	m_pushToStack(reg.reg);
}

void CPU::_popToPairRegister(Register& reg)
{
	uint16_t val = m_popFromStack();
	reg.reg = val;
}

//some misc instructions
void CPU::_disableInterrupts()
{
	m_interruptManager->disableInterrupts();
}

void CPU::_enableInterrupts()
{
	m_interruptManager->enableInterrupts();
}

void CPU::_stop()
{
	/*
	uint8_t speedSwitchState = m_bus->read(REG_KEY1);
	if (speedSwitchState & 0b1)
	{
		m_isInDoubleSpeedMode = !m_isInDoubleSpeedMode;
		speedSwitchState ^= 0b10000001;	//flip current mode bit, and clear switch bit
		Logger::getInstance()->msg(LoggerSeverity::Info, "CPU speed switched");
		m_bus->write(REG_KEY1, speedSwitchState);
		m_bus->toggleDoubleSpeedMode();
	}
	else
		Logger::getInstance()->msg(LoggerSeverity::Warn, "STOP instruction should never be called in DMG mode.");
		*/

	Logger::getInstance()->msg(LoggerSeverity::Error, "Invalid instruction STOP hit");
}

void CPU::_halt()
{
	m_halted = true;
}

void CPU::_resetToVector(uint8_t vectorIdx)
{
	m_bus->tick();	//internal cycle
	uint16_t resetAddr = vectorIdx * 8;
	m_pushToStack(PC);
	PC = resetAddr;
}

void CPU::_adjustBCD()
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

void CPU::_loadHLStackIdx()
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

void CPU::_complement()
{
	m_setSubtractFlag(true);
	m_setHalfCarryFlag(true);
	AF.high = ~AF.high;
}

void CPU::_RLA()
{
	uint8_t msb = (AF.high & 0b10000000) >> 7;
	AF.high <<= 1;
	uint8_t m_lastCarry = (m_getCarryFlag()) ? 0b00000001 : 0b0;
	AF.high |= m_lastCarry;
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setZeroFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RLCA()
{
	uint8_t msb = (AF.high & 0b10000000) >> 7;
	AF.high <<= 1;
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setZeroFlag(false);
	m_setHalfCarryFlag(false);
	AF.high |= msb;
}

void CPU::_RRA()
{
	uint8_t lsb = (AF.high & 0b00000001);
	AF.high >>= 1;
	uint8_t m_lastCarry = (m_getCarryFlag()) ? 0b10000000 : 0b0;
	AF.high |= m_lastCarry;
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setZeroFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RRCA()
{
	uint8_t lsb = (AF.high & 0b00000001);
	AF.high >>= 1;
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setZeroFlag(false);
	m_setHalfCarryFlag(false);
	AF.high |= (lsb << 7);
}

void CPU::_RL(uint8_t& reg)
{
	uint8_t msb = (reg & 0b10000000) >> 7;
	uint8_t lastCarry = (m_getCarryFlag()) ? 0b00000001 : 0b0;
	reg <<= 1;
	reg |= lastCarry;
	m_setZeroFlag(!reg);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RLC(uint8_t& reg)
{
	uint8_t msb = (reg & 0b10000000) >> 7;
	reg <<= 1;
	reg |= msb;
	m_setCarryFlag(msb);
	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RL(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	
	uint8_t msb = (val & 0b10000000) >> 7;
	uint8_t lastCarry = (m_getCarryFlag()) ? 0b00000001 : 0b0;
	val <<= 1;
	val |= lastCarry;
	m_setZeroFlag(!val);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_RLC(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t msb = (val & 0b10000000) >> 7;
	val <<= 1;
	val |= msb;
	m_setCarryFlag(msb);
	m_setZeroFlag(!val);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_RR(uint8_t& reg)
{
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	uint8_t m_lastCarry = (m_getCarryFlag()) ? 0b10000000 : 0b0;
	reg |= m_lastCarry;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RRC(uint8_t& reg)
{
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	m_setCarryFlag(lsb);
	reg |= (lsb << 7);
	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_RR(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t lsb = (val & 0b00000001);
	val >>= 1;
	uint8_t m_lastCarry = (m_getCarryFlag()) ? 0b10000000 : 0b0;
	val |= m_lastCarry;
	m_setZeroFlag(!val);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_RRC(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t lsb = (val & 0b00000001);
	val >>= 1;
	m_setCarryFlag(lsb);
	val |= (lsb << 7);
	m_setZeroFlag(!val);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_SLA(uint8_t& reg)
{
	uint8_t msb = (reg & 0b10000000) >> 7;
	reg <<= 1;
	m_setZeroFlag(!reg);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_SLA(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t msb = (val & 0b10000000) >> 7;
	val <<= 1;
	m_setZeroFlag(!val);
	m_setCarryFlag(msb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_SRA(uint8_t& reg)
{
	uint8_t lsb = (reg & 0b00000001);
	uint8_t msb = (reg & 0b10000000);
	reg >>= 1;
	reg |= msb;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_SRA(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	uint8_t lsb = (val & 0b00000001);
	uint8_t msb = (val & 0b10000000);
	val >>= 1;
	val |= msb;
	m_setZeroFlag(!val);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_SRL(uint8_t& reg)
{
	uint8_t lsb = (reg & 0b00000001);
	reg >>= 1;
	m_setZeroFlag(!reg);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
}

void CPU::_SRL(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t lsb = (val & 0b00000001);
	val >>= 1;
	m_setZeroFlag(!val);
	m_setCarryFlag(lsb);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_SWAP(uint8_t& reg)
{
	uint8_t low = (reg & 0x0F) << 4;
	uint8_t high = (reg & 0xF0) >> 4;

	reg = low | high;

	m_setZeroFlag(!reg);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);
}

void CPU::_SWAP(Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();

	uint8_t low = (val & 0x0F) << 4;
	uint8_t high = (val & 0xF0) >> 4;

	val = low | high;

	m_setZeroFlag(!val);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(false);
	m_setCarryFlag(false);

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_BIT(int idx, uint8_t& reg)
{
	uint8_t bitVal = (reg >> idx) & 0b00000001;
	m_setZeroFlag(!bitVal);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);
}

void CPU::_BIT(int idx, Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	uint8_t bitVal = (val >> idx) & 0b00000001;
	m_setZeroFlag(!bitVal);
	m_setSubtractFlag(false);
	m_setHalfCarryFlag(true);
}

void CPU::_RES(int idx, uint8_t& reg)
{
	uint8_t mask = 0b00000001 << idx;
	mask ^= 0b11111111;	//invert all bits
	reg &= mask;

}

void CPU::_RES(int idx, Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	uint8_t mask = 0b00000001 << idx;
	mask ^= 0b11111111;	//invert all bits
	val &= mask;

	m_bus->write(reg.reg, val);
	m_bus->tick();
}

void CPU::_SET(int idx, uint8_t& reg)
{
	uint8_t mask = 0b00000001 << idx;
	reg |= mask;
}

void CPU::_SET(int idx, Register& reg)
{
	uint8_t val = m_bus->read(reg.reg);
	m_bus->tick();
	uint8_t mask = 0b00000001 << idx;
	val |= mask;
	m_bus->write(reg.reg, val);
	m_bus->tick();
}