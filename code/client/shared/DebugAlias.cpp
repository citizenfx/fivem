#include <StdInc.h>
#include <DebugAlias.h>

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#elif defined(__clang__) || defined(__GNUC__)
#define NOINLINE __attribute__((__noinline__))
#else
#define NOINLINE
#endif

namespace debug
{
NOINLINE void Alias(const void* var) {}
}
