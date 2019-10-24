#include "StdInc.h"

#include <Hooking.h>

#include <Streaming.h>
#include <EntitySystem.h>

#include <scrBind.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>

#include <Resource.h>
#include <fxScripting.h>
#include <VFSManager.h>

#include <grcTexture.h>

#include <RageParser.h>

#include <wrl.h>
#include <wincodec.h>

#include <CefOverlay.h>

#include <NetLibrary.h>

#include <ResourceCacheDevice.h>
#include <ResourceManager.h>
#include <json.hpp>

#include <atPool.h>

#include <concurrent_unordered_set.h>

using Microsoft::WRL::ComPtr;

class RuntimeTex
{
public:
	RuntimeTex(const char* name, int width, int height);

	RuntimeTex(rage::grcTexture* texture, const void* data, size_t size);

	RuntimeTex(rage::grcTexture* texture);

	virtual ~RuntimeTex();

	int GetWidth();

	int GetHeight();

	int GetPitch();

	void SetPixel(int x, int y, int r, int g, int b, int a);

	bool SetPixelData(const void* data, size_t length);

	void Commit();

	inline rage::grcTexture* GetTexture()
	{
		return m_texture;
	}

private:
	rage::grcTexture* m_texture;

	int m_pitch;

	std::vector<uint8_t> m_backingPixels;
};

class RuntimeTxd
{
public:
	RuntimeTxd(const char* name);

	RuntimeTex* CreateTexture(const char* name, int width, int height);

	RuntimeTex* CreateTextureFromImage(const char* name, const char* fileName);

	RuntimeTex* CreateTextureFromDui(const char* name, const char* duiHandle);

private:
	uint32_t m_txdIndex;
	std::string m_name;

	std::unordered_map<std::string, std::shared_ptr<RuntimeTex>> m_textures;

	rage::five::pgDictionary<rage::grcTexture>* m_txd;
};

RuntimeTex::RuntimeTex(const char* name, int width, int height)
{
	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	m_texture = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2, nullptr, true, &textureDef);

	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(0, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memset(lockedTexture.pBits, 0, lockedTexture.pitch * lockedTexture.height);
		m_backingPixels.resize(lockedTexture.pitch * lockedTexture.height);

		m_pitch = lockedTexture.pitch;

		m_texture->Unmap(&lockedTexture);
	}
}

RuntimeTex::RuntimeTex(rage::grcTexture* texture, const void* data, size_t size)
	: m_texture(texture)
{
	m_backingPixels.resize(size);
	memcpy(&m_backingPixels[0], data, m_backingPixels.size());
}

RuntimeTex::RuntimeTex(rage::grcTexture* texture)
	: m_texture(texture)
{
	m_backingPixels.resize(0);
}

RuntimeTex::~RuntimeTex()
{
	delete m_texture;
}

int RuntimeTex::GetWidth()
{
	return m_texture->GetWidth();
}

int RuntimeTex::GetHeight()
{
	return m_texture->GetHeight();
}

int RuntimeTex::GetPitch()
{
	return m_pitch;
}

void RuntimeTex::SetPixel(int x, int y, int r, int g, int b, int a)
{
	auto offset = (y * m_pitch) + (x * 4);

	if (offset < 0 || offset >= m_backingPixels.size() - 4)
	{
		return;
	}

	auto start = &m_backingPixels[offset];

	start[3] = b;
	start[2] = g;
	start[1] = r;
	start[0] = a;
}

bool RuntimeTex::SetPixelData(const void* data, size_t length)
{
	if (length != m_backingPixels.size())
	{
		return false;
	}

	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(0, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memcpy(lockedTexture.pBits, data, length);
		memcpy(m_backingPixels.data(), data, length);
		m_texture->Unmap(&lockedTexture);
	}

	return true;
}

void RuntimeTex::Commit()
{
	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(0, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memcpy(lockedTexture.pBits, m_backingPixels.data(), m_backingPixels.size());
		m_texture->Unmap(&lockedTexture);
	}
}

RuntimeTxd::RuntimeTxd(const char* name)
{
	streaming::Manager* streaming = streaming::Manager::GetInstance();
	auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");

	txdStore->FindSlotFromHashKey(&m_txdIndex, name);

	if (m_txdIndex != 0xFFFFFFFF)
	{
		auto& entry = streaming->Entries[txdStore->baseIdx + m_txdIndex];

		if (!entry.handle)
		{
			m_name = name;
			m_txd = new rage::five::pgDictionary<rage::grcTexture>();

			streaming::strAssetReference ref;
			ref.asset = m_txd;

			txdStore->SetResource(m_txdIndex, ref);
			entry.flags = (512 << 8) | 1;
		}
	}
}

RuntimeTex* RuntimeTxd::CreateTexture(const char* name, int width, int height)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		return nullptr;
	}

	auto tex = std::make_shared<RuntimeTex>(name, width, height);
	m_txd->Add(name, tex->GetTexture());

	m_textures[name] = tex;

	scrBindAddSafePointer(tex.get());
	return tex.get();
}

RuntimeTex* RuntimeTxd::CreateTextureFromDui(const char* name, const char* duiHandle)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		return nullptr;
	}

	auto tex = std::make_shared<RuntimeTex>((rage::grcTexture*)nui::GetWindowTexture(duiHandle)->GetHostTexture());
	m_txd->Add(name, tex->GetTexture());

	m_textures[name] = tex;

	scrBindAddSafePointer(tex.get());
	return tex.get();
}

#pragma comment(lib, "windowscodecs.lib")

ComPtr<IWICImagingFactory> g_imagingFactory;

class VfsStream : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IStream>
{
private:
	fwRefContainer<vfs::Stream> m_stream;

public:
	VfsStream(fwRefContainer<vfs::Stream> stream)
	{
		m_stream = stream;
	}

	// Inherited via RuntimeClass
	virtual HRESULT Read(void * pv, ULONG cb, ULONG * pcbRead) override
	{
		*pcbRead = m_stream->Read(pv, cb);

		return S_OK;
	}
	virtual HRESULT Write(const void * pv, ULONG cb, ULONG * pcbWritten) override
	{
		*pcbWritten = m_stream->Write(pv, cb);
		return S_OK;
	}
	virtual HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition) override
	{
		auto p = m_stream->Seek(dlibMove.QuadPart, dwOrigin);

		if (plibNewPosition)
		{
			plibNewPosition->QuadPart = p;
		}

		return S_OK;
	}

	virtual HRESULT SetSize(ULARGE_INTEGER libNewSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Commit(DWORD grfCommitFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Revert(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Stat(STATSTG * pstatstg, DWORD grfStatFlag) override
	{
		pstatstg->cbSize.QuadPart = m_stream->GetLength();
		pstatstg->type = STGTY_STREAM;
		pstatstg->grfMode = STGM_READ;

		return S_OK;
	}
	virtual HRESULT Clone(IStream ** ppstm) override
	{
		return E_NOTIMPL;
	}
};

static ComPtr<IStream> CreateComStream(fwRefContainer<vfs::Stream> stream)
{
	return Microsoft::WRL::Make<VfsStream>(stream);
}

RuntimeTex* RuntimeTxd::CreateTextureFromImage(const char* name, const char* fileName)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (!g_imagingFactory)
	{
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)g_imagingFactory.GetAddressOf());
	}

	fx::OMPtr<IScriptRuntime> runtime;

	if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return nullptr;
	}

	fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

	ComPtr<IWICBitmapDecoder> decoder;

	ComPtr<IStream> stream = CreateComStream(vfs::OpenRead(resource->GetPath() + "/" + fileName));

	HRESULT hr = g_imagingFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		ComPtr<IWICBitmapFrameDecode> frame;

		hr = decoder->GetFrame(0, frame.GetAddressOf());

		if (SUCCEEDED(hr))
		{
			ComPtr<IWICBitmapSource> source;
			ComPtr<IWICBitmapSource> convertedSource;

			UINT width = 0, height = 0;

			frame->GetSize(&width, &height);

			// try to convert to a pixel format we like
			frame.As(&source);

			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, source.Get(), convertedSource.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				source = convertedSource;
			}

			// create a pixel data buffer
			uint32_t* pixelData = new uint32_t[width * height];

			hr = source->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixelData));

			if (SUCCEEDED(hr))
			{
				rage::grcTextureReference reference;
				memset(&reference, 0, sizeof(reference));
				reference.width = width;
				reference.height = height;
				reference.depth = 1;
				reference.stride = width * 4;
				reference.format = 11; // should correspond to DXGI_FORMAT_B8G8R8A8_UNORM
				reference.pixelData = (uint8_t*)pixelData;

				auto tex = std::make_shared<RuntimeTex>(rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr), pixelData, width * height * 4);
				m_txd->Add(name, tex->GetTexture());

				m_textures[name] = tex;

				scrBindAddSafePointer(tex.get());
				return tex.get();
			}
		}
	}

	return nullptr;
}

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	rage::ResourceFlags flags; // out
};

void DLL_IMPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

static hook::cdecl_stub<void(fwArchetype*)> registerArchetype([]()
{
	return hook::get_pattern("48 8B D9 8A 49 60 80 F9", -11);
});

static void* MakeStructFromMsgPack(uint32_t hash, const std::map<std::string, msgpack::object>& data, void* old = nullptr);

static bool FillStructure(rage::parStructure* structure, const std::function<bool(rage::parMember* member)>& fn)
{
	if (structure->m_baseClass)
	{
		if (!FillStructure(structure->m_baseClass, fn))
		{
			return false;
		}
	}

	for (auto& member : structure->m_members)
	{
		if (!fn(member))
		{
			return false;
		}
	}

	return true;
}

template<typename T>
static bool SetValue(void* ptr, const msgpack::object& obj)
{
	*(T*)ptr = obj.as<T>();

	return true;
}

struct Vector2
{
	float x;
	float y;

	Vector2(float x, float y, float z, float w)
		: x(x), y(y)
	{

	}
};

struct Vector3f
{
	float x;
	float y;
	float z;

	Vector3f(float x, float y, float z, float w)
		: x(x), y(y), z(z)
	{

	}
};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
	{

	}
};

template<typename T>
static bool SetVector(void* ptr, const msgpack::object& obj)
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	if (obj.type == msgpack::type::EXT)
	{
		if (obj.via.ext.type() == 20 && obj.via.ext.size == 8) // vector2
		{
			x = *(float*)(obj.via.ext.data() + 0);
			y = *(float*)(obj.via.ext.data() + 4);
		}
		else if (obj.via.ext.type() == 21 && obj.via.ext.size == 12) // vector3
		{
			x = *(float*)(obj.via.ext.data() + 0);
			y = *(float*)(obj.via.ext.data() + 4);
			z = *(float*)(obj.via.ext.data() + 8);
		}
		else if ((obj.via.ext.type() == 22 || obj.via.ext.type() == 23) && obj.via.ext.size == 16) // vector4/quat
		{
			x = *(float*)(obj.via.ext.data() + 0);
			y = *(float*)(obj.via.ext.data() + 4);
			z = *(float*)(obj.via.ext.data() + 8);
			w = *(float*)(obj.via.ext.data() + 12);
		}
	}

	*(T*)ptr = T{ x, y, z, w };

	return true;
}

static bool SetString(void* ptr, const msgpack::object& obj)
{
	if (obj.type == msgpack::type::STR || obj.type == msgpack::type::BIN)
	{
		*(uint32_t*)ptr = HashString(obj.as<std::string>().c_str());
	}
	else if (obj.type == msgpack::type::POSITIVE_INTEGER || obj.type == msgpack::type::NEGATIVE_INTEGER)
	{
		*(uint32_t*)ptr = obj.as<int32_t>();
	}

	return true;
}

static bool SetValueEnum(void* ptr, const msgpack::object& obj, rage::parEnumDefinition* enum_)
{
	uint32_t str = 0;

	if (obj.type == msgpack::type::STR || obj.type == msgpack::type::BIN)
	{
		str = HashRageString(obj.as<std::string>().c_str());
	}
	else if (obj.type == msgpack::type::POSITIVE_INTEGER || obj.type == msgpack::type::NEGATIVE_INTEGER)
	{
		str = obj.as<int32_t>();
	}

	for (auto p = enum_->fields; p->index != 0xFFFFFFFF; p++)
	{
		if (p->hash == str)
		{
			*(uint32_t*)ptr = p->index;
			return true;
		}
	}

	return false;
}

static bool SetStruct(void* ptr, const msgpack::object& obj, rage::parMemberDefinition* member)
{
	if (obj.type != msgpack::type::MAP)
	{
		return false;
	}

	auto structData = obj.as<std::map<std::string, msgpack::object>>();

	if (member->structType == rage::parStructType::Inline)
	{
		MakeStructFromMsgPack(member->structure->m_nameHash, structData, ptr);
	}
	else
	{
		*(void**)ptr = MakeStructFromMsgPack(member->structure->m_nameHash, structData);
	}

	return true;
}

template<typename TIndex>
struct atArrayBase
{
	void* offset;
	TIndex count;
	TIndex size;

	atArrayBase(int count, int size)
	{
		offset = rage::GetAllocator()->allocate(count * size, 16, 0);
		count = count;
		size = count;
	}

	~atArrayBase()
	{
		if (offset)
		{
			rage::GetAllocator()->free(offset);
			offset = nullptr;
		}
	}
};

using atArray_16 = atArrayBase<uint16_t>;
using atArray_32 = atArrayBase<uint32_t>;

static bool SetFromMsgPack(rage::parMember* memberBase, void* structVal, const msgpack::object& value);

static bool SetArray(void* ptr, const msgpack::object& value, rage::parMember* member)
{
	if (value.type != msgpack::type::ARRAY)
	{
		return false;
	}

	// set up the array
	auto arrayData = value.as<std::vector<msgpack::object>>();

	char* arrayBase = nullptr;
	uint32_t arrayElemSize = 0;

	switch (member->m_definition->arrayType)
	{
	case rage::parArrayType::atArray:
		*(atArray_16*)ptr = atArray_16{int(arrayData.size()), int(member->m_definition->arrayElemSize)};
		arrayBase = (char*)(((atArray_16*)ptr)->offset);
		arrayElemSize = member->m_definition->arrayElemSize;

		break;
	case rage::parArrayType::FixedTrailingCount:
		trace("Unhandled array type: FixedTrailingCount\n");
		return false;
	case rage::parArrayType::Fixed:
		arrayBase = (char*)ptr;
		arrayElemSize = member->m_definition->arrayElemSize;
		break;
	case rage::parArrayType::FixedPointer:
		trace("Unhandled array type: FixedPointer\n");
		return false;
	case rage::parArrayType::Fixed_2:
		arrayBase = (char*)ptr;
		arrayElemSize = member->m_definition->arrayElemSize;
		break;
	case rage::parArrayType::atArray32:
		*(atArray_32*)ptr = atArray_32{ int(arrayData.size()), int(member->m_definition->arrayElemSize) };
		arrayBase = (char*)(((atArray_32*)ptr)->offset);
		arrayElemSize = member->m_definition->arrayElemSize;

		break;
	case rage::parArrayType::TrailerInt:
		trace("Unhandled array type: TrailerInt\n");
		return false;
	case rage::parArrayType::TrailerByte:
		trace("Unhandled array type: TrailerByte\n");
		return false;
	case rage::parArrayType::TrailerShort:
		trace("Unhandled array type: TrailerShort\n");
		return false;
	default:
		break;

	}

	size_t offset = 0;
	for (const auto& entry : arrayData)
	{
		void* entryPtr = arrayBase + offset;
		offset += arrayElemSize;

		if (!SetFromMsgPack(member->m_arrayDefinition, entryPtr, entry))
		{
			return false;
		}
	}

	return true;
}

static void* MakeStructFromMsgPack(const char* structType, const std::map<std::string, msgpack::object>& data, void* old = nullptr)
{
	return MakeStructFromMsgPack(HashRageString(structType), data, old);
}

static bool SetFromMsgPack(rage::parMember* memberBase, void* structVal, const msgpack::object& value)
{
	auto member = memberBase->m_definition;

	switch (member->type)
	{
	case rage::parMemberType::Float:
		return SetValue<float>(structVal, value);
	case rage::parMemberType::String:
		return SetString(structVal, value);
	case rage::parMemberType::Vector3:
		return SetVector<Vector3f>(structVal, value);
	case rage::parMemberType::Bool:
		return SetValue<bool>(structVal, value);
	case rage::parMemberType::Int8:
		return SetValue<int8_t>(structVal, value);
	case rage::parMemberType::UInt8:
		return SetValue<uint8_t>(structVal, value);
	case rage::parMemberType::Int16:
		return SetValue<int16_t>(structVal, value);
	case rage::parMemberType::UInt16:
		return SetValue<uint16_t>(structVal, value);
	case rage::parMemberType::Int32:
		return SetValue<int32_t>(structVal, value);
	case rage::parMemberType::UInt32:
		return SetValue<uint32_t>(structVal, value);
	case rage::parMemberType::Vector2:
		return SetVector<Vector2>(structVal, value);
	case rage::parMemberType::Vector4:
		return SetVector<Vector4>(structVal, value);
	case rage::parMemberType::Struct:
		return SetStruct(structVal, value, member);
	case rage::parMemberType::Array:
		return SetArray(structVal, value, memberBase);
	case rage::parMemberType::Enum:
		return SetValueEnum(structVal, value, member->enumData);
	case rage::parMemberType::Bitset:
		break;
	case rage::parMemberType::Map:
		break;
	case rage::parMemberType::Matrix4x3:
		break;
	case rage::parMemberType::Matrix4x4:
		break;
	case rage::parMemberType::Vector2_Padded:
		return SetVector<Vector4>(structVal, value);
	case rage::parMemberType::Vector3_Padded:
		return SetVector<Vector4>(structVal, value);
	case rage::parMemberType::Vector4_Padded:
		return SetVector<Vector4>(structVal, value);
	case rage::parMemberType::Matrix3x4:
		break;
	case rage::parMemberType::Matrix4x3_2:
		break;
	case rage::parMemberType::Matrix4x4_2:
		break;
	case rage::parMemberType::Vector1_Padded:
		break;
	case rage::parMemberType::Flag1_Padded:
		break;
	case rage::parMemberType::Flag4_Padded:
		break;
	case rage::parMemberType::Int32_64:
		return SetValue<int64_t>(structVal, value);
	case rage::parMemberType::Int32_U64:
		return SetValue<uint64_t>(structVal, value);
	case rage::parMemberType::Half:
		trace("Unhandled parMemberType::Half\n");
		break;
	case rage::parMemberType::Int64:
		return SetValue<int64_t>(structVal, value);
	case rage::parMemberType::UInt64:
		return SetValue<uint64_t>(structVal, value);
	case rage::parMemberType::Double:
		return SetValue<double>(structVal, value);
	default:
		break;
	}

	return true;
}

static void* MakeStructFromMsgPack(uint32_t hash, const std::map<std::string, msgpack::object>& data, void* old)
{
	std::string structTypeReal;

	{
		auto typeIt = data.find("__type");

		if (typeIt != data.end())
		{
			structTypeReal = typeIt->second.as<std::string>();
		}
	}

	auto structDef = rage::GetStructureDefinition(hash);

	if (!structTypeReal.empty())
	{
		structDef = rage::GetStructureDefinition(structTypeReal.c_str());
	}

	if (!structDef)
	{
		return nullptr;
	}

	auto retval = (!old) ? structDef->m_new() : structDef->m_placementNew(old);

	std::map<uint32_t, msgpack::object> mappedData;

	for (auto& entry : data)
	{
		mappedData[HashRageString(entry.first.c_str())] = std::move(entry.second);
	}

	if (retval)
	{
		FillStructure(structDef, [retval, &mappedData](rage::parMember* memberBase)
		{
			auto member = memberBase->m_definition;
			auto structVal = (char*)retval + member->offset;

			auto it = mappedData.find(member->hash);

			if (it == mappedData.end())
			{
				return true;
			}

			const msgpack::object& value = it->second;

			return SetFromMsgPack(memberBase, structVal, value);
		});
	}

	return retval;
}

static hook::cdecl_stub<CMapDataContents*()> makeMapDataContents([]()
{
	return hook::pattern("48 00 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 14").count(1).get(0).get<void>(-7);
});

static hook::cdecl_stub<void(CMapDataContents*, CMapData*, bool, bool)> addToScene([]()
{
	return hook::pattern("48 83 EC 50 83 79 18 00 0F 29 70 C8 41 8A F1").count(1).get(0).get<void>(-0x18);
});

static hook::cdecl_stub<void*(fwEntityDef*, int fileIdx, fwArchetype* archetype, uint64_t* archetypeUnk)> fwEntityDef__instantiate([]()
{
	return hook::get_call(hook::pattern("4C 8D 4C 24 40 4D 8B C6 41 8B D7 48 8B CF").count(1).get(0).get<void>(14));
});

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, uint64_t* archetypeUnk)> getArchetype([]()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

fwArchetype* GetArchetypeSafe(uint32_t archetypeHash, uint64_t* archetypeUnk)
{
	__try
	{
		return getArchetype(archetypeHash, archetypeUnk);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return nullptr;
	}
}

static InitFunction initFunction([]()
{
	scrBindClass<RuntimeTxd>()
		.AddConstructor<void(*)(const char*)>("CREATE_RUNTIME_TXD")
		.AddMethod("CREATE_RUNTIME_TEXTURE", &RuntimeTxd::CreateTexture)
		.AddMethod("CREATE_RUNTIME_TEXTURE_FROM_IMAGE", &RuntimeTxd::CreateTextureFromImage)
		.AddMethod("CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE", &RuntimeTxd::CreateTextureFromDui);

	scrBindClass<RuntimeTex>()
		.AddMethod("GET_RUNTIME_TEXTURE_WIDTH", &RuntimeTex::GetWidth)
		.AddMethod("GET_RUNTIME_TEXTURE_HEIGHT", &RuntimeTex::GetHeight)
		.AddMethod("GET_RUNTIME_TEXTURE_PITCH", &RuntimeTex::GetPitch)
		.AddMethod("SET_RUNTIME_TEXTURE_PIXEL", &RuntimeTex::SetPixel)
		.AddMethod("SET_RUNTIME_TEXTURE_ARGB_DATA", &RuntimeTex::SetPixelData)
		.AddMethod("COMMIT_RUNTIME_TEXTURE", &RuntimeTex::Commit);

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_ARCHETYPES", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		std::string factoryRef = context.CheckArgument<const char*>(0);

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				msgpack::unpacked unpacked;
				auto factoryObject = resource->GetManager()->CallReferenceUnpacked<msgpack::object>(factoryRef, &unpacked);

				if (factoryObject.type != msgpack::type::ARRAY && factoryObject.type != msgpack::type::MAP)
				{
					throw std::runtime_error("Wrong type in REGISTER_ARCHETYPES.");
				}

				std::vector<std::map<std::string, msgpack::object>> archetypes =
					(factoryObject.type == msgpack::type::ARRAY) ?
						factoryObject.as<std::vector<std::map<std::string, msgpack::object>>>() :
						std::vector<std::map<std::string, msgpack::object>>{ factoryObject.as<std::map<std::string, msgpack::object>>() };

				for (const auto& archetypeData : archetypes)
				{
					fwArchetypeDef* archetypeDef = (fwArchetypeDef*)MakeStructFromMsgPack("CBaseArchetypeDef", archetypeData);

					// assume this is a CBaseModelInfo
					// TODO: get [mi] from [miPtr]
					void* miPtr = g_archetypeFactories->Get(1)->GetOrCreate(archetypeDef->name, 1);

					fwArchetype* mi = g_archetypeFactories->Get(1)->Get(archetypeDef->name);

					mi->InitializeFromArchetypeDef(1390, archetypeDef, true);

					// TODO: clean up
					mi->flags &= ~(1 << 31);

					// register the archetype in the streaming module
					registerArchetype(mi);
				}
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_ENTITIES", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		std::string factoryRef = context.CheckArgument<const char*>(0);

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				msgpack::unpacked unpacked;
				auto factoryObject = resource->GetManager()->CallReferenceUnpacked<msgpack::object>(factoryRef, &unpacked);

				if (factoryObject.type != msgpack::type::ARRAY && factoryObject.type != msgpack::type::MAP)
				{
					throw std::runtime_error("Wrong type in REGISTER_ENTITIES.");
				}

				std::vector<std::map<std::string, msgpack::object>> entities =
					(factoryObject.type == msgpack::type::ARRAY) ?
					factoryObject.as<std::vector<std::map<std::string, msgpack::object>>>() :
					std::vector<std::map<std::string, msgpack::object>>{ factoryObject.as<std::map<std::string, msgpack::object>>() };

				static int idx;
				std::string nameRef = fmt::sprintf("reg_ents_%d", idx++);

				CMapData* mapData = new CMapData();

				// 1604, temp
				*(uintptr_t*)mapData = 0x1419343E0;
				mapData->name = HashString(nameRef.c_str());
				mapData->contentFlags = 73;

				float aabbMin[3];
				float aabbMax[3];

				aabbMin[0] = FLT_MAX;
				aabbMin[1] = FLT_MAX;
				aabbMin[2] = FLT_MAX;

				aabbMax[0] = 0.0f - FLT_MAX;
				aabbMax[1] = 0.0f - FLT_MAX;
				aabbMax[2] = 0.0f - FLT_MAX;

				mapData->entities.Expand(entities.size());

				size_t i = 0;

				for (const auto& entityData : entities)
				{
					fwEntityDef* entityDef = (fwEntityDef*)MakeStructFromMsgPack("CEntityDef", entityData);
					mapData->entities.Set(i, entityDef);

					uint64_t archetypeUnk = 0xFFFFFFF;
					fwArchetype* archetype = GetArchetypeSafe(entityDef->archetypeName, &archetypeUnk);

					if (archetype)
					{
						float radius = archetype->radius;

						if (archetype->radius < 0.01f)
						{
							radius = 250.f;
						}

						// update AABB
						float xMin = entityDef->position[0] - radius;
						float yMin = entityDef->position[1] - radius;
						float zMin = entityDef->position[2] - radius;

						float xMax = entityDef->position[0] + radius;
						float yMax = entityDef->position[1] + radius;
						float zMax = entityDef->position[2] + radius;

						aabbMin[0] = (xMin < aabbMin[0]) ? xMin : aabbMin[0];
						aabbMin[1] = (yMin < aabbMin[1]) ? yMin : aabbMin[1];
						aabbMin[2] = (zMin < aabbMin[2]) ? zMin : aabbMin[2];

						aabbMax[0] = (xMax > aabbMax[0]) ? xMax : aabbMax[0];
						aabbMax[1] = (yMax > aabbMax[1]) ? yMax : aabbMax[1];
						aabbMax[2] = (zMax > aabbMax[2]) ? zMax : aabbMax[2];
					}

					i++;
				}

				mapData->entitiesExtentsMin[0] = aabbMin[0];
				mapData->entitiesExtentsMin[1] = aabbMin[1];
				mapData->entitiesExtentsMin[2] = aabbMin[2];

				mapData->entitiesExtentsMax[0] = aabbMax[0];
				mapData->entitiesExtentsMax[1] = aabbMax[1];
				mapData->entitiesExtentsMax[2] = aabbMax[2];

				mapData->streamingExtentsMin[0] = aabbMin[0];
				mapData->streamingExtentsMin[1] = aabbMin[1];
				mapData->streamingExtentsMin[2] = aabbMin[2];

				mapData->streamingExtentsMax[0] = aabbMax[0];
				mapData->streamingExtentsMax[1] = aabbMax[1];
				mapData->streamingExtentsMax[2] = aabbMax[2];

				auto mapTypesStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");
				auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
				
				uint32_t mehId;
				mapTypesStore->FindSlot(&mehId, "v_int_1");

				uint32_t mapId;
				mapDataStore->FindSlotFromHashKey(&mapId, nameRef.c_str());

				if (mapId != -1)
				{
					void* unkRef[4] = { 0 };
					unkRef[0] = mapData;

					// 1604, temp (pso store placement cookie)
					((void(*)(void*, uint32_t, const void*))0x14158FCD4)((void*)0x142DC9678, mapId + mapDataStore->baseIdx, unkRef);

					auto pool = (atPoolBase*)((char*)mapDataStore + 56);
					*(int32_t*)(pool->GetAt<char>(mapId) + 32) |= 2048;
					*(int16_t*)(pool->GetAt<char>(mapId) + 38) = 1;
					*(int32_t*)(pool->GetAt<char>(mapId) + 24) = mehId; // TODO: FIGURE OUT

					//auto contents = (CMapDataContents*)mapDataStore->GetPtr(mapId);
					auto mapMeta = (void*)mapDataStore->GetDataPtr(mapId); // not sure?

					// TODO: leak
					mapData->CreateMapDataContents()->PrepareInteriors(mapMeta, mapData, mapId);
					
					// reference is ignored but we pass it for formality - it actually uses PSO store placement cookies
					streaming::strAssetReference ref;
					ref.asset = mapData;

					mapDataStore->SetResource(mapId, ref);
					streaming::Manager::GetInstance()->Entries[mapId + mapDataStore->baseIdx].flags |= (512 << 8) | 1;

					// 1604
					((void(*)(int))0x1408CF07C)(0);

					((void(*)(void*))((*(void***)0x142DCA970)[2]))((void*)0x142DCA970);
				}
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_STREAMING_FILE_FROM_CACHE", [](fx::ScriptContext& context)
	{
		std::string resourceName = context.CheckArgument<const char*>(0);
		std::string fileName = context.CheckArgument<const char*>(1);
		std::string dataString = context.CheckArgument<const char*>(2);

		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto resource = resourceManager->GetResource(resourceName);

		if (!resource.GetRef())
		{
			throw std::runtime_error("No valid resource name in REGISTER_STREAMING_FILE_FROM_CACHE.");
			return;
		}

		auto json = nlohmann::json::parse(dataString);

		uint32_t version;
		uint32_t pagesPhysical;
		uint32_t pagesVirtual;

		if (json.value("isResource", false))
		{
			pagesPhysical = json.value<uint32_t>("rscPagesPhysical", 0);
			pagesVirtual = json.value<uint32_t>("rscPagesVirtual", 0);
			version = json.value<uint32_t>("rscVersion", 0);
		}
		else
		{
			version = 0;
			pagesPhysical = 0;
			pagesVirtual = json.value<uint32_t>("size", 0);
		}

		ResourceCacheEntryList::Entry rclEntry;
		rclEntry.basename = fileName;
		rclEntry.referenceHash = json.value("hash", "");
		rclEntry.remoteUrl = fmt::sprintf("https://%s/files/%s/%s", Instance<NetLibrary>::Get()->GetCurrentPeer().ToString(), resourceName, fileName);
		rclEntry.resourceName = resource->GetName();
		rclEntry.size = json.value("size", -1);
		rclEntry.extData = {
			{ "rscVersion", std::to_string(version) },
			{ "rscPagesPhysical", std::to_string(pagesPhysical) },
			{ "rscPagesVirtual", std::to_string(pagesVirtual) },
		};

		resource->GetComponent<ResourceCacheEntryList>()->AddEntry(rclEntry);

		rage::ResourceFlags flags{ pagesVirtual, pagesPhysical };

		// TODO: don't use compcache if not using the adhesive mounter!
		CfxCollection_AddStreamingFileByTag(resource->GetName(), fmt::sprintf("compcache:/%s/%s", resourceName, fileName), flags);
	});

	static concurrency::concurrent_unordered_set<std::string> g_readyStreamFiles;

	fx::ScriptEngine::RegisterNativeHandler("IS_STREAMING_FILE_READY", [](fx::ScriptContext& context)
	{
		std::string fileName = context.CheckArgument<const char*>(0);

		context.SetResult<int>(g_readyStreamFiles.find(fileName) != g_readyStreamFiles.end());
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_STREAMING_FILE_FROM_KVS", [](fx::ScriptContext& context)
	{
		std::string key = context.CheckArgument<const char*>(0);

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_FAILED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			throw std::runtime_error("no current script runtime");
		}

		auto resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		std::string fileName = fmt::sprintf("kvs:/%s/%s", resource->GetName(), key);

		GetRagePageFlagsExtension ext;
		ext.fileName = fileName.c_str();

		auto device = vfs::GetDevice(fileName);

		uint32_t rscPagesVirtual = device->GetLength(fileName);
		uint32_t rscPagesPhysical = 0;

		if (device->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &ext, sizeof(ext)))
		{
			rscPagesVirtual = ext.flags.flag1;
			rscPagesPhysical = ext.flags.flag2;
		}

		rage::ResourceFlags flags{ rscPagesVirtual, rscPagesPhysical };
		CfxCollection_AddStreamingFileByTag(resource->GetName(), fileName, flags);

		g_readyStreamFiles.insert(fileName);
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_STREAMING_FILE_FROM_URL", [](fx::ScriptContext& context)
	{
		std::string fileName = context.CheckArgument<const char*>(0);
		std::string sourceUrl = context.CheckArgument<const char*>(1);

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_FAILED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			throw std::runtime_error("no current script runtime");
		}

		auto resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		auto headerList = std::make_shared<HttpHeaderList>();

		HttpRequestOptions options;
		options.headers["range"] = "bytes=0-15";
		options.responseHeaders = headerList;

		Instance<HttpClient>::Get()->DoGetRequest(sourceUrl, options, [fileName, sourceUrl, resource, headerList](bool success, const char* data, size_t size)
		{
			if (success)
			{
				struct
				{
					uint32_t magic;
					uint32_t version;
					uint32_t virtPages;
					uint32_t physPages;
				} rsc7Header;

				memcpy(&rsc7Header, data, sizeof(rsc7Header));

				auto it = headerList->find("content-range");
				size_t length = 0;

				if (it == headerList->end())
				{
					it = headerList->find("content-length");
					length = atoi(it->second.c_str());

					if (it == headerList->end())
					{
						trace("Invalid HTTP response from %s.\n", sourceUrl);
						return;
					}
				}
				else
				{
					if (it->second.length() >= 6 && it->second.find("bytes ") == 0)
					{
						length = atoi(it->second.substr(it->second.find_last_of("/") + 1).c_str());
					}
				}

				bool isResource = false;
				uint32_t rscVersion = 0;
				uint32_t rscPagesPhysical = 0;
				uint32_t rscPagesVirtual = length;

				if (rsc7Header.magic == 0x37435352) // RSC7
				{
					rscVersion = rsc7Header.version;
					rscPagesPhysical = rsc7Header.physPages;
					rscPagesVirtual = rsc7Header.virtPages;
					isResource = true;
				}

				// TODO: marshal this to the main thread
				ResourceCacheEntryList::Entry rclEntry;
				rclEntry.basename = fileName;
				rclEntry.referenceHash = ""; // empty reference hash should trigger RCD to use the source URL as key
				rclEntry.remoteUrl = sourceUrl;
				rclEntry.resourceName = resource->GetName();
				rclEntry.size = length;
				rclEntry.extData = {
					{ "rscVersion", std::to_string(rscVersion) },
					{ "rscPagesPhysical", std::to_string(rscPagesPhysical) },
					{ "rscPagesVirtual", std::to_string(rscPagesVirtual) },
				};

				resource->GetComponent<ResourceCacheEntryList>()->AddEntry(rclEntry);

				rage::ResourceFlags flags{ rscPagesVirtual, rscPagesPhysical };

				CfxCollection_AddStreamingFileByTag(resource->GetName(), fmt::sprintf("cache:/%s/%s", resource->GetName(), fileName), flags);

				g_readyStreamFiles.insert(fileName);
			}
		});
	});
});
