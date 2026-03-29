#!/bin/bash
# PC Principal PGP - Fixed Build Script with all dependencies

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

# [1/6] Check dependencies
echo "[1/6] Checking dependencies..."
command -v cmake >/dev/null 2>&1 || { echo "cmake required but not installed."; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "ninja required but not installed."; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "g++ required but not installed."; exit 1; }
pkg-config --exists gpgme || { echo "gpgme not found. Install with: pacman -S mingw-w64-x86_64-gpgme"; exit 1; }
pkg-config --exists libsodium || { echo "libsodium not found. Install with: pacman -S mingw-w64-x86_64-libsodium"; exit 1; }
echo "All dependencies ready!"

# [2/6] Clean previous build
echo ""
echo "[2/6] Cleaning previous build..."
rm -rf "$BUILD_DIR"
rm -rf "$OUTPUT_DIR"

# [3/6] Configure with CMake
echo ""
echo "[3/6] Configuring with CMake..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ "$SCRIPT_DIR"

# [4/6] Building
echo ""
echo "[4/6] Building... (this may take a few minutes)"
cd "$BUILD_DIR"
ninja -j$(nproc)

# [5/6] Deploying Qt dependencies
echo ""
echo "[5/6] Deploying Qt dependencies..."
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw --no-compiler-runtime "$BUILD_DIR/PCPrincipalPGP.exe"

# [6/6] Copy to portable folder with ALL dependencies
echo ""
echo "[6/6] Deploying ALL dependencies..."
mkdir -p "$OUTPUT_DIR"
cp -r "$BUILD_DIR"/* "$OUTPUT_DIR/"

# Copy additional Qt/MinGW dependencies that windeployqt misses
echo "Copying additional DLLs..."

# Core ICU libraries (required by Qt)
cp "$MSYS_BIN/icuin78.dll" "$OUTPUT_DIR/" 2>/dev/null || \
cp "$MSYS_BIN/libicuin78.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/icuuc78.dll" "$OUTPUT_DIR/" 2>/dev/null || \
cp "$MSYS_BIN/libicuuc78.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/icudt78.dll" "$OUTPUT_DIR/" 2>/dev/null || \
cp "$MSYS_BIN/libicudt78.dll" "$OUTPUT_DIR/" 2>/dev/null || true

# Double conversion library
cp "$MSYS_BIN/libdouble-conversion.dll" "$OUTPUT_DIR/" 2>/dev/null || true

# libsodium - CRITICAL for encryption
cp "$MSYS_BIN/libsodium-26.dll" "$OUTPUT_DIR/" 2>/dev/null || \
cp "$MSYS_BIN/libsodium.dll" "$OUTPUT_DIR/" 2>/dev/null || \
cp "$MSYS_BIN/libsodium-23.dll" "$OUTPUT_DIR/" 2>/dev/null || \
{ echo "WARNING: libsodium DLL not found!"; }

# Additional Qt dependencies
cp "$MSYS_BIN/libpcre2-16-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libzstd.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libharfbuzz-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libfreetype-6.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libpng16-16.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libbz2-1.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libbrotlidec.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libbrotlicommon.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libglib-2.0-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libintl-8.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libiconv-2.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libgraphite2.dll" "$OUTPUT_DIR/" 2>/dev/null || true

# GPGME and dependencies
cp "$MSYS_BIN/libgpgme-11.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libgpg-error-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libassuan-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libgcrypt-20.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libgnutls-30.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libnettle-8.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libhogweed-6.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libgmp-10.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libidn2-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libunistring-5.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libtasn1-6.dll" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$MSYS_BIN/libp11-kit-0.dll" "$OUTPUT_DIR/" 2>/dev/null || true

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
