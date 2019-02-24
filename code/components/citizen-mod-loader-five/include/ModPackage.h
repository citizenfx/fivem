#pragma once

#include <memory>
#include <string>

#include <om/core.h>

namespace fx
{
class ModPackage
{
public:
	ModPackage(const std::string& path);

	virtual ~ModPackage() = default;

private:
	void ParsePackage(const std::string& path);

public:
	struct Metadata
	{
		std::string name;
		std::string version;
		std::string authorName;
		std::string description;
	};

	struct Content
	{
		struct Entry
		{
			enum class Type
			{
				Add
			};

			Type type;

			// in case of 'Add'
			std::vector<std::string> archiveRoots;
			std::string sourceFile;
			std::string targetFile;
		};

		std::vector<Entry> entries;
	};

	inline const Content& GetContent()
	{
		return m_content;
	}

	inline const std::string& GetRootPath()
	{
		return m_rootPath;
	}

	inline std::string GetGuidString()
	{
		return fmt::sprintf("%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
			m_guid.data1, m_guid.data2, m_guid.data3, m_guid.data4[0], m_guid.data4[1], m_guid.data4[2], m_guid.data4[3], m_guid.data4[4], m_guid.data4[5], m_guid.data4[6], m_guid.data4[7]);
	}

private:
	guid_t m_guid;

	std::string m_rootPath;

	Metadata m_metadata;

	Content m_content;

public:
	static std::shared_ptr<ModPackage> CreateFromZip(const std::string& zipPath);
};
}
