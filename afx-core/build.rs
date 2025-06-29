extern crate embed_resource;

use std::env;
use std::fs;
use std::path::Path;

fn main() {
    embed_resource::compile("afx-core.rc", embed_resource::NONE).manifest_optional().unwrap();

    let rpc_env = env::var("AFX_CORE_RPC").unwrap();
    let out_env = env::var("OUT_DIR").unwrap();
    let rpc_dir = Path::new(&rpc_env);
    let out_dir = Path::new(&out_env);
    fs::copy(Path::join(rpc_dir, "client/rust/src/index.rs"), Path::join(out_dir, "afx-core-rpc.rs")).unwrap();
    fs::copy(Path::join(rpc_dir, "client/rust/src/openrpc.json"), Path::join(out_dir, "afx-core-rpc.json")).unwrap();
}
