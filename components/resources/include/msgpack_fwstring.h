#pragma once

namespace msgpack
{
	inline fwString& operator>> (object o, fwString& v)
	{
		if (o.type != type::RAW) { throw type_error(); }
		v.assign(o.via.raw.ptr, o.via.raw.size);
		return v;
	}

	template <typename Stream>
	inline packer<Stream>& operator<< (packer<Stream>& o, const fwString& v)
	{
		o.pack_raw(v.size());
		o.pack_raw_body(v.data(), v.size());
		return o;
	}

	inline void operator<< (object::with_zone& o, const fwString& v)
	{
		o.type = type::RAW;
		char* ptr = (char*)o.zone->malloc(v.size());
		o.via.raw.ptr = ptr;
		o.via.raw.size = (uint32_t)v.size();
		memcpy(ptr, v.data(), v.size());
	}

	inline void operator<< (object& o, const fwString& v)
	{
		o.type = type::RAW;
		o.via.raw.ptr = v.data();
		o.via.raw.size = (uint32_t)v.size();
	}
}