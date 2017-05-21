#pragma once

#include <Resource.h>

namespace fx
{
	class ResourceFilesComponent : public fwRefCountable, public IAttached<fx::Resource>
	{
	public:
		std::map<std::string, std::string> GetFileHashPairs();

		void AddFileToDefaultSet(const std::string& fileName);

		std::string GetDefaultSetName();

		virtual void AttachToObject(fx::Resource* object) override;

	private:
		std::vector<std::string> GetFilesForSet(const std::string& setName);

		bool BuildResourceSet(const std::string& setName);

		bool ShouldBuildSet(const std::string& setName);

		std::string GetSetFileName(const std::string& setName);

	private:
		fx::Resource* m_resource;

		std::set<std::string> m_additionalFiles;

		std::map<std::string, std::string> m_fileHashPairs;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceFilesComponent);
