#pragma once

#include <VFSDevice.h>

#include <map>

namespace fx
{
class ResourceFileDatabase
{
public:
	ResourceFileDatabase();

	virtual ~ResourceFileDatabase() = default;

	// Loads a resource file database from disk.
	bool Load(const std::string& fileName);

	// Saves the current resource file database from disk.
	bool Save(const std::string& fileName);

	// Returns true if the resource files on disk are changed compared to the database.
	bool Check(const std::vector<std::string>& fileList);

	// Snapshots state of resource files in the database.
	void Snapshot(const std::vector<std::string>& fileList);

private:
	struct Entry
	{
		// on Windows, the file ID
		// on POSIX systems, the inode number
		vfs::FileId fileId;

		// the modification time
		std::time_t mtime;

		// the file size
		uint64_t size;

		inline Entry()
			: fileId(), mtime(0), size(-1)
		{

		}

		inline bool operator==(const Entry& other)
		{
			return (fileId == other.fileId && mtime == other.mtime && size == other.size);
		}

		inline bool operator!=(const Entry& other)
		{
			return !(*this == other);
		}
	};

	Entry GetEntry(const std::string& file);

private:
	std::map<std::string, Entry> m_entries;
};
}
