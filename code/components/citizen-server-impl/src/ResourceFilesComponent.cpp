#include "StdInc.h"
#include <ResourceFilesComponent.h>
#include <ResourceMetaDataComponent.h>

#include <VFSManager.h>

#include <SHA1.h>

#include <any>

class MarkedWriter
{
public:
	inline MarkedWriter(fwRefContainer<vfs::Stream> stream)
	{
		m_stream = stream;
	}

	inline ~MarkedWriter()
	{
		assert(m_marks.empty());
	}

	template<typename T>
	void Write(const T& value)
	{
		static_assert(std::is_pod_v<T>, "Type isn't POD!");

		m_stream->Write(&value, sizeof(value));
	}

	template<typename T>
	void WriteMark(const std::string& mark, const T& value)
	{
		assert(m_marks.find(mark) != m_marks.end());

		size_t curPos = Tell();

		m_stream->Seek(m_marks[mark], SEEK_SET);

		Write(value);

		m_stream->Seek(curPos, SEEK_SET);

		m_marks.erase(mark);
	}

	inline void Write(const std::vector<uint8_t>& data)
	{
		m_stream->Write(data);
	}

	inline void Align(size_t alignment)
	{
		size_t pos = Tell();
		size_t difference = ((pos % alignment) == 0) ? 0 : (alignment - (pos % alignment));

		Write(std::vector<uint8_t>(difference));
	}

	inline void Mark(const std::string& key)
	{
		m_marks[key] = Tell();
	}

	inline void PermaMark(const std::string& key)
	{
		m_permaMarks[key] = Tell();
	}

	inline size_t GetPositionSince(const std::string& key)
	{
		return (Tell() - m_permaMarks[key]);
	}

	inline size_t Tell()
	{
		return m_stream->Seek(0, SEEK_CUR);
	}

	std::any userState;

private:
	std::map<std::string, size_t> m_marks;
	std::map<std::string, size_t> m_permaMarks;

	fwRefContainer<vfs::Stream> m_stream;
};

namespace fi
{
	class PackfileBuilder
	{
	private:
		class Entry
		{
		public:
			inline Entry(const std::string& name, bool isDirectory)
			{
				m_name = name;
				m_isDirectory = isDirectory;
			}

			inline Entry(const std::string& name, const std::string& file)
			{
				m_name = name;
				m_isDirectory = false;
				m_backingFile = file;
			}

			inline void SetBackingFile(const std::string& file)
			{
				m_backingFile = file;
			}

			inline void AddFile(const std::string& fileName, const std::string& backingFile)
			{
				// check if the file is readable
				{
					auto backingStream = vfs::OpenRead(backingFile);

					if (!backingStream.GetRef())
					{
						return;
					}
				}

				// continue
				std::string_view sv = fileName;
				int pos = sv.find_first_of("/\\");

				if (pos == std::string::npos)
				{
					auto entry = std::make_shared<Entry>(fileName, backingFile);
					entry->m_fullName = fileName;

					m_subEntries.push_back(entry);
				}
				else
				{
					auto directory = FindDirectory(sv, 0);

					if (directory)
					{
						auto entry = std::make_shared<Entry>(
							fileName.substr(fileName.find_last_of("/\\") + 1),
							backingFile
						);

						entry->m_fullName = fileName;

						directory->m_subEntries.push_back(entry);
					}
				}
			}

			inline void Write(MarkedWriter& writer)
			{
				writer.Mark("nOff_" + m_fullName);
				writer.Write(0);

				if (!m_isDirectory)
				{
					writer.Mark("fLen_" + m_fullName);
					writer.Write(0);

					writer.Mark("fOff_" + m_fullName);
					writer.Write(0);

					writer.Mark("fLen2_" + m_fullName);
					writer.Write(0);
				}
				else
				{
					writer.Write<uint32_t>(m_subEntries.size());

					writer.Mark("cIdx_" + m_fullName);
					writer.Write(0);

					writer.Write<uint32_t>(m_subEntries.size());
				}

				writer.userState = std::any_cast<int>(writer.userState) + 1;
			}

			inline void WriteSubEntries(MarkedWriter& writer)
			{
				if (m_isDirectory)
				{
					writer.WriteMark("cIdx_" + m_fullName, std::any_cast<int>(writer.userState) | 0x80000000);
				}

				auto subEntries = m_subEntries;
				std::sort(subEntries.begin(), subEntries.end(), [](const auto& left, const auto& right)
				{
					return (left->m_name < right->m_name);
				});

				for (auto& entry : subEntries)
				{
					entry->Write(writer);
				}

				for (auto& entry : subEntries)
				{
					entry->WriteSubEntries(writer);
				}
			}

			inline void WriteNames(MarkedWriter& writer)
			{
				if (!m_name.empty())
				{
					writer.WriteMark<uint32_t>("nOff_" + m_fullName, writer.GetPositionSince("baseName"));
					writer.Write(std::vector<uint8_t>(m_name.begin(), m_name.end()));
					writer.Write<uint8_t>(0);
				}
				else
				{
					writer.PermaMark("baseName");

					writer.WriteMark("nOff_", 0);
					writer.Write<uint8_t>('/');
					writer.Write<uint8_t>(0);
				}

				auto subEntries = m_subEntries;
				std::sort(subEntries.begin(), subEntries.end(), [](const auto& left, const auto& right)
				{
					return (left->m_name < right->m_name);
				});

				for (auto& entry : subEntries)
				{
					entry->WriteNames(writer);
				}
			}

			inline void WriteFiles(MarkedWriter& writer)
			{
				if (!m_isDirectory)
				{
					writer.WriteMark<uint32_t>("fOff_" + m_fullName, writer.Tell());

					auto backingStream = vfs::OpenRead(m_backingFile);

					writer.WriteMark<uint32_t>("fLen_" + m_fullName, backingStream->GetLength());
					writer.WriteMark<uint32_t>("fLen2_" + m_fullName, backingStream->GetLength());

					// TODO(fxserver): optimize
					writer.Write(backingStream->ReadToEnd());
					writer.Align(2048);
				}

				for (auto& entry : m_subEntries)
				{
					entry->WriteFiles(writer);
				}
			}

		private:
			inline std::shared_ptr<Entry> FindDirectory(const std::string_view& sv, int pos)
			{
				int newPos = sv.find_first_of("/\\", pos);
				int initialPos = newPos;

				// skip blanks
				if (newPos != std::string::npos)
				{
					if (sv[newPos + 1] == '/' || sv[newPos + 1] == '\\')
					{
						newPos = sv.find_first_of("/\\", newPos + 1);
					}
				}

				std::shared_ptr<Entry> entry;

				for (const auto& thisEntry : m_subEntries)
				{
					if (thisEntry->m_name == sv.substr(pos, initialPos - pos))
					{
						entry = thisEntry;
						break;
					}
				}

				if (entry)
				{
					if (!entry->m_isDirectory)
					{
						return nullptr;
					}
				}
				else
				{
					entry = std::make_shared<Entry>(std::string(sv.substr(pos, initialPos - pos)), true);
					entry->m_fullName = sv.substr(0, initialPos);

					m_subEntries.emplace_back(entry);
				}

				if (sv.find_first_of("/\\", newPos + 1) != std::string::npos)
				{
					entry = entry->FindDirectory(sv, newPos + 1);
				}

				return entry;
			}

		private:
			std::string m_name;
			bool m_isDirectory;

			std::vector<std::shared_ptr<Entry>> m_subEntries;

			std::string m_backingFile;

			std::string m_fullName;
		};

	public:
		inline PackfileBuilder()
		{
			m_rootEntry = std::make_shared<Entry>("", true);
		}

		inline void AddFile(const std::string& fileName, const std::string& backing)
		{
			m_rootEntry->AddFile(fileName, backing);
		}

		inline bool Write(const std::string& outFileName)
		{
			fwRefContainer<vfs::Device> device = vfs::GetDevice(outFileName);
			device->CreateDirectory(outFileName.substr(0, outFileName.find_last_of('/')));

			auto handle = device->Create(outFileName);

			if (handle == INVALID_DEVICE_HANDLE)
			{
				return false;
			}

			fwRefContainer<vfs::Stream> stream(new vfs::Stream(device, handle));

			MarkedWriter writer(stream);
			writer.userState = 0;

			writer.Write(0x32465052);

			writer.Mark("tocSize");
			writer.Write(0);

			writer.Mark("numEntries");
			writer.Write(0);

			writer.Write(0);
			writer.Write(0);

			writer.Align(2048);

			m_rootEntry->Write(writer);
			m_rootEntry->WriteSubEntries(writer);
			m_rootEntry->WriteNames(writer);

			writer.WriteMark("numEntries", std::any_cast<int>(writer.userState));

			writer.Align(2048);

			writer.WriteMark<int>("tocSize", stream->Seek(0, SEEK_CUR));

			m_rootEntry->WriteFiles(writer);

			return true;
		}

	private:
		std::shared_ptr<Entry> m_rootEntry;
	};
}

namespace fx
{
	bool ResourceFilesComponent::BuildResourceSet(const std::string& setName)
	{
		fi::PackfileBuilder packfile;
		auto files = GetFilesForSet(setName);

		for (const auto& file : files)
		{
			packfile.AddFile(file, m_resource->GetPath() + "/" + file);
		}

		packfile.Write(GetSetFileName(setName));

		return true;
	}

	bool ResourceFilesComponent::ShouldBuildSet(const std::string& setName)
	{
		std::string setFileName = GetSetFileName(setName);
		
		fwRefContainer<vfs::Device> device = vfs::GetDevice(setFileName);

		if (!device.GetRef())
		{
			return true;
		}

		// get the last-modified time for the set
		std::time_t setModifiedTime = device->GetModifiedTime(setFileName);

		// get the highest modified time for the set's files
		std::time_t lastModifiedTime = std::numeric_limits<std::time_t>::min();

		auto files = GetFilesForSet(setName);

		for (auto& file : files)
		{
			device = vfs::GetDevice(m_resource->GetPath() + "/" + file);

			if (device.GetRef())
			{
				lastModifiedTime = std::max(device->GetModifiedTime(m_resource->GetPath() + "/" + file), lastModifiedTime);
			}
		}

		// return true if the files were modified after the set
		return (setModifiedTime == 0 || lastModifiedTime > setModifiedTime);
	}

	void ResourceFilesComponent::AddFileToDefaultSet(const std::string& fileName)
	{
		m_additionalFiles.insert(fileName);
	}

	std::vector<std::string> ResourceFilesComponent::GetFilesForSet(const std::string& setName)
	{
		if (setName == "resource.rpf")
		{
			std::vector<std::string> fileEntries(m_additionalFiles.begin(), m_additionalFiles.end());
			
			auto metaData = m_resource->GetComponent<fx::ResourceMetaDataComponent>();

			fileEntries.emplace_back("__resource.lua");

			// add files
			auto files = metaData->GetEntries("file");

			for (auto& file : files)
			{
				fileEntries.emplace_back(file.second);
			}

			// add client scripts
			files = metaData->GetEntries("client_script");

			for (auto& file : files)
			{
				fileEntries.emplace_back(file.second);
			}

			// add shared scripts
			files = metaData->GetEntries("shared_script");

			for (auto& file : files)
			{
				fileEntries.emplace_back(file.second);
			}

			// TEMP DBG: add `map`
			files = metaData->GetEntries("map");

			for (auto& file : files)
			{
				fileEntries.emplace_back(file.second);
			}

			return fileEntries;
		}

		return {};
	}

	std::map<std::string, std::string> ResourceFilesComponent::GetFileHashPairs()
	{
		return m_fileHashPairs;
	}

	std::shared_ptr<ResourceFilesFilter> ResourceFilesComponent::CreateFilesFilter(const std::string& file, const fwRefContainer<fwRefCountable>& context)
	{
		if (m_filesFilter)
		{
			return m_filesFilter(m_resource->GetName(), file, context);
		}

		return {};
	}

	void ResourceFilesComponent::SetFilesFilter(const TFilesFilterFactory& filter)
	{
		m_filesFilter = filter;
	}

	void ResourceFilesComponent::AttachToObject(fx::Resource* object)
	{
		m_resource = object;

		object->OnStart.Connect([=]()
		{
			if (ShouldBuildSet(GetDefaultSetName()))
			{
				BuildResourceSet(GetDefaultSetName());
			}

			// TODO(fxserver): clean up
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(GetSetFileName(GetDefaultSetName()));

			if (stream.GetRef())
			{
				// calculate a hash of the file
				std::vector<uint8_t> data(8192);
				sha1nfo sha1;
				size_t numRead;

				// initialize context
				sha1_init(&sha1);

				// read from the stream
				while ((numRead = stream->Read(data)) > 0)
				{
					sha1_write(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
				}

				// get the hash result and convert it to a string
				uint8_t* hash = sha1_result(&sha1);

				m_fileHashPairs["resource.rpf"] = fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
					hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);
			}
		}, 500);
	}

	std::string ResourceFilesComponent::GetSetFileName(const std::string& setName)
	{
		return "cache:/files/" + m_resource->GetName() + "/" + setName;
	}

	std::string ResourceFilesComponent::GetDefaultSetName()
	{
		return "resource.rpf";
	}
}

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new fx::ResourceFilesComponent());
	});
});
