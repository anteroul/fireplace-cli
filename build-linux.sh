#!/usr/bin/env bash
set -euo pipefail

# -------------------------
# Configuration
# -------------------------
PROJECT_NAME="fireplace"
SRC_FILES="main.c terminfo.c"
CFLAGS="-std=c11 -Wall -Wextra -O2"
LDFLAGS="-lncursesw"

# Allow user to override via env vars
CC="${CC:-gcc}"

# -------------------------
# Build
# -------------------------
echo "Building $PROJECT_NAME for Linux..."
$CC $CFLAGS $SRC_FILES $LDFLAGS -o "$PROJECT_NAME"

echo "Build complete: ./$PROJECT_NAME"