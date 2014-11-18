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
		return CRGBA(r * 255.0f, g * 255.0f, b * 255.0f, a * 255.0f);
	}
};