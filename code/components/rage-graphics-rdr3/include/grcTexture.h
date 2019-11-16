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

		static grcTexture* GetNoneTexture();
	};
}
