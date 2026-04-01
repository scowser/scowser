#pragma once

#include <QObject>
#include <QString>

class ProcessSandbox : public QObject {
    Q_OBJECT

public:
    explicit ProcessSandbox(QObject *parent = nullptr);

    // Apply sandbox restrictions to the current process
    bool applySandbox();

    // Check if sandboxing is available on this platform
    static bool isAvailable();

    // Get sandbox status description
    QString statusDescription() const;

signals:
    void sandboxApplied();
    void sandboxFailed(const QString &reason);

private:
#ifdef __APPLE__
    bool applyMacOSSandbox();
#elif defined(__linux__)
    bool applyLinuxSandbox();
#endif

    bool m_applied = false;
    QString m_statusDescription;
};
