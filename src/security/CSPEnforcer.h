#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>

class CSPEnforcer : public QObject {
    Q_OBJECT

public:
    explicit CSPEnforcer(QObject *parent = nullptr);

    // Default strict CSP policy applied when pages don't have one
    QString defaultPolicy() const;
    void setDefaultPolicy(const QString &policy);

    // Evaluate and optionally strengthen a page's existing CSP
    QString enforcePolicy(const QUrl &pageUrl, const QString &existingCSP) const;

    // Check if a specific resource should be allowed under CSP
    bool allowsResource(const QString &directive, const QUrl &resourceUrl,
                        const QUrl &pageUrl) const;

    // Generate CSP violation report (internal only, never sent externally)
    void logViolation(const QUrl &pageUrl, const QString &directive,
                      const QUrl &blockedUrl);

    // JavaScript to inject into pages for CSP enforcement
    QString enforcementScript() const;

signals:
    void violationDetected(const QUrl &pageUrl, const QString &directive,
                           const QUrl &blockedUrl);

private:
    QString m_defaultPolicy;
    QMap<QString, int> m_violationCounts;
};
