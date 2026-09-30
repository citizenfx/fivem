#pragma once
#include <StdInc.h>
#include <Timecycle.h>

class SelectedTimecycle
{
	rage::tcModifier* m_selection = nullptr;
	std::map<int, rage::tcModData> m_cachedMods;
	std::set<int> m_disabled;

public:
	void RestoreVar(int index);
	void RemoveVar(int index);
	void DisableVar(int index);
	void EnableVar(int index);
	void UpdateVars();
	bool IsVarDisabled(int index);
	rage::tcModData* GetVar(int index);
	std::map<int, rage::tcModData>& GetVars();

	void SetModifier(rage::tcModifier* mod);
	void ResetModifier();
	bool HasModifier();
	rage::tcModifier* GetModifier();
};

class TimecycleEditor
{
public:
	bool m_editorEnabled = false; // hack

private:
	SelectedTimecycle m_selectedModifier;
	std::set<uint32_t> m_searchCache{};

	static constexpr size_t kMaxBufferSize = 128;
	char m_searchNameBuffer[kMaxBufferSize]{};
	char m_searchParamBuffer[kMaxBufferSize]{};
	char m_detailNameBuffer[kMaxBufferSize]{};
	char m_detailFilterBuffer[kMaxBufferSize]{};
	char m_detailDropDownBuffer[kMaxBufferSize]{};

	bool m_applySelectedTimecycle = false;
	tinyxml2::XMLPrinter g_timecycleXmlOutput;

public:
	bool ShouldDraw();
	void Draw();
	void Reset();

private:
	bool ShouldUseSearchCache();
	void RebuildSearchCache();
};

