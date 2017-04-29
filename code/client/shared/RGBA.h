/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

struct CRGBA
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	inline CRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: red(r), green(g), blue(b), alpha(a)
	{

	}

	inline CRGBA()
		: CRGBA(0, 0, 0, 255)
	{

	}

	inline CRGBA(uint8_t r, uint8_t g, uint8_t b)
		: CRGBA(r, g, b, 255)
	{

	}

	inline static CRGBA FromFloat(float r, float g, float b, float a)
	{
		return CRGBA((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f), (uint8_t)(a * 255.0f));
	}

	inline static CRGBA FromARGB(uint32_t argb)
	{
		return CRGBA((argb & 0xFF0000) >> 16, ((argb & 0xFF00) >> 8), argb & 0xFF, (argb & 0xFF000000) >> 24);
	}

	inline uint32_t AsARGB() const
	{
		return (alpha << 24) | (red << 16) | (green << 8) | blue;
	}
};