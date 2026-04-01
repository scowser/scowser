#!/usr/bin/env bash
# Package Scowser as a self-contained macOS .app bundle
set -euo pipefail

BUILD_DIR="${1:-build}"
APP_BUNDLE="$BUILD_DIR/scowser.app"

if [ ! -d "$APP_BUNDLE" ]; then
    echo "Error: $APP_BUNDLE not found. Run 'make release' first."
    exit 1
fi

echo "==> Running macdeployqt..."
if command -v macdeployqt &>/dev/null; then
    macdeployqt "$APP_BUNDLE" -verbose=1
elif [ -d "/opt/homebrew/opt/qt@6/bin" ]; then
    /opt/homebrew/opt/qt@6/bin/macdeployqt "$APP_BUNDLE" -verbose=1
else
    echo "Error: macdeployqt not found. Install Qt6: brew install qt@6"
    exit 1
fi

echo "==> Signing bundle with ad-hoc signature..."
codesign --force --deep --sign - "$APP_BUNDLE"

echo "==> Creating DMG..."
DMG_NAME="Scowser-$(grep 'project(scowser' CMakeLists.txt | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')"
hdiutil create -volname "Scowser" \
    -srcfolder "$APP_BUNDLE" \
    -ov -format UDZO \
    "$BUILD_DIR/${DMG_NAME}.dmg"

echo "==> Done: $BUILD_DIR/${DMG_NAME}.dmg"
