#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>

#include "nutsnbolts.h"
#include "ResourceCallbackComponent.h"
#include "console/Console.Base.h"

constexpr int KEYS_COUNT = 256;

using keysData = unsigned char[2][KEYS_COUNT];

int* ioKeyboardActive = nullptr;
keysData* ioKeyboardKeys = nullptr;

inline int ioKeyboard_KeyDown(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive][key];
}

inline int ioKeyboard_KeyUp(int key)
{
	return !ioKeyboard_KeyDown(key);
}

inline int ioKeyboard_KeyChanged(int key)
{
	return (*ioKeyboardKeys)[0][key] ^ (*ioKeyboardKeys)[1][key];
}

inline int ioKeyboard_KeyPressed(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive][key] & ioKeyboard_KeyChanged(key);
}

inline int ioKeyboard_KeyReleased(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive ^ 1][key] & ioKeyboard_KeyChanged(key);
}

// List of raw keys to count as disabled until the next frame
static std::set<int> disabledKeys{};

template<bool HandleDisabled = true>
static bool IsRawKeyInvalidOrDisabled(int key)
{
	if constexpr (HandleDisabled)
	{
		if (disabledKeys.find(key) != disabledKeys.end())
		{
			// the  key should be disabled
			return true;
		}
	}

	if (key >= 0 && key < KEYS_COUNT)
	{
		// the keys valid, we shouldn't disable it
		return false;
	}

	// :( out of bounds
	return true;
}

template<bool HandleDisabled = true, auto fn>
static void IsRawKeyWrapper(fx::ScriptContext& context)
{
	auto rawKeyIndex = context.GetArgument<uint32_t>(0);

	if (!IsRawKeyInvalidOrDisabled<HandleDisabled>(rawKeyIndex))
	{
		context.SetResult<bool>(fn(rawKeyIndex) != 0);
	}
	else
	{
		context.SetResult<bool>(false);
	}
}

using OptionalRef = std::optional<fx::FunctionRef>;

class RawKeymap
{
public:
	// The keymap name, this should be unique as we will use this to 
	// remap the keyIndex during regular play
	std::string m_keymapName;
	// the resource that the keymap should be bound to
	std::string m_resource;
	// the ref to the key down, this is optional
	OptionalRef m_keyDownRef;
	// the ref to the key up, this is optional
	OptionalRef m_keyUpRef;
	// the key index that will be used for this keymap
	uint8_t m_keyIndex;
	// if the keymap can be disabled when DisableRawKeyThisFrame is called
	bool m_canBeDisabled : 1;
	// if the keymap was being held down before, used so we don't ignore key ups
	// if the key was disabled while being pressed
	bool m_wasTriggered : 1;

	RawKeymap(const std::string& keymap, const std::string& resourceName, OptionalRef& keyDown, OptionalRef& keyUp, uint8_t keyIndex, bool canBeDisabled):
		m_keymapName(keymap),
		m_resource(resourceName),
		m_keyDownRef(std::move(keyDown)),
		m_keyUpRef(std::move(keyUp)),
		m_keyIndex(keyIndex),
		m_canBeDisabled(canBeDisabled)
	{
	}
};

// TODO: eastl::vector_map working here
class RawKeymapContainer
{
	std::map<uint8_t, std::map<std::string, std::shared_ptr<RawKeymap>>> m_rawKeymaps;
	// cross-map to easily find the index
	std::map<std::string, uint8_t> m_usedKeys {};
public:
	RawKeymapContainer()
	{
		for (uint8_t key = 0; key < (KEYS_COUNT - 1); ++key)
		{
			m_rawKeymaps[key] = std::map<std::string, std::shared_ptr<RawKeymap>>();
		}
	}

	void AddKeymap(const std::shared_ptr<RawKeymap>& keymap)
	{
		if (m_usedKeys.find(keymap->m_keymapName) != m_usedKeys.end())
		{
			console::PrintError(keymap->m_resource, "%s already has a keymap bound by another resource\n", keymap->m_keymapName.c_str());
			return;
		}

		m_usedKeys[keymap->m_keymapName] = keymap->m_keyIndex;

		auto& vector = m_rawKeymaps[keymap->m_keyIndex];
		vector[keymap->m_keymapName] = keymap;
	}

	void ChangeKeymapIndex(const std::string& keyName, uint8_t newKeyIndex)
	{
		const auto it = m_usedKeys.find(keyName);
		if (it != m_usedKeys.end())
		{
			auto oldIndex = it->second;
			auto& oldKeymap = m_rawKeymaps[oldIndex];

			// We shouldn't have to do a find here because our m_usedKeys should always be up to date.
			auto keyData = oldKeymap[keyName];
			oldKeymap.erase(keyName);

			auto& newKeymap = m_rawKeymaps[newKeyIndex];
			newKeymap[keyName] = keyData;

			// update our used keys to the latest
			m_usedKeys[keyName] = newKeyIndex;
		}
	}

	void RemoveKeymap(const std::shared_ptr<RawKeymap>& keymap)
	{
		m_usedKeys.erase(keymap->m_keymapName);
		auto& container = m_rawKeymaps[keymap->m_keyIndex];
		container.erase(keymap->m_keymapName);
	}

	void Update()
	{
		const auto rm = fx::ResourceManager::GetCurrent();
		for (auto& [key, keymaps] : m_rawKeymaps)
		{
			if (keymaps.empty())
			{
				continue;
			}

			bool wasPressed = ioKeyboard_KeyPressed(key);
			bool wasReleased = ioKeyboard_KeyReleased(key);

			if (wasPressed || wasReleased)
			{
				bool isDisabled = disabledKeys.find(key) != disabledKeys.end();
				for (auto& [_, keymap] : keymaps)
				{
					if (isDisabled && keymap->m_canBeDisabled && !keymap->m_wasTriggered)
					{
						continue;
					}

					if (wasPressed && !keymap->m_wasTriggered)
					{
						keymap->m_wasTriggered = true;

						if (keymap->m_keyDownRef)
						{
							rm->CallReference<void>(keymap->m_keyDownRef->GetRef());
						}
					}

					if (wasReleased && keymap->m_wasTriggered)
					{
						keymap->m_wasTriggered = false;

						if (keymap->m_keyUpRef)
						{
							rm->CallReference<void>(keymap->m_keyUpRef->GetRef());
						}
					}
				}
			}
		}
		
	}
};

static HookFunction initFunction([]()
{
	static RawKeymapContainer rawKeymaps;
#ifdef IS_RDR3
	uint8_t* location = hook::get_pattern<uint8_t>("48 63 05 ? ? ? ? 4C 8D 35 ? ? ? ? 48 83 F0 ? B9");
	ioKeyboardActive = hook::get_address<int*>(location, 3, 7);
	ioKeyboardKeys = hook::get_address<keysData*>(location + 7, 3, 7);
#else
	ioKeyboardActive = hook::get_address<int*>(hook::get_pattern("8B 2D ? ? ? ? 48 8B 03"), 2, 6);
	ioKeyboardKeys = hook::get_address<keysData*>(hook::get_pattern("48 8D 2D ? ? ? ? 49 C1 E6"), 3, 7);
#endif

	// reset the disabled keys every frame
	OnGameFrame.Connect([&]
	{

		rawKeymaps.Update();

		disabledKeys.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_PRESSED", IsRawKeyWrapper<true, &ioKeyboard_KeyPressed>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_RELEASED", IsRawKeyWrapper<true, &ioKeyboard_KeyReleased>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_DOWN", IsRawKeyWrapper<true, &ioKeyboard_KeyDown>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_UP", IsRawKeyWrapper<true, &ioKeyboard_KeyUp>);

	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_PRESSED", IsRawKeyWrapper<false, &ioKeyboard_KeyPressed>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_RELEASED", IsRawKeyWrapper<false, &ioKeyboard_KeyReleased>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_DOWN", IsRawKeyWrapper<false, &ioKeyboard_KeyDown>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_UP", IsRawKeyWrapper<false, &ioKeyboard_KeyUp>);

	fx::ScriptEngine::RegisterNativeHandler("DISABLE_RAW_KEY_THIS_FRAME", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		// We only want the bounds check here, we don't care if its already disabled
		if (!IsRawKeyInvalidOrDisabled<false>(rawKeyIndex))
		{
			disabledKeys.insert(rawKeyIndex);
		}
	});


	fx::ScriptEngine::RegisterNativeHandler("REMAP_RAW_KEYMAP", [](fx::ScriptContext& context)
	{
		auto name = std::string { context.CheckArgument<const char*>(0) };
		auto rawKeyIndex = context.GetArgument<uint32_t>(1);
		if (IsRawKeyInvalidOrDisabled<false>(rawKeyIndex))
		{
			return;
		}

		rawKeymaps.ChangeKeymapIndex(name, rawKeyIndex);
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_RAW_KEYMAP", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			return;
		}

		auto name = std::string { context.CheckArgument<const char*>(0) };

		auto onKeyDown = context.GetArgument<const char*>(1);
		auto onKeyUp = context.GetArgument<const char*>(2);

		OptionalRef onKeyDownRef = onKeyDown ? std::optional(fx::FunctionRef { onKeyDown }) : std::nullopt;
		OptionalRef onKeyUpRef = onKeyUp ? std::optional(fx::FunctionRef { onKeyUp }) : std::nullopt;

		auto index = context.GetArgument<uint32_t>(3);
		auto resourceName = resource->GetName();

		if (IsRawKeyInvalidOrDisabled<false>(index))
		{
			console::PrintError(resourceName, "Keymap index %d was not valid, please refer to the native documentation for the max and minimum key codes\n", index);
			return;
		}

		auto canBeDisabled = context.GetArgument<bool>(4);


		auto keyData = std::make_shared<RawKeymap>(name, resourceName, onKeyDownRef, onKeyUpRef, index, canBeDisabled);

		auto weakKey = std::weak_ptr(keyData);

		rawKeymaps.AddKeymap(keyData);

		resource->OnStop.Connect([keyData = std::move(weakKey)]()
		{
			if (auto key = keyData.lock())
			{
				rawKeymaps.RemoveKeymap(key);
			}
		});
	});

});
