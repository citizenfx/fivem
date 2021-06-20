use std::ffi::{c_void, CStr};

pub type LogFunc = extern "C" fn(msg: *const i8);
pub type InvokeFunc = extern "C" fn(args: *mut c_void) -> u32;
pub type CanonicalizeRefFunc =
    extern "C" fn(ref_idx: u32, buffer: *mut i8, buffer_size: u32) -> i32;

#[no_mangle]
pub extern "C" fn wasm_create_runtime() -> *mut c_void {
    let runtime = cfx_wasm_runtime::Runtime::new();
    let boxed = Box::new(runtime);

    Box::into_raw(boxed) as *mut _
}

#[no_mangle]
pub unsafe extern "C" fn wasm_destroy_runtime(runtime: *mut c_void) {
    let runtime = runtime as *mut cfx_wasm_runtime::Runtime;

    Box::from_raw(runtime);
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_create_module(
    runtime: *mut c_void,
    bytes: *const u8,
    length: u64,
) -> bool {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    let bytes = std::slice::from_raw_parts(bytes, length as usize);

    match runtime.load_module(bytes, true) {
        Ok(_) => true,
        Err(err) => {
            println!("WASM error: {:?}", err);
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_destroy_module(runtime: *mut c_void) {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    runtime.unload_module();
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_tick(runtime: *mut c_void) -> bool {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    runtime.tick().is_ok()
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_trigger_event(
    runtime: *mut c_void,
    event_name: *const i8,
    args: *const u8,
    args_len: u32,
    source: *const i8,
) -> bool {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);

    let event = CStr::from_ptr(event_name);
    let args = std::slice::from_raw_parts(args, args_len as _);
    let source = CStr::from_ptr(source);

    runtime.trigger_event(event, args, source).is_ok()
}

#[no_mangle]
pub unsafe extern "C" fn wasm_set_logger_function(log: LogFunc) {
    cfx_wasm_runtime::set_logger(log);
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_memory_usage(runtime: *mut c_void) -> u32 {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);

    runtime.memory_size() * 64 * 1024
}

#[no_mangle]
pub unsafe extern "C" fn wasm_set_invoke_native(invoke: InvokeFunc) {
    cfx_wasm_runtime::set_native_invoke(invoke);
}

#[no_mangle]
pub unsafe extern "C" fn wasm_set_canonicalize_ref(canonicalize_ref: CanonicalizeRefFunc) {
    cfx_wasm_runtime::set_canonicalize_ref(canonicalize_ref);
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_call_ref(
    runtime: *mut c_void,
    ref_idx: u32,
    args: *const u8,
    args_len: u32,
    ret: *mut *const u8,
    ret_size: *mut u32,
) -> bool {
    use std::cell::RefCell;

    thread_local! {
        static RETVAL: RefCell<Vec<u8>> = RefCell::new(vec![0; 1 << 15]);
    }

    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    let args = std::slice::from_raw_parts(args, args_len as _);

    RETVAL.with(|retval| {
        let written = { runtime.call_ref(ref_idx, args, &mut retval.borrow_mut()) };

        match written {
            Ok(bytes) => {
                *ret = retval.borrow().as_ptr();
                *ret_size = bytes;
            }

            Err(_) => return false,
        }

        true
    })
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_duplicate_ref(runtime: *mut c_void, ref_idx: u32) -> u32 {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    runtime.duplicate_ref(ref_idx)
}

#[no_mangle]
pub unsafe extern "C" fn wasm_runtime_remove_ref(runtime: *mut c_void, ref_idx: u32) {
    let runtime = &mut *(runtime as *mut cfx_wasm_runtime::Runtime);
    runtime.remove_ref(ref_idx);
}
