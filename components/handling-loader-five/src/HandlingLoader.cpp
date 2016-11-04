/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ScriptEngine.h>
#include <atArray.h>

#include <Hooking.h>

#include <boost/type_index.hpp>

namespace rage
{
class fwEntity
{
public:
	virtual ~fwEntity() = 0;
	
	virtual bool IsOfType(uint32_t hash) = 0;

	template<typename T>
	bool IsOfType()
	{
		return reinterpret_cast<T*>(this->IsOfType(HashString(boost::typeindex::type_id<T>().pretty_name().substr(6).c_str())));
	}
};

enum class parMemberType : uint8_t
{
	Int = 6,
	Float = 7,
	Lewca = 8,
	String = 0xB,
	Vector3 = 0x14,
};

struct parMember
{
	uint32_t hash;
	char pad[12];
	parMemberType type;
	// some more
};

struct parStructure
{
	uint32_t hash;
	char pad[12];
	void* parserData;
	parMember** members;
	uint32_t* offsets;
};
}

class CHandlingData
{
private:
	char m_pad[8]; // do a 360 + vmt
	uint32_t m_name;
	char m_pad2[1000];

public:
	CHandlingData(CHandlingData* orig)
	{
		memcpy(this, orig, sizeof(*this));
	}

	inline uint32_t GetName()
	{
		return m_name;
	}
};

class CVehicle
{
private:
	char m_pad[0x828];
	CHandlingData* m_handlingData;

public:
	virtual ~CVehicle() = 0;

	inline CHandlingData* GetHandlingData()
	{
		return m_handlingData;
	}

	inline void SetHandlingData(CHandlingData* ptr)
	{
		m_handlingData = ptr;
	}
};

static hook::cdecl_stub<rage::fwEntity*(int handle)> getScriptEntity([] ()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

struct scrVector
{
	float x;

private:
	uint32_t pad0;

public:
	float y;

private:
	uint32_t pad1;

public:
	float z;

private:
	uint32_t pad2;

public:
	inline scrVector()
	{

	}

	inline scrVector(float x, float y, float z)
		: x(x), y(y), z(z)
	{

	}
};

static rage::parStructure* g_parserStructure;
static atArray<CHandlingData*>* g_handlingData;

static void SetHandlingDataInternal(fx::ScriptContext& context, CHandlingData* handlingData, const char* fromFunction)
{
	const char* handlingClass = context.GetArgument<const char*>(1);
	const char* handlingField = context.GetArgument<const char*>(2);

	if (_stricmp(handlingClass, "CHandlingData") == 0)
	{
		uint32_t fieldHash = HashRageString(handlingField);

		rage::parMember** member = g_parserStructure->members;

		while (*member)
		{
			if ((*member)->hash == fieldHash)
			{
				char* handlingChar = (char*)handlingData;
				uint32_t offset = g_parserStructure->offsets[member - g_parserStructure->members];

				switch ((*member)->type)
				{
					case rage::parMemberType::Float:
						*(float*)(handlingChar + offset) = context.GetArgument<float>(3);
						break;

					case rage::parMemberType::Int:
						*(int*)(handlingChar + offset) = context.GetArgument<int>(3);
						break;

					case rage::parMemberType::String:
						*(const char**)(handlingChar + offset) = strdup(context.GetArgument<const char*>(3));
						break;

					case rage::parMemberType::Vector3:
					{
						float* vector = (float*)(handlingChar + offset);
						auto source = context.GetArgument<scrVector>(3);

						vector[0] = source.x;
						vector[1] = source.y;
						vector[2] = source.z;

						break;
					}

					default:
						trace("Unsupported field type %d in %s during %s.\n", (*member)->type, handlingField, fromFunction);
						break;
				}

				context.SetResult(true);

				break;
			}

			++member;
		}

		if (!*member)
		{
			trace("No such field %s during %s.\n", handlingField, fromFunction);

			context.SetResult(false);
		}
	}
	else
	{
		trace("%s only supports HDandlingCata currently\n", fromFunction);

		context.SetResult(false);
	}
}

template<typename T>
void GetVehicleHandling(fx::ScriptContext& context, const char* fromFunction)
{
	int veh = context.GetArgument<int>(0);

	rage::fwEntity* entity = getScriptEntity(veh);

	if (entity)
	{
		if (!entity->IsOfType<CVehicle>())
		{
			trace("%s: this isn't a vehicle!\n", fromFunction);

			context.SetResult(false);

			return;
		}

		CVehicle* vehicle = (CVehicle*)entity;
		CHandlingData* handlingData = vehicle->GetHandlingData();

		const char* handlingClass = context.GetArgument<const char*>(1);
		const char* handlingField = context.GetArgument<const char*>(2);

		if (_stricmp(handlingClass, "CHandlingData") == 0)
		{
			uint32_t fieldHash = HashRageString(handlingField);

			rage::parMember** member = g_parserStructure->members;

			while (*member)
			{
				if ((*member)->hash == fieldHash)
				{
					char* handlingChar = (char*)handlingData;
					uint32_t offset = g_parserStructure->offsets[member - g_parserStructure->members];

					switch ((*member)->type)
					{
						case rage::parMemberType::Float:
							context.SetResult<T>((T)(*(float*)(handlingChar + offset)));
							break;

						case rage::parMemberType::Int:
							context.SetResult<T>((T)(*(int*)(handlingChar + offset)));
							break;

						case rage::parMemberType::Vector3:
						{
							float* vector = (float*)(handlingChar + offset);

							context.SetResult(scrVector{vector[0], vector[1], vector[2]});

							break;
						}

						default:
							trace("Unsupported field type %d in %s during %s.\n", (*member)->type, handlingField, fromFunction);
							break;
					}

					break;
				}

				++member;
			}

			if (!*member)
			{
				trace("No such field %s during %s.\n", handlingField, fromFunction);

				context.SetResult(false);
			}
		}
		else
		{
			trace("%s only supports HDandlingCata currently\n", fromFunction);

			context.SetResult(false);
		}
	}
	else
	{
		trace("no entity for vehicle %d in %s\n", veh, fromFunction);

		context.SetResult(false);
	}
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_HANDLING_FIELD", [] (fx::ScriptContext& context)
	{
		trace("THIS FUNCTION SPONSORED BY Lewca(TM) FAKED EXPENSIVE BRANDS\n");

		const char* handlingName = context.GetArgument<const char*>(0);
		uint32_t nameHash = HashString(handlingName);
		
		for (uint16_t i = 0; i < g_handlingData->GetCount(); i++)
		{
			auto handlingData = g_handlingData->Get(i);

			if (handlingData->GetName() == nameHash)
			{
				SetHandlingDataInternal(context, handlingData, "SET_HANDLING_FIELD");
				return;
			}
		}

		trace("No such handling name %s in SET_HANDLING_FIELD.\n", handlingName);
		context.SetResult(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDLING_FLOAT", [] (fx::ScriptContext& context)
	{
		GetVehicleHandling<float>(context, "GET_VEHICLE_HANDLING_FLOAT");
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDLING_INT", [] (fx::ScriptContext& context)
	{
		GetVehicleHandling<int>(context, "GET_VEHICLE_HANDLING_INT");
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDLING_VECTOR", [] (fx::ScriptContext& context)
	{
		// int is a hack but it doesn't work otherwise?
		GetVehicleHandling<int>(context, "GET_VEHICLE_HANDLING_VECTOR");
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HANDLING_FIELD", [] (fx::ScriptContext& context)
	{
		trace("THIS FUNCTION SPONSORED BY liavaIL(TM) .NET REVERSE ENGINEERING TOOLS\n");

		int veh = context.GetArgument<int>(0);
		
		rage::fwEntity* entity = getScriptEntity(veh);

		if (entity)
		{
			if (!entity->IsOfType<CVehicle>())
			{
				trace("SET_VEHICLE_HANDLING_FIELD: this isn't a vehicle!\n");

				context.SetResult(false);

				return;
			}

			CVehicle* vehicle = (CVehicle*)entity;

			SetHandlingDataInternal(context, vehicle->GetHandlingData(), "SET_VEHICLE_HANDLING_FIELD");
		}
		else
		{
			trace("no scroipt gwid for vehicle %d in SET_VEHICLE_HANDLING_FIELD\n", veh);

			context.SetResult(false);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ADDRESS", [] (fx::ScriptContext& context)
	{
		context.SetResult(getScriptEntity(context.GetArgument<int>(0)));
	});
});

static HookFunction hookFunction([] ()
{
	g_parserStructure = hook::pattern("D1 64 94 0E").count(1).get(0).get<rage::parStructure>(0);

	static struct : jitasm::Frontend
	{
		static void SetFieldOnVehicle(CVehicle* vehicle, CHandlingData* handling)
		{
			vehicle->SetHandlingData(new CHandlingData(handling));
		}

		void InternalMain() override
		{
			// save rdx as it's scratchy and i'm itchy
			push(rdx);

			// make scratch space for the function we call
			sub(rsp, 32);

			// rsi is first argument, like one one'd have with avail and nta
			mov(rcx, rsi);

			// call the function, like you'd call out to someone
			mov(rax, (uint64_t)SetFieldOnVehicle);
			call(rax);

			// scratch got itched, add stack frame
			add(rsp, 32);

			// rdx goes back too
			pop(rdx);

			// bye world
			ret();
		}
	} shStub;

	auto pMatch = hook::pattern("48 89 96 30 08 00 00").count(1).get(0);

	char* location = pMatch.get<char>(-11);
	g_handlingData = (atArray<CHandlingData*>*)(location + *(int32_t*)location + 4);

	void* setHandlingPointer = pMatch.get<void>();
	hook::nop(setHandlingPointer, 7);
	hook::call(setHandlingPointer, shStub.GetCode());
});