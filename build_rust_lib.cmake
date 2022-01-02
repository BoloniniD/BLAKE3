function(build_cargo target_name project_dir)
    set(output_library ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/lib${target_name}.a)
    file(GLOB sources ${project_dir}/src/**/*.rs)

    set(compile_message "Compiling ${target_name}")

    if(CARGO_RELEASE_FLAG STREQUAL "--release")
        set(compile_message "${compile_message} in release mode")
    endif()

    add_custom_command(
        COMMENT ${compile_message}
        COMMAND env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} cargo build ${CARGO_RELEASE_FLAG}
        COMMAND cp ${output_library} ${CMAKE_CURRENT_BINARY_DIR}
        OUTPUT ${output_library}
        WORKING_DIRECTORY ${project_dir})

    if(NOT TARGET ${target_name})
        add_custom_target(${target_name} ALL DEPENDS ${output_library})
    endif()

    set_property(
        TARGET ${target_name}
        APPEND PROPERTY
            INTERFACE_DEPENDENCIES ${output_library}
    )

    set_target_properties(${target_name} PROPERTIES LOCATION ${output_library})
endfunction()