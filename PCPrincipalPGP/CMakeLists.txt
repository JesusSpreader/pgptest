cmake_minimum_required(VERSION 3.16)
project(PCPrincipalPGP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
find_package(PkgConfig REQUIRED)

# Find GPGME
pkg_check_modules(GPGME REQUIRED gpgme)

# Find libsodium
pkg_check_modules(SODIUM REQUIRED libsodium)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PGPManager.cpp
    src/SecureStorage.cpp
    src/ConfigManager.cpp
    src/QuantumCrypto.cpp
    src/KeyProfile.cpp
    src/NotepadWidget.cpp
    src/KeyManagerDialog.cpp
    src/GenerateKeyDialog.cpp
    src/PasswordDialog.cpp
    src/EncryptDialog.cpp
    src/ProfileManager.cpp
    src/FileOperations.cpp
    src/SetupWizard.cpp
    src/MassImportDialog.cpp
    src/ContextMenuHandler.cpp
)

set(HEADERS
    include/MainWindow.h
    include/PGPManager.h
    include/SecureStorage.h
    include/ConfigManager.h
    include/QuantumCrypto.h
    include/KeyProfile.h
    include/NotepadWidget.h
    include/KeyManagerDialog.h
    include/GenerateKeyDialog.h
    include/PasswordDialog.h
    include/EncryptDialog.h
    include/ProfileManager.h
    include/FileOperations.h
    include/SetupWizard.h
    include/MassImportDialog.h
    include/ContextMenuHandler.h
)

# Resource file
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(PCPrincipalPGP WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(PCPrincipalPGP PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${GPGME_INCLUDE_DIRS}
    ${SODIUM_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(PCPrincipalPGP PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    ${GPGME_LIBRARIES}
    ${SODIUM_LIBRARIES}
)

# Compiler flags
target_compile_options(PCPrincipalPGP PRIVATE
    ${GPGME_CFLAGS_OTHER}
    ${SODIUM_CFLAGS_OTHER}
    -Wall -Wextra
)

# Link flags
target_link_options(PCPrincipalPGP PRIVATE
    -static-libgcc
    -static-libstdc++
)

# Install target
install(TARGETS PCPrincipalPGP
    RUNTIME DESTINATION .
)

# Copy required DLLs on Windows
if(WIN32)
    include(DeployQt.cmake)
endif()
