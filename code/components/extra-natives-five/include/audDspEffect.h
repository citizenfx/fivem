#pragma once

#include <array>

namespace rage
{
struct audDspEffectBuffer
{
	std::array<float, 256>* channels[6];
};

class audDspEffect
{
public:
	virtual ~audDspEffect() = default;

	virtual bool Init(uint32_t, uint32_t) = 0;

	virtual void Shutdown() = 0;

	virtual void Process(audDspEffectBuffer& buffers) = 0;

	virtual void SetParam(uint32_t param, uint32_t value) = 0;

	virtual void SetParam(uint32_t param, float value) = 0;
};
}
