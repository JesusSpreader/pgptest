#!/bin/bash

# PC Principal PGP - Internal Build Script
# This script runs inside MSYS2 MinGW 64-bit environment

set -e  # Exit on error

echo "========================================"
echo "  PC Principal PGP - Build Script"
echo "========================================"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT_DIR="$SCRIPT_DIR/PCPrincipalPGP_Portable"

echo "Source: $SOURCE_DIR"
echo "Build:  $BUILD_DIR"
echo "Output: $OUTPUT_DIR"
echo ""

# Step 1: Check dependencies
echo "[1/6] Checking dependencies..."

# Check for required tools
for tool in cmake make g++ pkg-config; do
    if ! command -v $tool &> /dev/null; then
        echo "ERROR: $tool not found. Please install it first."
        exit 1
    fi
done

# Check for Qt6
if ! pkg-config --exists Qt6Core Qt6Widgets; then
    echo "ERROR: Qt6 not found. Install with: pacman -S mingw-w64-x86_64-qt6-base"
    exit 1
fi

# Check for GPGME
if ! pkg-config --exists gpgme; then
    echo "ERROR: GPGME not found. Install with: pacman -S mingw-w64-x86_64-gpgme"
    exit 1
fi

# Check for libsodium
if ! pkg-config --exists libsodium; then
    echo "ERROR: libsodium not found. Install with: pacman -S mingw-w64-x86_64-libsodium"
    exit 1
fi

# Check for windeployqt
if ! command -v windeployqt &> /dev/null; then
    echo "WARNING: windeployqt not found. Qt deployment may fail."
fi

echo "All dependencies ready!"
echo ""

# Step 2: Clean previous build
echo "[2/6] Cleaning previous build..."
rm -rf "$BUILD_DIR"
rm -rf "$OUTPUT_DIR"
mkdir -p "$BUILD_DIR"
echo ""

# Step 3: Configure with CMake
echo "[3/6] Configuring with CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_COMPILER=g++ \
         -DCMAKE_C_COMPILER=gcc

echo ""

# Step 4: Build
echo "[4/6] Building... (this may take a few minutes)"
cmake --build . --parallel $(nproc)
echo ""

# Step 5: Create portable package
echo "[5/6] Creating portable package..."
mkdir -p "$OUTPUT_DIR"

# Copy executable
cp "$BUILD_DIR/PCPrincipalPGP.exe" "$OUTPUT_DIR/"

# Copy icon if it exists
if [ -f "$SOURCE_DIR/resources/icon.png" ]; then
    cp "$SOURCE_DIR/resources/icon.png" "$OUTPUT_DIR/"
fi

# Create necessary directories
mkdir -p "$OUTPUT_DIR/keys/public"
mkdir -p "$OUTPUT_DIR/keys/private"
mkdir -p "$OUTPUT_DIR/profiles"
mkdir -p "$OUTPUT_DIR/gpg"
mkdir -p "$OUTPUT_DIR/temp"

echo ""

# Step 6: Deploy Qt dependencies and additional DLLs
echo "[6/6] Deploying Qt dependencies and GPG..."

# Run windeployqt
echo "Running windeployqt..."
windeployqt "$OUTPUT_DIR/PCPrincipalPGP.exe" --release --no-translations 2>/dev/null || true

# Find MSYS2 installation path
MSYS2_PATH=""
if [ -n "$MINGW_PREFIX" ]; then
    MSYS2_PATH="$(dirname "$MINGW_PREFIX")"
elif [ -d "/mingw64" ]; then
    MSYS2_PATH="/"
elif [ -d "C:/msys64" ]; then
    MSYS2_PATH="C:/msys64"
elif [ -d "D:/msys64" ]; then
    MSYS2_PATH="D:/msys64"
fi

if [ -z "$MSYS2_PATH" ]; then
    echo "WARNING: Could not find MSYS2 installation path"
else
    echo "Found MSYS2 at: $MSYS2_PATH"
    
    MINGW_BIN="$MSYS2_PATH/mingw64/bin"
    
    # Copy GPG and related executables
    echo "Copying GPG executables..."
    for exe in gpg.exe gpg-agent.exe gpgsm.exe scdaemon.exe dirmngr.exe; do
        if [ -f "$MINGW_BIN/$exe" ]; then
            cp "$MINGW_BIN/$exe" "$OUTPUT_DIR/"
            echo "  Copied: $exe"
        fi
    done
    
    # Copy GPG configuration
    if [ -d "$MSYS2_PATH/mingw64/etc/gnupg" ]; then
        cp -r "$MSYS2_PATH/mingw64/etc/gnupg" "$OUTPUT_DIR/"
        echo "  Copied: gnupg config"
    fi
    
    # Copy additional required DLLs that windeployqt might miss
    echo "Copying additional DLLs..."
    ADDITIONAL_DLLS=(
        "libicuin78.dll"
        "libicuuc78.dll"
        "libicudt78.dll"
        "libdouble-conversion.dll"
        "libsodium-26.dll"
        "libpcre2-16-0.dll"
        "libzstd.dll"
        "libharfbuzz-0.dll"
        "libfreetype-6.dll"
        "libpng16-16.dll"
        "libbz2-1.dll"
        "libbrotlidec.dll"
        "libbrotlicommon.dll"
        "libglib-2.0-0.dll"
        "libintl-8.dll"
        "libiconv-2.dll"
        "libgraphite2.dll"
        "libmd4c.dll"
        "libgpgme-11.dll"
        "libgpg-error-0.dll"
        "libassuan-0.dll"
        "libgcrypt-20.dll"
        "libgnutls-30.dll"
        "libnettle-8.dll"
        "libhogweed-6.dll"
        "libgmp-10.dll"
        "libidn2-0.dll"
        "libunistring-5.dll"
        "libtasn1-6.dll"
        "libp11-kit-0.dll"
        "libsqlite3-0.dll"
        "liblz4.dll"
        "liblzma-5.dll"
        "libxml2-2.dll"
        "libxslt-1.dll"
        "libssh2-1.dll"
        "libcurl-4.dll"
        "libnghttp2-14.dll"
        "libpsl-5.dll"
        "libbrotlienc.dll"
        "libz.dll"
        "libwinpthread-1.dll"
        "libgcc_s_seh-1.dll"
        "libstdc++-6.dll"
    )
    
    for dll in "${ADDITIONAL_DLLS[@]}"; do
        if [ -f "$MINGW_BIN/$dll" ]; then
            # Only copy if not already present
            if [ ! -f "$OUTPUT_DIR/$dll" ]; then
                cp "$MINGW_BIN/$dll" "$OUTPUT_DIR/"
                echo "  Copied: $dll"
            fi
        fi
    done
fi

echo ""

# Count DLLs
echo "DLLs in output directory:"
ls -1 "$OUTPUT_DIR"/*.dll 2>/dev/null | wc -l | xargs echo "Total DLL count:"
echo ""

# Create a README for the portable package
cat > "$OUTPUT_DIR/README.txt" << 'EOF'
PC Principal PGP - Portable Edition
====================================

This is a portable version of PC Principal PGP.
No installation required - just run PCPrincipalPGP.exe

First Time Setup:
1. Run PCPrincipalPGP.exe
2. Follow the setup wizard to configure your key storage
3. Start encrypting!

Directory Structure:
- keys/public/     : Public PGP keys
- keys/private/    : Private PGP keys  
- profiles/        : Contact profiles
- gpg/             : GPG configuration
- temp/            : Temporary files

For help, visit: https://github.com/JesusSpreader/pgptest
EOF

echo "========================================"
echo "  BUILD SUCCESSFUL!"
echo "========================================"
echo ""
echo "Your portable PGP application is ready at:"
echo "  $OUTPUT_DIR"
echo ""
echo "To use:"
echo "1. Copy 'PCPrincipalPGP_Portable' to USB or keep it here"
echo "2. Run PCPrincipalPGP.exe"
echo "3. Follow the setup wizard"
echo ""
echo "Features:"
echo "- Generate/import/export PGP keys"
echo "- Encrypt/decrypt messages and files"
echo "- Sign and verify signatures"
echo "- Manage contact profiles"
echo "- Post-quantum encryption support"
echo "- 100% portable - no installation needed"
echo ""
