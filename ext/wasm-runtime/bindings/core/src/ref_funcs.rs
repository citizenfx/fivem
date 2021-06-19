use crate::types::ScrObject;
use serde::{de::DeserializeOwned, Deserialize, Serialize};
use std::{
    cell::{Cell, RefCell},
    collections::HashMap,
    rc::Rc,
};

mod ffi {
    #[link(wasm_import_module = "host")]
    extern "C" {
        pub fn canonicalize_ref(ref_idx: u32, buffer: *mut i8, buffer_size: usize) -> i32;
    }
}

thread_local! {
    static BUFFER: RefCell<Vec<u8>> = RefCell::new(vec![0; 1 << 15]);
    static RETVAL: RefCell<ScrObject> = RefCell::new(ScrObject { data: 0, length: 0 });
    static HANDLERS: RefCell<HashMap<u32, Rc<InnerRefFunction>>> = RefCell::new(HashMap::new());
    static REF_IDX: RefCell<u32> = RefCell::new(0);
}

#[no_mangle]
pub unsafe extern "C" fn __cfx_call_ref(
    ref_idx: u32,
    args: *const u8,
    args_len: usize,
) -> *const ScrObject {
    let args = std::slice::from_raw_parts(args, args_len);

    HANDLERS.with(|handlers| {
        let handlers = handlers.borrow();

        if let Some(handler) = handlers.get(&ref_idx) {
            BUFFER.with(|buf| {
                handler.handle(args, buf);

                RETVAL.with(|retval| {
                    let mut retval = retval.borrow_mut();
                    let buf = buf.borrow();

                    retval.data = buf.as_ptr() as _;
                    retval.length = buf.len() as _;
                });
            });
        }
    });

    RETVAL.with(|scr| scr.as_ptr())
}

#[no_mangle]
pub unsafe extern "C" fn __cfx_duplicate_ref(ref_idx: u32) -> u32 {
    HANDLERS.with(|handlers| {
        let handlers = handlers.borrow();
        if let Some(handler) = handlers.get(&ref_idx) {
            let refs = handler.refs.get();
            handler.refs.set(refs + 1);
        }
    });

    ref_idx
}

#[no_mangle]
pub unsafe extern "C" fn __cfx_remove_ref(ref_idx: u32) {
    HANDLERS.with(|handlers| {
        let remove = {
            let handlers = handlers.borrow();

            if let Some(handler) = handlers.get(&ref_idx) {
                let refs = handler.refs.get();
                handler.refs.set(refs - 1);

                refs <= 1
            } else {
                false
            }
        };

        if remove {
            handlers.borrow_mut().remove(&ref_idx);
        }
    });
}

fn canonicalize_ref(ref_idx: u32) -> String {
    thread_local! {
        static CANON_REF: RefCell<Vec<u8>> = RefCell::new(vec![0; 1024]);
    }

    CANON_REF.with(|vec| {
        let mut resized = false;

        loop {
            let (buffer, buffer_size) = {
                let mut vec = vec.borrow_mut();

                (vec.as_mut_ptr() as *mut _, vec.capacity())
            };

            let result = unsafe { ffi::canonicalize_ref(ref_idx, buffer, buffer_size) };

            if result == 0 {
                // some error?
                return String::new();
            }

            if result < 0 {
                if resized {
                    return String::new();
                }

                vec.borrow_mut().resize(result.abs() as _, 0);
                resized = true;
            } else {
                unsafe {
                    let slice = std::slice::from_raw_parts(buffer as *mut u8, (result - 1) as _);

                    return std::str::from_utf8_unchecked(slice).to_owned();
                }
            }
        }
    })
}

struct InnerRefFunction {
    _idx: u32,
    func: Box<dyn Fn(&[u8], &RefCell<Vec<u8>>)>,
    refs: Cell<i32>,
}

impl InnerRefFunction {
    fn handle(&self, input: &[u8], output: &RefCell<Vec<u8>>) {
        (self.func)(input, output);
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename = "_ExtStruct")]
pub struct ExternRefFunction((i8, serde_bytes::ByteBuf));

impl ExternRefFunction {
    pub(crate) fn new(name: &str) -> ExternRefFunction {
        let bytes = serde_bytes::ByteBuf::from(name.bytes().collect::<Vec<u8>>());
        ExternRefFunction((10, bytes))
    }

    pub(crate) fn name(&self) -> &str {
        unsafe { std::str::from_utf8_unchecked(&self.0 .1) }
    }

    pub fn invoke<Out, In>(&self, args: In) -> Option<Out>
    where
        In: Serialize,
        Out: DeserializeOwned,
    {
        crate::invoker::invoke_ref_func(&self, args)
    }
}

#[derive(Clone)]
pub struct RefFunction {
    name: String,
    inner: Rc<InnerRefFunction>,
}

impl RefFunction {
    pub fn new<Handler, Input, Output>(handler: Handler) -> RefFunction
    where
        Handler: Fn(Input) -> Output + 'static,
        Input: DeserializeOwned,
        Output: Serialize,
    {
        let idx = REF_IDX.with(|idx| {
            let mut idx = idx.borrow_mut();
            *idx += 1;
            *idx
        });

        let name = canonicalize_ref(idx);

        let func = move |input: &[u8], out_buf: &RefCell<Vec<u8>>| {
            let input = rmp_serde::decode::from_read(input).unwrap();
            let out = handler(input);
            {
                let mut out_buf = out_buf.borrow_mut();

                unsafe {
                    out_buf.set_len(0);
                }

                let _ = rmp_serde::encode::write_named(&mut *out_buf, &out);
            }
        };

        let inner = InnerRefFunction {
            _idx: idx,
            func: Box::new(func),
            refs: Cell::new(0),
        };

        let inner = Rc::new(inner);

        HANDLERS.with(|handlers| {
            let mut handlers = handlers.borrow_mut();
            handlers.insert(idx, inner.clone());
        });

        RefFunction { name, inner }
    }

    pub fn new_raw<Handler>(handler: Handler) -> RefFunction
    where
        Handler: Fn(&[u8]) -> Vec<u8> + 'static,
    {
        let idx = REF_IDX.with(|idx| {
            let mut idx = idx.borrow_mut();
            *idx += 1;
            *idx
        });

        let name = canonicalize_ref(idx);

        let func = move |input: &[u8], out_buf: &RefCell<Vec<u8>>| {
            let out = handler(input);
            {
                let mut out_buf = out_buf.borrow_mut();

                unsafe {
                    out_buf.set_len(0);
                }

                out_buf.extend(out.iter());
            }
        };

        let inner = InnerRefFunction {
            _idx: idx,
            func: Box::new(func),
            refs: Cell::new(0),
        };

        let inner = Rc::new(inner);

        HANDLERS.with(|handlers| {
            let mut handlers = handlers.borrow_mut();
            handlers.insert(idx, inner.clone());
        });

        RefFunction { name, inner }
    }

    pub(crate) fn name(&self) -> &str {
        &self.name
    }

    pub fn as_extern_ref_func(&self) -> ExternRefFunction {
        ExternRefFunction::new(self.name())
    }
}
