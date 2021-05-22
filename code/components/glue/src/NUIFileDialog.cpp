#include <StdInc.h>

#if __has_include(<ConsoleHost.h>) && __has_include(<imgui.h>) && __has_include(<grcTexture.h>)
#include <ConsoleHost.h>
#include <CefOverlay.h>
#include <grcTexture.h>
#include <DrawCommands.h>
#include <nutsnbolts.h>

#include <json.hpp>

using json = nlohmann::json;

#define STB_IMAGE_IMPLEMENTATION
#include "imfd/ImFileDialog.h"

static std::mutex g_pfdMutex;
static std::map<std::string, std::shared_ptr<ifd::FileDialog>> g_pendingFileDialogs;
static std::set<void*> g_deleteTexQueue;

static InitFunction initFunction([]()
{
	nui::OnInvokeNative.Connect([](const wchar_t* type, const wchar_t* arg)
	{
		if (wcscmp(type, L"openFileDialog") == 0)
		{
			if (!nui::HasMainUI())
			{
				return;
			}

			std::unique_lock _(g_pfdMutex);
			if (g_pendingFileDialogs.empty())
			{
				ConHost::SetCursorMode(true);
			}

			auto id = std::make_shared<ifd::FileDialog>();
			id->CreateTexture = [](uint8_t* data, int width, int height, char fmt) -> void*
			{
				if (width > 2048 || height > 2048)
				{
					return nullptr;
				}

				rage::grcTextureReference reference;
				memset(&reference, 0, sizeof(reference));
				reference.width = width;
				reference.height = height;
				reference.depth = 1;
				reference.stride = width * 4;
				reference.format = (fmt) ? 12 : 11;
				reference.pixelData = (uint8_t*)data;

				auto iconPtr = new void*[2];
				iconPtr[0] = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
				iconPtr[1] = nullptr;
				return iconPtr;
			};

			id->DeleteTexture = [](void* tex)
			{
				g_deleteTexQueue.insert(tex);
			};

			id->Open(ToNarrow(arg), "Select a file", "Image files (*.png;*.jpg;*.jpeg;*.bmp){.png,.jpg,.jpeg,.bmp}");

			g_pendingFileDialogs.insert({ ToNarrow(arg), id });
		}
	});

	OnPostFrontendRender.Connect([]()
	{
		auto deleteThat = [](uintptr_t, uintptr_t)
		{
			std::unique_lock _(g_pfdMutex);

			for (auto& entry : g_deleteTexQueue)
			{
				if (entry)
				{
					void** td = (void**)entry;
					delete (rage::grcTexture*)td[0];
					delete[] td;
				}
			}

			g_deleteTexQueue.clear();
		};

		if (IsOnRenderThread())	
		{
			deleteThat(0, 0);
		}
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || !g_pendingFileDialogs.empty();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		std::unique_lock _(g_pfdMutex);

		if (!g_pendingFileDialogs.empty())
		{
			auto nowPending = g_pendingFileDialogs;

			for (auto& [ entryName, id ] : nowPending)
			{
				if (id->IsDone(entryName))
				{
					// has result?
					json result = nullptr;

					if (id->HasResult())
					{
						result = id->GetResult().u8string();
					}

					id->Close();

					nui::PostFrameMessage("mpMenu", json::object({
						{ "type", "fileDialogResult" },
						{ "dialogKey", entryName },
						{ "result", result }
					}).dump());

					// remove dialog
					g_pendingFileDialogs.erase(entryName);

					if (g_pendingFileDialogs.empty())
					{
						ConHost::SetCursorMode(false);
					}
				}
			}
		}
	});
});

#include "imfd/ImFileDialog-inl.h"
#endif
