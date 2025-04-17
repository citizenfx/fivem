#pragma once

#include <msgpack.hpp>

namespace fx
{
	struct scrObject
	{
		const char* data;
		uintptr_t length;
	};

	template<typename T>
	inline scrObject SerializeObject(const T& object)
	{
		static msgpack::sbuffer buf;
		buf.clear();

		msgpack::packer<msgpack::sbuffer> packer(buf);
		packer.pack(object);

		scrObject packed;
		packed.data = buf.data();
		packed.length = buf.size();

		return packed;
	}

	template<typename T>
	inline T DeserializeObject(const scrObject& obj, bool* outSuccess = nullptr)
	{
		T result {};
		try
		{
			msgpack::unpacked unpacked;
			msgpack::unpack(unpacked, obj.data, obj.length);
			result = unpacked.get().as<T>();
			if (outSuccess)
				*outSuccess = true;
		}
		catch (...)
		{
			if (outSuccess)
				*outSuccess = false;
		}
		return result;
	}

}
