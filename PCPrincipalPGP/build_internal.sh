#!/bin/bash
# Internal build script - runs inside MSYS2 MINGW64

set -e  # Exit on error

echo "========================================"
echo "  PC Principal PGP - Build Script"
echo "========================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DIST_DIR="${SCRIPT_DIR}/PCPrincipalPGP_Portable"

echo -e "${YELLOW}Source:${NC} ${SCRIPT_DIR}"
echo -e "${YELLOW}Build:${NC}  ${BUILD_DIR}"
echo -e "${YELLOW}Output:${NC} ${DIST_DIR}"
echo ""

# Function to check if package is installed
package_installed() {
    pacman -Q "$1" &>/dev/null
}

# Install dependencies
echo "[1/6] Checking dependencies..."

# Update package database (silently)
pacman -Sy --noconfirm &>/dev/null

DEPS=(
    "mingw-w64-x86_64-qt6-base"
    "mingw-w64-x86_64-qt6-tools"
    "mingw-w64-x86_64-gpgme"
    "mingw-w64-x86_64-libsodium"
    "mingw-w64-x86_64-cmake"
    "mingw-w64-x86_64-ninja"
    "mingw-w64-x86_64-gcc"
    "mingw-w64-x86_64-pkg-config"
)

for dep in "${DEPS[@]}"; do
    if ! package_installed "$dep"; then
        echo "Installing ${dep}..."
        pacman -S --noconfirm --needed "$dep" 2>/dev/null || {
            echo -e "${RED}Failed to install ${dep}${NC}"
            exit 1
        }
    fi
done

echo -e "${GREEN}All dependencies ready!${NC}"
echo ""

# Clean previous build
echo "[2/6] Cleaning previous build..."
rm -rf "${BUILD_DIR}"
rm -rf "${DIST_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${DIST_DIR}"

# Configure with CMake
echo ""
echo "[3/6] Configuring with CMake..."
cd "${BUILD_DIR}"

cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${MINGW_PREFIX}" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    "${SCRIPT_DIR}" 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: CMake configuration failed!${NC}"
    read -p "Press Enter to continue..."
    exit 1
fi

# Build
echo ""
echo "[4/6] Building... (this may take a few minutes)"
cmake --build . --parallel 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Build failed!${NC}"
    read -p "Press Enter to continue..."
    exit 1
fi

# Copy executable
echo ""
echo "[5/6] Copying executable..."
cp -f "PCPrincipalPGP.exe" "${DIST_DIR}/"

# Deploy Qt
echo ""
echo "[6/6] Deploying dependencies..."
windeployqt "${DIST_DIR}/PCPrincipalPGP.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw 2>/dev/null || true

# Copy required DLLs
echo "Copying libraries..."
cp -f "${MINGW_PREFIX}/bin/libgpgme-11.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/libassuan-0.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/libgpg-error-0.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/libsodium-23.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/libgcrypt-20.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/libntlm-0.dll" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/gpg.exe" "${DIST_DIR}/" 2>/dev/null || true
cp -f "${MINGW_PREFIX}/bin/gpgconf.exe" "${DIST_DIR}/" 2>/dev/null || true

# Copy icon
cp -f "${SCRIPT_DIR}/resources/icon.png" "${DIST_DIR}/" 2>/dev/null || true

# Create directories
mkdir -p "${DIST_DIR}/keys/public"
mkdir -p "${DIST_DIR}/keys/private"
mkdir -p "${DIST_DIR}/profiles"
mkdir -p "${DIST_DIR}/gpg"
mkdir -p "${DIST_DIR}/temp"

cd "${SCRIPT_DIR}"

echo ""
echo "========================================"
echo -e "${GREEN}  BUILD SUCCESSFUL!${NC}"
echo "========================================"
echo ""
echo "Your portable PGP application is ready at:"
echo "  ${DIST_DIR}"
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

# Open the output folder in Windows Explorer
if command -v explorer &>/dev/null; then
    explorer "${DIST_DIR}" 2>/dev/null || true
fi

read -p "Press Enter to continue..."
