#include "StdInc.h"
#include <leveldb/db.h>

#include <VFSManager.h>
#include <ScriptEngine.h>

#include <msgpack.hpp>

#include <Resource.h>
#include <fxScripting.h>

leveldb::Env* GetVFSEnvironment();

struct DatabaseHolder
{
	DatabaseHolder()
	{
		leveldb::DB* dbPointer;

		leveldb::Options options;
		options.env = GetVFSEnvironment();
		options.create_if_missing = true;

		auto status = leveldb::DB::Open(options, "fxd:/kvs/", &dbPointer);

		if (!status.ok())
		{
			if (status.IsCorruption() || status.IsIOError())
			{
				leveldb::Options repairOptions;
				repairOptions.reuse_logs = false;
				repairOptions.create_if_missing = true;
				repairOptions.env = GetVFSEnvironment();
				
				status = leveldb::RepairDB("fxd:/kvs/", repairOptions);

				if (status.ok())
				{
					status = leveldb::DB::Open(options, "fxd:/kvs/", &dbPointer);
				}

				if (!status.ok())
				{
					status = leveldb::DestroyDB("fxd:/kvs/", options);

					status = leveldb::DB::Open(options, "fxd:/kvs/", &dbPointer);
				}
			}
		}

		assert(status.ok());

		db = std::unique_ptr<leveldb::DB>(dbPointer);
		dbPointer = nullptr; // as the unique_ptr 'owns' it now
	}

	inline leveldb::DB* get()
	{
		return db.get();
	}

private:
	std::unique_ptr<leveldb::DB> db;
};

static leveldb::DB* EnsureDatabase()
{
	static DatabaseHolder db;
	return db.get();
}

static fx::Resource* GetCurrentResource()
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
	}

	return nullptr;
}

static std::string FormatKey(const char* key, const std::string& resource = {})
{
	std::string resName = resource;

	if (resource.empty())
	{
		resName = GetCurrentResource()->GetName();
	}

	return "res:" + resName + ":" + key;
}

static void PutResourceKvp(fx::ScriptContext& context, const char* data, size_t size)
{
	auto db = EnsureDatabase();
	auto key = FormatKey(context.GetArgument<const char*>(0));

	leveldb::WriteOptions options;
	options.sync = true;

	db->Put(options, key, leveldb::Slice{ data, size });
}

template<typename T>
static void SetResourceKvp(fx::ScriptContext& context)
{
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> packer(buffer);
	packer.pack(context.GetArgument<T>(1));

	PutResourceKvp(context, buffer.data(), buffer.size());
}

static void SetResourceKvpRaw(fx::ScriptContext& context)
{
	PutResourceKvp(context, context.GetArgument<const char*>(1), context.GetArgument<size_t>(2));
}

struct AnyType {};
struct RawType {};

template<typename T>
struct SerializeValue
{
	static void Deserialize(fx::ScriptContext& context, const std::string& value)
	{
		msgpack::unpacked msg = msgpack::unpack(value.c_str(), value.size());
		context.SetResult<T>(msg.get().as<T>());
	}
};

template<>
struct SerializeValue<const char*>
{
	static void Deserialize(fx::ScriptContext& context, const std::string& value)
	{
		msgpack::unpacked msg = msgpack::unpack(value.c_str(), value.size());
		static std::string str;
		str = msg.get().as<std::string>();

		context.SetResult<const char*>(str.c_str());
	}
};

template<>
struct SerializeValue<AnyType>
{
	static void Deserialize(fx::ScriptContext& context, const std::string& value)
	{
		msgpack::unpacked msg = msgpack::unpack(value.c_str(), value.size());
		switch (msg.get().type)
		{
			case msgpack::type::POSITIVE_INTEGER:
			case msgpack::type::NEGATIVE_INTEGER:
				SerializeValue<int>::Deserialize(context, value);
				break;
			case msgpack::type::FLOAT:
				SerializeValue<float>::Deserialize(context, value);
				break;
			case msgpack::type::STR:
				SerializeValue<const char*>::Deserialize(context, value);
				break;
		}
	}
};

template<>
struct SerializeValue<RawType>
{
	static void Deserialize(fx::ScriptContext& context, const std::string& value)
	{
		static std::string str;
		str = value;

		*context.GetArgument<int*>(1) = str.size();
		context.SetResult<const char*>(str.c_str());
	}
};

template<typename T>
static void GetResourceKvp(fx::ScriptContext& context)
{
	auto db = EnsureDatabase();
	auto key = FormatKey(context.GetArgument<const char*>(0));

	std::string value;
	if (db->Get(leveldb::ReadOptions{}, key, &value).IsNotFound())
	{
		context.SetResult<const char*>(nullptr);
		return;
	}

	SerializeValue<T>::Deserialize(context, value);
}

#include <memory>

struct FindHandle
{
	std::shared_ptr<leveldb::Iterator> dbIter;
	std::string key;
};

static FindHandle g_handles[64];

static FindHandle* GetFindHandle()
{
	for (auto& handle : g_handles)
	{
		if (!handle.dbIter)
		{
			return &handle;
		}
	}

	return nullptr;
}

static void StartFindKvp(fx::ScriptContext& context)
{
	auto db = EnsureDatabase();
	auto key = FormatKey(context.GetArgument<const char*>(0));

	auto handle = GetFindHandle();

	if (!handle)
	{
		context.SetResult(-1);
		return;
	}

	handle->dbIter = std::shared_ptr<leveldb::Iterator>(db->NewIterator(leveldb::ReadOptions{}));
	handle->dbIter->Seek(key);

	handle->key = key;

	context.SetResult(handle - g_handles);
}

static void FindKvp(fx::ScriptContext& context)
{
	auto db = EnsureDatabase();
	auto handleIdx = context.GetArgument<int>(0);

	if (handleIdx < 0 || handleIdx >= _countof(g_handles))
	{
		return;
	}

	// get handle
	auto handle = &g_handles[handleIdx];

	if (!handle->dbIter->Valid() || !handle->dbIter->key().starts_with(handle->key))
	{
		context.SetResult<const char*>(nullptr);
		return;
	}

	static std::string keyName;
	std::string k(handle->dbIter->key().data(), handle->dbIter->key().size());

	keyName = k.substr(FormatKey("").size());

	context.SetResult<const char*>(keyName.c_str());

	handle->dbIter->Next();
}

static void EndFindKvp(fx::ScriptContext& context)
{
	auto handleIdx = context.GetArgument<int>(0);

	if (handleIdx < 0 || handleIdx >= _countof(g_handles))
	{
		return;
	}

	// get handle
	auto handle = &g_handles[handleIdx];
	handle->dbIter.reset();
}

static void DeleteResourceKvp(fx::ScriptContext& context)
{
	auto db = EnsureDatabase();
	auto key = FormatKey(context.GetArgument<const char*>(0));

	db->Delete(leveldb::WriteOptions{}, key);
}

#include <VFSStreamDevice.h>

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct ResourceFlags
{
	uint32_t flag1;
	uint32_t flag2;
};

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	ResourceFlags flags; // out
};

struct KvpStream : public vfs::SeekableStream
{
	KvpStream(std::string&& value)
		: m_value(std::move(value)), m_pos(0)
	{

	}

	size_t Read(void* outBuffer, size_t size)
	{
		size_t toRead = std::min(m_value.size() - m_pos, size);
		memcpy(outBuffer, m_value.data() + m_pos, toRead);

		m_pos += toRead;

		return toRead;
	}

	size_t Seek(intptr_t off, int at)
	{
		auto oldPos = m_pos;

		switch (at)
		{
		case SEEK_SET:
			m_pos = off;
			break;
		case SEEK_CUR:
			m_pos += off;
			break;
		case SEEK_END:
			m_pos = m_value.size() - off;
			break;
		}

		if (m_pos >= m_value.size())
		{
			// roll back the position
			m_pos = oldPos;
			return -1;
		}

		return m_pos;
	}

private:
	std::string m_value;
	size_t m_pos;
};

struct KvpBulkStream
{
	KvpBulkStream(std::string&& value)
		: m_value(std::move(value))
	{

	}

	size_t ReadBulk(uint64_t ptr, void* outBuffer, size_t size)
	{
		size_t toRead = std::min(m_value.size() - ptr, size);
		memcpy(outBuffer, m_value.data() + ptr, toRead);

		return toRead;
	}

private:
	std::string m_value;
};

class KvpDevice : public vfs::BulkStreamDevice<KvpStream, KvpBulkStream>
{
public:
	virtual std::shared_ptr<KvpBulkStream> OpenBulkStream(const std::string& fileName, uint64_t* ptr) override
	{
		std::string value;

		auto status = FindValue(fileName, &value);

		if (status.ok())
		{
			return std::make_shared<KvpBulkStream>(std::move(value));
		}

		return {};
	}


	virtual std::shared_ptr<KvpStream> OpenStream(const std::string& fileName, bool readOnly) override
	{
		if (readOnly)
		{
			std::string value;

			auto status = FindValue(fileName, &value);

			if (status.ok())
			{
				return std::make_shared<KvpStream>(std::move(value));
			}
		}

		return {};
	}

	virtual std::shared_ptr<KvpStream> CreateStream(const std::string& fileName) override
	{
		return {};
	}

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override
	{
		if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
		{
			auto data = (GetRagePageFlagsExtension*)controlData;

			auto fileKey = FormatFileName(data->fileName);
			auto verKey = fmt::sprintf("rv:%s", fileKey);

			struct
			{
				int version;
				ResourceFlags flags;
			} saveData;

			leveldb::ReadOptions ro;
			ro.fill_cache = false;

			std::string saveDataStr;

			bool found = true;

			if (EnsureDatabase()->Get(ro, verKey, &saveDataStr).ok())
			{
				if (saveDataStr.size() == sizeof(saveData))
				{
					saveData = *(decltype(&saveData))saveDataStr.c_str();
					found = true;
				}
			}

			if (!found)
			{
				std::string valueStr;

				if (EnsureDatabase()->Get(ro, fileKey, &valueStr).ok())
				{
					struct
					{
						uint32_t magic;
						uint32_t version;
						uint32_t virtPages;
						uint32_t physPages;
					} rsc7Header;

					rsc7Header = *(decltype(&rsc7Header))valueStr.c_str();

					if (rsc7Header.magic == 0x37435352) // RSC7
					{
						saveData.flags.flag1 = rsc7Header.virtPages;
						saveData.flags.flag2 = rsc7Header.physPages;
						saveData.version = rsc7Header.version;

						leveldb::WriteOptions wo;
						wo.sync = true;

						saveDataStr = std::string((const char*)&saveData, sizeof(saveData));

						EnsureDatabase()->Put(wo, fileKey, saveDataStr);

						found = true;
					}
				}
			}

			if (found)
			{
				data->flags = saveData.flags;
				data->version = saveData.version;
			}

			return found;
		}

		return false;
	}

	void SetPathPrefix(const std::string& pathPrefix) override
	{
		m_pathPrefix = pathPrefix;
	}

private:
	leveldb::Status FindValue(const std::string& fileName, std::string* value)
	{
		leveldb::ReadOptions ro;
		ro.fill_cache = false;

		auto db = EnsureDatabase();
		return db->Get(ro, FormatFileName(fileName), value);
	}

	std::string FormatFileName(const std::string& fileName)
	{
		std::string trimName = fileName.substr(m_pathPrefix.length());
		int slashPos = trimName.find_first_of("/");

		return FormatKey(trimName.substr(slashPos + 1).c_str(), trimName.substr(0, slashPos));
	}

private:
	std::string m_pathPrefix;
};

void MountKvpDevice()
{
	vfs::Mount(new KvpDevice(), "kvs:/");
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_KVP", GetResourceKvp<AnyType>);
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_KVP_INT", GetResourceKvp<int>);
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_KVP_STRING", GetResourceKvp<const char*>);
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_KVP_FLOAT", GetResourceKvp<float>);
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_RAW_KVP", GetResourceKvp<RawType>);

	fx::ScriptEngine::RegisterNativeHandler("SET_RESOURCE_KVP", SetResourceKvp<const char*>);
	fx::ScriptEngine::RegisterNativeHandler("SET_RESOURCE_KVP_INT", SetResourceKvp<int>);
	fx::ScriptEngine::RegisterNativeHandler("SET_RESOURCE_KVP_FLOAT", SetResourceKvp<float>);
	fx::ScriptEngine::RegisterNativeHandler("SET_RESOURCE_RAW_KVP", SetResourceKvpRaw);

	fx::ScriptEngine::RegisterNativeHandler("DELETE_RESOURCE_KVP", DeleteResourceKvp);

	fx::ScriptEngine::RegisterNativeHandler("START_FIND_KVP", StartFindKvp);
	fx::ScriptEngine::RegisterNativeHandler("FIND_KVP", FindKvp);
	fx::ScriptEngine::RegisterNativeHandler("END_FIND_KVP", EndFindKvp);
});
