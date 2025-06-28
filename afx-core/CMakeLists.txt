cmake_minimum_required (VERSION 3.14)

project ("afx-core")

add_afx_rpc_generator_target(afx-core-rpc client rust "${afx-core_BINARY_DIR}/bin/afx-core-openrpc.json"
    "${afx-core_SOURCE_DIR}/../interfaces/openrpc/afx-core/common.json"
    "${afx-core_SOURCE_DIR}/../interfaces/openrpc/afx-core/injector-win32.json"
)

add_custom_target(
    afx-core
    DEPENDS afx-core-rpc
)
