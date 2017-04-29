#pragma once

namespace msgpack
{
	MSGPACK_API_VERSION_NAMESPACE(v1)
	{
		inline object const& operator>> (object const& o, fwString& v)
		{
			switch (o.type)
			{
				case type::BIN:
					v.assign(o.via.bin.ptr, o.via.bin.size);
					break;
				case type::STR:
					v.assign(o.via.str.ptr, o.via.str.size);
					break;
				default:
					throw type_error();
					break;
			}
			return o;
		}

		template <typename Stream>
		inline packer<Stream>& operator<< (packer<Stream>& o, const fwString& v)
		{
			o.pack_str(v.size());
			o.pack_str_body(v.data(), v.size());
			return o;
		}

		inline void operator<< (object::with_zone& o, const fwString& v)
		{
			o.type = type::STR;
			char* ptr = static_cast<char*>(o.zone.allocate_align(v.size()));
			o.via.str.ptr = ptr;
			o.via.str.size = static_cast<uint32_t>(v.size());
			memcpy(ptr, v.data(), v.size());
		}

		inline void operator<< (object& o, const fwString& v)
		{
			o.type = type::STR;
			o.via.str.ptr = v.data();
			o.via.str.size = static_cast<uint32_t>(v.size());
		}
	}
}