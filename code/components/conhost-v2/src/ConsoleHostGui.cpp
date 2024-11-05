/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"

#include <imgui.h>

#define GImGui ImGui::GetCurrentContext()
#include <imgui_internal.h>
#include <Textselect.hpp>
#include <ConsoleHost.h>

#include <CoreConsole.h>

#include <ICoreGameInit.h>

#include <CfxRGBA.h>

#include <regex>

#if __has_include("nutsnbolts.h")
#include <nutsnbolts.h>
#endif

#include <se/Security.h>

#include <boost/circular_buffer.hpp>
#include <concurrent_queue.h>

bool IsNonProduction();

DLL_EXPORT console::Context* g_customConsoleContext;

console::Context* GetConsoleContext()
{
	if (g_customConsoleContext)
	{
		return g_customConsoleContext;
	}

	return console::GetDefaultContext();
}

extern ImFont* consoleFontSmall;

// following 2 functions based on https://www.programmingalgorithms.com/algorithm/hsl-to-rgb/cpp/
static float HueToRGB(float v1, float v2, float vH) {
	if (vH < 0)
		vH += 1;

	if (vH > 1)
		vH -= 1;

	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);

	if ((2 * vH) < 1)
		return v2;

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}

struct HSL
{
	int H;
	float S;
	float L;
};

static CRGBA HSLToRGB(HSL hsl) {
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	if (hsl.S == 0)
	{
		r = g = b = (unsigned char)(hsl.L * 255);
	}
	else
	{
		float v1, v2;
		float hue = (float)hsl.H / 360;

		v2 = (hsl.L < 0.5) ? (hsl.L * (1 + hsl.S)) : ((hsl.L + hsl.S) - (hsl.L * hsl.S));
		v1 = 2 * hsl.L - v2;

		r = (unsigned char)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
		g = (unsigned char)(255 * HueToRGB(v1, v2, hue));
		b = (unsigned char)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
	}

	return CRGBA(r, g, b);
}

struct FiveMConsoleBase
{
	const size_t BufferSize = 2500;
	boost::circular_buffer<std::string> Items{ BufferSize };
	boost::circular_buffer<std::string> ItemKeys{ BufferSize };

	std::recursive_mutex ItemsMutex;

	int ItemsAdded = 0;

	virtual std::string_view GetLineAtIdx(const size_t idx)
	{
		if (idx >= Items.size()) return "";

		return Items[idx];
	}

	virtual size_t GetNumLines()
	{
		return Items.size();
	}

	virtual float GetTextOffset(const size_t idx)
	{
		if (idx >= Items.size()) return 0.0f;

		const std::string& itemKey = ItemKeys[idx];
		if (itemKey.empty()) return 0.0f;

		const float textSize = ImGui::CalcTextSize(itemKey.c_str()).x;
		return ImGui::GetCursorPosX() + textSize + (textSize > 0 ? 16.0f : 0.0f);
	}

	FiveMConsoleBase()
		: textSelect(
			[this](const size_t idx) { return GetLineAtIdx(idx); },
			[this]() { return GetNumLines(); },
			[this](const size_t idx) { return GetTextOffset(idx); }
		) {}

	TextSelect textSelect;

	virtual void RunCommandQueue()
	{
	}

	// Portable helpers
	static int Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

	virtual void AddLog(const char* key, const char* fmt, ...)
	{
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, _countof(buf), fmt, args);
		buf[_countof(buf) - 1] = 0;
		va_end(args);

		{
			std::unique_lock<std::recursive_mutex> lock(ItemsMutex);
			ItemKeys.push_back(key);
			Items.push_back(buf);

			if (Items.size() == BufferSize)
			{
				ItemsAdded++;
			}

			OnAddLog(key, buf);
		}
	}

	virtual bool FilterLog(const std::string& channel)
	{
		return true;
	}

	virtual void OnAddLog(std::string_view key, std::string_view message)
	{

	}

	virtual void UpdateLog(int idx)
	{

	}

	virtual void Draw(const char* title, bool* p_open) = 0;

	virtual void DrawItem(int i, float alpha = 1.0f, bool fullBg = false)
	{
		const auto& item = Items[i];
		auto key = ItemKeys[i];

		if (strlen(key.c_str()) > 0 && strlen(item.c_str()) > 0)
		{
			const auto hue = static_cast<int>(HashRageString(key) % 360);
			auto color = HSLToRGB(HSL{ hue, 0.8f, 0.4f });
			color.alpha = alpha * 255.0f;

			std::string refStr;

			if (fullBg)
			{
				key = fmt::sprintf("[%s]:", key);
			}

			auto textSize = ImGui::CalcTextSize(key.c_str());

			auto dl = ImGui::GetCurrentContext()->CurrentWindow->DrawList;
			auto startPos = ImGui::GetCursorScreenPos();

			ImVec2 itemSize(0.0, 0.0);

			if (fullBg)
			{
				itemSize = ImGui::CalcTextSize(item.c_str());
				itemSize.x += 8.0f;

				color.alpha *= 0.8f;
			}

			dl->AddRectFilled(ImVec2(startPos.x, startPos.y + 1.0f), ImVec2(startPos.x + textSize.x + itemSize.x + 8.0f, startPos.y + textSize.y - 1.0f), color.AsARGB(), fullBg ? 10.0f : 6.0f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);

			//ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(key.c_str());
			//ImGui::PopStyleColor();

			ImGui::SameLine(textSize.x + 16.0f);
		}

		ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, alpha);

		if (strstr(item.c_str(), "[error]"))
			col = ImColor(1.0f, 0.4f, 0.4f, alpha);
		else if (strncmp(item.c_str(), "# ", 2) == 0)
			col = ImColor(1.0f, 0.78f, 0.58f, alpha);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item.c_str());
		ImGui::PopStyleColor();
	}

	virtual void ClearLog()
	{
		std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

		Items.clear();
		ItemKeys.clear();
	}
};

extern float g_menuHeight;

#ifndef IS_FXSERVER
#include <HostSharedData.h>
#include <../launcher/TickCountData.h>

#include <shellapi.h>

static void OpenLogFile()
{
	static TickCountData initTickCount = ([]()
	{
		HostSharedData<TickCountData> initTickCountRef("CFX_SharedTickCount");
		return *initTickCountRef;
	})();

	static fwPlatformString dateStamp = fmt::sprintf(L"%04d-%02d-%02dT%02d%02d%02d", initTickCount.initTime.wYear, initTickCount.initTime.wMonth,
	initTickCount.initTime.wDay, initTickCount.initTime.wHour, initTickCount.initTime.wMinute, initTickCount.initTime.wSecond);

	auto fileName = MakeRelativeCitPath(fmt::sprintf(L"logs/CitizenFX_log_%s.log", dateStamp));

	ShellExecuteW(NULL, L"open", fileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static void SetAutoScroll(bool enabled);

static std::shared_ptr<ConVar<bool>> g_conAutoScroll;

#endif

struct CfxBigConsole : FiveMConsoleBase
{
	char InputBuf[1024];
	bool ScrollToBottom;
	bool AutoScrollEnabled;
	ImVector<char*> History;
	int HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<const char*> Commands;

	concurrency::concurrent_queue<std::string> CommandQueue;

	CfxBigConsole()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("LOADLEVEL");
		Commands.push_back("CONNECT");
		Commands.push_back("QUIT");
		Commands.push_back("NETGRAPH");
		Commands.push_back("STRDBG");

#ifndef IS_FXSERVER
		AutoScrollEnabled = g_conAutoScroll->GetValue();
#else
		AutoScrollEnabled = true;
#endif
	}

	virtual ~CfxBigConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	virtual void ClearLog() override
	{
		FiveMConsoleBase::ClearLog();

		ScrollToBottom = true;
	}

	virtual void OnAddLog(std::string_view key, std::string_view msg) override
	{
		if (AutoScrollEnabled)
		{
			ScrollToBottom = true;
		}
	}

	virtual bool PreStartWindow()
	{
#ifndef IS_FXSERVER
		if (GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState(VK_MENU) & 0x8000)
		{
			return false;
		}
#endif

		return true;
	}

	virtual bool StartWindow(const char* title, bool* p_open)
	{
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + 0, ImGui::GetMainViewport()->Pos.y + g_menuHeight));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x,
#ifndef IS_FXSERVER
										ImGui::GetFrameHeightWithSpacing() * 12.0f
#else
										ImGui::GetIO().DisplaySize.y - g_menuHeight
#endif
								 ),
		ImGuiCond_Always);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4.0f, 3.0f });

		constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		return ImGui::Begin(title, nullptr, flags);
	}

	virtual void EndWindow()
	{
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	void Draw(const char* title, bool* p_open) override
	{
		if (!PreStartWindow())
		{
			return;
		}

		if (!StartWindow(title, p_open))
		{
			EndWindow();
			return;
		}

		std::unique_lock lock(ItemsMutex);

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 8.0f), false);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		ImGuiListClipper clipper(Items.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				DrawItem(i);
			}
		}

		textSelect.update(ItemsAdded);
		ItemsAdded = 0;

		if (ImGui::BeginPopupContextWindow("CopyPopup"))
		{
			ImGui::BeginDisabled(!textSelect.hasSelection());
			if (ImGui::MenuItem("Copy", "Ctrl+C"))
			{
				textSelect.copy();
			}
			ImGui::EndDisabled();

			if (ImGui::MenuItem("Select all", "Ctrl+A"))
			{
				textSelect.selectAll();
			}
			ImGui::EndPopup();
		}

		if (ScrollToBottom)
		{
			ImGui::SetScrollHereY();
		}

		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		if (ImGui::BeginTable("InputTable", 2, ImGuiTableFlags_SizingStretchProp))
		{
			// Set up columns: first column stretches, second column has fixed width
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			// Input field in the first column
			ImGui::PushItemWidth(-FLT_MIN);
			if (ImGui::InputText("##_Input", InputBuf, _countof(InputBuf),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
			{
				char* input_end = InputBuf + strlen(InputBuf);
				while (input_end > InputBuf && input_end[-1] == ' ')
				{
					input_end--;
				}
				*input_end = 0;
				if (InputBuf[0])
				{
					ExecCommand(InputBuf);
				}
				strcpy(InputBuf, "");
			}
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			if (ImGui::IsWindowAppearing() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ||
				(ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) ||
				ImGui::IsKeyPressed(ImGuiKey_Tab))
			{
				ImGui::SetKeyboardFocusHere(-1);
			}

			bool preAutoScrollValue = AutoScrollEnabled;

			// Controls in the second column
			ImGui::Checkbox("Auto scroll", &AutoScrollEnabled);

			if (preAutoScrollValue != AutoScrollEnabled)
			{
#ifndef IS_FXSERVER
				SetAutoScroll(AutoScrollEnabled);
#else
				if (AutoScrollEnabled)
				{
					// Force scroll to bottom on enabling autoscroll
					ScrollToBottom = true;
				}
#endif
			}

#ifndef IS_FXSERVER
			ImGui::SameLine();
			if (ImGui::Button("Open log"))
			{
				OpenLogFile();
			}

			ImGui::CaptureKeyboardFromApp(true);
#endif

			ImGui::EndTable();
		}

		EndWindow();
	}

	void ExecCommand(const char* command_line)
	{
		AddLog("", "> %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}

		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "clear") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "history") == 0)
		{
			for (int i = History.Size >= 10 ? History.Size - 10 : 0; i < History.Size; i++)
				AddLog("history", "%3d: %s\n", i, History[i]);
		}
		else
		{
			CommandQueue.push(command_line);
		}
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		CfxBigConsole* console = (CfxBigConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			std::set<std::string> candidates;

			auto hasAccess = [](const std::string& command)
			{
				if (IsNonProduction())
				{
					return true;
				}

				se::ScopedPrincipalReset reset;
				se::ScopedPrincipal principal{
					se::Principal{
					"system.extConsole" }
				};

				return seCheckPrivilege(fmt::sprintf("command.%s", command));
			};

			GetConsoleContext()->GetCommandManager()->ForAllCommands([&](const std::string& command)
			{
				if (Strnicmp(command.c_str(), word_start, (int)(word_end - word_start)) == 0 && hasAccess(command))
				{
					candidates.insert(command);
				}
			});

			console::GetDefaultContext()->GetCommandManager()->ForAllCommands([&](const std::string& command)
			{
				if (Strnicmp(command.c_str(), word_start, (int)(word_end - word_start)) == 0 && hasAccess(command))
				{
					candidates.insert(command);
				}
			});

			if (candidates.empty())
			{
				// No match
				AddLog("", "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);

				// Scroll to bottom anyway if "Tab" pressed
				ScrollToBottom = true;
			}
			else if (candidates.size() == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates.begin()->c_str());
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					int i = 0;
					for (const auto& candidate : candidates)
					{
						if (i == 0)
							c = toupper(candidate[match_len]);
						else if (c == 0 || c != toupper(candidate[match_len]))
							all_candidates_matches = false;

						i++;

						if (!all_candidates_matches)
						{
							break;
						}
					}
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates.begin()->c_str(), candidates.begin()->c_str() + match_len);
				}

				// List matches
				AddLog("", "Possible matches:\n");
				for (const auto& candidate : candidates)
					AddLog("", "- %s\n", candidate.c_str());

				// Scroll to bottom anyway if "Tab" pressed
				ScrollToBottom = true;
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)_snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
				data->BufDirty = true;
			}
		}
		}
		return 0;
	}

	virtual void RunCommandQueue() override
	{
		std::string command_line;

		while (CommandQueue.try_pop(command_line))
		{
			se::ScopedPrincipal scope(se::Principal{ (IsNonProduction()) ? "system.console" : "system.extConsole" });

			GetConsoleContext()->AddToBuffer(command_line);
			GetConsoleContext()->AddToBuffer("\n");
			GetConsoleContext()->ExecuteBuffer();
		}
	}

	virtual bool FilterLog(const std::string& channel) override
	{
		if (IsNonProduction())
		{
			return true;
		}

		if (channel == "script:game:nui")
		{
			return false;
		}

		if (channel == "cmd" || channel == "IO")
		{
			return true;
		}

		return channel.find("script:") == 0;
	}
};

// WinConsole is a midway point between minicon and the f8 console
struct CfxWinConsole : CfxBigConsole
{
	virtual bool PreStartWindow() override
	{
		return true;
	}

	virtual bool StartWindow(const char* title, bool* p_open) override
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

		ImGui::SetNextWindowSize(ImVec2(800, 480), ImGuiCond_FirstUseEver);

		return ImGui::Begin(title, p_open, flags);
	}

	virtual void EndWindow() override
	{
		ImGui::End();
	}
};

auto msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()
		);
}

#include <boost/algorithm/string.hpp>

struct MiniConsole : CfxBigConsole
{
	boost::circular_buffer<std::chrono::milliseconds> ItemTimes{ 2500 };

	ConVar<std::string>* m_miniconChannels;
	std::string m_miniconLastValue;
	std::regex m_miniconRegex;

	MiniConsole()
	{
		m_miniconChannels = new ConVar<std::string>("con_miniconChannels", ConVar_Archive | ConVar_UserPref, "minicon:*");
		m_miniconLastValue = m_miniconChannels->GetValue();

		try
		{
			m_miniconRegex = MakeRegex(m_miniconLastValue);
		}
		catch (std::exception & e)
		{
			m_miniconLastValue = "minicon:*";
			m_miniconChannels->GetHelper()->SetValue("minicon:*");
		}
	}

	virtual void Draw(const char* title, bool* p_open) override
	{
		auto& io = ImGui::GetIO();

		ImGui::PushFont(consoleFontSmall);

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + 20, ImGui::GetMainViewport()->Pos.y + io.DisplaySize.y - 20), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.4f, ImGui::GetFrameHeightWithSpacing() * 12.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4)); // Tighten spacing

		if (ImGui::Begin("MiniCon", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs))
		{
			auto t = msec();

			std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

			// clip old entries
			{
				while ((ItemTimes.size() > 1 && (t - ItemTimes[0]) > std::chrono::seconds(10)) || Items.size() > 14)
				{
					Items.erase(Items.begin());
					ItemKeys.erase(ItemKeys.begin());
					ItemTimes.erase(ItemTimes.begin());
				}

				if (ItemTimes.size() == 1 && ((t - ItemTimes[0]) > std::chrono::seconds(10)))
				{
					Items[0] = "";
					ItemKeys[0] = "";
					ItemTimes[0] = t;
				}
			}

			ImGuiListClipper clipper(Items.size());
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					auto deltaTime = t - ItemTimes[i];

					float alpha = 1.0f;

					if (deltaTime < std::chrono::milliseconds(100))
					{
						alpha = deltaTime.count() / 100.0f;
					}
					else if (deltaTime > std::chrono::seconds(10) - std::chrono::milliseconds(100))
					{
						alpha = (std::chrono::seconds(10) - deltaTime).count() / 100.0f;
					}

					DrawItem(i, alpha, true);
				}
			}

			ImGui::SetScrollHereY();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::End();

		ImGui::PopFont();
	}

	virtual void ClearLog() override
	{
		FiveMConsoleBase::ClearLog();

		std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

		ItemTimes.clear();
	}

	virtual bool FilterLog(const std::string& channel) override
	{
		std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

		if (m_miniconLastValue != m_miniconChannels->GetValue())
		{
			try
			{
				m_miniconRegex = MakeRegex(m_miniconChannels->GetValue());
				m_miniconLastValue = m_miniconChannels->GetValue();
			}
			catch (std::exception& e)
			{
				m_miniconChannels->GetHelper()->SetValue(m_miniconLastValue);
			}
		}

		return std::regex_match(channel, m_miniconRegex);
	}

	std::regex MakeRegex(const std::string& pattern)
	{
		std::string re = pattern;
		re = std::regex_replace(re, std::regex{"[.^$|()\\[\\]{}?\\\\]"}, "\\$&");

		boost::algorithm::replace_all(re, " ", "|");
		boost::algorithm::replace_all(re, "+", "|");
		boost::algorithm::replace_all(re, "*", ".*");

		return std::regex{ "(?:" + re + ")", std::regex::icase };
	}

	virtual void UpdateLog(int idx) override
	{
		ItemTimes[idx] = msec();
	}

	virtual void OnAddLog(std::string_view key, std::string_view msg) override
	{
		ItemTimes.push_back(
			msec()
		);

		ScrollToBottom = true;
	}
};

static std::unique_ptr<FiveMConsoleBase> g_consoles[3];
static std::recursive_mutex g_consolesMutex;

static void EnsureConsoles()
{
	std::unique_lock _(g_consolesMutex);

	if (!g_consoles[0])
	{
		g_consoles[0] = std::make_unique<CfxBigConsole>();
		g_consoles[1] = std::make_unique<MiniConsole>();
		g_consoles[2] = std::make_unique<CfxWinConsole>();
	}
}

bool IsNonProduction()
{
#if !defined(GTA_FIVE) || defined(_DEBUG)
	return true;
#else
	static ConVar<int> moo("moo", ConVar_None, 0);

	static auto isProd = ([]()
	{
		wchar_t resultPath[1024];
		static std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
		GetPrivateProfileString(L"Game", L"UpdateChannel", L"production", resultPath, std::size(resultPath), fpath.c_str());

		return _wcsicmp(resultPath, L"production") == 0 || _wcsicmp(resultPath, L"prod") == 0;
	})();

	return !isProd || moo.GetValue() == 31337;
#endif
}

void DrawConsole()
{
	EnsureConsoles();

	static bool pOpen = true;
	g_consoles[0]->Draw("##_BigConsole", &pOpen);
}

void DrawMiniConsole()
{
	EnsureConsoles();

	static bool pOpen = true;
	g_consoles[1]->Draw("##_MiniConsole", &pOpen);
}

void DrawWinConsole(bool* pOpen)
{
	EnsureConsoles();

	g_consoles[2]->Draw("WinConsole", pOpen);
}

#ifndef IS_FXSERVER
static void SetAutoScroll(const bool enabled)
{
	std::unique_lock _(g_consolesMutex);

	if (auto* bigConsole = dynamic_cast<CfxBigConsole*>(g_consoles[0].get())) {
		bigConsole->AutoScrollEnabled = enabled;
		if (enabled)
		{
			bigConsole->ScrollToBottom = true;
		}
	}

	if (auto* winConsole = dynamic_cast<CfxBigConsole*>(g_consoles[2].get())) {
		winConsole->AutoScrollEnabled = enabled;
		if (enabled)
		{
			winConsole->ScrollToBottom = true;
		}
	}

	g_conAutoScroll->GetHelper()->SetRawValue(enabled);
}
#endif

#include <sstream>

void SendPrintMessage(const std::string& channel, const std::string& message)
{
	if (channel == "no_console")
	{
		return;
	}

	{
		std::unique_lock _(g_consolesMutex);

		EnsureConsoles();

		for (auto& console : g_consoles)
		{
			if (console->Items.size() == 0)
			{
				console->AddLog("", "");
			}
		}
	}

	static bool wasNewLine;
	std::stringstream msgStream;

	auto flushStream = [&]()
	{
		auto str = msgStream.str();

		for (auto& console : g_consoles)
		{
			if (console->FilterLog(channel))
			{
				std::unique_lock<std::recursive_mutex> lock(console->ItemsMutex);

				auto strRef = (console->Items[console->Items.size() - 1] + str);
				std::swap(console->Items[console->Items.size() - 1], strRef);

				console->UpdateLog(console->Items.size() - 1);
			}
		}

		msgStream.str("");
	};

	for (auto c = 0; c < message.size(); c++)
	{
		char b[2] = { message[c], 0 };

		if (wasNewLine)
		{
			for (auto& console : g_consoles)
			{
				if (console->FilterLog(channel))
				{
					console->AddLog(channel.c_str(), "");
				}
			}

			wasNewLine = false;
		}

		if (b[0] == '\n')
		{
			flushStream();

			wasNewLine = true;

			continue;
		}

		msgStream << std::string_view(b);
	}

	flushStream();
}

DLL_EXPORT void RunConsoleGameFrame()
{
	for (auto& console : g_consoles)
	{
		console->RunCommandQueue();
	}
}

#if __has_include("nutsnbolts.h")
static InitFunction initFunction([]()
{
	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		if (g_consoles[1] && !*should)
		{
			std::unique_lock<std::recursive_mutex> lock(g_consoles[1]->ItemsMutex);

			*should = *should || ((g_consoles[1]->Items.size() > 1) || (!g_consoles[1]->Items[0].empty()));
		}
	}, 999);

	OnCriticalGameFrame.Connect([] { RunConsoleGameFrame(); });
});
#endif

#include "ProductionWhitelist.h"

static InitFunction initFunctionCon([]()
{
#ifndef IS_FXSERVER
	g_conAutoScroll = std::make_shared<ConVar<bool>>("con_autoScroll", ConVar_Archive | ConVar_UserPref, true);
#endif

	console::GetDefaultContext()->GetCommandManager()->AccessDeniedEvent.Connect([](std::string_view commandName)
	{
		if (!IsNonProduction())
		{
			console::Printf("cmd", "Command %s is disabled in production mode. See ^2https://aka.cfx.re/prod-console^7 for further information.\n", commandName);
			return false;
		}

		return true;
	});

	for (auto& command : g_prodCommandsWhitelist)
	{
		seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.extConsole" }, se::Object{ fmt::sprintf("command.%s", command) }, se::AccessType::Allow);
	}
});
