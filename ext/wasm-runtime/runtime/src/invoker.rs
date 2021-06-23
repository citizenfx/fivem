use crate::ptr_out_of_bounds;
use cfx_wasm_rt_types::{call_result::*, GuestArg, ReturnType, ReturnValue, ScrObject, Vector3};
use std::ffi::CStr;
use wasmtime::{Caller, Func, Memory, Val};

pub type InvokeFunc = extern "C" fn(args: *mut NativeContext) -> u32;

static mut INVOKE: Option<InvokeFunc> = None;

const INVOKE_FUNCTION_REFERENCE: u64 = 0xE3551879;
const CFX_EXTEND_RETVAL_BUFFER: &str = "__cfx_extend_retval_buffer";

/// A generic (?) structure passed to the `Invoke` function of FiveM runtime.
/// `citizen-scripting-core/include/fxNativeContext.h`
#[repr(C)]
#[derive(Default)]
pub struct NativeContext {
    arguments: [usize; 32],
    num_arguments: u32,
    num_results: u32,
    native_identifier: u64,
}

/// An critical error that happened at invoking native.
#[derive(Debug)]
pub enum NativeError {
    /// WASM module has no memory export.
    NoMemory,
    /// `IScriptHost::InvokeNative` returns an error.
    InvokeError,
    IncorrectPtr,
}

/// A result of executing native functions. This results (even if not succeeded) isn't critical, meaning the runtime
/// can continue an execution process.
/// All results can be converted to `i32` values that is defined in `fivem::types::call_result`.
#[derive(Debug, Clone, Copy)]
pub enum CallResult {
    /// A required argument wasn't passed (currently used only in `invoke_ref_func_wrapper` when 0 pointer in ref_name).
    WrongArgs,
    /// A native function returns 0 pointer for strings / buffers.
    NullResult,
    /// A guest script passed more than 32 arguments.
    TooMuchArgs,
    /// For results that cannot be described by another results.
    NoReturn,
    /// A buffer with a small size (not capable to store the requested return value) was passed.
    /// WASM runtime tries to call `__cfx_extend_retval_buffer` before rising this result.
    SmallReturnBuffer,
    /// Return value has been written in the buffer for a return value.
    OkWithLen(u32),
    /// Success execution, used when a guest doesn't want a return value (or zero-sized type).
    Ok,
}

impl Into<i32> for CallResult {
    fn into(self) -> i32 {
        match self {
            CallResult::WrongArgs => WRONG_ARGS,
            CallResult::NullResult => NULL_RESULT,
            CallResult::TooMuchArgs => TOO_MUCH_ARGS,
            CallResult::NoReturn => NO_RETURN_VALUE,
            CallResult::SmallReturnBuffer => SMALL_RETURN_BUFFER,
            CallResult::OkWithLen(len) => len as _,
            CallResult::Ok => SUCCESS,
        }
    }
}

#[inline]
pub fn call_native_wrapper<'a, T>(
    mut caller: Caller<'a, T>,
    hash: u64,
    ptr: i32,
    len: i32,
    retval: i32,
) -> Result<CallResult, NativeError> {
    let mut args = None;

    let mem = caller
        .get_export("memory")
        .and_then(|ext| ext.into_memory())
        .ok_or(NativeError::NoMemory)?;

    if ptr_out_of_bounds!(ptr, len, mem, &caller) {
        return Err(NativeError::IncorrectPtr);
    }

    if len > 0 && ptr != 0 {
        unsafe {
            let ptr = mem.data_ptr(&mut caller).add(ptr as _) as *const GuestArg;
            args = Some(std::slice::from_raw_parts(ptr, len as _));
        }
    }

    let retval = if ptr_out_of_bounds!(retval, mem, &caller) {
        None
    } else {
        Some(unsafe {
            let ptr = mem.data_ptr(&mut caller).add(retval as _) as *const ReturnValue;
            (&*ptr).clone()
        })
    };

    let resize_func = caller
        .get_export(CFX_EXTEND_RETVAL_BUFFER)
        .and_then(|export| export.into_func());

    let call_result = call_native(
        caller,
        hash,
        args.unwrap_or_else(|| &[]),
        mem,
        retval,
        resize_func,
    )?;

    Ok(call_result)
}

#[inline]
fn call_native<'a, T>(
    mut caller: Caller<'a, T>,
    hash: u64,
    args: &[GuestArg],
    memory: Memory,
    retval: Option<ReturnValue>,
    resize_func: Option<Func>,
) -> Result<CallResult, NativeError> {
    let mut ctx = NativeContext::default();

    ctx.native_identifier = hash;
    ctx.num_arguments = args.len() as _;

    let mem_start = memory.data_ptr(&mut caller);
    let ctx_args = ctx.arguments.as_mut_ptr() as *mut u8;

    let mut idx = 0;
    for arg in args.iter() {
        let arg_size = std::cmp::max(std::mem::size_of::<usize>(), arg.size as usize);

        if (idx + arg_size) > (ctx.arguments.len() * std::mem::size_of::<usize>()) {
            return Ok(CallResult::TooMuchArgs);
        }

        unsafe {
            if arg.is_ref {
                if ptr_out_of_bounds!(arg.value, memory, &caller) {
                    return Err(NativeError::IncorrectPtr);
                }

                (ctx_args.add(idx) as *mut usize).write(mem_start.add(arg.value as usize) as _);
            } else {
                std::ptr::copy(
                    mem_start.add(arg.value as usize),
                    ctx_args.add(idx),
                    arg.size as _,
                );
            }
        }

        idx += arg_size;
    }

    if let Some(invoke) = unsafe { INVOKE } {
        if !crate::fx_succeeded(invoke(&mut ctx)) {
            return Err(NativeError::InvokeError);
        }
    }

    if let Some(retval) = retval {
        if ctx.num_results == 0 && retval.rettype == ReturnType::Empty {
            return Ok(CallResult::Ok);
        }

        if ptr_out_of_bounds!(retval.buffer, retval.capacity, memory, &caller) {
            return Err(NativeError::IncorrectPtr);
        }

        let mut buffer = unsafe { memory.data_ptr(&mut caller).add(retval.buffer as _) };

        let mut resize_buffer = |new_size: usize| -> Option<*mut u8> {
            if let Some(resizer) = resize_func.as_ref() {
                let ptr = resizer.call(&mut caller, &[Val::I32(new_size as _)]).ok()?;

                ptr.get(0).and_then(|val| val.i32()).and_then(|ptr| {
                    if ptr == 0 {
                        None
                    } else {
                        Some(unsafe { memory.data_ptr(&mut caller).add(ptr as _) })
                    }
                })
            } else {
                None
            }
        };

        match retval.rettype {
            ReturnType::Empty => return Ok(CallResult::Ok),
            ReturnType::Number => {
                if retval.capacity < 8 {
                    if let Some(new_buffer) = resize_buffer(8) {
                        buffer = new_buffer
                    } else {
                        return Ok(CallResult::SmallReturnBuffer);
                    }
                }

                unsafe {
                    *(buffer as *mut usize) = ctx.arguments[0];
                }

                return Ok(CallResult::OkWithLen(8));
            }

            ReturnType::String => {
                if ctx.arguments[0] == 0 {
                    return Ok(CallResult::NullResult);
                }

                let cstr = unsafe { CStr::from_ptr(ctx.arguments[0] as *const _) };
                let bytes = cstr.to_bytes();
                let len = bytes.len();

                if len == 0 {
                    return Ok(CallResult::Ok);
                }

                if retval.capacity < len as _ {
                    if let Some(new_buffer) = resize_buffer(len) {
                        buffer = new_buffer
                    } else {
                        return Ok(CallResult::SmallReturnBuffer);
                    }
                }

                unsafe {
                    std::ptr::copy(bytes.as_ptr(), buffer, len);
                }

                return Ok(CallResult::OkWithLen(len as _));
            }

            ReturnType::Vector3 => {
                let vec = ctx.arguments.as_ptr() as *const Vector3;
                let len = std::mem::size_of::<Vector3>();

                if retval.capacity < len as _ {
                    if let Some(new_buffer) = resize_buffer(len) {
                        buffer = new_buffer
                    } else {
                        return Ok(CallResult::SmallReturnBuffer);
                    }
                }

                unsafe {
                    std::ptr::copy(vec, buffer as *mut _, 1);
                }

                return Ok(CallResult::OkWithLen(len as _));
            }

            ReturnType::MsgPack => {
                let scrobj = unsafe { &*(ctx.arguments.as_ptr() as *const ScrObject) };

                let len = scrobj.length;

                if len == 0 || scrobj.data == 0 {
                    return Ok(CallResult::NullResult);
                }

                if (retval.capacity as u64) < len {
                    if let Some(new_buffer) = resize_buffer(len as usize) {
                        buffer = new_buffer
                    } else {
                        return Ok(CallResult::SmallReturnBuffer);
                    }
                }

                unsafe {
                    std::ptr::copy(scrobj.data as *const u8, buffer, len as _);
                }

                return Ok(CallResult::OkWithLen(len as _));
            }

            ReturnType::Unk => CallResult::NoReturn,
        };
    } else if ctx.num_results == 0 {
        return Ok(CallResult::Ok);
    }

    Ok(CallResult::NoReturn)
}

#[inline]
pub fn invoke_ref_func_wrapper<'a, T>(
    mut caller: Caller<'a, T>,
    ref_name: i32,
    args: i32,
    args_len: i32,
    buffer: i32,
    buffer_cap: i32,
) -> Result<CallResult, NativeError> {
    let mem = caller
        .get_export("memory")
        .and_then(|ext| ext.into_memory())
        .ok_or(NativeError::NoMemory)?;

    let resize_func = caller
        .get_export(CFX_EXTEND_RETVAL_BUFFER)
        .and_then(|export| export.into_func());

    if ref_name == 0 || args == 0 {
        return Ok(CallResult::WrongArgs);
    }

    if ptr_out_of_bounds!(args, args_len, mem, &caller) {
        return Err(NativeError::IncorrectPtr);
    }

    if ptr_out_of_bounds!(buffer, buffer_cap, mem, &caller) {
        return Err(NativeError::IncorrectPtr);
    }

    let mut ctx = NativeContext::default();

    ctx.native_identifier = INVOKE_FUNCTION_REFERENCE;
    ctx.num_arguments = 4;

    let mut retval_length = 0usize;

    unsafe {
        ctx.arguments[0] = mem.data_ptr(&mut caller).add(ref_name as _) as _; // char* referenceIdentity
        ctx.arguments[1] = mem.data_ptr(&mut caller).add(args as _) as _; // char* argsSerialized
        ctx.arguments[2] = args_len as _; // int argsLength
        ctx.arguments[3] = &mut retval_length as *mut _ as _; // int* retvalLength

        if let Some(invoke) = INVOKE {
            if !crate::fx_succeeded(invoke(&mut ctx)) {
                return Err(NativeError::InvokeError);
            }
        }
    }

    if ctx.num_results == 0 || ctx.arguments[0] == 0 {
        return Ok(CallResult::NullResult);
    }

    let mut resize_buffer = |new_size: usize| -> Option<*mut u8> {
        if let Some(resizer) = resize_func.as_ref() {
            let ptr = resizer.call(&mut caller, &[Val::I32(new_size as _)]).ok()?;

            ptr.get(0).and_then(|val| val.i32()).and_then(|ptr| {
                if ptr == 0 {
                    None
                } else {
                    Some(unsafe { mem.data_ptr(&mut caller).add(ptr as _) })
                }
            })
        } else {
            None
        }
    };

    let buffer = if buffer == 0 || (buffer_cap as usize) < retval_length {
        resize_buffer(retval_length)
    } else {
        unsafe { Some(mem.data_ptr(&mut caller).add(buffer as _)) }
    };

    if let Some(buffer) = buffer {
        unsafe {
            std::ptr::copy(ctx.arguments[0] as *const u8, buffer, retval_length);
        }

        return Ok(CallResult::OkWithLen(retval_length as _));
    }

    Ok(CallResult::SmallReturnBuffer)
}

pub fn set_native_invoke(func: InvokeFunc) {
    unsafe {
        INVOKE = Some(func);
    }
}
