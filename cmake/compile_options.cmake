function(compile_options TARGET_NAME )
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
                /nologo
                /W2
                /WX
                /std:c++latest
                /experimental:module
                /fp:fast
        )
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${TARGET_NAME} PRIVATE
                    /Od
                    /GS
                    /Ob0
                    /RTC1
        )
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
                /O2
                /GS-
                /Gy
                /Oi
                /Ot
        )
    endif()
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
                -Wno-deprecated-declarations
                -Wno-nullability-completeness
                -Werror
        )
        target_link_libraries(${TARGET_NAME} -static)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${TARGET_NAME} PRIVATE
                    -O0
            )
        else()
            target_compile_options(${TARGET_NAME} PRIVATE
                    -O3
                    -DNDEBUG
            )
        endif()
    endif()
    target_link_libraries(${TARGET_NAME} std-cxx-modules)
endfunction()