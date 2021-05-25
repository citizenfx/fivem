/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ClientEngineMapper.h"

#include "Hooking.h"

#include <udis86.h>

ClientEngineMapper::ClientEngineMapper(void* clientEngine, HSteamPipe steamPipe, HSteamUser steamUser)
	: InterfaceMapperBase(clientEngine), m_pipe(steamPipe), m_user(steamUser)
{
	LookupMethods();
}

typedef void* (__thiscall *ClientMethodType) (void*, HSteamPipe, const char*);
typedef void* (__thiscall *UserMethodType)   (void*, HSteamPipe, HSteamUser, const char*);

void ClientEngineMapper::LookupMethods()
{
	void** methodPtr = *(void***)m_interface;
	bool found = false;

	methodPtr += 7;

	while (IsValidCodePointer(*methodPtr))
	{
		// disassemble the method to find a name
		bool isUser;
		bool isInterface = IsMethodAnInterface(*methodPtr, &isUser);

		// if it's a valid interface
		if (isInterface)
		{
			void* interfaceRef;

			// call the interface pointer, hoping for an interface
			if (isUser)
			{
				interfaceRef = ((UserMethodType)*methodPtr)(m_interface, m_pipe, m_user, "CLIENTUSER_INTERFACE_VERSION001");
			}
			else
			{
				interfaceRef = ((ClientMethodType)*methodPtr)(m_interface, m_pipe, "CLIENTSHORTCUTS_INTERFACE_VERSION001");
			}

			if (interfaceRef)
			{
				struct RttiLocator
				{
					int signature;
					int offset;
					int cdOffset;
					uint32_t pTypeDescriptor;
				};

				uintptr_t* vtbl = *(uintptr_t**)interfaceRef;
				vtbl--;

				RttiLocator* locator = (RttiLocator*)(*vtbl);
				char* typeDescriptorPtr;

				if (locator->signature == 0)
				{
					typeDescriptorPtr = (char*)(locator->pTypeDescriptor);
				}
				else
				{
					typeDescriptorPtr = (char*)(GetModuleStart() + locator->pTypeDescriptor);
				}

				typeDescriptorPtr += sizeof(uintptr_t) * 2;

				m_interfaces[typeDescriptorPtr] = interfaceRef;
			}
		}
		
		// continue looking
		methodPtr++;
	}
}

bool ClientEngineMapper::IsMethodAnInterface(void* methodPtr, bool* isUser, bool child)
{
	// 2021-05 Steam removes strings entirely from user interfaces
	if (!child && hook::range_pattern((uintptr_t)methodPtr, (uintptr_t)methodPtr + 128, "42 3B 74 11 10 4A 8D 14 11 7C").count_hint(1).size() > 0)
	{
		*isUser = true;
		return true;
	}

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

	bool hadUnwantedCall = false;
	bool hadExternCall = false;
	bool hadMov = false;

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(methodPtr));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(methodPtr), INT32_MAX);
	
	// matching callback
	auto matchPtr = [&] (const char* operandPtr)
	{
		if (_strnicmp(operandPtr, "Assertion Failed:", 17) == 0)
		{
			if (!child)
			{
				if (strstr(operandPtr, "m_map"))
				{
					if (strstr(operandPtr, "mapClient"))
					{
						*isUser = false;
					}
					else
					{
						*isUser = true;
					}

					return true;
				}
			}
			else
			{
				if (strstr(operandPtr, "m_LessFunc"))
				{
					*isUser = true;
					return true;
				}
			}
		}

		return false;
	};

	std::map<int, int> offCounts;

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

		// to catch functions that are simply mov+jmp
		if (ud_insn_mnemonic(&ud) == UD_Ijmp &&
			ud_insn_off(&ud) < ((uint64_t)methodPtr + 16))
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
						if (matchPtr(operandPtr))
						{
							return true;
						}
					}
				}
			}
		}
#elif defined(_M_AMD64)
		if (ud_insn_mnemonic(&ud) == UD_Ilea)
		{
			// get the second operand
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
						if (matchPtr(operandPtr))
						{
							return true;
						}
					}
				}
			}
		}
		else if (!child && ud_insn_mnemonic(&ud) == UD_Icall)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 0);

			bool isWantedCall = false;

			// if the operand is immediate
			if (operand->type == UD_OP_JIMM)
			{
				// cast the relative offset as a char
				char* operandPtr = reinterpret_cast<char*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);

				// if it's a valid data pointer as well
				if (IsValidCodePointer(operandPtr))
				{
					// it's probably our pointer of interest!
					if (IsMethodAnInterface(operandPtr, isUser, true) && hadExternCall)
					{
						return true;
					}

					hadExternCall = false;
				}
			}
			else if (operand->type == UD_OP_MEM)
			{
				if (operand->base == UD_R_RIP)
				{
					char* operandPtr = reinterpret_cast<char*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);

					if (*(char**)operandPtr == (char*)GetProcAddress(GetModuleHandleW(L"tier0_s64.dll"), "?Lock@CThreadMutex@@QEAAXXZ"))
					{
						hadExternCall = true;
						isWantedCall = true;
					}
				}
				else
				{
					// 2019-05 Steam seems to refactor this again, now user functions will do `call qword ptr [rdi+0F8h]` twice and don't have any nested normal CALL
					if (hadExternCall)
					{
						offCounts[operand->lval.sdword]++;

						if (offCounts[operand->lval.sdword] == 2)
						{
							*isUser = true;
							return true;
						}
					}
				}
			}

			if (!isWantedCall)
			{
				hadUnwantedCall = true;
			}
		}
		// and another 2019-05 update breaks yet another thing
		else if (!child && ud_insn_mnemonic(&ud) == UD_Icmp)
		{
			auto operand1 = ud_insn_opr(&ud, 0);
			auto operand = ud_insn_opr(&ud, 1);

			if (operand1->type == UD_OP_REG && operand->type == UD_OP_IMM && operand->lval.udword == 0xFF && hadExternCall && hadMov && !hadUnwantedCall)
			{
				*isUser = true;
				return true;
			}
		}
		else if (!child && ud_insn_mnemonic(&ud) == UD_Imov)
		{
			auto operand = ud_insn_opr(&ud, 1);

			if (operand->type == UD_OP_REG && operand->base == UD_R_R8D)
			{
				hadMov = true;
			}
		}
#else
#error Current machine type not supported in InterfaceMapper
#endif
	}

	return false;
}

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

void* ClientEngineMapper::GetInterface(const std::string& interfaceName)
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	auto it = m_interfaces.find(interfaceName);

	if (it != m_interfaces.end())
	{
		return it->second;
	}

	if (interfaceName.find("_VERSION") != std::string::npos)
	{
		std::string interfaceBit = interfaceName.substr(0, interfaceName.find_first_of('_')) + "Map";

		for (auto& interfacePair : m_interfaces)
		{
			if (StrStrIA(interfacePair.first.c_str(), interfaceBit.c_str()))
			{
				void* interfacePtr = interfacePair.second;

				m_interfaces[interfaceName] = interfacePtr;

				return interfacePtr;
			}
		}
	}

	return nullptr;
}
