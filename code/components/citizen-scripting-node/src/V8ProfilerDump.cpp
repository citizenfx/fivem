/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <v8-profiler.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <chrono>

inline static std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

using v8::CpuProfile;
using v8::CpuProfileNode;
using v8::String;

namespace fx
{
void SaveProfileNodeToValue(v8::Isolate* isolate, const CpuProfileNode* node, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<>& allocator)
{
	value.SetObject();

	String::Utf8Value functionName(isolate, node->GetFunctionName());
	String::Utf8Value url(isolate, node->GetScriptResourceName());

	value.AddMember("functionName", rapidjson::Value(*functionName, allocator), allocator);
	value.AddMember("url", rapidjson::Value(*url, allocator), allocator);
	value.AddMember("lineNumber", rapidjson::Value(node->GetLineNumber()), allocator);
	value.AddMember("bailoutReason", rapidjson::Value(node->GetBailoutReason(), allocator), allocator);
	value.AddMember("id", rapidjson::Value(node->GetNodeId()), allocator);
	value.AddMember("scriptId", rapidjson::Value(node->GetScriptId()), allocator);
	value.AddMember("hitCount", rapidjson::Value(node->GetHitCount()), allocator);

	{
		rapidjson::Value children;
		children.SetArray();

		int count = node->GetChildrenCount();
		for (int i = 0; i < count; i++)
		{
			rapidjson::Value child;
			SaveProfileNodeToValue(isolate, node->GetChild(i), child, allocator);

			children.PushBack(child, allocator);
		}

		value.AddMember("children", children, allocator);
	}

	{
		uint32_t hitLines = node->GetHitLineCount();
		std::vector<CpuProfileNode::LineTick> lineTicks(hitLines);

		rapidjson::Value lineTicksValue;

		if (node->GetLineTicks(&lineTicks[0], lineTicks.size()))
		{
			lineTicksValue.SetArray();

			for (auto& lineTick : lineTicks)
			{
				rapidjson::Value tickValue;
				tickValue.SetObject();

				tickValue.AddMember("line", rapidjson::Value(lineTick.line), allocator);
				tickValue.AddMember("hitCount", rapidjson::Value(lineTick.hit_count), allocator);

				lineTicksValue.PushBack(tickValue, allocator);
			}
		}
		else
		{
			lineTicksValue.SetNull();
		}

		value.AddMember("lineTicks", lineTicksValue, allocator);
	}
}

void SaveProfileToValue(v8::Isolate* isolate, CpuProfile* profile, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<>& allocator)
{
	value.SetObject();

	String::Utf8Value title(isolate, profile->GetTitle());

	value.AddMember("typeId", rapidjson::Value("CPU"), allocator);
	value.AddMember("uid", rapidjson::Value(static_cast<uint32_t>(msec().count())), allocator);

	if (title.length() == 0)
	{
		value.AddMember("title", rapidjson::Value(va("Profiling at tick count %d", msec().count()), allocator), allocator);
	}
	else
	{
		value.AddMember("title", rapidjson::Value(*title, allocator), allocator);
	}

	{
		rapidjson::Value head;
		SaveProfileNodeToValue(isolate, profile->GetTopDownRoot(), head, allocator);

		value.AddMember("head", head, allocator);
	}

	value.AddMember("startTime", rapidjson::Value(profile->GetStartTime() / 1000000), allocator);
	value.AddMember("endTime", rapidjson::Value(profile->GetEndTime() / 1000000), allocator);

	{
		rapidjson::Value samples;
		rapidjson::Value timestamps;

		samples.SetArray();
		timestamps.SetArray();

		int count = profile->GetSamplesCount();

		for (int i = 0; i < count; i++)
		{
			samples.PushBack(rapidjson::Value(profile->GetSample(i)->GetNodeId()), allocator);
			timestamps.PushBack(rapidjson::Value(static_cast<double>(profile->GetSampleTimestamp(i))), allocator);
		}

		value.AddMember("samples", samples, allocator);
		value.AddMember("timestamps", timestamps, allocator);
	}
}

std::string SaveProfileToString(v8::Isolate* isolate, CpuProfile* profile)
{
	rapidjson::Document document;
	SaveProfileToValue(isolate, profile, document, document.GetAllocator());

	rapidjson::StringBuffer sbuffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

	document.Accept(writer);

	return std::string(sbuffer.GetString(), sbuffer.GetSize());
}
}
