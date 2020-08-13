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

#include <HandlingLoader.h>

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

// special post-processing helpers
static int getIntField(char* handlingChar, uint32_t offset, const char* fieldName)
{
	auto hash = HashRageString(fieldName);

	if (hash == HashRageString("strModelFlags"))
	{
		return *(int*)(handlingChar + 284);
	}
	else if (hash == HashRageString("strHandlingFlags"))
	{
		return *(int*)(handlingChar + 288);
	}
	else if (hash == HashRageString("strDamageFlags"))
	{
		return *(int*)(handlingChar + 292);
	}
	else if (hash == HashRageString("nInitialDriveGears"))
	{
		return *(uint8_t*)(handlingChar + offset);
	}

	return *(int*)(handlingChar + offset);
}

static float getFloatField(char* handlingChar, uint32_t offset, const char* fieldName)
{
	auto hash = HashRageString(fieldName);

	if (hash == HashRageString("fDriveBiasFront"))
	{
		float fDriveBiasFront = *(float*)(handlingChar + offset);
		float fDriveBiasRear = *(float*)(handlingChar + offset + 4);
		
		if ((fDriveBiasFront == 1.0f && fDriveBiasRear == 0.0f) || (fDriveBiasFront == 0.0f && fDriveBiasRear == 1.0f))
		{
			return fDriveBiasFront;
		}
		else
		{
			return fDriveBiasFront / 2.0f;
		}
	}
	else if (hash == HashRageString("fBrakeBiasFront") || hash == HashRageString("fSuspensionBiasFront") ||
		hash == HashRageString("fTractionBiasFront") || hash == HashRageString("fAntiRollBarBiasFront"))
	{
		return *(float*)(handlingChar + offset) / 2.0f;
	}
	else if (hash == HashRageString("fPercentSubmerged"))
	{
		// empty
	}
	else if (hash == HashRageString("fSteeringLock") || hash == HashRageString("fTractionCurveLateral"))
	{
		return *(float*)(handlingChar + offset) / 0.017453292f; // rad to deg
	}
	else if (hash == HashRageString("fInitialDriveMaxFlatVel"))
	{
		return *(float*)(handlingChar + offset) * 3.6f;
	}
	else if (hash == HashRageString("fInitialDragCoeff"))
	{
		return *(float*)(handlingChar + offset) / 0.0001f;
	}
	else if (hash == HashRageString("fSuspensionReboundDamp") || hash == HashRageString("fSuspensionCompDamp"))
	{
		return *(float*)(handlingChar + offset) / 0.1f;
	}

	return *(float*)(handlingChar + offset);
}

static void setIntField(char* handlingChar, uint32_t offset, int value, const char* fieldName)
{
	auto hash = HashRageString(fieldName);

	if (hash == HashRageString("strModelFlags"))
	{
		*(int*)(handlingChar + 284) = value;
		return;
	}
	else if (hash == HashRageString("strHandlingFlags"))
	{
		*(int*)(handlingChar + 288) = value;
		return;
	}
	else if (hash == HashRageString("strDamageFlags"))
	{
		*(int*)(handlingChar + 292) = value;
		return;
	}
	else if (hash == HashRageString("nInitialDriveGears"))
	{
		*(uint8_t*)(handlingChar + offset) = std::min(value, 7);
		return;
	}

	*(int*)(handlingChar + offset) = value;
}

static void setFloatField(char* handlingChar, uint32_t offset, float value, const char* fieldName)
{
	auto hash = HashRageString(fieldName);

	if (hash == HashRageString("fDriveBiasFront"))
	{
		if (value < 0.1f)
		{
			*(float*)(handlingChar + offset) = 0.0f;
			*(float*)(handlingChar + offset + 4) = 1.0f; // rear
		}
		else if (value > 0.9f)
		{
			*(float*)(handlingChar + offset) = 1.0f;
			*(float*)(handlingChar + offset + 4) = 0.0f; // rear
		}
		else
		{
			*(float*)(handlingChar + offset) = value * 2.0f;
			*(float*)(handlingChar + offset + 4) = (1.0f - value) * 2.0f;
		}

		return;
	}
	else if (hash == HashRageString("fBrakeBiasFront") || hash == HashRageString("fSuspensionBiasFront") ||
		hash == HashRageString("fTractionBiasFront") || hash == HashRageString("fAntiRollBarBiasFront"))
	{
		*(float*)(handlingChar + offset) = value * 2.0f;
		*(float*)(handlingChar + offset + 4) = (1.0f - value) * 2.0f; // rear

		return;
	}
	else if (hash == HashRageString("fPercentSubmerged"))
	{
		*(float*)(handlingChar + 68) = 100.f / value;

		// no return
	}
	else if (hash == HashRageString("fTractionCurveMin") || hash == HashRageString("fTractionCurveMax"))
	{
		// this one depends on both min and max, so set and then use value to recalculate
		*(float*)(handlingChar + offset) = value;

		float fTractionCurveMax = *(float*)(handlingChar + 136);
		float fTractionCurveMin = *(float*)(handlingChar + 144);
		
		if (fTractionCurveMax == 0.0f)
		{
			*(float*)(handlingChar + 140) = 100000000.f;
		}
		else
		{
			*(float*)(handlingChar + 140) = 1.0 / fTractionCurveMax;
		}

		if (fTractionCurveMin > fTractionCurveMax)
		{
			*(float*)(handlingChar + 148) = 100000000.f;
		}
		else
		{
			*(float*)(handlingChar + 148) = 1.0 / (fTractionCurveMax - fTractionCurveMin);
		}

		return;
	}
	else if (hash == HashRageString("fTractionSpringDeltaMax"))
	{
		*(float*)(handlingChar + offset + 4) = 1.0f / value;

		// no return
	}
	else if (hash == HashRageString("fSteeringLock") || hash == HashRageString("fTractionCurveLateral"))
	{
		*(float*)(handlingChar + offset) = value * 0.017453292f; // deg to rad
		*(float*)(handlingChar + offset + 4) = 1.0f / (value * 0.017453292f);

		return;
	}
	else if (hash == HashRageString("fInitialDriveMaxFlatVel"))
	{
		*(float*)(handlingChar + offset) = value / 3.6f;
		*(float*)(handlingChar + offset - 4) = (value / 3.6f) * 1.2f;

		return;
	}
	else if (hash == HashRageString("fInitialDragCoeff"))
	{
		*(float*)(handlingChar + offset) = value * 0.0001f;
		
		return;
	}
	else if (hash == HashRageString("fSuspensionReboundDamp") || hash == HashRageString("fSuspensionCompDamp"))
	{
		*(float*)(handlingChar + offset) = value * 0.1f;

		return;
	}

	*(float*)(handlingChar + offset) = value;
}

atArray<CHandlingData*>* g_handlingData;

static void SetHandlingDataInternal(fx::ScriptContext& context, CHandlingData* handlingData, const char* fromFunction)
{
	const char* handlingClass = context.GetArgument<const char*>(1);
	const char* handlingField = context.GetArgument<const char*>(2);

	auto parserStructure = rage::GetStructureDefinition(handlingClass);

	if (_stricmp(handlingClass, "CHandlingData") == 0)
	{
		uint32_t fieldHash = HashRageString(handlingField);

		auto& members = parserStructure->m_members;
		bool found = false;

		for (rage::parMember* member : members)
		{
			if (member->m_definition->hash == fieldHash)
			{
				char* handlingChar = (char*)handlingData;
				uint32_t offset = member->m_definition->offset;

				switch (member->m_definition->type)
				{
					case rage::parMemberType::Float:
						setFloatField(handlingChar, offset, context.GetArgument<float>(3), handlingField);
						break;

					case rage::parMemberType::UInt8:
						*(uint8_t*)(handlingChar + offset) = uint8_t(context.GetArgument<int>(3));
						break;

					case rage::parMemberType::UInt32:
						setIntField(handlingChar, offset, context.GetArgument<int>(3), handlingField);
						break;

					case rage::parMemberType::String:
						*(const char**)(handlingChar + offset) = strdup(context.GetArgument<const char*>(3));
						break;

					case rage::parMemberType::Vector3_Padded:
					{
						float* vector = (float*)(handlingChar + offset);
						auto source = context.GetArgument<scrVector>(3);

						vector[0] = source.x;
						vector[1] = source.y;
						vector[2] = source.z;

						break;
					}

					default:
						trace("Unsupported field type %d in %s during %s.\n", member->m_definition->type, handlingField, fromFunction);
						break;
				}

				found = true;
				context.SetResult(true);

				break;
			}
		}

		if (!found)
		{
			trace("No such field %s during %s.\n", handlingField, fromFunction);

			context.SetResult(false);
		}
	}
	else
	{
		trace("%s only supports CHandlingData currently\n", fromFunction);

		context.SetResult(false);
	}
}

template<typename T>
void GetVehicleHandling(fx::ScriptContext& context, const char* fromFunction)
{
	int veh = context.GetArgument<int>(0);

	rage::fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(veh);

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

		auto parserStructure = rage::GetStructureDefinition(handlingClass);

		if (_stricmp(handlingClass, "CHandlingData") == 0)
		{
			uint32_t fieldHash = HashRageString(handlingField);

			auto& members = parserStructure->m_members;
			bool found = false;

			for (rage::parMember* member : members)
			{
				if (member->m_definition->hash == fieldHash)
				{
					char* handlingChar = (char*)handlingData;
					uint32_t offset = member->m_definition->offset;

					switch (member->m_definition->type)
					{
						case rage::parMemberType::Float:
							context.SetResult<T>((T)(getFloatField(handlingChar, offset, handlingField)));
							break;

						case rage::parMemberType::UInt8:
							context.SetResult<T>((T)*(uint8_t*)(handlingChar + offset));
							break;

						case rage::parMemberType::UInt32:
							context.SetResult<T>((T)(getIntField(handlingChar, offset, handlingField)));
							break;

						case rage::parMemberType::Vector3_Padded:
						{
							float* vector = (float*)(handlingChar + offset);

							context.SetResult(scrVector{vector[0], vector[1], vector[2]});

							break;
						}

						default:
							trace("Unsupported field type %d in %s during %s.\n", member->m_definition->type, handlingField, fromFunction);
							break;
					}

					found = true;

					break;
				}
			}

			if (!found)
			{
				trace("No such field %s during %s.\n", handlingField, fromFunction);

				context.SetResult(false);
			}
		}
		else
		{
			trace("%s only supports CHandlingData currently\n", fromFunction);

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
	auto handlingFieldFunc = [](fx::ScriptContext& context)
	{
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
	};

	fx::ScriptEngine::RegisterNativeHandler("SET_HANDLING_FIELD", handlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_HANDLING_FLOAT", handlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_HANDLING_INT", handlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_HANDLING_VECTOR", handlingFieldFunc);

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

	auto vehicleHandlingFieldFunc = [](fx::ScriptContext& context)
	{
		int veh = context.GetArgument<int>(0);

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(veh);

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
			trace("no script guid for vehicle %d in SET_VEHICLE_HANDLING_FIELD\n", veh);

			context.SetResult(false);
		}
	};

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HANDLING_FIELD", vehicleHandlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HANDLING_FLOAT", vehicleHandlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HANDLING_INT", vehicleHandlingFieldFunc);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HANDLING_VECTOR", vehicleHandlingFieldFunc);

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ADDRESS", [] (fx::ScriptContext& context)
	{
		context.SetResult(rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0)));
	});
});

static HookFunction hookFunction([] ()
{
	static struct : jitasm::Frontend
	{
		static void SetFieldOnVehicle(CVehicle* vehicle, CHandlingData* handling)
		{
			vehicle->SetHandlingData(new CHandlingData(handling));
		}

		void InternalMain() override
		{
			// save rdx, it's a scratch register
			push(rdx);

			// make scratch space for the function we call
			sub(rsp, 32);

			// rsi is first argument
			mov(rcx, rsi);

			// call the function
			mov(rax, (uint64_t)SetFieldOnVehicle);
			call(rax);

			// remove scratch space
			add(rsp, 32);

			// restore rdx
			pop(rdx);

			// return from the function
			ret();
		}
	} shStub;

	// 505 SPECIFIC
	// 1103 now
	// 1290 now
	// also 1365
	// and 1493
	// 1604 changed 08 to 09
	// 1868-fine
	auto pMatch = hook::pattern("48 89 96 ? 09 00 00").count(1).get(0);

	char* location = pMatch.get<char>(-11);
	g_handlingData = (atArray<CHandlingData*>*)(location + *(int32_t*)location + 4);

	void* setHandlingPointer = pMatch.get<void>();
	hook::nop(setHandlingPointer, 7);
	hook::call(setHandlingPointer, shStub.GetCode());
});
