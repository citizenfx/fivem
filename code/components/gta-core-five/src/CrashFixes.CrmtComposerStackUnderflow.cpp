#include "StdInc.h"
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]()
{
	// crmtComposer::Visit (kNodeBlendN / kNodeMergeN / kNodeAddN case)
	// Root cause:
	// The child counting logic uses an incorrect boolean condition:
	//     if (!child->IsDisabled() || !child->IsSilent())
	//
	// This incorrectly counts children that are silent, even though silent
	// nodes do NOT push a frame onto the composer stack during traversal.
	//
	// As a result, the computed 'count' may exceed the actual number of
	// frames pushed onto m_FrameStack. When PushPair() is later executed,
	// it assumes at least two frames are available and reads:
	//     m_FrameStack[m_TopFrame - 1]
	//
	// If the stack is imbalanced, m_TopFrame can reach 0, causing an
	// underflow access (m_FrameStack[-1]).
	
	hook::put<char>(hook::get_pattern("74 ? 38 5A ? 75 ? 41 8B 00"), 0x75);
});
