use crate::{
    events::Event,
    ref_funcs::{ExternRefFunction, RefFunction},
};

use serde::{Deserialize, Serialize};
use std::{cell::RefCell, rc::Rc};

/// Imports a function from another resource
///
/// # Example
/// ```js
/// // resource name: qool
/// exports("print", (a, b, c) => console.log(`int: ${a} float: ${b} string: ${c}`));
/// ```
///
/// ```rust,ignore
/// #[derive(Serialize)]
/// struct Print(u32, f32, String);
///
/// let func = fivem::exports::import_function("qool", "print")?;
/// func.invoke::<(), _>(Print(512, 3.14, "you rocks".to_owned()));
/// ```
pub fn import_function(resource: &str, export: &str) -> Option<ExternRefFunction> {
    #[derive(Serialize, Deserialize)]
    struct Export(ExternRefFunction);

    let link = Rc::new(RefCell::new(None));
    let link_clone = link.clone();
    let export = export_name(resource, export);

    let func = RefFunction::new(move |input: Vec<Export>| -> Vec<bool> {
        *link_clone.borrow_mut() = input.get(0).map(|exp| exp.0.clone());

        vec![true]
    });

    let export_data = func.as_extern_ref_func();
    crate::events::emit(&export, vec![Export(export_data)]);

    let result = link.borrow();

    result.as_ref().cloned()
}

/// Make an export of the current resource with a given name.
///
/// # Example
/// ```rust,ignore
/// #[derive(Debug, Deserialize)]
/// struct Vector {
///     x: f32,
///     y: f32,
///     z: f32,
/// }
///
/// // define a function that will be exported
/// let export = RefFunction::new(|vector: Vec<Vector>| {
///     if let Some(vec) = vector.get(0) {
///         let length = (vec.x.powi(2) + vec.y.powi(2) + vec.z.powi(2)).sqrt();
///         return vec![length];
///     }
///
///     vec![0.0]
/// });
///
/// // make an actial export with `vecLength` name.
/// // for example the current resource name is `vectors`
/// // this export can be called from another resources.
/// // js: const length = exports.vectors.vecLength({ x: 21.0, y: 5.0, z: 12.5 });
/// fivem::exports::make_export("vecLength", export);
/// ```
pub fn make_export(export: &str, func: RefFunction) {
    #[derive(Serialize, Deserialize)]
    struct GetExport {
        func: ExternRefFunction,
    }

    let resource = crate::invoker::current_resource_name().unwrap();
    let export = export_name(&resource, export);

    crate::events::set_event_handler_closure(
        &export,
        move |event: Event<GetExport>| {
            let ext_func = &event.payload().func;
            ext_func.invoke::<(), _>(vec![func.as_extern_ref_func()]);
        },
        crate::events::EventScope::Local,
    );
}

fn export_name(resource: &str, export: &str) -> String {
    format!("__cfx_export_{}_{}", resource, export)
}
