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