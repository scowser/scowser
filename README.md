# scowser

A security-focused web browser built in C++ with Qt6 WebEngine.

## Features

- **Sandboxed Tabs** — Each tab runs in an isolated process with OS-level sandboxing
- **Ad & Tracker Blocking** — Built-in EasyList/EasyPrivacy filtering, no extensions needed
- **DNS-over-HTTPS** — Encrypted DNS via Cloudflare or Quad9, preventing ISP snooping
- **Zero Telemetry** — No analytics, no crash reports, no data collection whatsoever
- **Strict TLS** — Certificate pinning, TLS 1.2+ only, certificate transparency checks
- **CSP Enforcement** — Content Security Policy headers injected and enforced
- **Ephemeral by Default** — All browsing data wiped on exit; no persistent cookies, cache, or history

## Building

### Prerequisites

- C++20 compiler (Clang 14+ or GCC 12+)
- CMake 3.22+
- Qt6 6.5+ with WebEngine module

**macOS:**
```bash
brew install qt@6 cmake
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt install qt6-webengine-dev qt6-base-dev cmake g++
```

**Linux (Fedora):**
```bash
sudo dnf install qt6-qtwebengine-devel qt6-qtbase-devel cmake gcc-c++
```

### Compile & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/scowser
```

### Run Tests

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Architecture

scowser embeds Chromium via Qt6 WebEngine and layers security controls on top:

```
┌─────────────────────────────────┐
│           scowser UI            │
│  (MainWindow, Tabs, AddressBar) │
├─────────────────────────────────┤
│        Security Layer           │
│  AdBlocker · DoH · CertPinner  │
│  CSP Enforcer · SessionManager  │
├─────────────────────────────────┤
│     Request Interceptor         │
│  (filters all network traffic)  │
├─────────────────────────────────┤
│      Qt6 WebEngine (Chromium)   │
│   (rendering, JS, sandboxing)   │
└─────────────────────────────────┘
```

## Security Model

| Threat                  | Mitigation                                    |
|-------------------------|-----------------------------------------------|
| Ad/tracker surveillance | Request-level blocking via filter lists        |
| DNS snooping            | DNS-over-HTTPS with trusted resolvers          |
| Data exfiltration       | Zero telemetry, ephemeral sessions             |
| MITM attacks            | Certificate pinning, strict TLS, CT checks     |
| XSS / injection         | CSP enforcement on all pages                   |
| Tab compromise          | Process isolation + OS-level sandboxing         |

## License

See [LICENSE](LICENSE) for details.
