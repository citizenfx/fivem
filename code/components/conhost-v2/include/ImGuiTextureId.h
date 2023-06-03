#pragma once

struct ID3D11ShaderResourceView;

namespace rage
{
class grcTexture;
}

namespace conhost
{
struct ImGuiTexture
{
	static constexpr const uint32_t Magic = 'IMTX';
	uint32_t magic = Magic;
	void* gameTexture = nullptr;
	void* extTexture = nullptr;

	inline explicit ImGuiTexture(void* gameTexture, void* extTexture)
		: gameTexture(gameTexture), extTexture(extTexture)
	{

	}

	static inline bool IsTexture(const void* ptr)
	{
		return ptr && *reinterpret_cast<const uint32_t*>(ptr) == Magic;
	}

	static rage::grcTexture* ToGrcoreTexture(void* textureId);
	static ID3D11ShaderResourceView* ToShaderResourceView(void* textureId);
};

inline ImGuiTexture* MakeImGuiTexture(rage::grcTexture* gameTexture)
{
	return new ImGuiTexture(gameTexture, nullptr);
}

inline ImGuiTexture* MakeImGuiTexture(rage::grcTexture* gameTexture, void* extTexture)
{
	return new ImGuiTexture(gameTexture, extTexture);
}
}
