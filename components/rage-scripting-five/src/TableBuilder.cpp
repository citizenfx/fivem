/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <udis86.h>

class FunctionTable
{
private:
	int m_componentId;

	std::vector<std::pair<uint64_t, void*>> m_functions;

public:
	FunctionTable(int componentId);

	FunctionTable(std::function<void(void*, size_t)> reader);

	inline void AddFunction(uint64_t identifier, void* functionPtr)
	{
		m_functions.push_back(std::make_pair(identifier, functionPtr));
	}

	inline int GetComponentId()
	{
		return m_componentId;
	}

	inline uint64_t GetAt(int index) const
	{
		return m_functions[index].first;
	}

	static void CrossReference(const FunctionTable& oldTable, const FunctionTable& newTable, std::unordered_map<uint64_t, uint64_t>& map);

	void Serialize(std::function<void(const void*, size_t)> writer);
};

FunctionTable::FunctionTable(int componentId)
	: m_componentId(componentId)
{

}

FunctionTable::FunctionTable(std::function<void(void*, size_t)> reader)
{
	reader(&m_componentId, sizeof(m_componentId));

	int numFunctions;
	reader(&numFunctions, sizeof(numFunctions));

	for (int i = 0; i < numFunctions; i++)
	{
		uint64_t functionValue;
		reader(&functionValue, sizeof(functionValue));

		m_functions.push_back(std::make_pair(functionValue, nullptr));
	}
}

void FunctionTable::CrossReference(const FunctionTable& oldTable, const FunctionTable& newTable, std::unordered_map<uint64_t, uint64_t>& map)
{
	for (int i = 0; i < oldTable.m_functions.size(); i++)
	{
		map.insert(std::make_pair(oldTable.GetAt(i), newTable.GetAt(i)));
	}
}

void FunctionTable::Serialize(std::function<void(const void*, size_t)> writer)
{
	writer(&m_componentId, sizeof(m_componentId));

	int numFunctions = m_functions.size();
	writer(&numFunctions, sizeof(numFunctions));

	for (auto& function : m_functions)
	{
		auto functionValue = function.first;

		writer(&functionValue, sizeof(functionValue));
	}
}

class TableGenerator
{
public:
	void GenerateForComponent(void* regFunc, FunctionTable& table);
};

void TableGenerator::GenerateForComponent(void* regFunc, FunctionTable& table)
{
	// initialize udis
	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(regFunc));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(regFunc), INT32_MAX);

	// last r8 value
	void* lastR8 = nullptr;

	// loop through the function
	while (true)
	{
		// disassemble the next instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iret)
		{
			break;
		}

		// if this is a mov rdx, imm64, store it as function
		if (ud_insn_mnemonic(&ud) == UD_Imov)
		{
			auto op = ud_insn_opr(&ud, 0);

			if (op->type == UD_OP_REG)
			{
				if (op->base == UD_R_RDX || op->base == UD_R_RCX)
				{
					op = ud_insn_opr(&ud, 1);

					if (op->type == UD_OP_IMM)
					{
						table.AddFunction(op->lval.uqword, lastR8);
					}
				}
			}
		}
		// else, if it's a function pointer, store the pointer locally
		else if (ud_insn_mnemonic(&ud) == UD_Ilea)
		{
			auto op = ud_insn_opr(&ud, 0);

			if (op->base == UD_R_R8 || op->base == UD_R_RDX)
			{
				op = ud_insn_opr(&ud, 1);

				if (op->type == UD_OP_MEM && op->base == UD_R_RIP)
				{
					// get the relative offset
					void* funcPtr = reinterpret_cast<void*>(ud_insn_len(&ud) + ud_insn_off(&ud) + op->lval.sdword);

					lastR8 = funcPtr;
				}
			}
		}
	}
}

static bool g_needsMapping;
static std::unordered_map<uint64_t, uint64_t> g_mappingTable;

namespace rage
{
	uint64_t MapNative(uint64_t inNative)
	{
		if (!g_needsMapping)
		{
			return inNative;
		}

		if (g_mappingTable.size() == 0)
		{
			FatalError("Attempted to call a game native function, but no mapping tables were generated and this game executable needs mapping!");
		}

		return g_mappingTable[inNative];
	}
}

static TableGenerator tableGen;

static HookFunction hookFunction([] ()
{
	// function tables!
	std::map<int, std::shared_ptr<FunctionTable>> functionTables;

	// find the root function for native registration, and perform matching
	char* regFunc = hook::pattern("48 89 05 ? ? ? ? 88 1D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 8D 4B 04").count(1).get(0).get<char>(13);

	// generate order lookup tables for the two registration functions
	auto coreComponent = std::make_shared<FunctionTable>(-1);
	tableGen.GenerateForComponent(hook::get_call(regFunc), *coreComponent);

	functionTables[-1] = coreComponent;

	// generate order lookup tables for *other* functions
	regFunc += 5;

	char* gameRegFunc = hook::get_call(regFunc);

	// ud stuff!
	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);

	// set pc
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(gameRegFunc));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(gameRegFunc), INT32_MAX);

	int componentId = 0;

	while (true)
	{
		// disassemble the instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iret)
		{
			break;
		}

		// if it's a call
		if (ud_insn_mnemonic(&ud) == UD_Icall)
		{
			auto operand = ud_insn_opr(&ud, 0);

			// get the relative offset
			void* funcPtr = reinterpret_cast<void*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);

			// and register it as some kind of component
			auto table = std::make_shared<FunctionTable>(componentId);
			tableGen.GenerateForComponent(funcPtr, *table);

			functionTables[componentId] = table;

			// increment the component id, too
			componentId++;
		}
	}

	// do mapping things

	// if this is the old style, generate a mapping file
	if (functionTables[0]->GetAt(0) == 0x846AA8E7D55EE5B6)
	{
		g_needsMapping = false;

		FILE* file = _wfopen(MakeRelativeCitPath(L"citizen\\natives_blob.dat").c_str(), L"wb");

		if (!file)
		{
			return;
		}

		auto writer = [=] (const void* data, size_t size)
		{
			fwrite(data, 1, size, file);
		};

		int numTables = functionTables.size();
		writer(&numTables, sizeof(numTables));

		for (auto pair : functionTables)
		{
			pair.second->Serialize(writer);
		}

		fclose(file);
	}
	else
	{
		g_needsMapping = true;

		FILE* file = _wfopen(MakeRelativeCitPath(L"citizen\\natives_blob.dat").c_str(), L"rb");

		if (!file)
		{
			return;
		}

		auto reader = [=] (void* data, size_t size)
		{
			fread(data, 1, size, file);
		};

		int numTables;
		reader(&numTables, sizeof(numTables));

		for (int i = 0; i < numTables; i++)
		{
			auto functionTable = std::make_shared<FunctionTable>(reader);
			auto newTable = functionTables.find(functionTable->GetComponentId());

			if (newTable != functionTables.end())
			{
				FunctionTable::CrossReference(*functionTable, *(newTable->second), g_mappingTable);
			}
		}

		fclose(file);
	}

	__debugbreak();
});