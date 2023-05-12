#pragma once
#include <StdInc.h>
#include <Timecycle.h>

class SelectedTimecycle
{
private:
	rage::tcModifier* m_selection;
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
