#pragma once

#include <msgpack.hpp>
#include <rapidjson/document.h>

inline void ConvertToMsgPack(const rapidjson::Value& json, msgpack::object& object, msgpack::zone& zone)
{
	switch (json.GetType())
	{
		case rapidjson::kFalseType:
			object = msgpack::object{ false };
			break;

		case rapidjson::kTrueType:
			object = msgpack::object{ true };
			break;

		case rapidjson::kNumberType:
		{
			if (json.IsInt())
			{
				object = msgpack::object{ json.GetInt() };
			}
			else if (json.IsUint())
			{
				object = msgpack::object{ json.GetUint() };
			}
			else if (json.IsInt64())
			{
				object = msgpack::object{ json.GetInt64() };
			}
			else if (json.IsUint64())
			{
				object = msgpack::object{ json.GetUint64() };
			}
#if 0
			// GH-2171: trying to round-trip float32 will lead to issues with
			// backwards compatibility and often lossy conversions.
			else if (json.IsLosslessFloat())
			{
				object = msgpack::object{ json.GetFloat() };
			}
#endif
			else if (json.IsDouble())
			{
				object = msgpack::object{ json.GetDouble() };
			}

			break;
		}

		case rapidjson::kStringType:
			// we allocate with 'zone', otherwise the std::string's raw pointer gets used, which won't work as it gets destructed later on
			object = msgpack::object(std::string(json.GetString(), json.GetStringLength()), zone);
			break;

		case rapidjson::kObjectType:
		{
			std::map<std::string, msgpack::object> list;

			for (auto it = json.MemberBegin(); it != json.MemberEnd(); it++)
			{
				msgpack::object newObject;
				ConvertToMsgPack(it->value, newObject, zone);

				list.insert({ it->name.GetString(), newObject });
			}

			object = msgpack::object(list, zone);

			break;
		}

		case rapidjson::kArrayType:
		{
			std::vector<msgpack::object> list;

			for (auto it = json.Begin(); it != json.End(); it++)
			{
				msgpack::object newObject;
				ConvertToMsgPack(*it, newObject, zone);

				list.push_back(newObject);
			}

			object = msgpack::object(list, zone);

			break;
		}

		default:
			object = msgpack::object{ msgpack::type::nil_t{} };
			break;
	}
}

inline void ConvertToJSON(const msgpack::object& object, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<>& allocator)
{
	switch (object.type)
	{
		case msgpack::type::BOOLEAN:
			value.SetBool(object.as<bool>());
			break;

		case msgpack::type::POSITIVE_INTEGER:
			value.SetUint64(object.as<uint64_t>());
			break;

		case msgpack::type::NEGATIVE_INTEGER:
			value.SetInt64(object.as<int64_t>());
			break;

		case msgpack::type::FLOAT32:
			value.SetFloat(object.as<float>());
			break;
		case msgpack::type::FLOAT:
			value.SetDouble(object.as<double>());
			break;

		case msgpack::type::STR:
		case msgpack::type::BIN:
		{
			std::string string = object.as<std::string>();
			value.SetString(string.c_str(), string.size(), allocator);
			break;
		}

		case msgpack::type::ARRAY:
		{
			auto list = object.as<std::vector<msgpack::object>>();
			value.SetArray();

			for (auto& entry : list)
			{
				rapidjson::Value inValue;
				ConvertToJSON(entry, inValue, allocator);

				value.PushBack(inValue, allocator);
			}

			break;
		}

		case msgpack::type::MAP:
		{
			std::map<std::string, msgpack::object> list;

			if (object.via.map.ptr)
			{
				if (object.via.map.ptr->key.type == msgpack::type::STR)
				{
					object.convert(list);
				}
				else
				{
					auto intList = object.as<std::map<int, msgpack::object>>();
					for (auto& [key, value] : intList)
					{
						list[std::to_string(key)] = std::move(value);
					}
				}
			}

			value.SetObject();

			for (auto& entry : list)
			{
				rapidjson::Value inValue;
				ConvertToJSON(entry.second, inValue, allocator);

				rapidjson::Value name;
				name.SetString(entry.first.c_str(), entry.first.size(), allocator);

				value.AddMember(name, inValue, allocator);
			}

			break;
		}

		default:
			value.SetNull();
			break;
	}
}
