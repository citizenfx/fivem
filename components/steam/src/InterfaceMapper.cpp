/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InterfaceMapper.h"

#include "Hooking.h"

#include <udis86.h>

InterfaceMapper::InterfaceMapper(void* interfacePtr)
	: InterfaceMapperBase(interfacePtr)
{

}

InterfaceMapperBase::InterfaceMapperBase(void* interfacePtr)
	: m_interface(interfacePtr)
{
	if (m_interface)
	{
		UpdateCachedModule();
	}
}

void* InterfaceMapper::LookupMethod(const char* methodName)
{
	void** methodPtr = *(void***)m_interface;
	bool found = false;

	while (IsValidCodePointer(*methodPtr))
	{
		// disassemble the method to find a name
		const char* thisMethodName = GetMethodName(*methodPtr);

		// if it's a valid method name
		if (thisMethodName)
		{
			// if it's our method, break
			if (!_stricmp(thisMethodName, methodName))
			{
				found = true;
				break;
			}

			// and add it to the cache if it's not
			m_methodCache[thisMethodName] = *methodPtr;
		}

		// continue looking
		methodPtr++;
	}

	return (found) ? *methodPtr : nullptr;
}

bool g_isOldSpawnProcess;

static void PatchSteamAMD64Bug(void* methodPtr)
{
	ud_t ud;
	ud_init(&ud);

	// set the correct architecture
#if defined(_M_IX86)
	ud_set_mode(&ud, 32);
#elif defined(_M_AMD64)
	ud_set_mode(&ud, 64);
#endif

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(methodPtr));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(methodPtr), INT32_MAX);

	// counter
	int immMoveCount = 0;

	// loop the instructions
	while (true)
	{
		// disassemble the next instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iret)
		{
			break;
		}

		// offending instruction is the first mov r8d, 8 after the first two mov r8d, 4 instructions
		if (ud_insn_mnemonic(&ud) == UD_Imov)
		{
			// get the target operand
			auto operand = ud_insn_opr(&ud, 0);

			if (operand->type == UD_OP_REG && operand->base == UD_R_R8D)
			{
				// get the source operand
				auto source = ud_insn_opr(&ud, 1);

				if (source->type == UD_OP_IMM)
				{
					if (immMoveCount == 2 && source->lval.sdword == 8)
					{
						hook::putVP<uint32_t>(ud_insn_off(&ud) + 2, 4);

						g_isOldSpawnProcess = true;

						return;
					}

					immMoveCount++;
				}
			}
		}
	}
}

const char* InterfaceMapper::GetMethodName(void* methodPtr)
{
	// output variable
	const char* name = nullptr;

	// initialize the disassembler
	ud_t ud;
	ud_init(&ud);

	// set the correct architecture
#if defined(_M_IX86)
	ud_set_mode(&ud, 32);
#elif defined(_M_AMD64)
	ud_set_mode(&ud, 64);
#endif

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(methodPtr));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(methodPtr), INT32_MAX);

	// loop the instructions
	while (true)
	{
		// disassemble the next instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iret)
		{
			break;
		}
		
#if defined(_M_IX86)
		// if this is a push...
		if (ud_insn_mnemonic(&ud) == UD_Ipush)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 0);

			// if the operand is immediate
			if (operand->type == UD_OP_IMM)
			{
				// and it's 32 bits in width
				if (operand->size == 32)
				{
					// cast it as a char as precaution
					char* operandPtr = reinterpret_cast<char*>(operand->lval.udword);

					// if it's a valid data pointer as well
					if (IsValidDataPointer(operandPtr))
					{
						// it's probably our pointer of interest!
						name = operandPtr;

						break;
					}
				}
			}
		}
#elif defined(_M_AMD64)
		if (ud_insn_mnemonic(&ud) == UD_Ilea)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 1);

			// if the operand is immediate
			if (operand->type == UD_OP_MEM)
			{
				// and relative to the instruction...
				if (operand->base == UD_R_RIP)
				{
					// cast the relative offset as a char
					char* operandPtr = reinterpret_cast<char*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);

					// if it's a valid data pointer as well
					if (IsValidDataPointer(operandPtr))
					{
						// it's probably our pointer of interest!
						name = operandPtr;

						// work around AMD64 steamclient dll bug (passed VAC blob pointer is 8 bytes in CUtlBuffer::Put - this API is clearly meant for in-process usage; should be 4)
						if (!_stricmp(name, "SpawnProcess"))
						{
							PatchSteamAMD64Bug(methodPtr);
						}

						break;
					}
				}
			}
		}
#else
#error Current machine type not supported in InterfaceMapper
#endif
	}

	return name;
}

void* InterfaceMapper::GetMethodByName(const char* methodName)
{
	auto it = m_methodCache.find(methodName);

	// if the method isn't cached
	if (it == m_methodCache.end())
	{
		// look it up and cache it
		void* newMethod = LookupMethod(methodName);

		it = m_methodCache.insert(std::make_pair(methodName, newMethod)).first;
	}

	return it->second;
}

void InterfaceMapperBase::UpdateCachedModule()
{
	// get the module base
	void* interfaceTable = *(void**)m_interface;

	HMODULE moduleBase = nullptr;
	
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)interfaceTable, &moduleBase))
	{
		// convert to a char* for byte offsets
		char* moduleBasePtr = reinterpret_cast<char*>(moduleBase);

		// get the PE header
		PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleBasePtr);
		PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(moduleBasePtr + dosHeader->e_lfanew);

		// get the start of the section table
		auto sectionCount = ntHeader->FileHeader.NumberOfSections;
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(ntHeader + 1);

		// loop through each section, and find the first code section
		for (int i = 0; i < sectionCount; i++)
		{
			// if the section is executable...
			if (sectionHeader[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)
			{
				// ... update the validity data
				m_moduleValidStart = reinterpret_cast<uintptr_t>(moduleBasePtr + sectionHeader[i].VirtualAddress);
				m_moduleValidEnd = reinterpret_cast<uintptr_t>(moduleBasePtr + sectionHeader[i + 1].VirtualAddress);
			}
			// if it's ".rdata"...
			else if (!strcmp(reinterpret_cast<const char*>(sectionHeader[i].Name), ".rdata"))
			{
				// ... this is our data section
				m_moduleValidDataStart = reinterpret_cast<uintptr_t>(moduleBasePtr + sectionHeader[i].VirtualAddress);
				m_moduleValidDataEnd = reinterpret_cast<uintptr_t>(moduleBasePtr + sectionHeader[i + 1].VirtualAddress);
			}
		}

		// store the module pointer
		m_moduleStart = (uintptr_t)moduleBasePtr;
	}
}