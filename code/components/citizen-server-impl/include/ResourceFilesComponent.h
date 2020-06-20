#pragma once

#include <Resource.h>

namespace fx
{
	class
#ifdef COMPILING_CITIZEN_SERVER_IMPL
		DLL_EXPORT
#endif
		ResourceFilesFilter
	{
	public:
		virtual ~ResourceFilesFilter() = default;

		virtual void Filter(void* data, size_t length) = 0;

		virtual bool ShouldTerminate(std::string* reason) = 0;
	};

	class ResourceFilesComponent : public fwRefCountable, public IAttached<fx::Resource>
	{
	public:
		using TFilesFilterFactory = std::function<std::shared_ptr<ResourceFilesFilter>(const std::string& resource, const std::string& file, const fwRefContainer<fwRefCountable>& context)>;

	public:
		std::map<std::string, std::string> GetFileHashPairs();

		void AddFileToDefaultSet(const std::string& fileName);

		std::string GetDefaultSetName();
		
		virtual std::shared_ptr<ResourceFilesFilter> CreateFilesFilter(const std::string& file, const fwRefContainer<fwRefCountable>& context);

		virtual void SetFilesFilter(const TFilesFilterFactory& filter);

		virtual void AttachToObject(fx::Resource* object) override;

	private:
		std::vector<std::string> GetFilesForSet(const std::string& setName);

		bool BuildResourceSet(const std::string& setName);

		bool ShouldBuildSet(const std::string& setName);

		std::string GetSetFileName(const std::string& setName);

		std::string GetSetDatabaseName(const std::string& setName);

	private:
		fx::Resource* m_resource;

		std::set<std::string> m_additionalFiles;

		std::map<std::string, std::string> m_fileHashPairs;

		TFilesFilterFactory m_filesFilter;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceFilesComponent);
