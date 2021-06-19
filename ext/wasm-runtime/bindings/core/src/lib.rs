#[cfg(feature = "full")]
pub mod events;
#[doc(hidden)]
#[cfg(feature = "full")]
pub mod exports;
#[cfg(feature = "full")]
pub mod invoker;
#[cfg(feature = "full")]
pub mod ref_funcs;
#[cfg(feature = "full")]
pub mod runtime;
#[cfg(feature = "types")]
pub mod types;

#[cfg(feature = "full")]
mod ffi {
    #[link(wasm_import_module = "host")]
    extern "C" {
        pub fn log(ptr: i32, len: i32);
    }
}

/// Logs a message to the FiveM server or client
#[cfg(feature = "full")]
pub fn log<T: AsRef<str>>(message: T) {
    let msg = message.as_ref();
    let cstr = std::ffi::CString::new(msg).unwrap();
    let bytes = cstr.as_bytes_with_nul();

    unsafe {
        ffi::log(bytes.as_ptr() as _, bytes.len() as _);
    }
}
