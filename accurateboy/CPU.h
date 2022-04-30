#pragma once

#include<iostream>
#include<sstream>
#include<format>
#include"Logger.h"
#include"Bus.h"
#include"InterruptManager.h"
#include"dmgRegisters.h"

union Register
{
	uint16_t reg;
	struct
	{
		uint8_t low;
		uint8_t high;
	};
};

class CPU
{
public:
	CPU(std::shared_ptr<Bus>& bus, std::shared_ptr<InterruptManager>& interruptManager);
	~CPU();

	void step();

	uint64_t getCycleCount();

	bool getInDoubleSpeedMode();

private:
	void m_dispatchInterrupt();
	uint8_t m_fetch();

	void m_initIO();

	void m_executeInstruction();		//standard opcodes
	void m_executePrefixedInstruction(); //opcodes prefixed with 0xCB

	void m_set8BitArithmeticFlags(uint8_t opA, uint8_t opB, bool carryIn, bool subtract);
	void m_set8BitLogicalFlags(uint8_t value, bool AND);

	bool m_getZeroFlag();
	void m_setZeroFlag(bool value);

	bool m_getCarryFlag();
	void m_setCarryFlag(bool value);

	bool m_getHalfCarryFlag();
	void m_setHalfCarryFlag(bool value);

	bool m_getSubtractFlag();
	void m_setSubtractFlag(bool value);

	uint8_t m_lastOpcode = 0;

	bool m_EIRequested = false;
	int m_instrSinceEI = 0;

	std::shared_ptr<Bus> m_bus;
	std::shared_ptr<InterruptManager> m_interruptManager;
	uint64_t m_cycleCount = 0;

	Register AF, BC, DE, HL, SP;	//General purpose registers, flags, and stack pointer
	uint16_t PC = 0;		//PC (Program Counter) - can be implemented as single uint16.
	uint16_t m_lastPC = 0;
	bool m_halted = false;
	bool m_isInDoubleSpeedMode = false;

	void m_pushToStack(uint16_t val);	//helper functions for pushing/popping items to/from stack
	uint16_t m_popFromStack();

	//register/condition code decoding
	uint8_t getR8(uint8_t id);
	void setR8(uint8_t id, uint8_t value);

	uint16_t getR16(uint8_t id, int group);
	void setR16(uint8_t id, uint16_t value, int group);

	bool checkConditionsMet(uint8_t conditionCode);

	//opcode encodings
	void _storeSPAtAddress();
	void _STOP();
	void _JRUnconditional();
	void _JRConditional();
	void _loadR16Immediate();
	void _addHLR16();
	void _storeAccum();
	void _loadAccum();
	void _incR16();
	void _decR16();
	void _incR8();
	void _decR8();
	void _ldR8Immediate();
	void _bitwiseOps();
	void _DAA();
	void _halt();
	void _ldR8();
	void _ALUOpsRegister();
	void _performALUOperation(uint8_t op, uint8_t operand);
	void _RETConditional();
	void _storeHiImmediate();
	void _addSPImmediate();
	void _loadHiImmediate();
	void _LDHLSPImmediate();
	void _popR16();
	void _miscStackOps();
	void _JPConditional();
	void _storeHi();
	void _storeAccumDirect();
	void _loadHi();
	void _loadAccumDirect();
	void _miscOpsEIDI();
	void _callConditional();
	void _pushR16();
	void _callImmediate();
	void _ALUOpsImmediate();
	void _reset();

	void _CBShiftsRotates();
	void _CBGetBitComplement();
	void _CBResetBit();
	void _CBSetBit();

	void _RLC();
	void _RRC();
	void _RL();
	void _RR();
	void _SLA();
	void _SRA();
	void _SWAP();
	void _SRL();
};