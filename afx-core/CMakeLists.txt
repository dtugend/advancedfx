cmake_minimum_required (VERSION 3.14)

project ("afx-core")

add_afx_rpc_generator_target(afx-core-rpc client rust "${afx-core_BINARY_DIR}/rpc/openrpc.json"
    "${afx-core_SOURCE_DIR}/../interfaces/openrpc/afx-core/common.json"
    "${afx-core_SOURCE_DIR}/../interfaces/openrpc/afx-core/injector-win32.json"
)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml)

add_dependencies(cargo-prebuild_afx-core afx-core-rpc)

corrosion_set_env_vars(afx-core "AFX_CORE_RPC=${afx-core_BINARY_DIR}/rpc")
