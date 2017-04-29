/* Copyright (C) 2005-2011, Thorvald Natvig <thorvald@natvig.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
- Neither the name of the Mumble Developers nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Modified for CitizenMP by NTAuthority.
 */

#ifndef MUMBLE_PACKETDATASTREAM_H_
#define MUMBLE_PACKETDATASTREAM_H_

/*
* GCC doesn't yet do inter-object-file inlining, so unfortunately, this all has to be defined here.
*/

class PacketDataStream
{
private:
	unsigned char *data;
	uint32_t maxsize;
	uint32_t offset;
	uint32_t overshoot;
	bool ok;
public:
	uint32_t size() const
	{
		return offset;
	}

	uint32_t capacity() const
	{
		return maxsize;
	}

	bool isValid() const
	{
		return ok;
	}

	uint32_t left() const
	{
		return maxsize - offset;
	}

	uint32_t undersize() const
	{
		return overshoot;
	}

	void append(const uint64_t v)
	{
		if (offset < maxsize)
			data[offset++] = static_cast<unsigned char>(v);
		else
		{
			ok = false;
			overshoot++;
		}
	};

	void append(const char *d, uint32_t len)
	{
		if (left() >= len)
		{
			memcpy(&data[offset], d, len);
			offset += len;
		}
		else
		{
			int l = left();
			memset(&data[offset], 0, l);
			offset += l;
			overshoot += len - l;
			ok = false;
		}
	}

	void skip(uint32_t len)
	{
		if (left() >= len)
			offset += len;
		else
			ok = false;
	}

	uint64_t next()
	{
		if (offset < maxsize)
			return data[offset++];
		else
		{
			ok = false;
			return 0;
		}
	};

	uint8_t next8()
	{
		if (offset < maxsize)
			return data[offset++];
		else
		{
			ok = false;
			return 0;
		}
	}

	void rewind()
	{
		offset = 0;
	}

	void truncate()
	{
		maxsize = offset;
	}

	const unsigned char *dataPtr() const
	{
		return reinterpret_cast<const unsigned char *>(&data[offset]);
	}

	const char *charPtr() const
	{
		return reinterpret_cast<const char *>(&data[offset]);
	}

	std::string dataBlock(uint32_t len)
	{
		if (len <= left())
		{
			std::string a(charPtr(), len);
			offset += len;
			return a;
		}
		else
		{
			ok = false;
			return std::string();
		}
	}

protected:
	void setup(unsigned char *d, int msize)
	{
		data = d;
		offset = 0;
		overshoot = 0;
		maxsize = msize;
		ok = true;
	}
public:
	PacketDataStream(const char *d, int msize)
	{
		setup(const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(d)), msize);
	};

	PacketDataStream(char *d, int msize)
	{
		setup(reinterpret_cast<unsigned char *>(d), msize);
	};

	PacketDataStream(unsigned char *d, int msize)
	{
		setup(d, msize);
	};

	PacketDataStream &operator <<(const uint64_t value)
	{
		uint64_t i = value;

		if ((i & 0x8000000000000000LL) && (~i < 0x100000000LL))
		{
			// Signed number.
			i = ~i;
			if (i <= 0x3)
			{
				// Shortcase for -1 to -4
				append(0xFC | i);
				return *this;
			}
			else
			{
				append(0xF8);
			}
		}
		if (i < 0x80)
		{
			// Need top bit clear
			append(i);
		}
		else if (i < 0x4000)
		{
			// Need top two bits clear
			append((i >> 8) | 0x80);
			append(i & 0xFF);
		}
		else if (i < 0x200000)
		{
			// Need top three bits clear
			append((i >> 16) | 0xC0);
			append((i >> 8) & 0xFF);
			append(i & 0xFF);
		}
		else if (i < 0x10000000)
		{
			// Need top four bits clear
			append((i >> 24) | 0xE0);
			append((i >> 16) & 0xFF);
			append((i >> 8) & 0xFF);
			append(i & 0xFF);
		}
		else if (i < 0x100000000LL)
		{
			// It's a full 32-bit integer.
			append(0xF0);
			append((i >> 24) & 0xFF);
			append((i >> 16) & 0xFF);
			append((i >> 8) & 0xFF);
			append(i & 0xFF);
		}
		else
		{
			// It's a 64-bit value.
			append(0xF4);
			append((i >> 56) & 0xFF);
			append((i >> 48) & 0xFF);
			append((i >> 40) & 0xFF);
			append((i >> 32) & 0xFF);
			append((i >> 24) & 0xFF);
			append((i >> 16) & 0xFF);
			append((i >> 8) & 0xFF);
			append(i & 0xFF);
		}
		return *this;
	}

	PacketDataStream &operator >>(uint64_t &i)
	{
		uint64_t v = next();

		if ((v & 0x80) == 0x00)
		{
			i = (v & 0x7F);
		}
		else if ((v & 0xC0) == 0x80)
		{
			i = (v & 0x3F) << 8 | next();
		}
		else if ((v & 0xF0) == 0xF0)
		{
			switch (v & 0xFC)
			{
				case 0xF0:
					i = next() << 24 | next() << 16 | next() << 8 | next();
					break;
				case 0xF4:
					i = next() << 56 | next() << 48 | next() << 40 | next() << 32 | next() << 24 | next() << 16 | next() << 8 | next();
					break;
				case 0xF8:
					*this >> i;
					i = ~i;
					break;
				case 0xFC:
					i = v & 0x03;
					i = ~i;
					break;
				default:
					ok = false;
					i = 0;
					break;
			}
		}
		else if ((v & 0xF0) == 0xE0)
		{
			i = (v & 0x0F) << 24 | next() << 16 | next() << 8 | next();
		}
		else if ((v & 0xE0) == 0xC0)
		{
			i = (v & 0x1F) << 16 | next() << 8 | next();
		}
		return *this;
	}

	PacketDataStream &operator <<(const std::string &a)
	{
		*this << a.size();
		append(a.c_str(), a.size());
		return *this;
	}

	PacketDataStream &operator >>(std::string &a)
	{
		uint32_t len;
		*this >> len;
		if (len > left())
		{
			len = left();
			ok = false;
		}
		a = std::string(reinterpret_cast<const char *>(&data[offset]), len);
		offset += len;
		return *this;
	}

	PacketDataStream &operator <<(const bool b)
	{
		uint32_t v = b ? 1 : 0;
		return *this << v;
	}

	PacketDataStream &operator >>(bool &b)
	{
		uint32_t v;
		*this >> v;
		b = v ? true : false;
		return *this;
	}

#define INTMAPOPERATOR(type) \
		PacketDataStream &operator <<(const type v) { \
			return *this << static_cast<uint64_t>(v); \
			} \
		PacketDataStream &operator >>(type &v) { \
			uint64_t vv; \
			*this >> vv; \
			v = static_cast<type>(vv); \
			return *this; \
			}


	INTMAPOPERATOR(int);
	INTMAPOPERATOR(unsigned int);
	INTMAPOPERATOR(short);
	INTMAPOPERATOR(unsigned short);
	INTMAPOPERATOR(char);
	INTMAPOPERATOR(unsigned char);

	union double64u
	{
		uint64_t ui;
		double d;
	};

	PacketDataStream &operator <<(const double v)
	{
		double64u u;
		u.d = v;
		return *this << u.ui;
	}

	PacketDataStream &operator >>(double &v)
	{
		double64u u;
		*this >> u.ui;
		v = u.d;
		return *this;
	}

	union float32u
	{
		uint8_t ui[4];
		float f;
	};

	PacketDataStream &operator <<(const float v)
	{
		float32u u;
		u.f = v;
		append(u.ui[0]);
		append(u.ui[1]);
		append(u.ui[2]);
		append(u.ui[3]);
		return *this;
	}

	PacketDataStream &operator >>(float &v)
	{
		float32u u;
		if (left() < 4)
		{
			ok = false;
			v = 0;
		}
		u.ui[0] = next8();
		u.ui[1] = next8();
		u.ui[2] = next8();
		u.ui[3] = next8();
		v = u.f;
		return *this;
	}

	template <typename T>
	PacketDataStream &operator <<(const std::vector<T> &l)
	{
		*this << l.size();
		for (int i = 0; i < l.size(); i++)
			*this << l.at(i);
		return *this;
	}

	template <typename T>
	PacketDataStream &operator >>(std::vector<T> &l)
	{
		l.clear();
		uint32_t len;
		*this >> len;
		if (len > left())
		{
			len = left();
			ok = false;
		}
		for (uint32_t i = 0; i<len; i++)
		{
			if (left() == 0)
			{
				ok = false;
				break;
			}

			T t;
			*this >> t;
			l.append(t);
		}
		return *this;
	}

	template <typename T, typename U>
	PacketDataStream &operator <<(const std::pair<T, U> &p)
	{
		return *this << p.first << p.second;
	}

	template <typename T, typename U>
	PacketDataStream &operator >>(std::pair<T, U> &p)
	{
		return *this >> p.first >> p.second;
	}

};

#endif