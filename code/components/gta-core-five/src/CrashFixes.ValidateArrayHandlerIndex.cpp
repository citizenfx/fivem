#include "StdInc.h"

#include "Hooking.FlexStruct.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

static uint32_t g_NumArrayElementsOffset;

static void (*g_CIncidentsArrayHandler_ExtractDataForSerialising)(hook::FlexStruct*, uint32_t);

static void CIncidentsArrayHandler_ExtractDataForSerialising(hook::FlexStruct* handler, uint32_t index)
{
	if (index >= static_cast<uint32_t>(handler->Get<uint16_t>(g_NumArrayElementsOffset)))
		return;

	g_CIncidentsArrayHandler_ExtractDataForSerialising(handler, index);
}

static void (*g_COrdersArrayHandler_ExtractDataForSerialising)(hook::FlexStruct*, uint32_t);

static void COrdersArrayHandler_ExtractDataForSerialising(hook::FlexStruct* handler, uint32_t index)
{
	if (index >= static_cast<uint32_t>(handler->Get<uint16_t>(g_NumArrayElementsOffset)))
		return;

	g_COrdersArrayHandler_ExtractDataForSerialising(handler, index);
}

static HookFunction hookFunction([]
{
	g_NumArrayElementsOffset = *hook::get_pattern<uint32_t>("0F B7 81 ? ? ? ? 48 C1 E0 03 C3", 3);

	auto pointerElementHandlers = hook::pattern("48 83 EC 28 48 8B 81 ? ? ? ? 8B D2 48 8B 14 D0 48 85 D2 74 ? 48 81 C1 ? ? ? ? E8").count(2);

	g_CIncidentsArrayHandler_ExtractDataForSerialising = hook::trampoline(pointerElementHandlers.get(0).get<void>(), CIncidentsArrayHandler_ExtractDataForSerialising);
	g_COrdersArrayHandler_ExtractDataForSerialising = hook::trampoline(pointerElementHandlers.get(1).get<void>(), COrdersArrayHandler_ExtractDataForSerialising);
});
