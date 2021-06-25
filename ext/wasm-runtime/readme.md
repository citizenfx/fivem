# WASM Runtime
A script runtime to run WASM modules.

**This is an experimental script engine. It will change in the future.**

## Concepts
`glue` crate uses `runtime` to build a static library and a C header. This library imported by C++ component `citizen-scripting-wasm`.

The runtime loads files with `.wasm` extensions and tries to build them in a module.

Each module can listen to events, invoke CitizenFX native functions and call reference functions from another runtimes.

**SCRIPTS (MODULES) CANNOT STORE VALUES THAT PASSED FROM THE RUNTIME (THEY ARE FREED AFTER EXECUTION)**

**SCRIPTS (MODULES) ARE RESPONSIBLE TO MANAGE VALUES THAT IS PASSED TO THE RUNTIME (BUT THEY SHOULD LIVE AS LONG AS RUNTIME (HOST) FUNCTION IS EXECUTED)**

The runtime exposes next functions that can be used by any WebAssembly module (runtime module name is `cfx`):
- `script_log(message: *const char)` - log a message in the console
    - `message` - null terminated C string
- `invoke(hash: u64, ptr: i32, len: i32, retval: i32) -> i32` - invokes a native CitizenFX function (returns status code `rt-types/lib.rs/call_result`)
    - `hash` - hash of a native function to call
    - `ptr` - pointer to an array of arguments
        - An argument is described by `GuestArg` structure (C repr).
    - `len` - count of arguments in the array
    - `retval` - structure holding an expected type of return value and a buffer (`ReturnValue`)
        - if the buffer is smaller than a return value the runtime will try to call `__cfx_extend_retval_buffer` function
- `canonicalize_ref(ref_idx: i32, ptr: i32, len: i32) -> i32` - calls `IScriptHost::CanonicalizeRef`. Returns length of a new string (0 if call is failed. you can retry with a bigger buffer)
    - `ref_idx` - index of a function
    - `ptr` - pointer to a buffer where to write
    - `len` - capacity of the buffer
- `invoke_ref_func(ref_name: i32, args: i32, len: i32, buffer: i32, buffer_capacity: i32) -> i32` - calls an external ref function
    - `ref_name` - null terminated C string
    - `args` - pointer to a serialized buffer (messagepack)
    - `len` - size of the buffer
    - `buffer` - pointer to a return value buffer
    - `buffer_capacity` - size of the retval buffer
        - like `invoke` can call `__cfx_extend_retval_buffer`

Expected exports from modules (scripts):
- `_start()` - entry point (optional)
- `__cfx_on_event(event: *const char, args: i32, args_len: i32, src: *const char)` - callback for events (optional)
    - `event` - null terminated C string with an event name
    - `args` - pointer to a buffer with arguments (packed bytes with messagepack)
    - `args_len` - size of the buffer
    - `src` - null terminated C string with a source of the event
- `__cfx_on_tick()` - called each server / client tick (optional)
- `__cfx_call_ref(ref_idx: i32, args: i32, args_len: i32) -> i32` - called when something wants to execute your ref func (optional)
    - `ref_idx` - index of a ref func
    - `args` - pointer to a buffer with arguments (packed bytes with messagepack)
    - `args_len` size of the buffer
    - should return a pointer to `ScrObject` object (can be nullptr) that should live **AT LEAST ONE SERVER/CLIENT TICK** (thread_local for example)
- `__cfx_alloc(size: i32, align: i32) -> i32` - calls when the runtime wants to allocate some memory in the module (required)
    - `size` - size of a value
    - `align` - align of a value
    - should return a valid pointer that should live until `__cfx_free` called
- `__cfx_free(ptr: i32, size: i32, align: i32)` - frees allocated memory
    - `ptr` - pointer to the memory to be freed (required)
    - `size` - size of a value
    - `align` - align of a value
- `__cfx_duplicate_ref(ref_idx: i32) -> i32` - duplicates an index to a ref func
    - `ref_idx` - an index of a ref func
- `__cfx_remove_ref(ref_idx: i32)` - removes a reference to a ref func
    - `ref_idx` - an index of a ref func
- `__cfx_extend_retval_buffer(new_size: i32) -> i32` - called from some functions to extend a return value buffer
    - `new_size` - expected size of a new buffer
    - returns a pointer to the new buffer (the old one may be freed)

An example of WASM modules implemented in Rust can be found here: (where?)

C++ version
```c++
// To build this code you need wasi-sdk (https://github.com/WebAssembly/wasi-sdk/)
// $CXX -std=c++17 -O3 -flto -fno-exceptions main.cpp -o main.wasm  -Wl,--allow-undefined,--export-all
// I've modified msgpack11 by removing all throws (https://github.com/ar90n/msgpack11)
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <msgpack11.hpp>

using namespace msgpack11;

#define FROM_CFX __attribute__((import_module("cfx")))

// Defined at ext/wasm-runtime/rt-types
struct guest_arg
{
    uint64_t value;
    std::size_t size;
    bool is_ref;
};

extern "C" void FROM_CFX script_log(const char *message);
extern "C" int32_t FROM_CFX invoke(uint64_t hash, const guest_arg *args, std::size_t len, void *retval);

void register_resource_as_event_handler(const std::string &event_name)
{
    std::vector<guest_arg> args{guest_arg{reinterpret_cast<uint64_t>(event_name.c_str()), 8, true}};
    invoke(0xD233A168, args.data(), args.size(), nullptr);
}

template <typename... T>
void log(T... args)
{
    std::stringstream stream("");

    (stream << ... << args);

    auto result = stream.str();
    script_log(result.c_str());
}

int main()
{
    std::string message = "hellow!";
    std::string event_name = "onServerResourceStart";

    register_resource_as_event_handler(event_name);

    log(message);

    return 0;
}

__attribute__((used)) extern "C" void __cfx_on_event(const char *event, const char *args, std::size_t size, const char *source)
{
    std::string err;
    std::string event_name(event);

    MsgPack des_msgpack = MsgPack::parse(args, size, err);

    if (event_name.compare("onServerResourceStart") == 0)
    {
        auto array = des_msgpack.array_items();
        auto resource_name = array[0];

        log("new resource: ", resource_name.string_value());
    }
}

__attribute__((used)) extern "C" void *__cfx_alloc(std::size_t size, std::size_t align)
{
    return aligned_alloc(align, size);
}

__attribute__((used)) extern "C" void __cfx_free(void *ptr, uint32_t size, uint32_t align)
{
    std::free(ptr);
}
```
