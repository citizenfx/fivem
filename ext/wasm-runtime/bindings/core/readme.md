# FiveM WASM bindings for Rust
## API
this API can be implemented with any other language that compiles into WASM.
### host
module name: `host`

exports:
* `log(ptr: u32, len: u32)`
* `invoke(hash: u64, ptr: *const u8, len: usize, retval: *const ReturnValue)`
    * `ptr, len` - a pointer to an array of pointers to arguments

```Rust
// All structures are C
// See also bindings/src/types.rs
#[repr(C)]
pub struct ReturnValue {
    return_type: u32, // expected return type of the native function
    buffer: u32, // pointer to the guest buffer
    capacity: u32, // capacity of the guest buffer
}
```

### wasm script
exports:
* `_start`
* `__cfx_alloc(size: u32, alignment: u32) -> u32`
* `__cfx_free(ptr: *const c_void, size: u32, alignment: u32)`
* `__cfx_on_tick()`
* `__cfx_on_event(event_name: *const u8, args: *const u8, args_len: u32, src: *const u8)`
    * `event_name` - C string
    * `args` - an array of msgpack bytes
    * `src` - C string of the event source

### allocations
all arguments that lives only in the execution context of the functions are managed by the host.

for example `__cfx_on_event`:

```Rust
// allocate memory
let ptr0 = __cfx_alloc(event_name.len(), ...);
let ptr1 = __cfx_alloc(args.len(), ...);

// the host copies event name and args in the allocated memory

// call the expoted function
__cfx_on_event(ptr0, ptr1, ...)

// cleanup the memory after the call
__cfx_free(ptr0)`
__cfx_free(ptr1)
```

when you call `host::invoke` all arguments are managed by the guest side meaning that memory should be available when you call the invoke function. you can use static arrays or allocate every time you call natives.

also there is an ability to resize your buffer used to get a returning value. you should export a function `__cfx_extend_retval_buffer(new_size: usize) -> *const u8`. this function will be executed in the middle of `host::invoke` execution (so, make sure you manage pointers properly). the function is called when you passing a buffer that is smaller than return value of a native. the extending function should return a new pointer to the buffer that is capable to store `new_size` bytes. if you cannot provide just return 0. `host::invoke` will return `-1`.
