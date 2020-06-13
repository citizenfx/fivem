#pragma once


template<int bits>
struct compressed_quaternion
{
	enum
	{
		max_value = (1 << bits) - 1
	};

	uint32_t largest;
	uint32_t integer_a;
	uint32_t integer_b;
	uint32_t integer_c;

	void Load(float x, float y, float z, float w)
	{
		static_assert(bits > 1, "bits > 1");
		//assert(bits <= 10);

		const float minimum = -1.0f / 1.414214f; // 1.0f / sqrt(2)
		const float maximum = +1.0f / 1.414214f;

		const float scale = float((1 << bits) - 1);

		const float abs_x = fabs(x);
		const float abs_y = fabs(y);
		const float abs_z = fabs(z);
		const float abs_w = fabs(w);

		largest = 0;
		float largest_value = abs_x;

		if (abs_y > largest_value)
		{
			largest = 1;
			largest_value = abs_y;
		}

		if (abs_z > largest_value)
		{
			largest = 2;
			largest_value = abs_z;
		}

		if (abs_w > largest_value)
		{
			largest = 3;
			largest_value = abs_w;
		}

		float a = 0;
		float b = 0;
		float c = 0;

		switch (largest)
		{
			case 0:
				if (x >= 0)
				{
					a = y;
					b = z;
					c = w;
				}
				else
				{
					a = -y;
					b = -z;
					c = -w;
				}
				break;

			case 1:
				if (y >= 0)
				{
					a = x;
					b = z;
					c = w;
				}
				else
				{
					a = -x;
					b = -z;
					c = -w;
				}
				break;

			case 2:
				if (z >= 0)
				{
					a = x;
					b = y;
					c = w;
				}
				else
				{
					a = -x;
					b = -y;
					c = -w;
				}
				break;

			case 3:
				if (w >= 0)
				{
					a = x;
					b = y;
					c = z;
				}
				else
				{
					a = -x;
					b = -y;
					c = -z;
				}
				break;

			default:
				assert(false);
		}

		const float normal_a = (a - minimum) / (maximum - minimum);
		const float normal_b = (b - minimum) / (maximum - minimum);
		const float normal_c = (c - minimum) / (maximum - minimum);

		integer_a = floor(normal_a * scale + 0.5f);
		integer_b = floor(normal_b * scale + 0.5f);
		integer_c = floor(normal_c * scale + 0.5f);
	}

	void Save(float& x, float& y, float& z, float& w) const
	{
		// note: you're going to want to normalize the quaternion returned from this function

		static_assert(bits > 1, "bits > 1");
		//assert(bits <= 10);

		const float minimum = -1.0f / 1.414214f; // 1.0f / sqrt(2)
		const float maximum = +1.0f / 1.414214f;

		const float scale = float((1 << bits) - 1);

		const float inverse_scale = 1.0f / scale;

		const float a = integer_a * inverse_scale * (maximum - minimum) + minimum;
		const float b = integer_b * inverse_scale * (maximum - minimum) + minimum;
		const float c = integer_c * inverse_scale * (maximum - minimum) + minimum;

		switch (largest)
		{
			case 0:
			{
				x = sqrtf(1 - a * a - b * b - c * c);
				y = a;
				z = b;
				w = c;
			}
			break;

			case 1:
			{
				x = a;
				y = sqrtf(1 - a * a - b * b - c * c);
				z = b;
				w = c;
			}
			break;

			case 2:
			{
				x = a;
				y = b;
				z = sqrtf(1 - a * a - b * b - c * c);
				w = c;
			}
			break;

			case 3:
			{
				x = a;
				y = b;
				z = c;
				w = sqrtf(1 - a * a - b * b - c * c);
			}
			break;

			default:
			{
				assert(false);
				x = 0;
				y = 0;
				z = 0;
				w = 1;
			}
		}
	}

#ifdef GTA_FIVE
	void Serialize(rage::CSyncDataBase* stream)
	{
		stream->SerialiseUnsigned(largest, 2, "Largest Component");
		stream->SerialiseUnsigned(integer_a, bits, "Component A");
		stream->SerialiseUnsigned(integer_b, bits, "Component B");
		stream->SerialiseUnsigned(integer_c, bits, "Component C");
	}
#endif

	bool operator==(const compressed_quaternion& other) const
	{
		if (largest != other.largest)
			return false;

		if (integer_a != other.integer_a)
			return false;

		if (integer_b != other.integer_b)
			return false;

		if (integer_c != other.integer_c)
			return false;

		return true;
	}

	bool operator!=(const compressed_quaternion& other) const
	{
		return !(*this == other);
	}
};
