cmake_minimum_required (VERSION 3.14)

project ("afx-rpc-generator")

add_custom_target(
    afx-rpc-generator
    DEPENDS npm
    WORKING_DIRECTORY "${afx-rpc-generator_SOURCE_DIR}"
    COMMAND npm install 
)

function(add_afx_rpc_generator_target target_name target_type target_lang output_file)
    add_custom_target(
        ${target_name}
        BYPRODUCTS  ${output_file}
        DEPENDS afx-rpc-generator
        WORKING_DIRECTORY "${afx-rpc-generator_SOURCE_DIR}"
        VERBATIM
        COMMAND_EXPAND_LISTS
        COMMAND npm start -- --name "${target_name}" --type "${target_type}" --lang ${target_lang} "$<$<BOOL:${ARGN}>:$<JOIN:${ARGN},;>>" "${output_file}"
        SOURCES ${ARGN}
    )
endfunction()
