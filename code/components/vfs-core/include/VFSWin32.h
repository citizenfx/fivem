#pragma once

#ifdef _WIN32
#include <wrl.h>

namespace vfs
{
	class Stream;

#if defined(COMPILING_VFS_CORE)
	DLL_EXPORT
#endif
		Microsoft::WRL::ComPtr<IStream> CreateComStream(fwRefContainer<vfs::Stream> stream);
}
#endif
