#include <StdInc.h>
#include <CefOverlay.h>

#include <ICoreGameInit.h>

#include <Resource.h>

#include <gameSkeleton.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

using fx::Resource;

static bool frameOn = false;
static bool primedMapLoad = false;

static void InvokeNUIScript(const std::string& eventName, rapidjson::Document& json)
{
	if (!frameOn)
	{
		return;
	}

	json.AddMember("eventName", rapidjson::Value(eventName.c_str(), json.GetAllocator()), json.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	
	if (json.Accept(writer))
	{
		nui::ExecuteRootScript(va("citFrames['loadingScreen'].contentWindow.postMessage(%s, '*');", sb.GetString()));
	}
}

int CountRelevantDataFileEntries();
extern fwEvent<int, const char*> OnReturnDataFileEntry;
extern fwEvent<int, void*, void*> OnInstrumentedFunctionCall;

static InitFunction initFunction([] ()
{
	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([] ()
	{
		frameOn = true;
		nui::CreateFrame("loadingScreen", "nui://game/ui/loadscreen/index.html");
	}, 100);

	rage::OnInitFunctionStart.Connect([] (rage::InitFunctionType type)
	{
		if (type == rage::INIT_AFTER_MAP_LOADED)
		{
			rapidjson::Document doc;
			doc.SetObject();

			InvokeNUIScript("endDataFileEntries", doc);
		}

		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("startInitFunction", doc);
		}
	});

	rage::OnInitFunctionStartOrder.Connect([] (rage::InitFunctionType type, int order, int count)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("order", order, doc.GetAllocator());
		doc.AddMember("count", count, doc.GetAllocator());

		InvokeNUIScript("startInitFunctionOrder", doc);
	});

	rage::OnInitFunctionInvoking.Connect([] (rage::InitFunctionType type, int idx, rage::InitFunctionData& data)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(data.GetName(), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("idx", idx, doc.GetAllocator());

		InvokeNUIScript("initFunctionInvoking", doc);
	});

	rage::OnInitFunctionInvoked.Connect([] (rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());
		doc.AddMember("name", rapidjson::Value(data.GetName(), doc.GetAllocator()), doc.GetAllocator());

		InvokeNUIScript("initFunctionInvoked", doc);
	});

	rage::OnInitFunctionEnd.Connect([] (rage::InitFunctionType type)
	{
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", rapidjson::Value(rage::InitFunctionTypeToString(type), doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("endInitFunction", doc);
		}

		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			primedMapLoad = true;
		}
		else if (type == rage::INIT_SESSION)
		{
			// TODO: replace with proper SHUTDOWN_LOADING_SCREEN
			nui::DestroyFrame("loadingScreen");

			frameOn = false;
		}
	});

	OnInstrumentedFunctionCall.Connect([] (int idx, void* origin, void* target)
	{
        static std::set<int> hadIndices;

		if (primedMapLoad)
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("count", CountRelevantDataFileEntries(), doc.GetAllocator());

			InvokeNUIScript("startDataFileEntries", doc);

			primedMapLoad = false;
		}

        if (hadIndices.find(idx) == hadIndices.end())
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("idx", idx, doc.GetAllocator());

			InvokeNUIScript("performMapLoadFunction", doc);

            hadIndices.insert(idx);
		}
	});

	OnReturnDataFileEntry.Connect([] (int type, const char* name)
	{
		// counting 0, 9 and 185 is odd due to multiple counts :/ (+ entries getting added all the time)

		static std::set<int> entryTypes = { 0, 9, 185, 3, 4, 20, 21, 28, 45, 48, 49, 51, 52, 53, 54, 55, 56, 59, 66, 71, 72, 73, 75, 76, 77, 84, 89, 97, 98, 99, 100, 106, 107, 112, 133, 184 };
		static thread_local std::set<std::pair<int, std::string>> hadEntries;

		if (entryTypes.find(type) != entryTypes.end())
		{
			bool isNew = false;

			if (hadEntries.find({ type, name }) == hadEntries.end())
			{
				hadEntries.insert({ type, name });

				if (type != 0 && type != 9 && type != 185)
				{
					isNew = true;
				}
			}

			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("type", type, doc.GetAllocator());
			doc.AddMember("isNew", isNew, doc.GetAllocator());
			doc.AddMember("name", rapidjson::Value(name, doc.GetAllocator()), doc.GetAllocator());

			InvokeNUIScript("onDataFileEntry", doc);
		}
	});
});