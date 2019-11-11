#pragma once

#ifdef COMPILING_RAGE_GRAPHICS_RDR3
#define GFX_EXPORT DLL_EXPORT
#else
#define GFX_EXPORT DLL_IMPORT
#endif

namespace rage
{
	namespace sga
	{
		class Texture
		{
		public:
			inline static bool IsRenderSystemColorSwapped()
			{
				return false;
			}
		};
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
