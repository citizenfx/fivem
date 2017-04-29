/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <VFSRagePackfile.h>

#include <VFSManager.h>

namespace vfs
{
	RagePackfile::RagePackfile()
		: m_parentHandle(InvalidHandle)
	{

	}

	RagePackfile::~RagePackfile()
	{
		if (m_parentHandle != InvalidHandle)
		{
			m_parentDevice->CloseBulk(m_parentHandle);

			m_parentHandle = InvalidHandle;
		}
	}

	bool RagePackfile::OpenArchive(const std::string& archivePath)
	{
		// get the containing device, and early out if we don't have one
		fwRefContainer<Device> parentDevice = vfs::GetDevice(archivePath);

		if (!parentDevice.GetRef())
		{
			return false;
		}

		// attempt opening the archive from the specified path
		m_parentHandle = parentDevice->OpenBulk(archivePath, &m_parentPtr);

		// early out if invalid, again
		if (m_parentHandle == InvalidHandle)
		{
			return false;
		}

		// read the packfile header
		m_parentDevice = parentDevice;

		if (m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr, &m_header, sizeof(m_header)) != sizeof(m_header))
		{
			trace(__FUNCTION__ ": ReadBulk of header failed\n");

			return false;
		}

		// verify if the header magic is, in fact, RPF2 and it's non-encrypted
		if (m_header.magic != 0x32465052 || m_header.cryptoFlag != 0)
		{
			trace(__FUNCTION__ ": only non-encrypted RPF2 is supported\n");

			return false;
		}

		// read the TOC into a temporary list - we split this into entry/name table later
		std::vector<char> toc(m_header.tocSize);

		m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + 2048, &toc[0], toc.size());

		// copy out the entry table
		size_t entryTableSize = m_header.numEntries * sizeof(Entry);
		m_entries.resize(m_header.numEntries);

		memcpy(&m_entries[0], &toc[0], entryTableSize);

		// copy out the name table as well
		m_nameTable.resize(m_header.tocSize - entryTableSize);
		
		memcpy(&m_nameTable[0], &toc[entryTableSize], m_nameTable.size());

		// return a success value
		return true;
	}

	const RagePackfile::Entry* RagePackfile::FindEntry(const std::string& path)
	{
		// first, remove the path prefix
		std::string relativePath = path.substr(m_pathPrefix.length());

		// then, traverse through each path element
		const Entry* entry = &m_entries[0];
		size_t pos = 1;

		while (relativePath[pos] == '/')
		{
			pos++;
		}

		size_t nextPos = relativePath.find_first_of('/', pos);

		// while there's still /'s to go
		while (true)
		{
			struct EntryProxy
			{
				const std::vector<char>& nameTable;
				std::string key;
			};

			// early validity check
			if (entry == nullptr)
			{
				return nullptr;
			}

			// if this is a directory entry
			if (entry->isDirectory)
			{
				auto proxy = EntryProxy{ m_nameTable, relativePath.substr(pos, nextPos - pos) };
				const auto origEntry = entry;

				entry = reinterpret_cast<const Entry*>(
							bsearch(&proxy,
								&m_entries[entry->dataOffset], entry->length,
								sizeof(Entry), [] (const void* keyPtr, const void* entryPtr)
								{
									const EntryProxy* key = reinterpret_cast<const EntryProxy*>(keyPtr);
									const Entry* entry = reinterpret_cast<const Entry*>(entryPtr);

									return strcmp(key->key.c_str(), &key->nameTable[entry->nameOffset]);
								}
							)
						);

				// try alternative search method
				if (entry == nullptr)
				{
					const char* compName = proxy.key.c_str();

					for (size_t i = 0; i < origEntry->length; ++i)
					{
						const Entry* thisEntry = &m_entries[origEntry->dataOffset + i];
						const char* name = &m_nameTable[thisEntry->nameOffset];

						if (_stricmp(compName, name) == 0)
						{
							entry = thisEntry;
							break;
						}
					}
				}
			}
			else
			{
				// we probably found a file entry - ignore any path suffixes after the filename
				return entry;
			}

			pos = nextPos + 1;

			// to strip additional slashes
			while (relativePath[pos] == '/')
			{
				pos++;
			}

			nextPos = relativePath.find_first_of('/', pos);

			if (entry == nullptr)
			{
				return nullptr;
			}
		}
	}

	RagePackfile::HandleData* RagePackfile::AllocateHandle(THandle* outHandle)
	{
		for (int i = 0; i < _countof(m_handles); i++)
		{
			if (!m_handles[i].valid)
			{
				*outHandle = i;

				return &m_handles[i];
			}
		}

		return nullptr;
	}

	RagePackfile::HandleData* RagePackfile::GetHandle(THandle inHandle)
	{
		if (inHandle >= 0 && inHandle < _countof(m_handles))
		{
			if (m_handles[inHandle].valid)
			{
				return &m_handles[inHandle];
			}
		}

		return nullptr;
	}

	RagePackfile::THandle RagePackfile::Open(const std::string& fileName, bool readOnly)
	{
		if (readOnly)
		{
			auto entry = FindEntry(fileName);

			if (entry)
			{
				THandle handle;
				auto handleData = AllocateHandle(&handle);

				if (handleData)
				{
					handleData->valid = true;
					handleData->entry = *entry;
					handleData->curOffset = 0;

					return handle;
				}
			}
		}

		return InvalidHandle;
	}

	RagePackfile::THandle RagePackfile::OpenBulk(const std::string& fileName, uint64_t* ptr)
	{
		auto entry = FindEntry(fileName);

		if (entry)
		{
			*ptr = entry->dataOffset;

			return reinterpret_cast<THandle>(entry);
		}

		return InvalidHandle;
	}

	size_t RagePackfile::Read(THandle handle, void* outBuffer, size_t size)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			// detect EOF
			if (handleData->curOffset >= handleData->entry.length)
			{
				return 0;
			}

			// calculate and read
			size_t toRead = handleData->entry.length - handleData->curOffset;
			
			if (toRead > size)
			{
				toRead = size;
			}

			size_t didRead = m_parentDevice->ReadBulk(m_parentHandle,
													  m_parentPtr + handleData->entry.dataOffset + handleData->curOffset,
													  outBuffer,
													  toRead);

			handleData->curOffset += didRead;

			return didRead;
		}

		return -1;
	}

	size_t RagePackfile::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
	{
		return m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + ptr, outBuffer, size);
	}

	bool RagePackfile::Close(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->valid = false;

			return true;
		}

		return false;
	}

	bool RagePackfile::CloseBulk(THandle handle)
	{
		return true;
	}

	size_t RagePackfile::Seek(THandle handle, intptr_t offset, int seekType)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			if (seekType == SEEK_CUR)
			{
				handleData->curOffset += offset;

				if (handleData->curOffset > handleData->entry.length)
				{
					handleData->curOffset = handleData->entry.length;
				}
			}
			else if (seekType == SEEK_SET)
			{
				handleData->curOffset = offset;
			}
			else if (seekType == SEEK_END)
			{
				handleData->curOffset = handleData->entry.length - offset;
			}
			else
			{
				return -1;
			}

			return handleData->curOffset;
		}

		return -1;
	}

	size_t RagePackfile::GetLength(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			return handleData->entry.length;
		}

		return -1;
	}

	size_t RagePackfile::GetLength(const std::string& fileName)
	{
		auto entry = FindEntry(fileName);

		if (entry)
		{
			// this has as side effect that directories will get their subentry count returned
			return entry->length;
		}

		return -1;
	}

	RagePackfile::THandle RagePackfile::FindFirst(const std::string& folder, FindData* findData)
	{
		auto entry = FindEntry(folder);

		if (entry)
		{
			if (entry->isDirectory && entry->length)
			{
				THandle handle;
				auto handleData = AllocateHandle(&handle);

				if (handleData)
				{
					handleData->curOffset = 0;
					handleData->entry = *entry;
					handleData->valid = true;

					FillFindData(findData, &m_entries[entry->dataOffset]);

					return handle;
				}
			}
		}

		return InvalidHandle;
	}

	bool RagePackfile::FindNext(THandle handle, FindData* findData)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->curOffset++;

			// have we passed the length already?
			if (handleData->curOffset >= handleData->entry.length)
			{
				return false;
			}

			// get the entry at the offset
			FillFindData(findData, &m_entries[handleData->entry.dataOffset + handleData->curOffset]);

			// will the next increment go past the length? if not, return true
			return ((handleData->curOffset + 1) < handleData->entry.length);
		}

		return false;
	}

	void RagePackfile::FindClose(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->valid = false;
		}
	}

	void RagePackfile::FillFindData(FindData* data, const Entry* entry)
	{
		data->attributes = (entry->isDirectory) ? FILE_ATTRIBUTE_DIRECTORY : 0;
		data->length = entry->length;
		data->name = &m_nameTable[entry->nameOffset];
	}

	void RagePackfile::SetPathPrefix(const std::string& pathPrefix)
	{
		m_pathPrefix = pathPrefix.substr(0, pathPrefix.find_last_not_of('/') + 1);
	}
}