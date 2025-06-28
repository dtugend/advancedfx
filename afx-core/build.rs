extern crate embed_resource;

fn main() {
    embed_resource::compile("afx-core.rc", embed_resource::NONE).manifest_optional().unwrap();
}
