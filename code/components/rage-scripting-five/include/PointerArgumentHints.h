#pragma once

namespace fx::scripting
{

#ifdef COMPILING_RAGE_SCRIPTING_FIVE
DLL_EXPORT
#else
DLL_IMPORT
#endif
const uint32_t* GetNativeTypeInfo(uint64_t hash);

}
