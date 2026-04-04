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
- **Qt6** with modules: Core, Widgets, WebEngineWidgets, Network
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
│   └── Application.h/cpp      # QApplication subclass, global setup
├── ui/
│   ├── MainWindow.h/cpp       # Main browser window
│   ├── TabWidget.h/cpp        # Tab bar and tab management
│   └── AddressBar.h/cpp       # URL bar with security indicators
├── security/
│   ├── AdBlocker.h/cpp        # EasyList-based ad/tracker blocking
│   ├── DnsOverHttps.h/cpp     # DoH resolver (Cloudflare/Quad9)
│   ├── CertificatePinner.h/cpp # Certificate pinning + strict TLS
│   ├── CSPEnforcer.h/cpp      # Content Security Policy enforcement
│   └── SessionManager.h/cpp   # Ephemeral session management
├── network/
│   ├── NetworkManager.h/cpp   # Custom QNetworkAccessManager
│   └── RequestInterceptor.h/cpp # URL request filtering
└── sandbox/
    └── ProcessSandbox.h/cpp   # OS-level process sandboxing
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

   - Encrypted DNS resolution via HTTPS
   - Default providers: Cloudflare (1.1.1.1), Quad9 (9.9.9.9)
   - Prevents DNS snooping by ISPs and network operators
   - Falls back gracefully if DoH is unavailable

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

7. **Ephemeral Sessions**

   - All browsing data cleared on exit by default
   - Off-the-record QWebEngineProfile (no disk persistence)
   - No cookies, cache, or history survive a session
   - Optional "remember this session" for user convenience

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
