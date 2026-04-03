# Contributing to scowser

Thank you for your interest in contributing to scowser! This document provides guidelines and information to help you get started.

## Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to julio@julioj.com.

## Getting Started

### Prerequisites

- C++20-capable compiler (Clang 14+ or GCC 12+)
- CMake 3.22+
- Qt6 (6.5+) with WebEngineWidgets

**macOS:**
```bash
brew install qt@6 cmake
```

**Linux:**
Install Qt6 and CMake via your distribution's package manager.

### Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/scowser
```

### Running Tests

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## How to Contribute

### Reporting Bugs

- Search existing issues before opening a new one.
- Include your OS, Qt version, and steps to reproduce.
- For security vulnerabilities, **do not** open a public issue. Email julio@julioj.com directly.

### Suggesting Features

Open an issue describing the feature, its motivation, and how it fits scowser's goal of being a security-focused browser with zero telemetry.

### Submitting Pull Requests

1. Fork the repository and create a branch from `main`.
2. Make your changes following the code conventions below.
3. Add tests for security-critical code.
4. Ensure the build passes and tests succeed.
5. Open a pull request against `main` with a clear description of your changes.

## Code Conventions

- **Header guards:** Use `#pragma once`.
- **Member variables:** Prefix with `m_`.
- **Naming:** camelCase for methods, PascalCase for classes (Qt convention).
- **Memory management:** Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers.
- **C++ standard:** C++20. Use modern features where they improve clarity.

## Architecture Overview

scowser is organized into the following modules:

| Directory | Purpose |
|-----------|---------|
| `src/app/` | Application lifecycle and global setup |
| `src/ui/` | Main window, tabs, and address bar |
| `src/security/` | Ad blocking, DoH, certificate pinning, CSP, sessions |
| `src/network/` | Custom network manager and request interception |
| `src/sandbox/` | OS-level process sandboxing |
| `tests/` | Unit and integration tests |
| `fuzz/` | Fuzz targets and corpora |

## Platform Support

scowser targets **macOS** and **Linux**. All contributions must build and work on both platforms. Use platform-specific code only when necessary, guarded by appropriate preprocessor checks (`#ifdef __APPLE__`, `#ifdef __linux__`).

## CI

Pull requests are checked by the following workflows:

- **ci.yml** — Build and test on both platforms.
- **sanitizers.yml** — ASan, UBSan, TSan, Valgrind, and AFL++ fuzzing.

Please ensure your changes pass CI before requesting review.

## Security Guidelines

scowser is a security-focused browser. When contributing, keep these principles in mind:

- **No telemetry.** Never add code that sends data externally without explicit user action.
- **Test security code.** All code in `src/security/` must have corresponding tests.
- **Minimize attack surface.** Prefer blocking by default and allowlisting over the reverse.
- **Report vulnerabilities privately.** Email julio@julioj.com instead of opening a public issue.

## License

By contributing, you agree that your contributions will be licensed under the [Apache License 2.0](LICENSE).
