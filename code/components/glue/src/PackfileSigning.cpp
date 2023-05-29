#include <StdInc.h>
#include <VFSRagePackfile7.h>

static bool ValidatePackfile(const std::vector<uint8_t>& headerSignature, const std::vector<uint8_t>& entries)
{
	return false;
}

static InitFunction initFunction([]
{
	vfs::RagePackfile7::SetValidationCallback(ValidatePackfile);
});
