use crate::{
    ref_funcs::{ExternRefFunction, RefFunction},
    types::{call_result, CharPtr, GuestArg, RetVal, ReturnValue, Vector3},
};

use serde::{de::DeserializeOwned, Serialize};
use std::cell::RefCell;

const RETVAL_BUFFER_SIZE: usize = 1 << 15;

thread_local! {
    static RETVAL_BUFFER: RefCell<Vec<u8>> = RefCell::new(vec![0; RETVAL_BUFFER_SIZE]);
}

pub mod ffi {
    #[link(wasm_import_module = "host")]
    extern "C" {
        pub fn invoke(
            hash: u64,
            ptr: *const crate::types::GuestArg,
            len: usize,
            retval: *const crate::types::ReturnValue,
        ) -> i32;

        pub fn invoke_ref_func(
            ref_name: *const i8,
            args: *const u8,
            args_len: usize,
            buffer: *mut u8,
            buffer_capacity: usize,
        ) -> i32;
    }
}

#[no_mangle]
pub extern "C" fn __cfx_extend_retval_buffer(new_size: usize) -> *const u8 {
    RETVAL_BUFFER.with(|retval| {
        let mut vec = retval.borrow_mut();
        vec.resize(new_size, 0);

        vec.as_ptr()
    })
}

pub enum Val<'a> {
    RefInteger(&'a i32),
    RefFloat(&'a f32),
    RefLong(&'a i64),
    RefBool(&'a bool),

    MutRefInteger(&'a mut i32),
    MutRefFloat(&'a mut f32),
    MutRefLong(&'a mut i64),
    MutRefBool(&'a mut bool),

    Integer(i32),
    Float(f32),
    Long(i64),
    Bool(bool),

    Vector3(Vector3),
    RefVector3(&'a Vector3),
    MutRefVector3(&'a mut Vector3),

    String(&'a str),
    Bytes(&'a [u8]),
    MutBytes(&'a mut [u8]),

    RefFunc(RefFunction),
}

macro_rules! impl_from {
    ($type:ty, $val:ident, $ref:ident, $mut:ident) => {
        impl<'a> From<$type> for Val<'a> {
            #[inline]
            fn from(val: $type) -> Val<'a> {
                Val::$val(val)
            }
        }

        impl<'a> From<&'a $type> for Val<'a> {
            #[inline]
            fn from(val: &'a $type) -> Val<'a> {
                Val::$ref(val)
            }
        }

        impl<'a> From<&'a mut $type> for Val<'a> {
            #[inline]
            fn from(val: &'a mut $type) -> Val<'a> {
                Val::$mut(val)
            }
        }
    };
}

impl_from!(i32, Integer, RefInteger, MutRefInteger);
impl_from!(f32, Float, RefFloat, MutRefFloat);
impl_from!(i64, Long, RefLong, MutRefLong);
impl_from!(bool, Bool, RefBool, MutRefBool);
impl_from!(Vector3, Vector3, RefVector3, MutRefVector3);

impl<'a> From<CharPtr<'a>> for Val<'a> {
    #[inline]
    fn from(char_ptr: CharPtr<'a>) -> Self {
        match char_ptr {
            CharPtr::Bytes(bytes) => Val::Bytes(bytes),
            CharPtr::String(str) => Val::String(str),
        }
    }
}

impl<'a> From<RefFunction> for Val<'a> {
    #[inline]
    fn from(ref_func: RefFunction) -> Self {
        Val::RefFunc(ref_func)
    }
}

#[derive(Debug, Clone)]
pub enum InvokeError {
    NullResult,
    NoSpace,
    Code(i32),
}

pub fn invoke<'a, Ret, Args>(hash: u64, arguments: Args) -> Result<Ret, InvokeError>
where
    Ret: RetVal,
    Args: IntoIterator<Item = &'a Val<'a>>,
{
    let iter = arguments.into_iter();
    let mut strings = Vec::new(); // cleanup memory after a call

    let args = iter
        .map(|arg| match arg {
            Val::Integer(int) => GuestArg::new(int, false),
            Val::Float(float) => GuestArg::new(float, false),
            Val::Long(long) => GuestArg::new(long, false),
            Val::Bool(bool) => GuestArg::new(bool, false),
            Val::RefInteger(int) => GuestArg::new(*int, true),
            Val::RefFloat(float) => GuestArg::new(*float, true),
            Val::RefLong(long) => GuestArg::new(*long, true),
            Val::RefBool(bool) => GuestArg::new(*bool, true),
            Val::MutRefInteger(int) => GuestArg::new(*int, true),
            Val::MutRefFloat(float) => GuestArg::new(*float, true),
            Val::MutRefLong(long) => GuestArg::new(*long, true),
            Val::MutRefBool(bool) => GuestArg::new(*bool, true),
            Val::Vector3(vec) => GuestArg::new(vec, false),
            Val::RefVector3(vec) => GuestArg::new(*vec, true),
            Val::MutRefVector3(vec) => GuestArg::new(*vec, true),

            Val::String(string) => {
                let cstr = std::ffi::CString::new(*string).unwrap();
                let ptr = cstr.as_bytes_with_nul().as_ptr();

                strings.push(cstr);

                GuestArg::new(unsafe { &*ptr }, true)
            }

            Val::Bytes(bytes) => GuestArg::new(unsafe { &*bytes.as_ptr() }, true),
            Val::MutBytes(bytes) => GuestArg::new(unsafe { &*bytes.as_ptr() }, true),

            Val::RefFunc(func) => {
                let cstr = std::ffi::CString::new(func.name()).unwrap();
                let ptr = cstr.as_bytes_with_nul().as_ptr();

                strings.push(cstr);

                GuestArg::new(unsafe { &*ptr }, true)
            }
        })
        .collect::<Vec<GuestArg>>();

    RETVAL_BUFFER.with(|buf| unsafe {
        let retval = ReturnValue::new::<Ret>(&buf.borrow());

        let ret_len = ffi::invoke(hash, args.as_ptr(), args.len(), (&retval) as *const _);

        if ret_len == call_result::NULL_RESULT {
            return Err(InvokeError::NullResult);
        }

        if ret_len == call_result::SMALL_RETURN_BUFFER {
            return Err(InvokeError::NoSpace);
        }

        if ret_len < call_result::SUCCESS {
            return Err(InvokeError::Code(ret_len));
        }

        let read_buf = std::slice::from_raw_parts(buf.borrow().as_ptr(), ret_len as usize);

        Ok(Ret::convert(read_buf))
    })
}

// TODO: Result ...
pub fn invoke_ref_func<Out, In>(func: &ExternRefFunction, args: In) -> Option<Out>
where
    In: Serialize,
    Out: DeserializeOwned,
{
    let ref_name = std::ffi::CString::new(func.name()).ok()?;
    let args = rmp_serde::to_vec_named(&args).ok()?;

    let (buffer, buffer_capacity) = RETVAL_BUFFER.with(|buf| {
        let mut buffer = buf.borrow_mut();
        (buffer.as_mut_ptr(), buffer.capacity())
    });

    let result = unsafe {
        ffi::invoke_ref_func(
            ref_name.as_ptr(),
            args.as_ptr(),
            args.len(),
            buffer,
            buffer_capacity,
        )
    };

    if result < call_result::SUCCESS {
        return None;
    }

    RETVAL_BUFFER.with(|buf| {
        let read_buf =
            unsafe { std::slice::from_raw_parts(buf.borrow().as_ptr(), result as usize) };

        rmp_serde::decode::from_read(read_buf).ok()
    })
}

/// A FiveM runtime native. Registers current resource as an event handler.
/// Means that if someone triggers an event with this name the resource will be notified.
pub fn register_resource_as_event_handler(event: &str) -> Result<(), InvokeError> {
    invoke(0xD233A168, &[Val::String(event)])
}

/// Gets a name of the current resource.
pub fn current_resource_name() -> Result<String, InvokeError> {
    invoke(0xE5E9EBBB, &[])
}
