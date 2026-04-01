#include "sandbox/ProcessSandbox.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#ifdef __APPLE__
#include <sys/resource.h>
#endif

#ifdef __linux__
#ifdef HAVE_SECCOMP
#include <seccomp.h>
#include <sys/prctl.h>
#endif
#include <sys/resource.h>
#endif

ProcessSandbox::ProcessSandbox(QObject *parent)
    : QObject(parent)
{
}

bool ProcessSandbox::applySandbox()
{
    if (m_applied) {
        qDebug() << "ProcessSandbox: Already applied";
        return true;
    }

    bool success = false;

#ifdef __APPLE__
    success = applyMacOSSandbox();
#elif defined(__linux__)
    success = applyLinuxSandbox();
#else
    m_statusDescription = "Sandboxing not supported on this platform";
    qWarning() << "ProcessSandbox:" << m_statusDescription;
    return false;
#endif

    if (success) {
        m_applied = true;
        emit sandboxApplied();
        qDebug() << "ProcessSandbox: Sandbox applied successfully";
    }

    return success;
}

bool ProcessSandbox::isAvailable()
{
#ifdef __APPLE__
    return true;
#elif defined(__linux__)
#ifdef HAVE_SECCOMP
    return true;
#else
    return false;
#endif
#else
    return false;
#endif
}

QString ProcessSandbox::statusDescription() const
{
    return m_statusDescription;
}

#ifdef __APPLE__
bool ProcessSandbox::applyMacOSSandbox()
{
    // Modern macOS sandboxing requires App Sandbox entitlements in the .app bundle
    // (configured via com.apple.security.app-sandbox in the entitlements plist).
    // At runtime we apply resource limits and rely on Chromium's built-in
    // process sandboxing via Qt WebEngine.

    // Prevent core dumps (may contain sensitive data)
    struct rlimit nocore = { 0, 0 };
    if (setrlimit(RLIMIT_CORE, &nocore) != 0) {
        qWarning() << "ProcessSandbox: Failed to disable core dumps";
    }

    // Limit open file descriptors
    struct rlimit nofile = { 1024, 1024 };
    setrlimit(RLIMIT_NOFILE, &nofile);

    m_statusDescription = "macOS resource limits active (App Sandbox via entitlements)";

    // Note: For full App Sandbox, distribute as a signed .app bundle with:
    //   com.apple.security.app-sandbox = true
    //   com.apple.security.network.client = true
    //   com.apple.security.files.user-selected.read-only = true
    // in the entitlements plist. Qt WebEngine's Chromium layer provides
    // additional process-level sandboxing for renderer processes.

    return true;
}
#endif

#ifdef __linux__
bool ProcessSandbox::applyLinuxSandbox()
{
#ifdef HAVE_SECCOMP
    // Prevent this process from gaining new privileges
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
        m_statusDescription = "Failed to set NO_NEW_PRIVS";
        emit sandboxFailed(m_statusDescription);
        return false;
    }

    // Create seccomp filter
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        m_statusDescription = "Failed to initialize seccomp";
        emit sandboxFailed(m_statusDescription);
        return false;
    }

    // Block dangerous syscalls
    // These are syscalls that renderer processes should never need
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(mount), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(umount2), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(ptrace), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(kexec_load), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(reboot), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(swapon), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(swapoff), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(init_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(delete_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(pivot_root), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(chroot), 0);

    // Load the filter
    int result = seccomp_load(ctx);
    seccomp_release(ctx);

    if (result != 0) {
        m_statusDescription = "Failed to load seccomp filter";
        emit sandboxFailed(m_statusDescription);
        return false;
    }

    m_statusDescription = "Linux seccomp sandbox active";
    return true;

#else
    // No seccomp available — apply basic resource limits
    struct rlimit nofile = { 1024, 1024 };
    setrlimit(RLIMIT_NOFILE, &nofile);

    // Prevent core dumps (may contain sensitive data)
    struct rlimit nocore = { 0, 0 };
    setrlimit(RLIMIT_CORE, &nocore);

    m_statusDescription = "Basic resource limits applied (seccomp not available)";
    qWarning() << "ProcessSandbox: seccomp not available, using basic limits";
    return true;
#endif
}
#endif
