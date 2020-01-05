/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ConsoleHost.h>

#include <CoreConsole.h>

#include <ICoreGameInit.h>

#include <RGBA.h>

#include <regex>

#ifndef IS_LAUNCHER
#include <nutsnbolts.h>
#endif

#include <se/Security.h>

#include <concurrent_queue.h>

extern ImFont* consoleFontSmall;
extern ImFont* consoleFontTiny;

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
	ImVector<char*> Items;
	ImVector<char*> ItemKeys;

	std::recursive_mutex ItemsMutex;

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

			ItemKeys.push_back(Strdup(key));
			Items.push_back(Strdup(buf));

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
		const char* item = Items[i];
		const char* key = ItemKeys[i];

		if (strlen(key) > 0 && strlen(item) > 0)
		{
			ImGui::PushFont(consoleFontSmall);

			auto hue = int{ HashRageString(key) % 360 };
			auto color = HSLToRGB(HSL{ hue, 0.8f, 0.4f });
			color.alpha = alpha * 255.0f;

			std::string refStr;

			if (fullBg)
			{
				refStr = fmt::sprintf("[%s]:", key);
				key = refStr.c_str();
			}

			auto textSize = ImGui::CalcTextSize(key);

			auto dl = ImGui::GetCurrentContext()->CurrentWindow->DrawList;
			auto startPos = ImGui::GetCursorScreenPos();

			ImVec2 itemSize(0.0, 0.0);

			if (fullBg)
			{
				itemSize = ImGui::CalcTextSize(item);
				itemSize.x += 8.0f;

				color.alpha *= 0.8f;
			}

			dl->AddRectFilled(ImVec2(startPos.x, startPos.y + 1.0f), ImVec2(startPos.x + textSize.x + itemSize.x + 8.0f, startPos.y + 22.f - 1.0f), color.AsARGB(), fullBg ? 10.0f : 6.0f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);

			//ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(key);
			//ImGui::PopStyleColor();

			ImGui::PopFont();

			ImGui::SameLine(textSize.x + 16.0f);
		}

		ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, alpha);

		if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, alpha);
		else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, alpha);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}

	virtual void ClearLog()
	{
		std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

		for (int i = 0; i < Items.Size; i++)
		{
			free(Items[i]);
			free(ItemKeys[i]);
		}

		Items.clear();
		ItemKeys.clear();
	}
};

struct CfxBigConsole : FiveMConsoleBase
{
	char InputBuf[1024];
	bool ScrollToBottom;
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
		// TODO: figure out if scrolled up somehow?
		ScrollToBottom = true;
	}

	void Draw(const char* title, bool* p_open) override
	{
		if (GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState(VK_MENU) & 0x8000)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetItemsLineHeightWithSpacing() * 12.0f), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings;

		if (!ImGui::Begin(title, nullptr, flags))
		{
			ImGui::End();
			ImGui::PopStyleVar();
			return;
		}

		std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

		/*ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
		ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

		// TODO: display items starting from the bottom

		if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
		if (ImGui::SmallButton("Add Dummy Error")) AddLog("[error] something went wrong"); ImGui::SameLine();
		if (ImGui::SmallButton("Clear")) ClearLog(); ImGui::SameLine();
		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;
		//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		static ImGuiTextFilter filter;
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::PopStyleVar();
		ImGui::Separator();*/

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, 0);

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		ImGuiListClipper clipper(Items.Size);
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				DrawItem(i);
			}
		}

		if (ScrollToBottom)
			ImGui::SetScrollHere();

		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		if (ImGui::InputText("_", InputBuf, _countof(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* input_end = InputBuf + strlen(InputBuf);
			while (input_end > InputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
			if (InputBuf[0])
				ExecCommand(InputBuf);
			strcpy(InputBuf, "");
		}

		// Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();

		ImGui::PopStyleVar();
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

	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		CfxBigConsole* console = (CfxBigConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiTextEditCallbackData* data)
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
			ImVector<const char*> candidates;

			console::GetDefaultContext()->GetCommandManager()->ForAllCommands([&](const std::string& command)
			{
				if (Strnicmp(command.c_str(), word_start, (int)(word_end - word_start)) == 0)
				{
					candidates.push_back(Strdup(command.c_str()));
				}
			});

			if (candidates.Size == 0)
			{
				// No match
				AddLog("", "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
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
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("", "Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("", "- %s\n", candidates[i]);
			}

			for (auto& candidate : candidates)
			{
				free(const_cast<char*>(candidate));
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

	void RunCommandQueue()
	{
		std::string command_line;

		while (CommandQueue.try_pop(command_line))
		{
			se::ScopedPrincipal scope(se::Principal{ "system.console" });

			console::GetDefaultContext()->AddToBuffer(command_line);
			console::GetDefaultContext()->AddToBuffer("\n");
			console::GetDefaultContext()->ExecuteBuffer();
		}
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
	ImVector<std::chrono::milliseconds> ItemTimes;

	ConVar<std::string>* m_miniconChannels;
	std::string m_miniconLastValue;
	std::regex m_miniconRegex;

	MiniConsole()
	{
		m_miniconChannels = new ConVar<std::string>("con_miniconChannels", ConVar_Archive, "minicon:*");
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
		ImGui::SetNextWindowPos(ImVec2(20, io.DisplaySize.y - 20), ImGuiCond_Once, ImVec2(0.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.4f, ImGui::GetItemsLineHeightWithSpacing() * 12.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4)); // Tighten spacing

		if (ImGui::Begin("MiniCon", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
		{
			auto t = msec();

			std::unique_lock<std::recursive_mutex> lock(ItemsMutex);

			// clip old entries
			{
				while ((ItemTimes.size() > 1 && (t - ItemTimes[0]) > std::chrono::seconds(10)) || Items.size() > 14)
				{
					free(Items[0]);
					free(ItemKeys[0]);

					Items.erase(Items.begin());
					ItemKeys.erase(ItemKeys.begin());
					ItemTimes.erase(ItemTimes.begin());
				}

				if (ItemTimes.size() == 1 && ((t - ItemTimes[0]) > std::chrono::seconds(10)))
				{
					free(Items[0]);
					free(ItemKeys[0]);

					Items[0] = Strdup("");
					ItemKeys[0] = Strdup("");
					ItemTimes[0] = t;
				}
			}

			ImGuiListClipper clipper(Items.Size);
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

			ImGui::SetScrollHere();
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

static std::unique_ptr<FiveMConsoleBase> g_consoles[2];

static void EnsureConsoles()
{
	if (!g_consoles[0])
	{
		g_consoles[0] = std::make_unique<CfxBigConsole>();
		g_consoles[1] = std::make_unique<MiniConsole>();
	}
}

void DrawConsole()
{
	EnsureConsoles();

	static bool pOpen = true;
	g_consoles[0]->Draw("", &pOpen);
}

void DrawMiniConsole()
{
	EnsureConsoles();

	static bool pOpen = true;
	g_consoles[1]->Draw("", &pOpen);
}

#include <sstream>

void SendPrintMessage(const std::string& channel, const std::string& message)
{
	EnsureConsoles();

	for (auto& console : g_consoles)
	{
		if (console->Items.Size == 0)
		{
			console->AddLog("", "");
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

				auto strRef = FiveMConsoleBase::Strdup((console->Items[console->Items.size() - 1] + str).c_str());
				std::swap(console->Items[console->Items.size() - 1], strRef);

				free(strRef);

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
	if (g_consoles[0])
	{
		((CfxBigConsole*)g_consoles[0].get())->RunCommandQueue();
	}
}

#ifndef IS_LAUNCHER
static InitFunction initFunction([]()
{
	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		if (g_consoles[1] && !*should)
		{
			std::unique_lock<std::recursive_mutex> lock(g_consoles[1]->ItemsMutex);

			*should = *should || ((g_consoles[1]->Items.size() > 1) || (g_consoles[1]->Items[0] && *g_consoles[1]->Items[0]));
		}
	}, 999);

	OnGameFrame.Connect([] { RunConsoleGameFrame(); });
});
#endif
