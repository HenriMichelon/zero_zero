##### compile GLSL sources files into SPIR-V
function(add_shaders TARGET_NAME BUILD_DIR SHADER_INCLUDE_DIR)
    set(SHADER_SOURCE_FILES ${ARGN}) # the rest of arguments to this function will be assigned as shader source files
    set(SHADER_BINARIES ${BUILD_DIR})

    # Validate that source files have been passed
    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if(FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif()

    set(SHADER_COMMANDS)
    set(SHADER_PRODUCTS)
    file(MAKE_DIRECTORY ${SHADER_BINARIES})

    foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)
        # Build command
        list(APPEND SHADER_COMMANDS COMMAND)
        list(APPEND SHADER_COMMANDS Vulkan::glslc)
        list(APPEND SHADER_COMMANDS "-g")
        list(APPEND SHADER_COMMANDS "-I")
        list(APPEND SHADER_COMMANDS "../${SHADER_INCLUDE_DIR}")
        list(APPEND SHADER_COMMANDS "-I")
        list(APPEND SHADER_COMMANDS "${SHADER_INCLUDE_DIR}")
        list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
        list(APPEND SHADER_COMMANDS "-o")
        list(APPEND SHADER_COMMANDS "${SHADER_BINARIES}/${SHADER_NAME}.spv")
        # Add product
        list(APPEND SHADER_PRODUCTS "${SHADER_BINARIES}/${SHADER_NAME}.spv")
    endforeach()

    add_custom_target(${TARGET_NAME} ALL
            ${SHADER_COMMANDS}
            COMMENT "Compiling Shaders [${TARGET_NAME}]"
            SOURCES ${SHADER_SOURCE_FILES}
            BYPRODUCTS ${SHADER_PRODUCTS}
    )

endfunction()
