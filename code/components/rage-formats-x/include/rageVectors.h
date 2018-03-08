/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace rage
{
	struct Vector3
	{
		float x, y, z;

	private:
		float __pad;

	public:
		Vector3(float x, float y, float z)
			: x(x), y(y), z(z), __pad(NAN)
		{

		}

		Vector3()
			: Vector3(0.0f, 0.0f, 0.0f)
		{

		}

		inline Vector3 operator*(const float value) const
		{
			return Vector3(x * value, y * value, z * value);
		}

		inline Vector3 operator*(const Vector3& value) const
		{
			return Vector3(x * value.x, y * value.y, z * value.z);
		}

		inline Vector3 operator+(const Vector3& right) const
		{
			return Vector3(x + right.x, y + right.y, z + right.z);
		}

		inline Vector3 operator-(const Vector3& right) const
		{
			return Vector3(x - right.x, y - right.y, z - right.z);
		}

		inline void operator*=(const float value)
		{
			x *= value;
			y *= value;
			z *= value;
		}

		inline void operator+=(const Vector3& right)
		{
			x += right.x;
			y += right.y;
			z += right.z;
		}

		inline void operator-=(const Vector3& right)
		{
			x -= right.x;
			y -= right.y;
			z -= right.z;
		}

		inline int MaxAxis() const
		{
			return x < y ? (y < z ? 2 : 1) : (x < z ? 2 : 0);
		}

		inline float operator[](int idx) const
		{
			switch (idx)
			{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				return NAN;
			}
		}

		inline static Vector3 CrossProduct(const Vector3& v1, const Vector3& v2)
		{
			return Vector3(
				(v1.y * v2.z) - (v1.z * v2.y),
				(v1.z * v2.x) - (v1.x * v2.z),
				(v1.x * v2.y) - (v1.y * v2.x)
			);
		}

		inline float Length() const
		{
			return sqrtf((x * x) + (y * y) + (z * z));
		}
	};

	struct Vector4
	{
		float x, y, z, w;

	public:
		Vector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{

		}

		Vector4()
			: Vector4(0.0f, 0.0f, 0.0f, 1.0f)
		{

		}
	};
}
