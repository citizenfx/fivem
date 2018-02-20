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
}
