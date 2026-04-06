# scowser — Development Guide

## Core Instructions

- Always write tests for C++ code.
- Always check to make sure website is relevant and accurate to current features.
- Always check to make sure README is updated.
- Always check to make sure CLAUDE.md is updated.

## Project Overview

scowser is a security-focused web browser built in C++ using Qt6 WebEngine (Chromium-based).
Target platforms: macOS and Linux.

## Build System

- **CMake** (minimum 3.22)
- **Qt6** with modules: Core, Widgets, WebEngineWidgets, Network, Svg
- **C++20** standard

### Build Commands

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/scowser
```

### Dependencies

- Qt6 (6.5+): `brew install qt@6` (macOS) or system package manager (Linux)
- CMake 3.22+
- C++20-capable compiler (Clang 14+, GCC 12+)

## Architecture

### Directory Structure

```
src/
├── main.cpp                    # Entry point
├── app/
│   ├── Application.h/cpp      # QApplication subclass, global setup
│   ├── Settings.h/cpp         # User preferences (QSettings-backed)
│   └── DownloadManager.h/cpp  # Download handling and tracking
├── ui/
│   ├── MainWindow.h/cpp       # Main browser window
│   ├── TabWidget.h/cpp        # Tab bar and tab management
│   ├── AddressBar.h/cpp       # URL bar with security indicators
│   ├── SettingsDialog.h/cpp   # Preferences dialog (General + Privacy + Security tabs)
│   ├── DownloadsDialog.h/cpp  # Downloads list with progress and actions
│   └── LogPanel.h/cpp         # Live log viewer dock widget with syntax highlighting
├── security/
│   ├── AdBlocker.h/cpp        # EasyList-based ad/tracker blocking
│   ├── DnsOverHttps.h/cpp     # DoH resolver (Cloudflare/Quad9)
│   ├── CertificatePinner.h/cpp # Certificate pinning + strict TLS
│   ├── CSPEnforcer.h/cpp      # Content Security Policy enforcement
│   └── SessionManager.h/cpp   # Ephemeral session management
├── network/
│   ├── NetworkManager.h/cpp   # Custom QNetworkAccessManager
│   └── RequestInterceptor.h/cpp # URL request filtering + DoH prefetch
└── sandbox/
    └── ProcessSandbox.h/cpp   # OS-level process sandboxing

resources/
├── resources.qrc               # Qt resource collection
├── icons/
│   ├── scowser.png/icns       # App icon
│   ├── back.svg               # Navigation: back
│   ├── forward.svg            # Navigation: forward
│   ├── reload.svg             # Navigation: reload
│   ├── new-tab.svg            # Tab bar: new tab
│   ├── lock-secure.svg        # Address bar: HTTPS
│   ├── lock-insecure.svg      # Address bar: HTTP
│   └── download.svg           # Toolbar: downloads
└── style/
    └── scowser.qss            # Application-wide dark theme stylesheet
```

### Security Features

1. **Sandboxed Tab Processes**

   - Leverages Chromium's multi-process architecture via Qt WebEngine
   - Additional OS-level sandboxing: seccomp-bpf (Linux), sandbox_init (macOS)
   - Each tab runs in an isolated renderer process

2. **Built-in Ad/Tracker Blocking**

   - EasyList and EasyPrivacy filter list support
   - Implemented via QWebEngineUrlRequestInterceptor
   - Blocks requests before they leave the browser
   - No third-party extension dependencies

3. **DNS-over-HTTPS (DoH)**

   - Two-layer approach: Chromium's built-in Secure DNS (via flags) + Qt-side DnsOverHttps resolver
   - Chromium flags enforce `--dns-over-https-mode=secure` (no plaintext DNS fallback)
   - Default provider: Cloudflare (1.1.1.1), configurable to Quad9 (9.9.9.9) or custom
   - RequestInterceptor fires async DoH prefetch for cache warming on allowed requests
   - Prevents DNS snooping by ISPs and network operators

4. **No Telemetry / No Data Collection**

   - Zero telemetry, analytics, or crash reporting
   - No data leaves the browser except user-initiated requests
   - All Chromium telemetry endpoints are blocked

5. **Certificate Pinning / Strict TLS**

   - HSTS preload list enforcement
   - Certificate transparency verification
   - Rejects connections with weak cipher suites
   - TLS 1.2 minimum, TLS 1.3 preferred

6. **Content Security Policy Enforcement**

   - Injects and enforces strict CSP headers
   - Blocks inline scripts on pages without CSP
   - Reports violations to an internal log (never external)

7. **Security Indicator**

   - Lock icon in the address bar via QAction (LeadingPosition)
   - Green locked icon for HTTPS, red unlocked icon for HTTP
   - Updates automatically on URL changes
   - SVG icons in resources/icons/

8. **Dark Theme UI**

   - Custom QSS stylesheet loaded from resources at startup (resources/style/scowser.qss)
   - Catppuccin Mocha-inspired dark palette (#1e1e2e base, #181825 mantle, #313244 surface)
   - SVG icons for navigation (back, forward, reload) and tab management (new-tab)
   - SVG lock icons (green for HTTPS, red for HTTP) replacing PNG originals
   - AddressBar uses Qt dynamic properties (`secure` bool) for QSS-driven border colors
   - Styled tab bar with active-tab accent, toolbar, status bar, scrollbars, tooltips, and context menus

9. **Ephemeral Sessions**

   - All browsing data cleared on exit by default
   - Off-the-record QWebEngineProfile (no disk persistence)
   - No cookies, cache, or history survive a session
   - Optional "remember this session" for user convenience

10. **Settings Framework**

    - `Settings` class backed by `QSettings` (INI format, persisted to user config dir)
    - `SettingsDialog` with General, Privacy, and Security tabs, live-apply (changes take effect immediately)
    - Accessible via menu bar: macOS app menu (Preferences) / Help > Preferences on Linux
    - Configurable options: download directory, DNS provider (Cloudflare/Quad9/Custom), search engine, ephemeral sessions, Do Not Track, ad blocking, JavaScript
    - Signal-based architecture: `Settings` emits change signals, `Application` connects them to security components
    - All settings have secure defaults (no config file needed for safe operation)

11. **Download Manager**

    - `DownloadManager` handles `QWebEngineProfile::downloadRequested` signals
    - Accepts downloads, sets configurable download directory (default: ~/Downloads)
    - Tracks download progress and state (in-progress, completed, cancelled, interrupted)
    - `DownloadsDialog` shows download list with progress bars, file sizes, and action buttons (Open File, Show in Folder)
    - Toolbar download button to the right of the address bar with active download count indicator
    - Download directory configurable in Preferences > General tab

12. **Live Log Viewer**

    - `LogPanel` is a `QDockWidget` that captures all Qt log messages (`qDebug`, `qWarning`, `qCritical`, etc.) in real time
    - Custom `LogHighlighter` (`QSyntaxHighlighter`) colors timestamps, log levels, component names, URLs, quoted strings, and numbers
    - Accessible via View > Show Logs menu item
    - Default dock position: right side (vertical pane); toggle button switches to bottom (horizontal pane)
    - Auto-scrolls to latest messages when scrolled to bottom; preserves scroll position otherwise
    - Clear button to reset the log view; max 10,000 lines to bound memory usage
    - Catppuccin Mocha color scheme: blue for DEBUG, green for INFO, yellow for WARNING, red for CRITICAL/FATAL

## Code Conventions

- Use `#pragma once` for header guards
- Prefix member variables with `m_`
- Use smart pointers (std::unique_ptr, std::shared_ptr) over raw pointers
- Follow Qt naming conventions (camelCase for methods, PascalCase for classes)
- All security-critical code must have corresponding unit tests in tests/

## Testing

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## CI Workflows

- `.github/workflows/sanitizers.yml` — Runs ASan, UBSan, TSan, Valgrind, and AFL++ fuzzing on push/PR to main and nightly
- Valgrind suppressions live in `fuzz/valgrind.supp` — add Qt/system false positives there
- Fuzz corpora are in `fuzz/corpus/`, fuzz targets in `fuzz/`
