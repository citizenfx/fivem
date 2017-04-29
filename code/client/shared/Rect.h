/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

///
/// A class to represent a floating-point rectangle.
///
class CRect
{
public:
	/// The values for the rectangle.
	float fX1;
	float fY1;
	float fX2;
	float fY2;

public:
	///
	/// Constructor, initializing the rectangle with the passed values.
	///
	inline CRect(float x1, float y1, float x2, float y2)
		: fX1(x1), fY1(y1), fX2(x2), fY2(y2)
	{

	}

	///
	/// Constructor, initializing the rectangle to be empty.
	///
	inline CRect()
		: CRect(0, 0, 0, 0)
	{

	}

	///
	/// Returns the left edge of the rectangle.
	///
	inline float Left() const { return min(fX1, fX2); }

	///
	/// Returns the right edge of the rectangle.
	///
	inline float Right() const { return max(fX1, fX2); }

	///
	/// Returns the top edge of the rectangle.
	///
	inline float Top() const { return min(fY1, fY2); }

	///
	/// Returns the bottom edge of the rectangle.
	///
	inline float Bottom() const { return max(fY1, fY2); }

	///
	/// Returns the width of the rectangle.
	///
	inline float Width() const { return Right() - Left(); }

	///
	/// Returns the height of the rectangle.
	///
	inline float Height() const { return Bottom() - Top(); }

	///
	/// Overwrites the rectangle with the passed values.
	///
	inline void SetRect(float x1, float y1, float x2, float y2)
	{
		fX1 = x1;
		fY1 = y1;
		fX2 = x2;
		fY2 = y2;
	}

	///
	/// Returns a string representation of the rectangle.
	///
	inline const char* Render() const { return va("CRect(%v, %v, %v, %v)", fX1, fX2, fY1, fY2); }
};