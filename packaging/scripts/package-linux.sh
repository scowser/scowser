#!/usr/bin/env bash
# Package Scowser as a Linux AppImage
set -euo pipefail

BUILD_DIR="${1:-build}"
BINARY="$BUILD_DIR/scowser"
APP_DIR="$BUILD_DIR/Scowser.AppDir"
VERSION=$(grep 'project(scowser' CMakeLists.txt | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')

if [ ! -f "$BINARY" ]; then
    echo "Error: $BINARY not found. Run 'make release' first."
    exit 1
fi

echo "==> Creating AppDir structure..."
rm -rf "$APP_DIR"
mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/lib"
mkdir -p "$APP_DIR/usr/share/applications"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"

# Copy binary
cp "$BINARY" "$APP_DIR/usr/bin/scowser"

# Copy desktop entry
cp packaging/linux/scowser.desktop "$APP_DIR/scowser.desktop"
cp packaging/linux/scowser.desktop "$APP_DIR/usr/share/applications/"

# Copy icon if it exists
if [ -f "resources/icons/scowser.png" ]; then
    cp "resources/icons/scowser.png" "$APP_DIR/scowser.png"
    cp "resources/icons/scowser.png" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/"
else
    # Create a placeholder icon (1x1 transparent PNG)
    echo "Warning: No icon found at resources/icons/scowser.png, using placeholder"
    printf '\x89PNG\r\n\x1a\n' > "$APP_DIR/scowser.png"
fi

# Create AppRun
cat > "$APP_DIR/AppRun" << 'APPRUN'
#!/usr/bin/env bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH:-}"
exec "${HERE}/usr/bin/scowser" "$@"
APPRUN
chmod +x "$APP_DIR/AppRun"

# Bundle Qt libraries using linuxdeployqt if available
if command -v linuxdeployqt &>/dev/null; then
    echo "==> Running linuxdeployqt..."
    linuxdeployqt "$APP_DIR/usr/bin/scowser" \
        -qmake="$(which qmake6 2>/dev/null || which qmake)" \
        -bundle-non-qt-libs
elif command -v linuxdeploy &>/dev/null; then
    echo "==> Running linuxdeploy..."
    linuxdeploy --appdir "$APP_DIR" \
        --plugin qt \
        --output appimage
    echo "==> Done"
    exit 0
else
    echo "Warning: Neither linuxdeployqt nor linuxdeploy found."
    echo "  The AppImage will not include Qt libraries."
    echo "  Install: https://github.com/linuxdeploy/linuxdeploy/releases"
fi

# Create AppImage using appimagetool
if command -v appimagetool &>/dev/null; then
    echo "==> Creating AppImage..."
    ARCH=$(uname -m) appimagetool "$APP_DIR" "$BUILD_DIR/Scowser-${VERSION}-$(uname -m).AppImage"
    echo "==> Done: $BUILD_DIR/Scowser-${VERSION}-$(uname -m).AppImage"
else
    echo "Warning: appimagetool not found. AppDir created at: $APP_DIR"
    echo "  Install: https://github.com/AppImage/appimagetool/releases"
fi
