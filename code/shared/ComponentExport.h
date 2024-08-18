#pragma once

#ifdef COMPONENT_EXPORT
#undef COMPONENT_EXPORT
#endif

// based on Chromium's //base/component_export.h (https://chromium.googlesource.com/chromium/src/+/db6fbd6a5ce779935f75ce85008c02aeefac47c4/base/component_export.h),
// 
// Copyright 2018 The Chromium Authors
// see Chromium license (https://chromium.googlesource.com/chromium/src/+/db6fbd6a5ce779935f75ce85008c02aeefac47c4/LICENSE)
#define COMPONENT_EXPORT(component) \
	COMPONENT_MACRO_CONDITIONAL_(COMPILING_##component, \
	DLL_EXPORT, \
	DLL_IMPORT)

// Helper for conditional expansion to one of two token strings. If |condition|
// expands to |1| then this macro expands to |consequent|; otherwise it expands
// to |alternate|.
#define COMPONENT_MACRO_CONDITIONAL_(condition, consequent, alternate) \
	COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(                            \
	COMPONENT_MACRO_CONDITIONAL_COMMA_(condition), consequent, alternate)

// Expands to a comma (,) if its first argument expands to |1|. Used in
// conjunction with |COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_()|, as the presence
// or absence of an extra comma can be used to conditionally shift subsequent
// argument positions and thus influence which argument is selected.
#define COMPONENT_MACRO_CONDITIONAL_COMMA_(...) \
	COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(__VA_ARGS__, )
#define COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(x, ...) \
	COMPONENT_MACRO_CONDITIONAL_COMMA_##x##_
#define COMPONENT_MACRO_CONDITIONAL_COMMA_1_ ,

// Helper which simply selects its third argument. Used in conjunction with
// |COMPONENT_MACRO_CONDITIONAL_COMMA_()| above to implement conditional macro
// expansion.
#define COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(...) \
	COMPONENT_MACRO_EXPAND_(                        \
	COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(__VA_ARGS__))
#define COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(a, b, c, ...) c
// Helper to work around MSVC quirkiness wherein a macro expansion like |,|
// within a parameter list will be treated as a single macro argument. This is
// needed to ensure that |COMPONENT_MACRO_CONDITIONAL_COMMA_()| above can expand
// to multiple separate positional arguments in the affirmative case, thus
// eliciting the desired conditional behavior with
// |COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_()|.
#define COMPONENT_MACRO_EXPAND_(x) x
