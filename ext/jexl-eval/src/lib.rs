extern crate libc;

use std::io;

use std::ffi::{CStr, CString};
use std::ptr;

use serde_json::{Result, Value, json};

pub fn null_check<T>(ptr: *const T) -> io::Result<()> {
    if ptr.is_null() {
        Err(io::Error::from_raw_os_error(libc::EIO))
    } else {
        Ok(())
    }
}

// via https://github.com/pop-os/distinst/blob/cdcae71864b1076f303cfad8337d83c3b04e7e39/ffi/src/lib.rs#L57
// via https://github.com/rust-lang/rust-bindgen/issues/738
pub fn get_str<'a>(ptr: *const libc::c_char) -> io::Result<&'a str> {
    null_check(ptr).and_then(|_| {
        unsafe { CStr::from_ptr(ptr) }.to_str().map_err(|_| {
            io::Error::from_raw_os_error(libc::EINVAL)
        })
    })
}

pub fn to_cstr(string: String) -> *mut libc::c_char {
    CString::new(string)
        .map(|string| string.into_raw())
        .unwrap_or(ptr::null_mut())
}

// main code
#[no_mangle]
pub extern "C" fn jexl_eval(expr_str: *const libc::c_char, context_str: *const libc::c_char) -> *mut libc::c_char {
    let expr_ref = get_str(expr_str);
    let context_ref = get_str(context_str);

    if expr_ref.is_err() || context_ref.is_err() {
        return ptr::null_mut();
    }

    let expr = expr_ref.unwrap();
    let context = context_ref.unwrap();

    let v: Result<Value> = serde_json::from_str(context);
    
    if v.is_err() {
        return ptr::null_mut();
    }

    let evaluator = jexl_eval::Evaluator::new();
    let mut result = evaluator.eval_in_context(expr, v.unwrap()); // oh no, mutability

    if result.is_err() {
        result = Ok(json!(result.err().unwrap().to_string()));
    }

    return serde_json::to_string(&result.unwrap())
        .ok()
        .map_or(ptr::null_mut(), |str| to_cstr(str));
}

#[no_mangle]
pub unsafe extern "C" fn jexl_free(str: *mut libc::c_char) {
    if str != ptr::null_mut() {
        let _ = CString::from_raw(str);
    }
}