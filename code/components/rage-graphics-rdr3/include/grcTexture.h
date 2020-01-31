#pragma once

struct ID3D12Resource;

#include <sysAllocator.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#ifdef COMPILING_RAGE_GRAPHICS_RDR3
#define GFX_EXPORT DLL_EXPORT
#else
#define GFX_EXPORT DLL_IMPORT
#endif

namespace rage
{
	namespace sga
	{
		class GFX_EXPORT Texture
		{
		public:
			virtual ~Texture() = default;

			inline static bool IsRenderSystemColorSwapped()
			{
				return false;
			}
		};

		enum class BufferFormat : uint8_t
		{
			UNKNOWN = 0,
			R32G32B32A32_TYPELESS,
			R32G32B32A32_FLOAT,
			R32G32B32A32_UINT,
			R32G32B32A32_SINT,
			R32G32B32_TYPELESS,
			R32G32B32_FLOAT,
			R32G32B32_UINT,
			R32G32B32_SINT,
			R16G16B16A16_TYPELESS,
			R16G16B16A16_FLOAT,
			R16G16B16A16_UNORM,
			R16G16B16A16_UINT,
			R16G16B16A16_SNORM,
			R16G16B16A16_SINT,
			R32G32_TYPELESS,
			R32G32_FLOAT,
			R32G32_UINT,
			R32G32_SINT,
			D32_FLOAT_S8X24_UINT = 20,
			B10G10R10A2_UNORM,
			R10G10B10A2_SNORM,
			R10G10B10A2_TYPELESS,
			R10G10B10A2_UNORM,
			R10G10B10A2_UINT,
			R11G11B10_FLOAT,
			R8G8B8A8_TYPELESS,
			R8G8B8A8_UNORM,
			R8G8B8A8_UNORM_SRGB,
			R8G8B8A8_UINT,
			R8G8B8A8_SNORM,
			R8G8B8A8_SINT,
			R16G16_TYPELESS,
			R16G16_FLOAT,
			R16G16_UNORM,
			R16G16_UINT,
			R16G16_SNORM,
			R16G16_SINT,
			R32_TYPELESS,
			D32_FLOAT,
			R32_FLOAT,
			R32_UINT,
			R32_SINT,
			R8G8_TYPELESS = 48,
			R8G8_UNORM,
			R8G8_UINT,
			R8G8_SNORM,
			R8G8_SINT,
			R16_TYPELESS,
			R16_FLOAT,
			D16_UNORM,
			R16_UNORM,
			R16_UINT,
			R16_SNORM,
			R16_SINT,
			R8_TYPELESS,
			R8_UNORM,
			R8_UINT,
			R8_SNORM,
			R8_SINT,
			A8_UNORM,
			R9G9B9E5_SHAREDEXP = 67,
			BC1_TYPELESS = 70,
			BC1_UNORM,
			BC1_UNORM_SRGB,
			BC2_TYPELESS,
			BC2_UNORM,
			BC2_UNORM_SRGB,
			BC3_TYPELESS,
			BC3_UNORM,
			BC3_UNORM_SRGB,
			BC4_TYPELESS,
			BC4_UNORM,
			BC4_SNORM,
			BC5_TYPELESS,
			BC5_UNORM,
			BC5_SNORM,
			B5G6R5_UNORM,
			B5G5R5A1_UNORM,
			B8G8R8A8_UNORM,
			B8G8R8A8_TYPELESS = 90,
			B8G8R8A8_UNORM_SRGB,
			BC6H_TYPELESS = 94,
			BC6H_UF16,
			BC6H_SF16,
			BC7_TYPELESS,
			BC7_UNORM,
			BC7_UNORM_SRGB,
			NV12 = 103,
			B4G4R4A4_UNORM = 115,
			D16_UNORM_S8_UINT = 118,
			R16_UNORM_X8_TYPELESS,
			X16_TYPELESS_G8_UINT,
			ETC1,
			ETC1_SRGB,
			ETC1A,
			ETC1A_SRGB,
			R4G4_UNORM = 127,
		};

		struct ImageParams
		{
			uint16_t width;
			uint16_t height;
			uint16_t depth;
			uint8_t dimension; // 1 for tex2d?
			BufferFormat bufferFormat;
			uint8_t tileMode; // 0xFF
			uint8_t antiAliasType; // 0
			uint8_t levels;
			bool unkFlag; // format-related
			uint8_t unk12;
			uint8_t unk13;

			ImageParams()
			{
				tileMode = 0xFF;
				antiAliasType = 0;
				unkFlag = false;
				unk12 = 0;
				unk13 = 0;
			}
		};

		class GFX_EXPORT Factory
		{
		public:
			static Texture* CreateTexture(const char* name, const ImageParams& params, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue, const void* conversionInfo, Texture* other);
		};

		class TextureD3D12 : public Texture
		{
		public:
			char pad[64];
			ID3D12Resource* resource;
		};

		class TextureVK : public Texture
		{
		public:
			struct ImageData : public sysUseAllocator
			{
				VkDeviceMemory memory;
				VkImage image;
				uint32_t pad[24];
			};

			char pad[64];
			ImageData* image;
		};

		enum BufferType
		{
			Image = 2
		};

		struct TextureViewDesc
		{
			BufferType bufferType; // +0
			int mostDetailedMip; // +4
			int mipLevels; // +8
			int arrayStart; // +12
			int arraySize; // +16
			float minLodClamp; // +20
			bool unkSingleChannel; // +24
			uint8_t dimension;
			uint8_t formatOverride;

			inline TextureViewDesc()
			{
				bufferType = BufferType::Image;
				mostDetailedMip = 0;
				mipLevels = 0xF;
				arrayStart = 0xFFF;
				minLodClamp = 0.0f;
				arraySize = 1;
				unkSingleChannel = false;
				dimension = 0;
				formatOverride = 0;
			}
		};

		void GFX_EXPORT Driver_Create_ShaderResourceView(Texture* texture, const TextureViewDesc& desc);

		void GFX_EXPORT Driver_Destroy_Texture(Texture* texture);
	}

	// rage::grcImage, in reality
	class grcTextureReference
	{
	public:
		int format; // also some flags in higher bits
		int fFlags;
		uint16_t width;
		uint16_t height;
		uint16_t depth;

		uint32_t stride;

		uint32_t pad4;

		uint8_t* pixelData; // 24

		uintptr_t pad3;

		grcTextureReference* nextMipLevel;
		grcTextureReference* nextMajorLevel;

		char pad[112]; // size = 112, more pad
	};

	class grcTexture : public sga::Texture
	{

	};

	struct grcManualTextureDef
	{
		int isStaging;
		char pad[32];
		int arraySize;
		char pad2[16];
	};

	class GFX_EXPORT grcTextureFactory
	{
	public:
		static grcTextureFactory* getInstance();

		// Create
		inline grcTexture* createImage(grcTextureReference* reference, void* createParams)
		{
			return createImage("grcImage", reference, createParams);
		}

		grcTexture* createImage(const char* name, grcTextureReference* reference, void* createParams);

		grcTexture* createManualTexture(short width, short height, int format, void* unknown, bool, const grcManualTextureDef* templ);

		static grcTexture* GetNoneTexture();
	};
}
