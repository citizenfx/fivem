#pragma once

#if defined(__GNUC__)
#define _ReturnAddress() __builtin_return_address(0)
#endif