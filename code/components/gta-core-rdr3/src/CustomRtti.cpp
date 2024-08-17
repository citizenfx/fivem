#include <StdInc.h>

#include <Hooking.h>

#include <string>

#include <CustomRtti.h>


namespace {

std::string GetVtableAddress(void* ptr, bool debugFormat)
{
	if (debugFormat)
	{
		return fmt::sprintf("unknown (vtable %p)", (void*)hook::get_unadjusted(*(uint64_t**)ptr));
	}
	return fmt::sprintf("%016llx", hook::get_unadjusted(*(uint64_t*)ptr));
}

} // namespace

std::string SearchTypeName(void* ptr, bool debugFormat)
{
	// Custom RTTI search is only supported for GTA5. Return vtable address instead.
	return GetVtableAddress(ptr, debugFormat);
}
