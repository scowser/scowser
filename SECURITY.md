# Security Policy

## Supported Versions

Only the latest release of scowser receives security updates. We follow a fix-forward model — patches are applied to the current version, not backported.

| Version | Supported |
|---------|-----------|
| Latest  | Yes       |
| Older   | No        |

## Reporting a Vulnerability

**Do not open a public issue for security vulnerabilities.**

Please report vulnerabilities through [GitHub Security Advisories](https://github.com/scowser/scowser/security/advisories/new). This allows us to discuss and coordinate a fix privately before public disclosure.

When filing an advisory, please include:

- A description of the vulnerability.
- Steps to reproduce or a proof of concept.
- The affected component (e.g., network, sandbox, certificate pinning).
- Any suggested fix, if you have one.

You should receive an acknowledgment within 72 hours.

## Disclosure Policy

- We ask that you give us reasonable time to address the issue before public disclosure.
- We will credit reporters in the release notes unless anonymity is requested.
- We aim to release a fix within 14 days of confirming a vulnerability.

## Scope

The following are in scope for security reports:

- Sandbox escapes or privilege escalation
- Bypasses of ad/tracker blocking or request interception
- DNS-over-HTTPS leaks or fallback to plaintext DNS
- Certificate pinning or TLS enforcement bypasses
- Content Security Policy enforcement failures
- Session data persisting after ephemeral session cleanup
- Any unintended external data transmission

Out of scope:

- Vulnerabilities in upstream Qt or Chromium (report those to their respective projects)
- Denial of service via local access
- Issues requiring physical access to the device
