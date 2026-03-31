#!/bin/bash
# PC Principal PGP - Build Script with Maximum Static Linking

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT_DIR="$SCRIPT_DIR/PCPrincipalPGP_Portable"
MSYS_BIN="/c/msys64/mingw64/bin"

echo "========================================"
echo "  PC Principal PGP - Build Script"
echo "========================================"
echo ""
echo "Source: $SCRIPT_DIR"
echo "Build:  $BUILD_DIR"
echo "Output: $OUTPUT_DIR"
echo ""

# [1/5] Check dependencies
echo "[1/5] Checking dependencies..."
command -v cmake >/dev/null 2>&1 || { echo "cmake required but not installed."; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "ninja required but not installed."; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "g++ required but not installed."; exit 1; }
pkg-config --exists gpgme || { echo "gpgme not found. Install with: pacman -S mingw-w64-x86_64-gpgme"; exit 1; }
pkg-config --exists libsodium || { echo "libsodium not found. Install with: pacman -S mingw-w64-x86_64-libsodium"; exit 1; }
echo "All dependencies ready!"

# [2/5] Clean previous build
echo ""
echo "[2/5] Cleaning previous build..."
rm -rf "$BUILD_DIR"
rm -rf "$OUTPUT_DIR"

# [3/5] Configure with CMake
echo ""
echo "[3/5] Configuring with CMake..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with static linking options
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++" \
    "$SCRIPT_DIR"

# [4/5] Building
echo ""
echo "[4/5] Building... (this may take a few minutes)"
cd "$BUILD_DIR"
ninja -j$(nproc)

# [5/5] Deploy Qt dependencies and create portable package
echo ""
echo "[5/5] Deploying Qt dependencies and creating portable package..."

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Copy the executable
cp "$BUILD_DIR/PCPrincipalPGP.exe" "$OUTPUT_DIR/"

# Run windeployqt to deploy Qt dependencies
echo "Running windeployqt..."
windeployqt --release \
    --no-translations \
    --no-system-d3d-compiler \
    --no-opengl-sw \
    --no-compiler-runtime \
    "$OUTPUT_DIR/PCPrincipalPGP.exe"

# Copy additional Qt/MinGW dependencies that windeployqt misses
echo "Copying additional DLLs..."

# Function to copy DLL with error handling
copy_dll() {
    local dll_name="$1"
    local dest="$2"
    if [ -f "$MSYS_BIN/$dll_name" ]; then
        cp "$MSYS_BIN/$dll_name" "$dest/"
        echo "  Copied: $dll_name"
    else
        echo "  WARNING: $dll_name not found"
    fi
}

# Core ICU libraries (required by Qt)
copy_dll "libicuin78.dll" "$OUTPUT_DIR"
copy_dll "libicuuc78.dll" "$OUTPUT_DIR"
copy_dll "libicudt78.dll" "$OUTPUT_DIR"

# Double conversion library
copy_dll "libdouble-conversion.dll" "$OUTPUT_DIR"

# libsodium - CRITICAL for encryption
copy_dll "libsodium-26.dll" "$OUTPUT_DIR"

# Additional Qt dependencies
copy_dll "libpcre2-16-0.dll" "$OUTPUT_DIR"
copy_dll "libzstd.dll" "$OUTPUT_DIR"
copy_dll "libharfbuzz-0.dll" "$OUTPUT_DIR"
copy_dll "libfreetype-6.dll" "$OUTPUT_DIR"
copy_dll "libpng16-16.dll" "$OUTPUT_DIR"
copy_dll "libbz2-1.dll" "$OUTPUT_DIR"
copy_dll "libbrotlidec.dll" "$OUTPUT_DIR"
copy_dll "libbrotlicommon.dll" "$OUTPUT_DIR"
copy_dll "libglib-2.0-0.dll" "$OUTPUT_DIR"
copy_dll "libintl-8.dll" "$OUTPUT_DIR"
copy_dll "libiconv-2.dll" "$OUTPUT_DIR"
copy_dll "libgraphite2.dll" "$OUTPUT_DIR"

# CRITICAL: libmd4c.dll - Required by Qt6Gui
copy_dll "libmd4c.dll" "$OUTPUT_DIR"

# GPGME and dependencies
copy_dll "libgpgme-11.dll" "$OUTPUT_DIR"
copy_dll "libgpg-error-0.dll" "$OUTPUT_DIR"
copy_dll "libassuan-0.dll" "$OUTPUT_DIR"
copy_dll "libgcrypt-20.dll" "$OUTPUT_DIR"
copy_dll "libgnutls-30.dll" "$OUTPUT_DIR"
copy_dll "libnettle-8.dll" "$OUTPUT_DIR"
copy_dll "libhogweed-6.dll" "$OUTPUT_DIR"
copy_dll "libgmp-10.dll" "$OUTPUT_DIR"
copy_dll "libidn2-0.dll" "$OUTPUT_DIR"
copy_dll "libunistring-5.dll" "$OUTPUT_DIR"
copy_dll "libtasn1-6.dll" "$OUTPUT_DIR"
copy_dll "libp11-kit-0.dll" "$OUTPUT_DIR"

# Create required directories
mkdir -p "$OUTPUT_DIR/keys/public"
mkdir -p "$OUTPUT_DIR/keys/private"
mkdir -p "$OUTPUT_DIR/profiles"
mkdir -p "$OUTPUT_DIR/temp"

# Copy GPG executables if they exist
if [ -d "/c/Program Files (x86)/gnupg/bin" ]; then
    cp "/c/Program Files (x86)/gnupg/bin/gpg.exe" "$OUTPUT_DIR/" 2>/dev/null || true
    cp "/c/Program Files (x86)/gnupg/bin/gpgconf.exe" "$OUTPUT_DIR/" 2>/dev/null || true
elif [ -d "/c/Program Files/gnupg/bin" ]; then
    cp "/c/Program Files/gnupg/bin/gpg.exe" "$OUTPUT_DIR/" 2>/dev/null || true
    cp "/c/Program Files/gnupg/bin/gpgconf.exe" "$OUTPUT_DIR/" 2>/dev/null || true
fi

# List all DLLs in output directory
echo ""
echo "DLLs in output directory:"
ls -1 "$OUTPUT_DIR"/*.dll 2>/dev/null | wc -l | xargs echo "Total DLL count:"

echo ""
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
