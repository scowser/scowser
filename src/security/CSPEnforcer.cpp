#include "security/CSPEnforcer.h"

#include <QDebug>
#include <QDateTime>

CSPEnforcer::CSPEnforcer(QObject *parent)
    : QObject(parent)
{
    // Default strict CSP policy
    m_defaultPolicy =
        "default-src 'self'; "
        "script-src 'self'; "
        "style-src 'self' 'unsafe-inline'; "  // Many sites need inline styles
        "img-src 'self' https: data:; "
        "font-src 'self' https:; "
        "connect-src 'self' https:; "
        "media-src 'self' https:; "
        "frame-src 'self'; "
        "object-src 'none'; "
        "base-uri 'self'; "
        "form-action 'self'; "
        "frame-ancestors 'self'; "
        "upgrade-insecure-requests";
}

QString CSPEnforcer::defaultPolicy() const
{
    return m_defaultPolicy;
}

void CSPEnforcer::setDefaultPolicy(const QString &policy)
{
    m_defaultPolicy = policy;
}

QString CSPEnforcer::enforcePolicy(const QUrl &pageUrl, const QString &existingCSP) const
{
    Q_UNUSED(pageUrl);

    if (existingCSP.isEmpty()) {
        // No CSP header — apply our default strict policy
        return m_defaultPolicy;
    }

    // If the page has a CSP, strengthen it by ensuring minimum requirements
    QString policy = existingCSP;

    // Ensure object-src is restricted
    if (!policy.contains("object-src")) {
        policy += "; object-src 'none'";
    }

    // Ensure base-uri is restricted (prevents base tag injection)
    if (!policy.contains("base-uri")) {
        policy += "; base-uri 'self'";
    }

    // Ensure frame-ancestors is set (prevents clickjacking)
    if (!policy.contains("frame-ancestors")) {
        policy += "; frame-ancestors 'self'";
    }

    // Always upgrade insecure requests
    if (!policy.contains("upgrade-insecure-requests")) {
        policy += "; upgrade-insecure-requests";
    }

    return policy;
}

bool CSPEnforcer::allowsResource(const QString &directive, const QUrl &resourceUrl,
                                  const QUrl &pageUrl) const
{
    Q_UNUSED(directive);

    // Block data: URIs in script contexts (XSS vector)
    if (resourceUrl.scheme() == "data" &&
        (directive == "script-src" || directive == "default-src")) {
        return false;
    }

    // Block javascript: URIs everywhere
    if (resourceUrl.scheme() == "javascript") {
        return false;
    }

    // Block blob: URIs in script contexts
    if (resourceUrl.scheme() == "blob" && directive == "script-src") {
        return false;
    }

    // Allow same-origin resources
    if (resourceUrl.host() == pageUrl.host()) {
        return true;
    }

    // Allow HTTPS resources from other origins
    if (resourceUrl.scheme() == "https") {
        return true;
    }

    return false;
}

void CSPEnforcer::logViolation(const QUrl &pageUrl, const QString &directive,
                                const QUrl &blockedUrl)
{
    QString key = pageUrl.host() + ":" + directive;
    m_violationCounts[key]++;

    qDebug() << "CSP Violation on" << pageUrl.host()
             << "| Directive:" << directive
             << "| Blocked:" << blockedUrl.toString()
             << "| Count:" << m_violationCounts[key];

    emit violationDetected(pageUrl, directive, blockedUrl);
}

QString CSPEnforcer::enforcementScript() const
{
    // JavaScript injected into pages to monitor CSP violations locally
    // Reports are NEVER sent externally — only logged internally
    return QStringLiteral(R"JS(
        (function() {
            'use strict';
            document.addEventListener('securitypolicyviolation', function(e) {
                console.warn('[scowser CSP]', e.violatedDirective,
                             'blocked:', e.blockedURI,
                             'on:', e.documentURI);
            });
        })();
    )JS");
}
