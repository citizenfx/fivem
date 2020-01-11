/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <CoreConsole.h>

#include <ConsoleHost.h>

#include <imgui.h>

#include <Streaming.h>

#include <CoreConsole.h>

#include <nutsnbolts.h>

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, uint64_t* archetypeUnk)> getArchetype([]()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

/*static ConsoleCommand consoleCmd("make_entity", [](const std::string& name)
{
	uint64_t index;
	fwArchetype* archetype = getArchetype(HashString(name.c_str()), &index);

	fwEntityDef entityDef;
	entityDef.archetypeName = HashString(name.c_str());
	entityDef.guid = 121212;

	entityDef.position[0] = -426.858f;
	entityDef.position[1] = -957.54f;
	entityDef.position[2] = 3.621f;

	entityDef.rotation[0] = 0.0f;
	entityDef.rotation[1] = 0.0f;
	entityDef.rotation[2] = 0.0f;
	entityDef.rotation[3] = 1.0f;

	fwEntity* entity = archetype->CreateEntity();
	entity->SetModelIndex((uint32_t*)&index);
	entity->SetupFromEntityDef(&entityDef, archetype, 0);
	entity->AddToSceneWrap();

	entity->RemoveFromScene();
	delete entity;
});*/

static hook::cdecl_stub<fwEntity*()> getLocalPlayerPed([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 85 C0 0F 84 C3 00 00 00 0F"));
});

static hook::cdecl_stub<void(fwArchetype*)> registerArchetype([]()
{
	return hook::get_pattern("48 8B D9 8A 49 60 80 F9", -11);
});

static std::string modelLoadAction;
static std::string txdLoadAction;
static bool deleteAction;
static bool deleteTxdsAction;

static bool modelChanged;

static fwEntity* currentEntity;

static std::string currentModelFile;
static std::string currentModel;
static uint32_t currentModelHash;

static std::vector<std::string> currentTxds;

struct CTxdRelationship
{
	void* vtbl;
	atArray<char> parent;
	atArray<char> child;
};

static hook::cdecl_stub<void(void*, CTxdRelationship*)> addGtxdRelationship([]()
{
	return hook::get_pattern("74 06 4C 8B 42 18 EB 03 4C", -0x1C);
});

static void AddGtxdRelationship(const std::string& parent, const std::string& child)
{
	CTxdRelationship rel;
	rel.parent.Expand(parent.size() + 1);
	rel.parent.m_count = rel.parent.m_size;
	strcpy(rel.parent.begin(), parent.c_str());

	rel.child.Expand(child.size() + 1);
	rel.child.m_count = rel.child.m_size;
	strcpy(rel.child.begin(), child.c_str());

	addGtxdRelationship(nullptr, &rel);
}

void RecreateModel()
{
	Vector3 newPosition = getLocalPlayerPed()->GetPosition();

	if (currentEntity)
	{
		if (!modelChanged)
		{
			newPosition = currentEntity->GetPosition();
		}

		currentEntity->RemoveFromScene();
		delete currentEntity;

		currentEntity = nullptr;
	}

	if (modelChanged)
	{
		modelChanged = false;
	}

	if (currentModelFile.empty())
	{
		return;
	}

	// get a resource name for the model
	static int uniqifier;

	currentModel = fmt::sprintf("%08x_%d", HashString(currentModelFile.c_str()), ++uniqifier);
	currentModelHash = HashString(currentModel.c_str());

	// register a streaming asset
	uint32_t dri = 0;
	streaming::RegisterRawStreamingFile(&dri, currentModelFile.c_str(), true, (currentModel + ".ydr").c_str(), false);

	if (dri == 0 || dri == -1)
	{
		return;
	}

	std::vector<std::string> txdNames;

	for (auto& txd : currentTxds)
	{
		std::string txdName = fmt::sprintf("%08x_%d", HashString(txd.c_str()), uniqifier);

		uint32_t txi = 0;
		streaming::RegisterRawStreamingFile(&txi, txd.c_str(), true, (txdName + ".ytd").c_str(), false);

		if (txi == 0 || txi == -1)
		{
			return;
		}

		txdNames.push_back(txdName);
	}

	if (!txdNames.empty())
	{
		for (int i = 0; i < (txdNames.size() - 1); i++)
		{
			AddGtxdRelationship(txdNames[i], txdNames[i + 1]);
		}
	}

	// TODO: load the streaming file to compute an AABB

	// register an archetype
	auto archetypeDef = new fwArchetypeDef();
	archetypeDef->lodDist = 299.0f;

	archetypeDef->bbMin[0] = -1500.0f;
	archetypeDef->bbMin[1] = -1500.0f;
	archetypeDef->bbMin[2] = -1500.0f;

	archetypeDef->bbMax[0] = 1500.0f;
	archetypeDef->bbMax[1] = 1500.0f;
	archetypeDef->bbMax[2] = 1500.0f;

	archetypeDef->bsCentre[0] = 0.0f;
	archetypeDef->bsCentre[1] = 0.0f;
	archetypeDef->bsCentre[2] = 0.0f;

	archetypeDef->bsRadius = 4242.0f;

	archetypeDef->name = currentModelHash;
	archetypeDef->textureDictionary = (txdNames.empty()) ? currentModelHash : HashString(txdNames.back().c_str());

	archetypeDef->flags = 0;
	archetypeDef->specialAttribute = 0;

	// assume this is a CBaseModelInfo
	void* miPtr = g_archetypeFactories->Get(1)->GetOrCreate(archetypeDef->name, 1);

	fwArchetype* mi = g_archetypeFactories->Get(1)->Get(archetypeDef->name);

	mi->InitializeFromArchetypeDef(1390, archetypeDef, true);

	// TODO: clean up
	mi->flags &= ~(1 << 31);

	// register the archetype in the streaming module
	registerArchetype(mi);

	// create an entity
	fwEntityDef entityDef;
	entityDef.archetypeName = currentModelHash;
	entityDef.guid = 121212;

	entityDef.position[0] = newPosition.x;
	entityDef.position[1] = newPosition.y;
	entityDef.position[2] = newPosition.z;

	entityDef.rotation[0] = 0.0f;
	entityDef.rotation[1] = 0.0f;
	entityDef.rotation[2] = 0.0f;
	entityDef.rotation[3] = 1.0f;

	entityDef.flags = 0;
	entityDef.lodLevel = 1;

	uint64_t index;
	getArchetype(archetypeDef->name, &index);

	fwEntity* entity = mi->CreateEntity();
	entity->SetModelIndex((uint32_t*)&index);
	entity->SetupFromEntityDef(&entityDef, mi, 0);
	entity->AddToSceneWrap();

	currentEntity = entity;
}

static InitFunction initFunction2([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (!modelLoadAction.empty())
		{
			std::string modelToLoad;
			std::swap(modelLoadAction, modelToLoad);

			currentModelFile = modelToLoad;

			modelChanged = true;
			RecreateModel();
		}

		if (!txdLoadAction.empty())
		{
			std::string txdToLoad;
			std::swap(txdLoadAction, txdToLoad);

			currentTxds.push_back(txdToLoad);

			RecreateModel();
		}

		if (deleteAction)
		{
			deleteAction = false;

			if (currentEntity)
			{
				currentEntity->RemoveFromScene();
				delete currentEntity;

				currentEntity = nullptr;
			}

			currentModelFile = "";
		}

		if (deleteTxdsAction)
		{
			currentTxds.clear();

			RecreateModel();

			deleteTxdsAction = false;
		}

		if (currentEntity)
		{
			/*auto playerCoords = getLocalPlayerPed()->GetPosition();
			auto entityTransform = currentEntity->GetTransform();

			entityTransform._41 = playerCoords.x;
			entityTransform._42 = playerCoords.y + 20.0f;
			entityTransform._43 = playerCoords.z;

			currentEntity->UpdateTransform(entityTransform, true);*/
		}
	});
});

std::string OpenFileBrowser(const std::string& extension, const std::string& fileType);

static InitFunction initFunction([]()
{
	static bool modelViewerEnabled;

	static ConVar<bool> streamingDebugVar("modelviewer", ConVar_Archive, false, &modelViewerEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || modelViewerEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!modelViewerEnabled)
		{
			return;
		}

		static bool open;

		if (ImGui::Begin("Model viewer", &open))
		{
			if (!currentModelFile.empty())
			{
				ImGui::Text("Current model: %s", currentModelFile.substr(currentModelFile.find_last_of('\\')).c_str());
			}

			if (ImGui::Button("Load Drawable"))
			{
				std::thread([]()
				{
					modelLoadAction = OpenFileBrowser("*.ydr", "gtaDrawable");
				}).detach();
			}

			if (ImGui::Button("Load TXD"))
			{
				std::thread([]()
				{
					txdLoadAction = OpenFileBrowser("*.ytd", "rage::pgDictionary<rage::grcTexture>");
				}).detach();
			}

			if (ImGui::Button("Delete"))
			{
				deleteAction = true;
			}

			if (ImGui::Button("Delete TXDs"))
			{
				deleteTxdsAction = true;
			}

			for (auto& txd : currentTxds)
			{
				ImGui::Selectable(txd.c_str());
			}
		}
	});
});

#include <wrl.h>

namespace WRL = Microsoft::WRL;

struct ScopedCoInitialize
{
	template<typename... TArg>
	ScopedCoInitialize(const TArg&&... args) : m_hr(CoInitializeEx(nullptr, args...))
	{
	}

	~ScopedCoInitialize()
	{
		if (SUCCEEDED(m_hr))
		{
			CoUninitialize();
		}
	}

	inline operator bool()
	{
		return (SUCCEEDED(m_hr));
	}

	inline HRESULT GetResult()
	{
		return m_hr;
	}

private:
	HRESULT m_hr;
};

#include <ShlObj.h>

std::string OpenFileBrowser(const std::string& extension, const std::string& fileType)
{
	ScopedCoInitialize coInit(COINIT_APARTMENTTHREADED);

	if (!coInit)
	{
		return "";
	}

	WRL::ComPtr<IFileDialog> fileDialog;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)fileDialog.GetAddressOf());

	if (FAILED(hr))
	{
		return "";
	}

	FILEOPENDIALOGOPTIONS opts;
	fileDialog->GetOptions(&opts);

	opts |= FOS_FORCEFILESYSTEM;

	std::wstring fileTypeWide = ToWide(fileType);
	std::wstring extensionWide = ToWide(extension);

	fileDialog->SetOptions(opts);
	fileDialog->SetTitle(fmt::sprintf(L"Select a %s to display", fileTypeWide).c_str());

	COMDLG_FILTERSPEC filterSpec;
	filterSpec.pszName = fileTypeWide.c_str();
	filterSpec.pszSpec = extensionWide.c_str();

	fileDialog->SetFileTypes(1, &filterSpec);

	hr = fileDialog->Show(nullptr);

	if (FAILED(hr))
	{
		return "";
	}

	WRL::ComPtr<IShellItem> result;
	hr = fileDialog->GetResult(result.GetAddressOf());

	if (!result)
	{
		return "";
	}

	PWSTR resultPath;

	if (FAILED(hr = result->GetDisplayName(SIGDN_FILESYSPATH, &resultPath)))
	{
		return "";
	}

	std::string retval = ToNarrow(resultPath);

	CoTaskMemFree(resultPath);

	return retval;
}
