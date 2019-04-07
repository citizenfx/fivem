#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Config
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "optick.config.h"

#if USE_OPTICK
#include <stdint.h>

#if defined(__clang__) || defined(__GNUC__)
#	define OPTICK_GCC (1)
#	if defined(__APPLE_CC__)
#		define OPTICK_OSX (1)
#	elif defined(__linux__)
#		define OPTICK_LINUX (1)
#	elif defined(__ORBIS__)
#		define OPTICK_PS4 (1)
#	endif
#elif defined(_MSC_VER)
#pragma comment(lib, "ProfilerCore64.lib")

#	define OPTICK_MSVC (1)
#	if defined(_DURANGO)
#		define OPTICK_XBOX (1)
#	else
#		define OPTICK_PC (1)
#endif
#else
#error Compiler not supported
#endif

////////////////////////////////////////////////////////////////////////
// Target Platform
////////////////////////////////////////////////////////////////////////

#if defined(OPTICK_GCC)
#define OPTICK_FUNC __PRETTY_FUNCTION__
#elif defined(OPTICK_MSVC)
#define OPTICK_FUNC __FUNCSIG__
#else
#error Compiler not supported
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXPORTS 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OPTICK_EXPORTS
#define OPTICK_API __declspec(dllexport)
#else
#define OPTICK_API //__declspec(dllimport)
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OPTICK_CONCAT_IMPL(x, y) x##y
#define OPTICK_CONCAT(x, y) OPTICK_CONCAT_IMPL(x, y)

#if defined(OPTICK_MSVC)
#define OPTICK_INLINE __forceinline
#elif defined(OPTICK_GCC)
#define OPTICK_INLINE __attribute__((always_inline)) inline
#else
#error Compiler is not supported
#endif


// Vulkan Forward Declarations
#define OPTICK_DEFINE_HANDLE(object) typedef struct object##_T* object;
OPTICK_DEFINE_HANDLE(VkDevice);
OPTICK_DEFINE_HANDLE(VkPhysicalDevice);
OPTICK_DEFINE_HANDLE(VkQueue);
OPTICK_DEFINE_HANDLE(VkCommandBuffer);

// D3D12 Forward Declarations
struct ID3D12CommandList;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Optick
{
	// Source: http://msdn.microsoft.com/en-us/library/system.windows.media.colors(v=vs.110).aspx
	// Image:  http://i.msdn.microsoft.com/dynimg/IC24340.png
	struct Color
	{
		enum
		{
			Null = 0x00000000,
			AliceBlue = 0xFFF0F8FF,
			AntiqueWhite = 0xFFFAEBD7,
			Aqua = 0xFF00FFFF,
			Aquamarine = 0xFF7FFFD4,
			Azure = 0xFFF0FFFF,
			Beige = 0xFFF5F5DC,
			Bisque = 0xFFFFE4C4,
			Black = 0xFF000000,
			BlanchedAlmond = 0xFFFFEBCD,
			Blue = 0xFF0000FF,
			BlueViolet = 0xFF8A2BE2,
			Brown = 0xFFA52A2A,
			BurlyWood = 0xFFDEB887,
			CadetBlue = 0xFF5F9EA0,
			Chartreuse = 0xFF7FFF00,
			Chocolate = 0xFFD2691E,
			Coral = 0xFFFF7F50,
			CornflowerBlue = 0xFF6495ED,
			Cornsilk = 0xFFFFF8DC,
			Crimson = 0xFFDC143C,
			Cyan = 0xFF00FFFF,
			DarkBlue = 0xFF00008B,
			DarkCyan = 0xFF008B8B,
			DarkGoldenRod = 0xFFB8860B,
			DarkGray = 0xFFA9A9A9,
			DarkGreen = 0xFF006400,
			DarkKhaki = 0xFFBDB76B,
			DarkMagenta = 0xFF8B008B,
			DarkOliveGreen = 0xFF556B2F,
			DarkOrange = 0xFFFF8C00,
			DarkOrchid = 0xFF9932CC,
			DarkRed = 0xFF8B0000,
			DarkSalmon = 0xFFE9967A,
			DarkSeaGreen = 0xFF8FBC8F,
			DarkSlateBlue = 0xFF483D8B,
			DarkSlateGray = 0xFF2F4F4F,
			DarkTurquoise = 0xFF00CED1,
			DarkViolet = 0xFF9400D3,
			DeepPink = 0xFFFF1493,
			DeepSkyBlue = 0xFF00BFFF,
			DimGray = 0xFF696969,
			DodgerBlue = 0xFF1E90FF,
			FireBrick = 0xFFB22222,
			FloralWhite = 0xFFFFFAF0,
			ForestGreen = 0xFF228B22,
			Fuchsia = 0xFFFF00FF,
			Gainsboro = 0xFFDCDCDC,
			GhostWhite = 0xFFF8F8FF,
			Gold = 0xFFFFD700,
			GoldenRod = 0xFFDAA520,
			Gray = 0xFF808080,
			Green = 0xFF008000,
			GreenYellow = 0xFFADFF2F,
			HoneyDew = 0xFFF0FFF0,
			HotPink = 0xFFFF69B4,
			IndianRed = 0xFFCD5C5C,
			Indigo = 0xFF4B0082,
			Ivory = 0xFFFFFFF0,
			Khaki = 0xFFF0E68C,
			Lavender = 0xFFE6E6FA,
			LavenderBlush = 0xFFFFF0F5,
			LawnGreen = 0xFF7CFC00,
			LemonChiffon = 0xFFFFFACD,
			LightBlue = 0xFFADD8E6,
			LightCoral = 0xFFF08080,
			LightCyan = 0xFFE0FFFF,
			LightGoldenRodYellow = 0xFFFAFAD2,
			LightGray = 0xFFD3D3D3,
			LightGreen = 0xFF90EE90,
			LightPink = 0xFFFFB6C1,
			LightSalmon = 0xFFFFA07A,
			LightSeaGreen = 0xFF20B2AA,
			LightSkyBlue = 0xFF87CEFA,
			LightSlateGray = 0xFF778899,
			LightSteelBlue = 0xFFB0C4DE,
			LightYellow = 0xFFFFFFE0,
			Lime = 0xFF00FF00,
			LimeGreen = 0xFF32CD32,
			Linen = 0xFFFAF0E6,
			Magenta = 0xFFFF00FF,
			Maroon = 0xFF800000,
			MediumAquaMarine = 0xFF66CDAA,
			MediumBlue = 0xFF0000CD,
			MediumOrchid = 0xFFBA55D3,
			MediumPurple = 0xFF9370DB,
			MediumSeaGreen = 0xFF3CB371,
			MediumSlateBlue = 0xFF7B68EE,
			MediumSpringGreen = 0xFF00FA9A,
			MediumTurquoise = 0xFF48D1CC,
			MediumVioletRed = 0xFFC71585,
			MidnightBlue = 0xFF191970,
			MintCream = 0xFFF5FFFA,
			MistyRose = 0xFFFFE4E1,
			Moccasin = 0xFFFFE4B5,
			NavajoWhite = 0xFFFFDEAD,
			Navy = 0xFF000080,
			OldLace = 0xFFFDF5E6,
			Olive = 0xFF808000,
			OliveDrab = 0xFF6B8E23,
			Orange = 0xFFFFA500,
			OrangeRed = 0xFFFF4500,
			Orchid = 0xFFDA70D6,
			PaleGoldenRod = 0xFFEEE8AA,
			PaleGreen = 0xFF98FB98,
			PaleTurquoise = 0xFFAFEEEE,
			PaleVioletRed = 0xFFDB7093,
			PapayaWhip = 0xFFFFEFD5,
			PeachPuff = 0xFFFFDAB9,
			Peru = 0xFFCD853F,
			Pink = 0xFFFFC0CB,
			Plum = 0xFFDDA0DD,
			PowderBlue = 0xFFB0E0E6,
			Purple = 0xFF800080,
			Red = 0xFFFF0000,
			RosyBrown = 0xFFBC8F8F,
			RoyalBlue = 0xFF4169E1,
			SaddleBrown = 0xFF8B4513,
			Salmon = 0xFFFA8072,
			SandyBrown = 0xFFF4A460,
			SeaGreen = 0xFF2E8B57,
			SeaShell = 0xFFFFF5EE,
			Sienna = 0xFFA0522D,
			Silver = 0xFFC0C0C0,
			SkyBlue = 0xFF87CEEB,
			SlateBlue = 0xFF6A5ACD,
			SlateGray = 0xFF708090,
			Snow = 0xFFFFFAFA,
			SpringGreen = 0xFF00FF7F,
			SteelBlue = 0xFF4682B4,
			Tan = 0xFFD2B48C,
			Teal = 0xFF008080,
			Thistle = 0xFFD8BFD8,
			Tomato = 0xFFFF6347,
			Turquoise = 0xFF40E0D0,
			Violet = 0xFFEE82EE,
			Wheat = 0xFFF5DEB3,
			White = 0xFFFFFFFF,
			WhiteSmoke = 0xFFF5F5F5,
			Yellow = 0xFFFFFF00,
			YellowGreen = 0xFF9ACD32,
		};
	};

	struct Filter
	{
		enum Type : uint32_t
		{
			None,
			
			// CPU
			AI,
			Animation, 
			Audio,
			Debug,
			Camera,
			Cloth,
			GameLogic,
			Input,
			Navigation,
			Network,
			Physics,
			Rendering,
			Scene,
			Script,
			Streaming,
			UI,
			VFX,
			Visibility,
			Wait,

			// IO
			IO,

			// GPU
			GPU_Cloth,
			GPU_Lighting,
			GPU_PostFX,
			GPU_Reflections,
			GPU_Scene,
			GPU_Shadows,
			GPU_UI,
			GPU_VFX,
			GPU_Water,

		};
	};

	#define OPTICK_MAKE_CATEGORY(filter, color) (((uint64_t)(1ull) << (filter + 32)) | (uint64_t)color)

	struct Category
	{
		enum Type : uint64_t
		{
			// CPU
			None			= OPTICK_MAKE_CATEGORY(Filter::None, Color::Null),
			AI				= OPTICK_MAKE_CATEGORY(Filter::AI, Color::Purple),
			Animation		= OPTICK_MAKE_CATEGORY(Filter::Animation, Color::LightSkyBlue),
			Audio			= OPTICK_MAKE_CATEGORY(Filter::Audio, Color::HotPink),
			Debug			= OPTICK_MAKE_CATEGORY(Filter::Debug, Color::Black),
			Camera			= OPTICK_MAKE_CATEGORY(Filter::Camera, Color::Black),
			Cloth			= OPTICK_MAKE_CATEGORY(Filter::Cloth, Color::DarkGreen),
			GameLogic		= OPTICK_MAKE_CATEGORY(Filter::GameLogic, Color::RoyalBlue),
			Input			= OPTICK_MAKE_CATEGORY(Filter::Input, Color::Ivory),
			Navigation		= OPTICK_MAKE_CATEGORY(Filter::Navigation, Color::Magenta),
			Network			= OPTICK_MAKE_CATEGORY(Filter::Network, Color::Olive),
			Physics			= OPTICK_MAKE_CATEGORY(Filter::Physics, Color::LawnGreen),
			Rendering		= OPTICK_MAKE_CATEGORY(Filter::Rendering, Color::BurlyWood),
			Scene			= OPTICK_MAKE_CATEGORY(Filter::Scene, Color::RoyalBlue),
			Script			= OPTICK_MAKE_CATEGORY(Filter::Script, Color::Plum),
			Streaming		= OPTICK_MAKE_CATEGORY(Filter::Streaming, Color::Gold),
			UI				= OPTICK_MAKE_CATEGORY(Filter::UI, Color::PaleTurquoise),
			VFX				= OPTICK_MAKE_CATEGORY(Filter::VFX, Color::SaddleBrown),
			Visibility		= OPTICK_MAKE_CATEGORY(Filter::Visibility, Color::Snow),
			Wait			= OPTICK_MAKE_CATEGORY(Filter::Wait, Color::Tomato),
			WaitEmpty		= OPTICK_MAKE_CATEGORY(Filter::Wait, Color::White),
			// IO
			IO				= OPTICK_MAKE_CATEGORY(Filter::IO, Color::Khaki),
			// GPU
			GPU_Cloth		= OPTICK_MAKE_CATEGORY(Filter::GPU_Cloth, Color::DarkGreen),
			GPU_Lighting	= OPTICK_MAKE_CATEGORY(Filter::GPU_Lighting, Color::Khaki),
			GPU_PostFX		= OPTICK_MAKE_CATEGORY(Filter::GPU_PostFX, Color::Maroon),
			GPU_Reflections = OPTICK_MAKE_CATEGORY(Filter::GPU_Reflections, Color::CadetBlue),
			GPU_Scene		= OPTICK_MAKE_CATEGORY(Filter::GPU_Scene, Color::RoyalBlue),
			GPU_Shadows		= OPTICK_MAKE_CATEGORY(Filter::GPU_Shadows, Color::LightSlateGray),
			GPU_UI			= OPTICK_MAKE_CATEGORY(Filter::GPU_UI, Color::PaleTurquoise),
			GPU_VFX			= OPTICK_MAKE_CATEGORY(Filter::GPU_VFX, Color::SaddleBrown),
			GPU_Water		= OPTICK_MAKE_CATEGORY(Filter::GPU_Water, Color::SteelBlue),
		};

		static uint32_t GetMask(Type t) { return (uint32_t)(t >> 32); }
		static uint32_t GetColor(Type t) { return (uint32_t)(t); }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


namespace Optick
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Mode
{
	enum Type
	{
		OFF = 0x0,
		INSTRUMENTATION_CATEGORIES = (1 << 0),
		INSTRUMENTATION_EVENTS = (1 << 1),
		INSTRUMENTATION = (INSTRUMENTATION_CATEGORIES | INSTRUMENTATION_EVENTS),
		SAMPLING = (1 << 2),
		TAGS = (1 << 3),
		AUTOSAMPLING = (1 << 4),
		SWITCH_CONTEXT = (1 << 5),
		IO = (1 << 6),
		GPU = (1 << 7),
		END_SCREENSHOT = (1 << 8),
		RESERVED_0 = (1 << 9),
		RESERVED_1 = (1 << 10),
		HW_COUNTERS = (1 << 11),
		LIVE = (1 << 12),
		RESERVED_2 = (1 << 13),
		RESERVED_3 = (1 << 14),
		RESERVED_4 = (1 << 15),

		DEFAULT = INSTRUMENTATION & AUTOSAMPLING & SWITCH_CONTEXT & IO & GPU,
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPTICK_API int64_t GetHighPrecisionTime();
OPTICK_API int64_t GetHighPrecisionFrequency();
OPTICK_API uint32_t NextFrame();
OPTICK_API bool IsActive();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EventStorage;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPTICK_API bool RegisterFiber(uint64_t fiberId, EventStorage** slot);
OPTICK_API bool RegisterThread(const char* name);
OPTICK_API bool RegisterThread(const wchar_t* name);
OPTICK_API bool UnRegisterThread(bool keepAlive);
OPTICK_API EventStorage** GetEventStorageSlotForCurrentThread();
OPTICK_API bool IsFiberStorage(EventStorage* fiberStorage);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ThreadMask
{
	enum Type
	{
		None	= 0,
		Main	= 1 << 0,
		GPU		= 1 << 1,
		IO		= 1 << 2,
		Idle	= 1 << 3,
	};
};

OPTICK_API EventStorage* RegisterStorage(const char* name, uint64_t threadID = uint64_t(-1), ThreadMask::Type type = ThreadMask::None);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct State
{
	enum Type
	{
		// Starting a new capture
		START_CAPTURE,

		// Stopping current capture
		STOP_CAPTURE,

		// Dumping capture to the GUI
		// Useful for attaching summary and screenshot to the capture
		DUMP_CAPTURE,
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets a state change callback
typedef bool (*StateCallback)(State::Type state);
OPTICK_API bool SetStateChangedCallback(StateCallback cb);

// Attaches a key-value pair to the capture's summary
// Example: AttachSummary("Version", "v12.0.1");
//			AttachSummary("Platform", "Windows");
//			AttachSummary("Config", "Release_x64");
//			AttachSummary("Settings", "Ultra");
//			AttachSummary("Map", "Atlantida");
//			AttachSummary("Position", "123.0,120.0,41.1");
//			AttachSummary("CPU", "Intel(R) Xeon(R) CPU E5410@2.33GHz");
//			AttachSummary("GPU", "NVIDIA GeForce GTX 980 Ti");
OPTICK_API bool AttachSummary(const char* key, const char* value);

struct File
{
	enum Type
	{
		// Supported formats: PNG, JPEG, BMP, TIFF
		OPTICK_IMAGE,
		
		// Text file
		OPTICK_TEXT,

		// Any other type
		OPTICK_OTHER,
	};
};
// Attaches a file to the current capture
OPTICK_API bool AttachFile(File::Type type, const char* name, const uint8_t* data, uint32_t size);
OPTICK_API bool AttachFile(File::Type type, const char* name, const char* path);
OPTICK_API bool AttachFile(File::Type type, const char* name, const wchar_t* path);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EventDescription;
struct Frame;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EventTime
{
	static const int64_t INVALID_TIMESTAMP = (int64_t)-1;

	int64_t start;
	int64_t finish;

	OPTICK_INLINE void Start() { start  = Optick::GetHighPrecisionTime(); }
	OPTICK_INLINE void Stop() 	{ finish = Optick::GetHighPrecisionTime(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EventData : public EventTime
{
	const EventDescription* description;

	bool operator<(const EventData& other) const
	{
		if (start != other.start)
			return start < other.start;

		// Reversed order for finish intervals (parent first)
		return  finish > other.finish;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API SyncData : public EventTime
{
	uint64_t newThreadId;
	uint64_t oldThreadId;
	uint8_t core;
	int8_t reason;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API FiberSyncData : public EventTime
{
	uint64_t threadId;

	static void AttachToThread(EventStorage* storage, uint64_t threadId);
	static void DetachFromThread(EventStorage* storage);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct TagData
{
	const EventDescription* description;
	int64_t timestamp;
	T data;
	TagData() {}
	TagData(const EventDescription& desc, T d) : description(&desc), timestamp(Optick::GetHighPrecisionTime()), data(d) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API EventDescription
{
	// HOT  \\
	// Have to place "hot" variables at the beginning of the class (here will be some padding)
	// COLD //

	const char* name;
	const char* file;
	uint32_t line;
	uint32_t index;
	uint32_t color;
	uint32_t filter;
	float budget;

	static EventDescription* Create(const char* eventName, const char* fileName, const unsigned long fileLine, const unsigned long eventColor = Color::Null, const unsigned long filter = 0);
	static EventDescription* CreateShared(const char* eventName, const char* fileName = nullptr, const unsigned long fileLine = 0, const unsigned long eventColor = Color::Null, const unsigned long filter = 0);

	EventDescription();
private:
	friend class EventDescriptionBoard;
	EventDescription& operator=(const EventDescription&);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API Event
{
	EventData* data;

	static EventData* Start(const EventDescription& description);
	static void Stop(EventData& data);

	static void Push(const char* name);
	static void Push(const EventDescription& description);
	static void Pop();

	static void Add(EventStorage* storage, const EventDescription* description, int64_t timestampStart, int64_t timestampFinish);
	static void Push(EventStorage* storage, const EventDescription* description, int64_t timestampStart);
	static void Pop(EventStorage* storage, int64_t timestampStart);


	Event(const EventDescription& description)
	{
		data = Start(description);
	}

	~Event()
	{
		if (data)
			Stop(*data);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPTICK_INLINE Optick::EventDescription* CreateDescription(const char* functionName, const char* fileName, int fileLine, const char* eventName = nullptr, const ::Optick::Category::Type category = ::Optick::Category::None)
{
	return ::Optick::EventDescription::Create(eventName != nullptr ? eventName : functionName, fileName, fileLine, ::Optick::Category::GetColor(category), ::Optick::Category::GetMask(category));
}
OPTICK_INLINE Optick::EventDescription* CreateDescription(const char* functionName, const char* fileName, int fileLine, const ::Optick::Category::Type category)
{
	return ::Optick::EventDescription::Create(functionName, fileName, fileLine, ::Optick::Category::GetColor(category), ::Optick::Category::GetMask(category));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API GPUEvent
{
	EventData* data;

	static EventData* Start(const EventDescription& description);
	static void Stop(EventData& data);

	GPUEvent(const EventDescription& description)
	{
		data = Start(description);
	}

	~GPUEvent()
	{
		if (data)
			Stop(*data);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API Tag
{
	static void Attach(const EventDescription& description, float val);
	static void Attach(const EventDescription& description, int32_t val);
	static void Attach(const EventDescription& description, uint32_t val);
	static void Attach(const EventDescription& description, uint64_t val);
	static void Attach(const EventDescription& description, float val[3]);
	static void Attach(const EventDescription& description, const char* val);

	// Derived
	static void Attach(const EventDescription& description, float x, float y, float z)
	{
		float p[3] = { x, y, z }; Attach(description, p);
	}

};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ThreadScope
{
    bool keepAlive;
    
    ThreadScope(const char* name, bool bKeepAlive = false) : keepAlive(bKeepAlive)
	{
		RegisterThread(name);
	}

	ThreadScope(const wchar_t* name)
	{
		RegisterThread(name);
	}

	~ThreadScope()
	{
		UnRegisterThread(keepAlive);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum OPTICK_API GPUQueueType
{
	GPU_QUEUE_GRAPHICS,
	GPU_QUEUE_COMPUTE,
	GPU_QUEUE_TRANSFER,
	GPU_QUEUE_VSYNC,

	GPU_QUEUE_COUNT,
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API GPUContext
{
	void* cmdBuffer;
	GPUQueueType queue;
	int node;
	GPUContext(void* c = nullptr, GPUQueueType q = GPU_QUEUE_GRAPHICS, int n = 0) : cmdBuffer(c), queue(q), node(n) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPTICK_API void InitGpuD3D12(void* device, void** cmdQueues, uint32_t numQueues);
OPTICK_API void InitGpuVulkan(void* vkDevices, void* vkPhysicalDevices, void* vkQueues, uint32_t* cmdQueuesFamily, uint32_t numQueues);
OPTICK_API void GpuFlip(void* swapChain);
OPTICK_API GPUContext SetGpuContext(GPUContext context);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OPTICK_API GPUContextScope
{
	GPUContext prevContext;

	GPUContextScope(ID3D12CommandList* cmdList, GPUQueueType queue = GPU_QUEUE_GRAPHICS, int node = 0)
	{
		prevContext = SetGpuContext(GPUContext(cmdList, queue, node));
	}

	GPUContextScope(VkCommandBuffer cmdBuffer, GPUQueueType queue = GPU_QUEUE_GRAPHICS, int node = 0)
	{
		prevContext = SetGpuContext(GPUContext(cmdBuffer, queue, node));
	}

	~GPUContextScope()
	{
		SetGpuContext(prevContext);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct FrameType
{
	enum Type
	{
		CPU,
		GPU,
		Render,
		COUNT,
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPTICK_API const EventDescription* GetFrameDescription(FrameType::Type frame);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

#define OPTICK_UNUSED(x) (void)(x)
// Workaround for gcc compiler
#define OPTICK_VA_ARGS(...) , ##__VA_ARGS__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scoped profiling event which automatically grabs current function name.
// Use tis macro 95% of the time.
// Example A:
//		void Function()
//		{
//			OPTICK_EVENT();
//			... code ...
//		}
//		or
//		void Function()
//		{
//			OPTICK_EVENT("CustomFunctionName");
//			... code ...
//		}
// Notes:
//		Optick captures full name of the function including name space and arguments.
//		Full name is usually shortened in the Optick GUI in order to highlight the most important bits.
#define OPTICK_EVENT(...)	 static ::Optick::EventDescription* OPTICK_CONCAT(autogen_description_, __LINE__) = nullptr; \
							 if (OPTICK_CONCAT(autogen_description_, __LINE__) == nullptr) OPTICK_CONCAT(autogen_description_, __LINE__) = ::Optick::CreateDescription(OPTICK_FUNC, __FILE__, __LINE__  OPTICK_VA_ARGS(__VA_ARGS__)); \
							 ::Optick::Event OPTICK_CONCAT(autogen_event_, __LINE__)( *(OPTICK_CONCAT(autogen_description_, __LINE__)) ); 

// Backward compatibility with previous versions of Optick
//#if !defined(PROFILE)
//#define PROFILE OPTICK_EVENT()
//#endif

// Scoped profiling macro with predefined color.
// Use this macro for high-level function calls (e.g. AI, Physics, Audio, Render etc.).
// Example:
//		void UpdateAI()
//		{
//			OPTICK_CATEGORY("UpdateAI", Optick::Category::AI);
//			... code ...
//		}
//	
//		Macro could automatically capture current function name:
//		void UpdateAI()
//		{
//			OPTICK_CATEGORY(OPTICK_FUNC, Optick::Category::AI);
//			... code ...
//		}
#define OPTICK_CATEGORY(NAME, CATEGORY)	OPTICK_EVENT(NAME, CATEGORY)

// Profiling event for Main Loop update.
// You need to call this function in the beginning of the each new frame.
// Example:
//		while (true)
//		{
//			OPTICK_FRAME("MainThread");
//			... code ...
//		}
#define OPTICK_FRAME(FRAME_NAME)	static ::Optick::ThreadScope mainThreadScope(FRAME_NAME);		\
									OPTICK_UNUSED(mainThreadScope);									\
									uint32_t frameNumber = ::Optick::NextFrame();					\
									::Optick::Event OPTICK_CONCAT(autogen_event_, __LINE__)(*::Optick::GetFrameDescription(::Optick::FrameType::CPU)); \
									OPTICK_TAG("Frame", frameNumber);


// Thread registration macro.
// Example:
//		void WorkerThread(...)
//		{
//			OPTICK_THREAD("Worker");
//			while (isRunning)
//			{
//				...
//			}
//		}
#define OPTICK_THREAD(THREAD_NAME) ::Optick::ThreadScope brofilerThreadScope(THREAD_NAME);	\
									 OPTICK_UNUSED(brofilerThreadScope);					\


// Thread registration macros.
// Useful for integration with custom job-managers.
#define OPTICK_START_THREAD(FRAME_NAME) ::Optick::RegisterThread(FRAME_NAME);
#define OPTICK_STOP_THREAD() ::Optick::UnRegisterThread(false);

// Attaches a custom data-tag.
// Supported types: int32, uint32, uint64, vec3, string (cut to 32 characters)
// Example:
//		OPTICK_TAG("PlayerName", name[index]);
//		OPTICK_TAG("Health", 100);
//		OPTICK_TAG("Score", 0x80000000u);
//		OPTICK_TAG("Height(cm)", 176.3f);
//		OPTICK_TAG("Address", (uint64)*this);
//		OPTICK_TAG("Position", 123.0f, 456.0f, 789.0f);
#define OPTICK_TAG(NAME, ...)		static ::Optick::EventDescription* OPTICK_CONCAT(autogen_tag_, __LINE__) = nullptr; \
									if (OPTICK_CONCAT(autogen_tag_, __LINE__) == nullptr) OPTICK_CONCAT(autogen_tag_, __LINE__) = ::Optick::EventDescription::Create( NAME, __FILE__, __LINE__ ); \
									::Optick::Tag::Attach(*OPTICK_CONCAT(autogen_tag_, __LINE__), __VA_ARGS__); \

// Scoped macro with DYNAMIC name.
// Optick holds a copy of the provided name.
// Each scope does a search in hashmap for the name.
// Please use variations with STATIC names where it's possible.
// Use this macro for quick prototyping or intergratoin with other profiling systems (e.g. UE4)
// Example:
//		const char* name = ... ;
//		OPTICK_EVENT_DYNAMIC(name);
#define OPTICK_EVENT_DYNAMIC(NAME)	OPTICK_CUSTOM_EVENT(::Optick::EventDescription::CreateShared(NAME, __FILE__, __LINE__));
// Push\Pop profiling macro with DYNAMIC name.
#define OPTICK_PUSH_DYNAMIC(NAME)		::Optick::Event::Push(NAME);		

// Push\Pop profiling macro with STATIC name.
// Please avoid using Push\Pop approach in favor for scoped macros.
// For backward compatibility with some engines.
// Example:
//		OPTICK_PUSH("ScopeName");
//		...
//		OPTICK_POP();
#define OPTICK_PUSH(NAME)				static ::Optick::EventDescription* OPTICK_CONCAT(autogen_description_, __LINE__) = nullptr; \
										if (OPTICK_CONCAT(autogen_description_, __LINE__) == nullptr) OPTICK_CONCAT(autogen_description_, __LINE__) = ::Optick::EventDescription::Create( NAME, __FILE__, __LINE__ ); \
										::Optick::Event::Push(*OPTICK_CONCAT(autogen_description_, __LINE__));		
#define OPTICK_POP()					::Optick::Event::Pop();


// Scoped macro with predefined Optick::EventDescription.
// Use these events instead of DYNAMIC macros to minimize overhead.
// Common use-case: integrating Optick with internal script languages (e.g. Lua, Actionscript(Scaleform), etc.).
// Example:
//		Generating EventDescription once during initialization:
//		Optick::EventDescription* description = Optick::EventDescription::CreateShared("FunctionName");
//
//		Then we could just use a pointer to cached description later for profiling:
//		OPTICK_CUSTOM_EVENT(description);
#define OPTICK_CUSTOM_EVENT(DESCRIPTION) 							::Optick::Event						  OPTICK_CONCAT(autogen_event_, __LINE__)( *DESCRIPTION ); \

// Registration of a custom EventStorage (e.g. GPU, IO, etc.)
// Use it to present any extra information on the timeline.
// Example:
//		Optick::EventStorage* IOStorage = Optick::RegisterStorage("I/O");
// Notes:
//		Registration of a new storage is thread-safe.
#define OPTICK_STORAGE_REGISTER(STORAGE_NAME)														::Optick::RegisterStorage(STORAGE_NAME);

// Adding events to the custom storage.
// Helps to integrate Optick into already existing profiling systems (e.g. GPU Profiler, I/O profiler, etc.).
// Example:
//			//Registering a storage - should be done once during initialization
//			static Optick::EventStorage* IOStorage = Optick::RegisterStorage("I/O");
//
//			int64_t cpuTimestampStart = Optick::GetHighPrecisionTime();
//			...
//			int64_t cpuTimestampFinish = Optick::GetHighPrecisionTime();
//
//			//Creating a shared event-description
//			static Optick::EventDescription* IORead = Optick::EventDescription::CreateShared("IO Read");
// 
//			OPTICK_STORAGE_EVENT(IOStorage, IORead, cpuTimestampStart, cpuTimestampFinish);
// Notes:
//		It's not thread-safe to add events to the same storage from multiple threads.
//		Please guarantee thread-safety on the higher level if access from multiple threads to the same storage is required.
#define OPTICK_STORAGE_EVENT(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START, CPU_TIMESTAMP_FINISH)	if (::Optick::IsActive()) { ::Optick::Event::Add(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START, CPU_TIMESTAMP_FINISH); }
#define OPTICK_STORAGE_PUSH(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START)							if (::Optick::IsActive()) { ::Optick::Event::Push(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START); }
#define OPTICK_STORAGE_POP(STORAGE, CPU_TIMESTAMP_FINISH)										if (::Optick::IsActive()) { ::Optick::Event::Pop(STORAGE, CPU_TIMESTAMP_FINISH); }


// Registers state change callback
// If callback returns false - the call is repeated the next frame
#define OPTICK_SET_STATE_CHANGED_CALLBACK(CALLBACK)			::Optick::SetStateChangedCallback(CALLBACK);


// GPU events
#define OPTICK_GPU_INIT_D3D12(DEVICE, CMD_QUEUES, NUM_CMD_QUEUS)			::Optick::InitGpuD3D12(DEVICE, CMD_QUEUES, NUM_CMD_QUEUS);
#define OPTICK_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS)			::Optick::InitGpuVulkan(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS);

// Setup GPU context:
// Params:
//		(CommandBuffer\CommandList, [Optional] Optick::GPUQueue queue, [Optional] int NodeIndex)
// Examples:
//		OPTICK_GPU_CONTEXT(cmdBuffer); - all OPTICK_GPU_EVENT will use the same command buffer within the scope
//		OPTICK_GPU_CONTEXT(cmdBuffer, Optick::GPU_QUEUE_COMPUTE); - all events will use the same command buffer and queue for the scope 
//		OPTICK_GPU_CONTEXT(cmdBuffer, Optick::GPU_QUEUE_COMPUTE, gpuIndex); - all events will use the same command buffer and queue for the scope 
#define OPTICK_GPU_CONTEXT(...)	 ::Optick::GPUContextScope OPTICK_CONCAT(gpu_autogen_context_, __LINE__)(__VA_ARGS__); \
									 (void)OPTICK_CONCAT(gpu_autogen_context_, __LINE__);

#define OPTICK_GPU_EVENT(NAME)	 OPTICK_EVENT(NAME); \
									 static ::Optick::EventDescription* OPTICK_CONCAT(gpu_autogen_description_, __LINE__) = nullptr; \
									 if (OPTICK_CONCAT(gpu_autogen_description_, __LINE__) == nullptr) OPTICK_CONCAT(gpu_autogen_description_, __LINE__) = ::Optick::EventDescription::Create( NAME, __FILE__, __LINE__ ); \
									 ::Optick::GPUEvent OPTICK_CONCAT(gpu_autogen_event_, __LINE__)( *(OPTICK_CONCAT(gpu_autogen_description_, __LINE__)) ); \

#define OPTICK_GPU_FLIP(SWAP_CHAIN)		::Optick::GpuFlip(SWAP_CHAIN);

#else
#define OPTICK_EVENT(...)
#define OPTICK_CATEGORY(NAME, COLOR)
#define OPTICK_FRAME(NAME)
#define OPTICK_THREAD(FRAME_NAME)
#define OPTICK_START_THREAD(FRAME_NAME)
#define OPTICK_STOP_THREAD()
#define OPTICK_TAG(NAME, DATA)
#define OPTICK_EVENT_DYNAMIC(NAME)	
#define OPTICK_PUSH_DYNAMIC(NAME)		
#define OPTICK_PUSH(NAME)				
#define OPTICK_POP()		
#define OPTICK_CUSTOM_EVENT(DESCRIPTION)
#define OPTICK_STORAGE_REGISTER(STORAGE_NAME)
#define OPTICK_STORAGE_EVENT(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START, CPU_TIMESTAMP_FINISH)
#define OPTICK_STORAGE_PUSH(STORAGE, DESCRIPTION, CPU_TIMESTAMP_START)
#define OPTICK_STORAGE_POP(STORAGE, CPU_TIMESTAMP_FINISH)				
#define OPTICK_SET_STATE_CHANGED_CALLBACK(CALLBACK)
#define OPTICK_GPU_INIT_D3D12(DEVICE, CMD_QUEUES, NUM_CMD_QUEUS)
#define OPTICK_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS)
#define OPTICK_GPU_CONTEXT(...)
#define OPTICK_GPU_EVENT(NAME)
#define OPTICK_GPU_FLIP(SWAP_CHAIN)
#endif
