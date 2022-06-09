#pragma once

namespace fx::scripting
{
enum class ResultType
{
	None,
	Void,
	String,
	Scalar,
	Vector,
};

class
#ifdef COMPILING_RAGE_SCRIPTING_FIVE
DLL_EXPORT
#else
DLL_IMPORT
#endif
PointerArgumentHints
{
public:
	static void CleanNativeResult(uint64_t nativeIdentifier, ResultType resultType, void* resultBuffer);
};
}
