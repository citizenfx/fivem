# CFX Hooking Library

The CFX hooking library provides a comprehensive set of tools for runtime code patching, pattern scanning, and function interception. It is primarily used for modifying game behavior in FiveM and RedM.

## Header Files

```cpp
#include <Hooking.h>           // Main header - includes all components
#include <Hooking.Patterns.h>   // Pattern scanning
#include <Hooking.Invoke.h>     // Function stubs (thiscall_stub, cdecl_stub)
#include <Hooking.FlexStruct.h> // Dynamic struct access
#include <Hooking.Stubs.h>      // Trampolines
#include <HookFunction.h>       // Initialization hooks
#include <jitasm.h>             // Runtime assembly generation
```

## Namespace

All hooking functions are in the `hook` namespace. (Not including MinHook or jitasm)

---

## Base Address Handling

The library handles ASLR (Address Space Layout Randomization) by maintaining a base address difference from the expected image base.

### Setting the Base Address

```cpp
// Set base from the main module
hook::set_base();

// Set base from a known address
hook::set_base(0x140000000);
```

### Adjusting Addresses

```cpp
// Adjust an address in-place
uintptr_t addr = 0x140001000;
hook::adjust_base(addr);

// Get adjusted address without modifying original
uintptr_t adjusted = hook::get_adjusted(0x140001000);

// Convert runtime address back to static address
uintptr_t unadjusted = hook::get_unadjusted(runtimeAddr);
```

### RVA (Relative Virtual Address) Helpers

```cpp
// Get pointer from RVA
auto ptr = hook::getRVA<void>(0x1000);

// Get pointer from RVA in specific module
auto ptr = hook::getRVA<SomeStruct>(hModule, 0x1000);
```

---

## Pattern Scanning

Pattern scanning allows finding code sequences using byte patterns with wildcards.

### Pattern Syntax

- Hex bytes: `48 8B 05`
- Wildcards: `?` matches any byte
- Example: `48 8B 05 ? ? ? ?` matches a `mov rax, [rip+offset]` instruction

### Basic Pattern Usage

```cpp
// Find a single match
auto match = hook::pattern("48 8B 05 ? ? ? ? 48 85 C0").get_one();
void* ptr = match.get<void>();

// Get with offset from match start
void* offsetPtr = match.get<void>(7);

// Shorthand for single match with offset
void* ptr = hook::get_pattern<void>("48 8B 05 ? ? ? ?", 0);
```

### Multiple Matches

```cpp
// Find all matches
auto pattern = hook::pattern("E8 ? ? ? ?");
for (size_t i = 0; i < pattern.size(); i++)
{
    auto match = pattern.get(i);
    // Process each match
}

// Assert exact number of matches
auto pattern = hook::pattern("E8 ? ? ? ?").count(5);
```

### Module-Specific Patterns

```cpp
// Search in a specific module
auto pattern = hook::module_pattern(hModule, "48 8B 05 ? ? ? ?");

// Search in a specific memory range
auto pattern = hook::range_pattern(startAddr, endAddr, "48 8B 05 ? ? ? ?");
```

---

## Memory Writing

### Direct Memory Writes

```cpp
// Write a value (assumes memory is writable)
hook::put<uint8_t>(address, 0x90);
hook::put<uint32_t>(address, 0x12345678);
hook::put<uint64_t>(address, 0x123456789ABCDEF0);
```

### Memory Writes with VirtualProtect

```cpp
// Write with automatic page protection handling
hook::putVP<uint8_t>(address, 0x90);
hook::putVP<uint32_t>(address, 0xDEADBEEF);
```

### NOP Instructions

```cpp
// Fill with NOP (0x90) instructions
hook::nop(address, 5);  // NOP 5 bytes
```

### Return Early from Function

```cpp
// Insert a RET instruction
hook::return_function(address);

// Insert RET with stack cleanup (for stdcall/thiscall)
hook::return_function(address, 0x10);  // RET 0x10
```

---

## Function Hooking

### Jump Hooks

Replace a function entirely by jumping to your code.

```cpp
void MyFunction()
{
    // Your replacement code
}

// Install jump hook
hook::jump(targetAddress, MyFunction);

// Jump that passes first arg in specific register
hook::jump_rcx(address, MyFunction);  // RCX register
hook::jump_reg<2>(address, MyFunction);  // RDX register (reg index 2)
```

### Call Hooks

Replace a CALL instruction to redirect to your function.

```cpp
static void* g_origFunction;

void MyHook(int arg1, int arg2)
{
    // Pre-processing

    // Call original
    ((void(*)(int, int))g_origFunction)(arg1, arg2);

    // Post-processing
}

// Hook a call site
hook::call(callSiteAddress, MyHook);

// Call with specific register for first arg
hook::call_rcx(address, MyHook);
```

### Getting Call Targets

```cpp
// Get the target address of a CALL instruction
auto target = hook::get_call(callAddress);

// Store call target in a variable
void* origFunc;
hook::set_call(&origFunc, callAddress);
```

### Getting Addresses from RIP-relative Instructions

```cpp
// Get address from RIP-relative operand (e.g., LEA, MOV)
auto ptr = hook::get_address<void*>(instructionAddress);

// With custom offset to the 4-byte displacement and instruction length
auto ptr = hook::get_address<void*>(address, offsetToDisp, instructionLength);

// Helper using offset template
auto ptr = hook::get_by_offset<SomeType, int32_t>(address, 3);
```

---

## Function Stubs

Create callable wrappers for game functions found via pattern scanning.

### cdecl Functions

```cpp
// Define a stub for a cdecl function
static hook::cdecl_stub<void(int, const char*)> someGameFunction([]()
{
    return hook::get_pattern("48 89 5C 24 ? 57 48 83 EC 20", 0);
});

// Call it like a normal function
someGameFunction(42, "hello");
```

### thiscall (Member) Functions

```cpp
// Define a stub for a member function
// First parameter is the 'this' pointer
static hook::thiscall_stub<int(void*, int)> memberFunction([]()
{
    return hook::get_pattern("40 53 48 83 EC 20 8B DA", 0);
});

// Call with object pointer as first argument
SomeClass* obj = ...;
int result = memberFunction(obj, 123);
```

---

## Trampolines

Install a detour hook that preserves the ability to call the original function.

```cpp
static int (*g_origFunction)(int arg);

int MyHook(int arg)
{
    // Pre-processing
    int result = g_origFunction(arg);  // Call original
    // Post-processing
    return result;
}

// Install trampoline - returns pointer to original
g_origFunction = hook::trampoline(targetAddress, MyHook);
```

---

## IAT (Import Address Table) Hooking

Hook functions imported from DLLs.

```cpp
// Hook by function name
auto origCreateFile = hook::iat("kernel32.dll", MyCreateFileW, "CreateFileW");

// Hook by ordinal
auto origFunc = hook::iat("someDll.dll", MyFunc, 42);

// Hook in specific module's IAT
auto origFunc = hook::iat(hModule, "kernel32.dll", MyFunc, "SomeFunction");
```

---

## HookFunction Initialization

Register code to run after the game loads but before it starts.

```cpp
static HookFunction hookFunction([]()
{
    // Pattern scanning and hook installation code here
    auto pattern = hook::pattern("48 8B 05 ? ? ? ?").get_one();
    hook::nop(pattern.get<void>(7), 5);
});
```

### Runtime Hook Functions

Register hooks that run when a specific key/event is triggered.

```cpp
static RuntimeHookFunction rhf("sessionInit", []()
{
    // Code to run when "sessionInit" is triggered
});

// Trigger from elsewhere
RuntimeHookFunction::Run("sessionInit");
```

---

## FlexStruct

Access fields in structures with unknown or changing layouts.

```cpp
hook::FlexStruct* obj = (hook::FlexStruct*)somePointer;

// Read a field at offset
int value = obj->Get<int>(0x10);
uint64_t id = obj->Get<uint64_t>(0x28);

// Write a field at offset
obj->Set<int>(0x10, 42);

// Get reference to field
int& valueRef = obj->At<int>(0x10);
valueRef = 100;

// Call virtual method at vtable offset
auto result = obj->CallVirtual<int>(0x18, arg1, arg2);
```

---

## Member Function Pointers

Get raw address from member function pointers.

```cpp
class SomeClass {
public:
    void MemberFunc();
};

// Get address of member function
uintptr_t addr = hook::get_member(&SomeClass::MemberFunc);
```

---

## TLS (Thread Local Storage)

Access the games TLS data.

```cpp
// Get TLS pointer
char* tls = hook::get_tls();

// With custom type
auto* tlsData = hook::get_tls<MyTlsStruct*>();
```

---

## Complete Example

```cpp
#include <StdInc.h>
#include <Hooking.h>

// Original function pointer for trampoline
static int (*g_origProcessInput)(void* inputManager, int key);

// Our hook function
int ProcessInputHook(void* inputManager, int key)
{
    // Check if we want to block this key
    if (key == VK_ESCAPE && ShouldBlockEscape())
    {
        return 0;  // Block input
    }

    // Call original function
    return g_origProcessInput(inputManager, key);
}

// Stub for calling a game function
static hook::cdecl_stub<void(const char*)> PrintToConsole([]()
{
    return hook::get_pattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8", 0);
});

static HookFunction hookFunction([]()
{
    // Find the input processing function
    auto location = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B FA");

    // Install trampoline hook
    g_origProcessInput = hook::trampoline(location, ProcessInputHook);

    // NOP out an unwanted check
    hook::nop(hook::get_pattern("74 15 48 8B CB E8"), 2);

    // Replace a constant
    hook::put<uint32_t>(hook::get_pattern("B8 E8 03 00 00", 1), 5000);

    // Hook a call site
    hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 20"), MyCallHook);

    // Get a global variable address
    auto globalPtr = hook::get_address<int*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 85 C0 74 10", 3));
});
```

---

## MinHook Integration

[MinHook](https://github.com/TsudaKageyu/minhook) is a third-party hooking library that provides robust function detouring. Use `hook::trampoline` if you intend to use minhook.

## jitasm (Runtime Assembly Generation)

jitasm is a C++ library for generating x86/x64 assembly code at runtime. Use it when you need to create custom assembly stubs, typically for:

- Injecting code at specific locations that need custom register handling
- Creating wrapper functions that manipulate registers before calling C++ code
- Implementing complex hooks that require precise control over the CPU state

### When to Use jitasm

- When `hook::call` or `hook::jump` don't provide enough control
- When you need to preserve or manipulate specific registers
- When injecting code in the middle of a function
- When creating stubs that bridge assembly and C++ code

### Basic Structure

```cpp
#include <jitasm.h>

static struct : jitasm::Frontend
{
    void InternalMain() override
    {
        // Your assembly code here
        mov(rax, rcx);
        ret();
    }
} myStub;

// Use the generated code
hook::call(someAddress, myStub.GetCode());
```

### Calling C++ Functions from Assembly

```cpp
static struct : jitasm::Frontend
{
    // Static function that will be called from assembly
    static void MyCppFunction(void* arg1, int arg2)
    {
        // Process arguments
    }

    void InternalMain() override
    {
        // Save registers we'll clobber
        push(rbx);
        sub(rsp, 0x20);  // Shadow space for Windows x64 calling convention

        // Set up arguments (Windows x64: rcx, rdx, r8, r9)
        // rcx already has first arg, set up second
        mov(edx, 42);

        // Call our C++ function
        mov(rax, (uintptr_t)MyCppFunction);
        call(rax);

        // Restore
        add(rsp, 0x20);
        pop(rbx);

        ret();
    }
} callCppStub;
```

### Conditional Logic with Labels

```cpp
static struct : jitasm::Frontend
{
    static bool ShouldSkip()
    {
        return g_skipProcessing;
    }

    void InternalMain() override
    {
        // Save state
        push(rax);
        sub(rsp, 0x20);

        // Call our check function
        mov(rax, (uintptr_t)ShouldSkip);
        call(rax);

        add(rsp, 0x20);

        // Check return value
        cmp(al, 1);
        pop(rax);

        // If true, skip to end
        je("skip");

        // Normal path, do original work
        mov(rcx, rbx);
        mov(rax, (uintptr_t)g_origFunction);
        call(rax);

        L("skip");
        ret();
    }
} conditionalStub;
```

### Preserving Registers for Mid-Function Hooks

```cpp
static void* g_continueAddress;

static struct : jitasm::Frontend
{
    static void ProcessData(void* data)
    {
        // Our C++ processing
    }

    void InternalMain() override
    {
        // Replicate the original instructions we overwrote
        mov(rcx, rdi);
        mov(rdx, rsi);

        // Save all volatile registers
        push(rax);
        push(rcx);
        push(rdx);
        push(r8);
        push(r9);
        push(r10);
        push(r11);
        sub(rsp, 0x28);

        // Call our function with rdi as argument
        mov(rcx, rdi);
        mov(rax, (uintptr_t)ProcessData);
        call(rax);

        // Restore registers
        add(rsp, 0x28);
        pop(r11);
        pop(r10);
        pop(r9);
        pop(r8);
        pop(rdx);
        pop(rcx);
        pop(rax);

        // Jump back to continue original execution
        mov(rax, (uintptr_t)g_continueAddress);
        jmp(rax);
    }
} midFunctionStub;

static HookFunction hookFunction([]()
{
    auto location = hook::get_pattern("48 8B CF 48 8B D6 E8");

    // Calculate where to return (after the bytes we're replacing)
    g_continueAddress = (void*)((uintptr_t)location + 7);

    // Install our stub
    hook::jump(location, midFunctionStub.GetCode());
});
```

### Common x64 Register Reference

| Register | Purpose (Windows x64) |
|----------|----------------------|
| rcx | 1st argument |
| rdx | 2nd argument |
| r8 | 3rd argument |
| r9 | 4th argument |
| rax | Return value |
| rbx, rbp, rdi, rsi, r12-r15 | Callee-saved (must preserve) |
| rsp | Stack pointer |

## Best Practices

1. **Always use patterns** instead of hardcoded addresses for maintainability across game updates
2. **Verify pattern matches** using `.count()` when you expect a specific number of results
3. **Use trampolines** when you need to hook a game function
4. **Use `HookFunction`** to ensure hooks are installed at the right time during initialization
5. **Use jitasm sparingly** - only when you need precise register control or mid-function hooks
6. **Always preserve callee-saved registers** in jitasm stubs (rbx, rbp, rdi, rsi, r12-r15 on x64)
7. **Allocate shadow space** (32 bytes) before calling functions in x64 jitasm code
