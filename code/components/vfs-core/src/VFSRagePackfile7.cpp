/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <VFSRagePackfile7.h>

#include <VFSManager.h>

#include <Error.h>
#include <zlib.h>

namespace vfs
{
	RagePackfile7::RagePackfile7()
		: m_parentHandle(InvalidHandle)
	{

	}

	RagePackfile7::~RagePackfile7()
	{
		if (m_parentHandle != InvalidHandle)
		{
			m_parentDevice->CloseBulk(m_parentHandle);

			m_parentHandle = InvalidHandle;
		}
	}

	static std::function<bool(const std::vector<uint8_t>&, const std::vector<uint8_t>&)> g_validationCallback;

	void RagePackfile7::SetValidationCallback(const std::function<bool(const std::vector<uint8_t>&, const std::vector<uint8_t>&)>& func)
	{
		g_validationCallback = func;
	}

	bool RagePackfile7::OpenArchive(const std::string& archivePath, bool needsEncryption /* = false */)
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
			trace("%s: ReadBulk of header failed\n", __func__);

			return false;
		}

		// verify if the header magic is, in fact, RPF7 and it's non-encrypted
		if (m_header.magic != 0x52504637 || (m_header.encryption != 0x4E45504F && m_header.encryption != 0x50584643)) // OPEN/CFXP
		{
			trace("%s: only non-encrypted RPF7 is supported\n", __func__);

			return false;
		}

		// read the TOC into a temporary list - we split this into entry/name table later
		m_entries.resize(m_header.entryCount);

		m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + sizeof(m_header), &m_entries[0], m_entries.size() * sizeof(Entry));

		// copy out the name table as well
		m_nameTable.resize(m_header.nameLength);
		
		m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + sizeof(m_header) + (m_entries.size() * sizeof(Entry)), &m_nameTable[0], m_nameTable.size());

		// copy out RSA signature hash if this is a CFXP file
		if (m_header.encryption == 0x50584643)
		{
			bool valid = true;

			if (g_validationCallback)
			{
				std::vector<uint8_t> headerSignature(256);
				m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + sizeof(m_header) + (m_entries.size() * sizeof(Entry) + m_nameTable.size()), headerSignature.data(), headerSignature.size());

				auto entriesStart = reinterpret_cast<uint8_t*>(m_entries.data());
				std::vector<uint8_t> data(entriesStart, entriesStart + (m_entries.size() * sizeof(Entry)));

				valid = g_validationCallback(headerSignature, data);
			}

			if (!valid)
			{
				trace("Opening packfile %s failed - it didn't pass signing validation!\n", archivePath);
				return false;
			}
		}
		else if (needsEncryption)
		{
			trace("Opening packfile %s failed - it needs to be signed...\n", archivePath);
			return false;
		}

		// return a success value
		return true;
	}

	const RagePackfile7::Entry* RagePackfile7::FindEntry(const std::string& path)
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

		if (relativePath == "/")
		{
			return entry;
		}

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
			if (entry->offset == 0x7fffff)
			{
				auto proxy = EntryProxy{ m_nameTable, relativePath.substr(pos, nextPos - pos) };
				const auto origEntry = entry;

				entry = reinterpret_cast<const Entry*>(
							bsearch(&proxy,
								&m_entries[entry->virtFlags], entry->physFlags,
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

					for (size_t i = 0; i < origEntry->physFlags; ++i)
					{
						const Entry* thisEntry = &m_entries[origEntry->virtFlags + i];
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

	RagePackfile7::HandleData* RagePackfile7::AllocateHandle(THandle* outHandle)
	{
		std::unique_lock _(m_handlesMutex);

		for (int i = 0; i < m_handles.size(); i++)
		{
			if (!m_handles[i].valid)
			{
				*outHandle = i;

				auto handle = &m_handles[i];
				handle->valid = true;
				return handle;
			}
		}

		m_handles.push_back({});

		auto handleIdx = m_handles.size() - 1;
		*outHandle = handleIdx;

		auto handle = &m_handles[handleIdx];
		handle->valid = true;
		return handle;
	}

	RagePackfile7::HandleData* RagePackfile7::GetHandle(THandle inHandle)
	{
		if (inHandle >= 0 && inHandle < m_handles.size())
		{
			if (m_handles[inHandle].valid)
			{
				return &m_handles[inHandle];
			}
		}

		return nullptr;
	}

	RagePackfile7::THandle RagePackfile7::Open(const std::string& fileName, bool readOnly)
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
					handleData->compressed = false;
					handleData->entry = *entry;
					handleData->curOffset = 0;
					handleData->curDecOffset = 0;

					// compressed
					if (handleData->entry.size)
					{
						handleData->compressed = true;

						memset(&handleData->strm, 0, sizeof(handleData->strm));
						inflateInit2(&handleData->strm, -15);
					}

					return handle;
				}
			}
		}

		return InvalidHandle;
	}

	RagePackfile7::THandle RagePackfile7::OpenBulk(const std::string& fileName, uint64_t* ptr)
	{
		auto entry = FindEntry(fileName);

		if (entry)
		{
			*ptr = ((entry->offset & 0x7FFFFF) * 512);

			return reinterpret_cast<THandle>(entry);
		}

		return InvalidHandle;
	}

	size_t RagePackfile7::Read(THandle handle, void* outBuffer, size_t size)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			// detect EOF
			if (handleData->curOffset >= handleData->entry.virtFlags)
			{
				return 0;
			}

			if (!handleData->compressed)
			{
				// calculate and read
				size_t toRead = handleData->entry.virtFlags - handleData->curOffset;

				if (toRead > size)
				{
					toRead = size;
				}

				size_t didRead = m_parentDevice->ReadBulk(m_parentHandle,
					m_parentPtr + (handleData->entry.offset * 512) + handleData->curOffset,
					outBuffer,
					toRead);

				handleData->curOffset += didRead;

				return didRead;
			}
			else
			{
				handleData->strm.next_out = (Bytef*)outBuffer;
				handleData->strm.avail_out = size;

				InternalRead(handleData);

				handleData->curDecOffset += size - handleData->strm.avail_out;

				return size - handleData->strm.avail_out;
			}
		}

		return -1;
	}

	void RagePackfile7::InternalRead(HandleData* handleData)
	{
		while (handleData->strm.avail_out > 0)
		{
			if (handleData->strm.avail_in == 0)
			{
				// calculate and read
				size_t toRead = handleData->entry.size - handleData->curOffset;

				if (toRead > 0x2000)
				{
					toRead = 0x2000;
				}

				size_t didRead = m_parentDevice->ReadBulk(m_parentHandle,
					m_parentPtr + (handleData->entry.offset * 512) + handleData->curOffset,
					handleData->zbuf,
					toRead);

				handleData->strm.next_in = handleData->zbuf;
				handleData->strm.avail_in = didRead;
				handleData->curOffset += didRead;
			}

			int e = inflate(&handleData->strm, Z_SYNC_FLUSH);

			if (e == Z_STREAM_END)
			{
				break;
			}

			if (e < 0)
			{
				FatalError("vfs::RagePackfile7 failed to inflate().");
			}
		}
	}

	size_t RagePackfile7::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
	{
		return m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + ptr, outBuffer, size);
	}

	bool RagePackfile7::Close(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			if (handleData->compressed)
			{
				inflateEnd(&handleData->strm);
			}

			handleData->valid = false;

			return true;
		}

		return false;
	}

	bool RagePackfile7::CloseBulk(THandle handle)
	{
		return true;
	}

	size_t RagePackfile7::Seek(THandle handle, intptr_t offset, int seekType)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			if (!handleData->compressed)
			{
				if (seekType == SEEK_CUR)
				{
					handleData->curOffset += offset;

					if (handleData->curOffset > handleData->entry.virtFlags)
					{
						handleData->curOffset = handleData->entry.virtFlags;
					}
				}
				else if (seekType == SEEK_SET)
				{
					handleData->curOffset = offset;
				}
				else if (seekType == SEEK_END)
				{
					handleData->curOffset = handleData->entry.virtFlags - offset;
				}
				else
				{
					return -1;
				}

				return handleData->curOffset;
			}
			else
			{
				size_t offsetTarget;

				if (seekType == SEEK_CUR)
				{
					offsetTarget = handleData->curDecOffset + offset;

					if (offsetTarget > handleData->entry.virtFlags)
					{
						offsetTarget = handleData->entry.virtFlags;
					}
				}
				else if (seekType == SEEK_SET)
				{
					offsetTarget = offset;
				}
				else if (seekType == SEEK_END)
				{
					offsetTarget = handleData->entry.virtFlags - offset;
				}

				if (offsetTarget < handleData->curDecOffset)
				{
					memset(&handleData->strm, 0, sizeof(handleData->strm));
					inflateInit2(&handleData->strm, -15);

					handleData->curDecOffset = 0;
					handleData->curOffset = 0;
				}

				while (offsetTarget > handleData->curDecOffset)
				{
					uint8_t readBuffer[0x2000];
					size_t readSize = std::min(offsetTarget - handleData->curDecOffset, sizeof(readBuffer));

					handleData->strm.next_out = (Bytef*)readBuffer;
					handleData->strm.avail_out = readSize;

					InternalRead(handleData);

					handleData->curDecOffset += readSize - handleData->strm.avail_out;
				}

				return offsetTarget;
			}
		}

		return -1;
	}

	size_t RagePackfile7::GetLength(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			return handleData->entry.virtFlags;
		}

		return -1;
	}

	size_t RagePackfile7::GetLength(const std::string& fileName)
	{
		auto entry = FindEntry(fileName);

		if (entry)
		{
			return (entry->offset & 0x800000) ? entry->size : entry->virtFlags;
		}

		return -1;
	}

	RagePackfile7::THandle RagePackfile7::FindFirst(const std::string& folder, FindData* findData)
	{
		auto entry = FindEntry(folder);

		if (entry)
		{
			if (entry->offset == 0x7FFFFF && entry->physFlags)
			{
				THandle handle;
				auto handleData = AllocateHandle(&handle);

				if (handleData)
				{
					handleData->curOffset = 0;
					handleData->entry = *entry;

					FillFindData(findData, &m_entries[entry->virtFlags]);

					return handle;
				}
			}
			// a single file?
			else
			{
				THandle handle;
				auto handleData = AllocateHandle(&handle);

				if (handleData)
				{
					handleData->curOffset = 0;
					handleData->entry = *entry;

					FillFindData(findData, entry);

					return handle;
				}
			}
		}

		return InvalidHandle;
	}

	bool RagePackfile7::FindNext(THandle handle, FindData* findData)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->curOffset++;

			// have we passed the length already?
			if (handleData->entry.offset != 0x7FFFFF || handleData->curOffset >= handleData->entry.physFlags)
			{
				return false;
			}

			// get the entry at the offset
			FillFindData(findData, &m_entries[handleData->entry.virtFlags + handleData->curOffset]);

			// will the next increment go past the length? if not, return true
			return (handleData->curOffset < handleData->entry.physFlags);
		}

		return false;
	}

	void RagePackfile7::FindClose(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->valid = false;
		}
	}

	void RagePackfile7::FillFindData(FindData* data, const Entry* entry)
	{
		data->attributes = (entry->offset == 0x7FFFFF) ? FILE_ATTRIBUTE_DIRECTORY : 0;
		data->length = entry->size;
		data->name = &m_nameTable[entry->nameOffset];
	}

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

	struct ResourceFlags
	{
		uint32_t virt;
		uint32_t phys;
	};

	struct GetRagePageFlagsExtension
	{
		const char* fileName; // in
		int version;
		ResourceFlags flags; // out
	};

	bool RagePackfile7::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
	{
		if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
		{
			auto data = (GetRagePageFlagsExtension*)controlData;
			auto entry = FindEntry(data->fileName);

			if (entry)
			{
				data->flags.phys = entry->physFlags;
				data->flags.virt = entry->virtFlags;
				data->version = (entry->physFlags >> 28) | ((entry->virtFlags >> 28) << 4);
				return true;
			}
		}

		return false;
	}

	void RagePackfile7::SetPathPrefix(const std::string& pathPrefix)
	{
		m_pathPrefix = pathPrefix.substr(0, pathPrefix.find_last_not_of('/') + 1);
	}
}
