#[cfg(feature = "in-module")]
use serde::de::DeserializeOwned;

#[repr(u32)]
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ReturnType {
    Empty = 0,
    Number,
    String,
    Vector3,
    MsgPack,
    Unk,
}

impl From<u32> for ReturnType {
    fn from(val: u32) -> Self {
        match val {
            0 => ReturnType::Empty,
            1 => ReturnType::Number,
            2 => ReturnType::String,
            3 => ReturnType::Vector3,
            4 => ReturnType::MsgPack,
            _ => ReturnType::Unk,
        }
    }
}

#[repr(C)]
#[derive(Debug, Clone)]
pub struct ReturnValue {
    pub rettype: ReturnType,
    pub buffer: u32, // ptr
    pub capacity: u32,
}

impl ReturnValue {
    #[inline]
    pub unsafe fn new<T: RetVal>(buf: &Vec<u8>) -> ReturnValue {
        ReturnValue {
            rettype: T::IDENT as _,
            buffer: buf.as_ptr() as _,
            capacity: buf.capacity() as _,
        }
    }
}

#[repr(C)]
#[derive(Debug, Clone, Default)]
pub struct Vector3 {
    pub x: f32,
    pad_0: u32,

    pub y: f32,
    pad_1: u32,

    pub z: f32,
    pad_2: u32,
}

#[repr(C)]
pub struct ScrObject {
    pub data: u64,
    pub length: u64,
}

#[cfg(feature = "in-module")]
pub struct Packed<T: DeserializeOwned> {
    inner: T,
}

#[cfg(feature = "in-module")]
impl<T: DeserializeOwned> Packed<T> {
    pub fn payload(&self) -> &T {
        &self.inner
    }

    pub fn into_inner(self) -> T {
        self.inner
    }
}

unsafe impl RetVal for () {
    const IDENT: ReturnType = ReturnType::Empty;

    #[inline]
    unsafe fn convert(_: &[u8]) -> Self {}
}

unsafe impl RetVal for String {
    const IDENT: ReturnType = ReturnType::String;

    #[inline]
    unsafe fn convert(bytes: &[u8]) -> Self {
        std::str::from_utf8_unchecked(bytes).to_owned()
    }
}

unsafe impl RetVal for Vector3 {
    const IDENT: ReturnType = ReturnType::Vector3;

    #[inline]
    unsafe fn convert(bytes: &[u8]) -> Self {
        (bytes.as_ptr() as *const Vector3).read()
    }
}

#[cfg(feature = "in-module")]
unsafe impl<T: DeserializeOwned> RetVal for Packed<T> {
    const IDENT: ReturnType = ReturnType::MsgPack;

    unsafe fn convert(bytes: &[u8]) -> Self {
        let inner = rmp_serde::from_read_ref(bytes).unwrap();
        Packed { inner }
    }
}

#[cfg(feature = "in-module")]
impl<T: Default + DeserializeOwned> Default for Packed<T> {
    fn default() -> Packed<T> {
        Packed {
            inner: T::default(),
        }
    }
}

pub unsafe trait RetVal {
    const IDENT: ReturnType;

    unsafe fn convert(bytes: &[u8]) -> Self;
}

macro_rules! impl_for_primitives {
    ($($type:ty),*) => {
        $(unsafe impl RetVal for $type {
            const IDENT: ReturnType = ReturnType::Number;

            #[inline]
            unsafe fn convert(bytes: &[u8]) -> Self {
                (bytes.as_ptr() as *const $type).read()
            }
        })*
    };
}

pub enum CharPtr<'a> {
    String(&'a str),
    Bytes(&'a [u8]),
}

pub trait AsCharPtr {
    fn as_char_ptr(&self) -> CharPtr;
}

impl AsCharPtr for &str {
    #[inline]
    fn as_char_ptr(&self) -> CharPtr {
        CharPtr::String(self)
    }
}

impl AsCharPtr for &[u8] {
    #[inline]
    fn as_char_ptr(&self) -> CharPtr {
        CharPtr::Bytes(self)
    }
}

impl_for_primitives! {
    i8, u8, i16, u16, i32, u32, i64, u64,
    f32, f64,
    bool
}

#[repr(C)]
#[derive(Default)]
pub struct GuestArg {
    pub is_ref: bool,
    pub value: u64,
    pub size: u32,
}

impl GuestArg {
    #[inline]
    pub fn new<T: Sized>(argument: &T, is_ref: bool) -> GuestArg {
        GuestArg {
            is_ref,
            value: argument as *const _ as u64,
            size: if is_ref {
                8
            } else {
                std::mem::size_of::<T>() as u32
            },
        }
    }
}

pub mod call_result {
    pub const SUCCESS: i32 = 0;
    pub const SMALL_RETURN_BUFFER: i32 = -1;
    pub const NO_RETURN_VALUE: i32 = -2;
    pub const TOO_MUCH_ARGS: i32 = -3;
    pub const NULL_RESULT: i32 = -4;
    pub const WRONG_ARGS: i32 = -5;
    pub const CRITICAL_ERROR: i32 = -6;
}
