function(build_cargo target_name project_dir)
    file(GLOB sources ${project_dir}/src/**/*.rs)

    set(compile_message "Compiling ${target_name}")

    if(CARGO_RELEASE_FLAG STREQUAL "--release")
        set(compile_message "${compile_message} in release mode")
    endif()
    
    set(TARGET_SPEC "")

    message(STATUS "Toolchain file for ${target_name}: ${CMAKE_TOOLCHAIN_FILE}")
    if(CMAKE_TOOLCHAIN_FILE MATCHES "toolchain-aarch64")
        set(TARGET_SPEC "aarch64-unknown-linux-gnu")
        message("Checking Rust toolchain for special target")
        execute_process(COMMAND rustup target add aarch64-unknown-linux-gnu)
        message(STATUS "Switch Rust target to ${TARGET_SPEC}")
        set(compile_message "${compile_message} for special target ${TARGET_SPEC}")
    endif()

    set(output_library ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_SPEC}/${TARGET_DIR}/lib${target_name}.a)

    set(TARGET_SPEC "--target=${TARGET_SPEC}")

    if(TARGET_SPEC STREQUAL "--target=")
        set(TARGET_SPEC "")
    endif()

    add_custom_command(
        COMMENT ${compile_message}
        COMMAND env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} cargo build ${CARGO_RELEASE_FLAG} ${TARGET_SPEC}
        COMMAND cp ${output_library} ${CMAKE_CURRENT_BINARY_DIR}
        OUTPUT ${output_library}
        WORKING_DIRECTORY ${project_dir})

    if(NOT TARGET ${target_name}-target)
        add_custom_target(${target_name}-target ALL DEPENDS ${output_library})
    endif()

    set_property(
        TARGET ${target_name}-target
        APPEND PROPERTY
            INTERFACE_DEPENDENCIES ${output_library}
    )

    set_target_properties(${target_name}-target PROPERTIES LOCATION ${output_library})
    
    add_library(ch_contrib::${target_name} STATIC IMPORTED GLOBAL)
    
    add_dependencies(ch_contrib::${target_name} ${target_name}-target)
    
    set_target_properties(ch_contrib::${target_name}
    	PROPERTIES
    	IMPORTED_LOCATION ${output_library}
    	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include/)
    
endfunction()


