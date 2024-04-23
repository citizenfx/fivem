/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <node/node.h>
#include <node/v8/v8-profiler.h>

#include <fstream>

#include <tbb/concurrent_queue.h>

namespace fx
{
	std::string SaveProfileToString(v8::Isolate* isolate, v8::CpuProfile* profile);
}

namespace fx::v8shared
{
	class FileOutputStream : public v8::OutputStream {
	public:
		FileOutputStream(FILE* stream) : stream_(stream) {}

		virtual int GetChunkSize() {
			return 65536;  // big chunks == faster
		}

		virtual void EndOfStream() {}

		virtual WriteResult WriteAsciiChunk(char* data, int size) {
			const size_t len = static_cast<size_t>(size);
			size_t off = 0;

			while (off < len && !feof(stream_) && !ferror(stream_))
				off += fwrite(data + off, 1, len - off, stream_);

			return off == len ? kContinue : kAbort;
		}

	private:
		FILE* stream_;
	};

	static void V8_Snap(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		FILE* fp = fopen("snap.heapsnapshot", "w");
		if (fp == NULL) return;
		const v8::HeapSnapshot* const snap = v8::Isolate::GetCurrent()->GetHeapProfiler()->TakeHeapSnapshot();
		FileOutputStream stream(fp);
		snap->Serialize(&stream, v8::HeapSnapshot::kJSON);
		int err = 0;
		fclose(fp);
		// Work around a deficiency in the API.  The HeapSnapshot object is const
		// but we cannot call HeapProfiler::DeleteAllHeapSnapshots() because that
		// invalidates _all_ snapshots, including those created by other tools.
		const_cast<v8::HeapSnapshot*>(snap)->Delete();
	}

	static thread_local v8::CpuProfiler* g_cpuProfiler;

	static void V8_StartProfiling(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		if (g_cpuProfiler)
		{
			return;
		}

		v8::CpuProfiler* profiler = v8::CpuProfiler::New(args.GetIsolate());
		profiler->StartProfiling((args.Length() == 0) ? v8::String::Empty(args.GetIsolate()) : v8::Local<v8::String>::Cast(args[0]), true);

		g_cpuProfiler = profiler;
	}

	static void V8_StopProfiling(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		if (!g_cpuProfiler)
		{
			return;
		}

		auto profiler = g_cpuProfiler;
		v8::CpuProfile* profile = profiler->StopProfiling((args.Length() == 0) ? v8::String::Empty(args.GetIsolate()) : v8::Local<v8::String>::Cast(args[0]));

		std::string jsonString = SaveProfileToString(args.GetIsolate(), profile);

		profile->Delete();

#ifdef _WIN32
		SYSTEMTIME systemTime;
		GetSystemTime(&systemTime);

		{
			std::ofstream stream(MakeRelativeCitPath(va(L"v8-%04d%02d%02d-%02d%02d%02d.cpuprofile", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond)));
			stream << jsonString;
		}
#endif
		
		args.GetReturnValue().Set(v8::JSON::Parse(args.GetIsolate()->GetCurrentContext(), v8::String::NewFromUtf8(args.GetIsolate(), jsonString.c_str(), v8::NewStringType::kNormal, jsonString.size()).ToLocalChecked()).ToLocalChecked());

		g_cpuProfiler->Dispose();
		g_cpuProfiler = nullptr;
	}

}
