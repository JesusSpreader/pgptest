# DeployQt.cmake - Qt deployment for Windows
# Note: windeployqt is now called from build_internal.sh to avoid double deployment
# This file is kept for compatibility but deployment is handled by the build script

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        PATHS
            ${QT6Core_DIR}/../../../bin
            $ENV{QT_DIR}/bin
            $ENV{MINGW_PREFIX}/bin
    )
    
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        # Deployment is handled by build_internal.sh to avoid running twice
        # and to allow better control over additional DLLs
    else()
        message(WARNING "windeployqt not found - manual deployment required")
    endif()
endif()
