# DeployQt.cmake - Qt deployment for Windows
# This file handles copying Qt dependencies after build

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        PATHS
            ${QT6Core_DIR}/../../../bin
            $ENV{QT_DIR}/bin
            $ENV{MINGW_PREFIX}/bin
    )
    
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        
        add_custom_command(TARGET PCPrincipalPGP POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --release
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-compiler-runtime
                $<TARGET_FILE:PCPrincipalPGP>
            COMMENT "Deploying Qt dependencies..."
        )
    else()
        message(WARNING "windeployqt not found - manual deployment required")
    endif()
endif()
