#pragma once

enum : int
{
	VFS_MAKE_HARDLINK = 0x10005
};

namespace vfs
{
	// in this case, the hard link *existing path* must be pre-checked to be local!
	struct MakeHardLinkExtension
	{
		std::string existingPath;
		std::string newPath;
	};
}
