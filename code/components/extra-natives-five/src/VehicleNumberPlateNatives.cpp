#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <array>
#include <random>

#include <MinHook.h>

static std::string g_customPlatePattern;
static std::map<int, std::string> g_customPlatePatterns;
static int plateBackOffset;

static hook::thiscall_stub<void(void*, char[8])> CCustomShaderEffectVehicle__setNumberPlate([]
{
	return hook::get_pattern("40 32 FF 33 DB 33 C0 48 8B F2 4C", -0x16);
});

static void (*CCustomShaderEffectVehicle__setDefaultPlateBack)(void* self, void* vehicle);
static void (*g_origCCustomShaderEffectVehicle__setDefaultNumberPlate)(void* self, uint32_t seed);

static void CCustomShaderEffectVehicle__setDefaultNumberPlate(char* self, uint32_t seed, void* vehicle)
{
	CCustomShaderEffectVehicle__setDefaultPlateBack(self, vehicle);

	auto cpp = g_customPlatePattern;
	auto plateType = *(int*)(self + plateBackOffset);

	if (auto it = g_customPlatePatterns.find(plateType); it != g_customPlatePatterns.end())
	{
		cpp = it->second;
	}

	if (cpp.empty() || cpp == "11AAA111")
	{
		return g_origCCustomShaderEffectVehicle__setDefaultNumberPlate(self, seed);
	}

	// 8 for Five
	static constexpr const size_t kPlateLength = 8;

	std::array<char, kPlateLength + 1> plateString;
	std::default_random_engine rng{ seed };
	std::uniform_int_distribution<short> number{ '0', '9' };
	std::uniform_int_distribution<short> letter{ 'A', 'Z' };
	std::uniform_int_distribution any{ 1, 2 };

	// fill the plate string with '        \0'
	for (size_t idx = 0; idx < kPlateLength; idx++)
	{
		plateString[idx] = ' ';
	}

	plateString[kPlateLength] = '\0';

	// execute the pattern
	size_t outCur = 0;

	auto emit = [&plateString, &outCur](auto c)
	{
		if (outCur >= kPlateLength)
		{
			return;
		}

		plateString[outCur] = char(c);
		++outCur;
	};

	for (size_t inCur = 0; inCur < cpp.size(); ++inCur)
	{
		char c = cpp[inCur];
		char nextC = ((inCur + 1) < cpp.size()) ? cpp[inCur + 1] : '\0';

		if (c == '1')
		{
			emit(number(rng));
		}
		else if (c == 'A')
		{
			emit(letter(rng));
		}
		else if (c == '.')
		{
			auto which = any(rng);

			if (which == 1)
			{
				emit(letter(rng));
			}
			else
			{
				emit(number(rng));
			}
		}
		// escape code
		else if (c == '^' && (nextC == '1' || nextC == 'A' || nextC == '.'))
		{
			emit(nextC);
			++inCur;
		}
		else
		{
			emit(c);
		}
	}

	return CCustomShaderEffectVehicle__setNumberPlate(self, plateString.data());
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_VEHICLE_NUMBER_PLATE_TEXT_PATTERN", [](fx::ScriptContext& cxt)
	{
		int index = cxt.GetArgument<int>(0);
		const char* newString = cxt.GetArgument<const char*>(1);
		std::string str = (newString) ? newString : "";

		if (index >= 0)
		{
			g_customPlatePatterns[index] = str;
		}
		else
		{
			g_customPlatePattern = str;
		}
	});
});

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			// CVehicle* rsi -> a3
			mov(r8, rsi);

			mov(rax, (uint64_t)CCustomShaderEffectVehicle__setDefaultNumberPlate);
			jmp(rax);
		}
	} setDefaultNumberPlateStub;

	plateBackOffset = *hook::get_pattern<int>("45 33 C9 41 89 93 ? ? ? ? 85 D2", 6);

	// no-op plate back selector so we can do this *before* setting default plate
	{
		auto location = hook::get_pattern("FF 50 ? 48 8B D6 48 8B CB E8 ? ? ? ? 48 8B D6", 9);
		hook::set_call(&CCustomShaderEffectVehicle__setDefaultPlateBack, location);
		hook::nop(location, 5);
	}

	// default plate setter
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8D 4D F0 45 33 C0 E8 ? ? ? ? 8B 45 F4", -0x1A), setDefaultNumberPlateStub.GetCode(), (void**)&g_origCCustomShaderEffectVehicle__setDefaultNumberPlate);
	MH_EnableHook(MH_ALL_HOOKS);
});
