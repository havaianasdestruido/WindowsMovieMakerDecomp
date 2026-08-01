use libloading::{Library, Symbol};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Adjust path if needed; assumes DLL is beside the executable or in PATH
    let lib = unsafe { Library::new("MovieMakerCore.dll")? };
    unsafe {
        let func: Symbol<unsafe extern "C" fn()> = lib.get(b"MovieMakerMain\0")?;
        func();
    }
    Ok(())
}
