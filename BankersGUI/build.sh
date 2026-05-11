#!/bin/bash
# Build script for Banker's Algorithm Simulator
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "============================================"
echo "  Banker's Algorithm Simulator — Build"
echo "============================================"

# 1. Install dependencies if missing
if ! command -v qmake &>/dev/null; then
    echo "[*] Installing Qt5 dev tools..."
    sudo apt-get install -y qtbase5-dev qttools5-dev-tools g++ make
fi

# 2. Create build directory
BUILD="$SCRIPT_DIR/build"
mkdir -p "$BUILD"
cd "$BUILD"

# 3. Run qmake
echo "[*] Running qmake..."
qmake "$SCRIPT_DIR/BankersGUI.pro" -spec linux-g++ CONFIG+=release

# 4. Build
echo "[*] Compiling..."
make -j$(nproc)

echo ""
echo "============================================"
echo "  Build successful!"
echo "  Run: $BUILD/BankersGUI"
echo "============================================"
