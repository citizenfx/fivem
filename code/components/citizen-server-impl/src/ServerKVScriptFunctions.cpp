#include "StdInc.h"
#include <rocksdb/db.h>

#include <ScriptEngine.h>

#include <msgpack.hpp>

#include <Resource.h>
#include <ResourceManager.h>

#include <fxScripting.h>

#include <filesystem>
#include <ServerInstanceBaseRef.h>

struct DatabaseHolder
{
	DatabaseHolder(fx::ServerInstanceBase* instance)
	{
		rocksdb::DB* dbPointer;

		rocksdb::Options options;
		options.create_if_missing = true;
		options.compression = rocksdb::kLZ4Compression;

		auto instanceRoot = std::filesystem::u8path(instance->GetRootPath());
		auto dbRoot = (instanceRoot / "db" / "default").lexically_normal();

		std::error_code err;
		std::filesystem::create_directories(dbRoot, err);

		auto status = rocksdb::DB::Open(options, dbRoot.string(), &dbPointer);

		if (!status.ok())
		{
			if (status.IsCorruption() || status.IsIOError())
			{
				rocksdb::Options repairOptions;
				repairOptions.create_if_missing = true;
				repairOptions.compression = rocksdb::kLZ4Compression;

				status = rocksdb::RepairDB(dbRoot.string(), repairOptions);

				if (status.ok())
				{
					status = rocksdb::DB::Open(options, dbRoot.string(), &dbPointer);
				}

				if (!status.ok())
				{
					// #TODOSERVER: don't
					status = rocksdb::DestroyDB(dbRoot.string(), options);

					status = rocksdb::DB::Open(options, dbRoot.string(), &dbPointer);
				}
			}
		}

		assert(status.ok());

		db = std::unique_ptr<rocksdb::DB>(dbPointer);
		dbPointer = nullptr; // as the unique_ptr 'owns' it now
	}

	inline rocksdb::DB* get()
	{
		return db.get();
	}

private:
	std::unique_ptr<rocksdb::DB> db;
};

static fx::Resource* GetCurrentResource()
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
	}

	return nullptr;
}

static rocksdb::DB* EnsureDatabase()
{
	auto instance = GetCurrentResource()->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();

	static DatabaseHolder db(instance);
	return db.get();
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
	auto key = FormatKey(context.CheckArgument<const char*>(0));

	rocksdb::WriteOptions options;
	options.sync = true;

	db->Put(options, key, rocksdb::Slice{ data, size });
}

template<typename T>
static void SetResourceKvp(fx::ScriptContext& context)
{
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> packer(buffer);
	packer.pack(std::is_pointer_v<T> ? context.CheckArgument<T>(1) : context.GetArgument<T>(1));

	PutResourceKvp(context, buffer.data(), buffer.size());
}

static void SetResourceKvpRaw(fx::ScriptContext& context)
{
	PutResourceKvp(context, context.GetArgument<const char*>(1), context.GetArgument<size_t>(2));
}

struct AnyType
{
};
struct RawType
{
};

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
	if (db->Get(rocksdb::ReadOptions{}, key, &value).IsNotFound())
	{
		context.SetResult<const char*>(nullptr);
		return;
	}

	SerializeValue<T>::Deserialize(context, value);
}

#include <memory>

struct FindHandle
{
	std::shared_ptr<rocksdb::Iterator> dbIter;
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

	handle->dbIter = std::shared_ptr<rocksdb::Iterator>(db->NewIterator(rocksdb::ReadOptions{}));
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

	db->Delete(rocksdb::WriteOptions{}, key);
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
