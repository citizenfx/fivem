#pragma once

#include <StdInc.h>

#include <Hooking.h>
#include <PatchUtils.h>

#include <udis86.h>

struct StackResizes
{
	int oldStackSize;
	int newStackSize;
};

inline size_t GetDisplacementOffset(const ud_t& ud, ud_mnemonic_code mnemonic, uint64_t stackSize, uint8_t dataSize)
{
	const uint8_t* instr = ud_insn_ptr(&ud);
	size_t len = ud_insn_len(&ud);

	if (!instr || len == 0 || (dataSize != 1 && dataSize != 2 && dataSize != 4 && dataSize != 8))
	{
		return SIZE_MAX;
	}

	size_t minOffset = 0;

	if (len >= 2)
	{
		uint16_t opcode = (instr[0] << 8) | instr[1];

		// these are Multiple byte opcodes.
		if (opcode >= 0x0F00 && opcode <= 0x0FFF)
		{
			minOffset = 1;
		}
	}

	for (size_t offset = minOffset + 1; offset + dataSize <= len; offset++)
	{
		uint64_t value = 0;
		memcpy(&value, instr + offset, dataSize);

		if (dataSize < 8)
		{
			const uint64_t mask = (1ULL << (dataSize * 8)) - 1;
			value &= mask;
		}

		if (value == stackSize)
		{
			return offset;
		}
	}

	return SIZE_MAX;
}

template<int NewStackSize, int kMaxInstructions = 2048, int kMaxStackResizes = 128>
void IncreaseFunctionStack(void* address, std::initializer_list<StackResizes> list)
{
	auto getImmValue = [](const struct ud_operand* operand, uint8_t* size = nullptr) -> int64_t
	{
		if (!operand)
		{
			return 0;
		}

		uint8_t dataSize = 0;
		if (operand->type == UD_OP_IMM)
		{
			dataSize = operand->size / 8; // Immediate size in bytes
		}
		else if (operand->type == UD_OP_MEM && (operand->offset != 0 && operand->offset <= 64))
		{
			dataSize = operand->offset / 8;
		}
		else
		{
			if (size)
			{
				*size = 0;
			}
			return 0;
		}

		int64_t disp = 0;
		switch (dataSize)
		{
			case 1:
				disp = operand->lval.sbyte;
				break;
			case 2:
				disp = operand->lval.sword;
				break;
			case 4:
				disp = operand->lval.sdword;
				break;
			case 8:
				disp = operand->lval.sqword;
				break;
			default:
				__debugbreak();
				break;
		}

		if (size)
		{
			*size = dataSize;
		}

		return disp;
	};

	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(address));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(address), INT32_MAX);

	int64_t functionStackSize = -1;
	bool attemptStackRelocation = list.size() >= 1;

	struct StackInfo
	{
	public:
		uintptr_t address;
		size_t offset;
		uint64_t origValue;
		uint8_t dataSize;
	};

	StackInfo stackData[kMaxStackResizes]{};
	size_t stackCount = 0;
	const char* analysisFailure = nullptr;

	auto pushStackInfo = [&](uintptr_t addr, size_t offset, uint64_t origValue, uint8_t valueSize)
	{
		if (offset == SIZE_MAX)
		{
			return;
		}

		if (stackCount >= kMaxStackResizes)
		{
			analysisFailure = "more stack references than kMaxStackResizes";
			return;
		}

		stackData[stackCount++] = { addr, offset, origValue, valueSize };
	};

	for (int i = 0; i < kMaxInstructions; i++)
	{
		if (!ud_disassemble(&ud))
		{
			// Nothing else to disassemble
			break;
		}

		uint8_t size;
		uint64_t dataSize;
		ud_mnemonic_code mnemonic = ud_insn_mnemonic(&ud);

		// if this is a retn, we've likely reached the end of the function.
		if (mnemonic == UD_Iret)
		{
			break;
		}

		auto addr = ud_insn_off(&ud);

		const auto* op0 = ud_insn_opr(&ud, 0);
		const auto* op1 = ud_insn_opr(&ud, 1);

		bool validOperands = op0 && op1;

		// Handle sub/add
		if ((mnemonic == UD_Isub || mnemonic == UD_Iadd) && validOperands
			&& op0->type == UD_OP_REG && op0->base == UD_R_RSP && op1->type == UD_OP_IMM)
		{
			// op1 is immediate value
			dataSize = getImmValue(op1, &size);

#ifdef VERBOSE
			trace("Found 0x%llx: %s %s, 0x%llx\n",
			ud_insn_off(&ud),
			ud_lookup_mnemonic(mnemonic),
			(op0->base == UD_R_RSP) ? "RSP" : "RBP",
			dataSize);
#endif

			size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
			pushStackInfo(addr, offset, dataSize, size);

			// if this is restoring the stack pointer, we can currently assume this is the end of the function and break out here
			if (mnemonic == UD_Iadd)
			{
				break;
			}

			functionStackSize = dataSize;
		}

		// If we haven't gotten the stack size of the function. Then skip.
		if (functionStackSize == -1)
		{
			continue;
		}

		// LEA is a special case as the function may be using it to restore rsp
		if (mnemonic == UD_Ilea && validOperands)
		{
			size_t offset = SIZE_MAX;

			// LEA: op0 = reg, op1 = mem
			if (op0->type == UD_OP_REG && op1->type == UD_OP_MEM && op1->base == UD_R_RSP)
			{
				dataSize = getImmValue(op1, &size);
				offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);

#ifdef VERBOSE
				trace("Found 0x%llx LEA %s, [RSP+0x%llx] (size=%zu, offset=%zu)\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				dataSize,
				size,
				offset);
#endif

				if (offset == SIZE_MAX)
				{
					continue;
				}

				// Check if this LEA restores RSP
				if (dataSize == functionStackSize)
				{
					pushStackInfo(addr, offset, dataSize, size);
					break;
				}

				if (attemptStackRelocation)
				{
					pushStackInfo(addr, offset, dataSize, size);
				}

				continue;
			}
		}

		// if we aren't attempting stack relocation, this code isn't needed.
		if (!attemptStackRelocation || !validOperands)
		{
			continue;
		}

		//op0 - register
		//op1 - immediate

		// if op0 is mem
		if (op0->type == UD_OP_MEM)
		{
			// if immediate value
			if (op1->type == UD_OP_IMM && op0->base == UD_R_RSP)
			{
				dataSize = getImmValue(op0, &size);
#ifdef VERBOSE
				trace("Found 0x%llx: %s [%s%+lld]\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				(op0->base == UD_R_RSP) ? "RSP" : "RBP",
				dataSize);
#endif
				size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
				pushStackInfo(addr, offset, dataSize, size);
				continue;
			}

			if (op1->type == UD_OP_REG && op0->base == UD_R_RSP)
			{
				dataSize = getImmValue(op0, &size);

#ifdef VERBOSE
				trace("Found 0x%llx: %s [%s%+lld]\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				(op0->base == UD_R_RSP) ? "RSP" : "RBP",
				getImmValue(op0));
#endif
				size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);

				pushStackInfo(addr, offset, dataSize, size);
				continue;
			}
		}

		if (op0->type == UD_OP_REG && op1->type == UD_OP_MEM && op1->base == UD_R_RSP)
		{
			dataSize = getImmValue(op1, &size);

#ifdef VERBOSE
			trace("Found 0x%llx: %s [%s%+lld]\n",
			ud_insn_off(&ud),
			ud_lookup_mnemonic(mnemonic),
			(op1->base == UD_R_RSP) ? "RSP" : "RBP",
			getImmValue(op1));
#endif
			size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
			pushStackInfo(addr, offset, dataSize, size);
			continue;
		}
	}

	if (analysisFailure)
	{
		trace("IncreaseFunctionStack: %p has %s, leaving it alone.\n", address, analysisFailure);
		return;
	}

	int stackFrameReplaced = 0;
	for (size_t i = 0; i < stackCount; i++)
	{
		const StackInfo& val = stackData[i];

		int newValue = (val.origValue == functionStackSize) ? NewStackSize : -1;
		if (newValue == -1 && attemptStackRelocation)
		{
			for (auto& value : list)
			{
				if (value.oldStackSize == val.origValue)
				{
					newValue = value.newStackSize;
				}
			}
		}

		if (newValue == -1)
		{
			continue;
		}

#ifdef VERBOSE
		trace("Changing value 0x%x (0x%x): 0x%x -> 0x%x\n", val.address + val.offset, val.address, val.origValue, newValue);
#endif

		switch (val.dataSize)
		{
			// 1 byte / 8 bit
			case 1:
				PatchValue<uint8_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 2 bytes / 16 bit
			case 2:
				PatchValue<uint16_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 4 bytes / 32 bit
			case 4:
				PatchValue<uint32_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 8 bytes / 64 bit
			case 8:
				PatchValue<uint64_t>(val.address, val.offset, val.origValue, newValue);
				break;
			default:
				__debugbreak();
				break;
		}

		if (val.origValue == functionStackSize)
		{
			stackFrameReplaced++;
		}
	}

	// Make sure that both the stack frame allocation and stack frame have been replaced.
	assert(stackFrameReplaced == 2);
}
