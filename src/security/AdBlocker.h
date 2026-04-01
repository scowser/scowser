#pragma once

#include <QObject>
#include <QSet>
#include <QRegularExpression>
#include <QString>
#include <QVector>

struct FilterRule {
    enum Type { Domain, Regex, Wildcard };

    Type type;
    QString pattern;
    QRegularExpression regex;
    bool isException = false;

    bool matches(const QString &url, const QString &domain) const;
};

class AdBlocker : public QObject {
    Q_OBJECT

public:
    explicit AdBlocker(QObject *parent = nullptr);

    bool shouldBlock(const QUrl &url, const QUrl &firstPartyUrl) const;
    int ruleCount() const { return m_rules.size(); }
    int blockedCount() const { return m_blockedCount; }

    void loadDefaultLists();
    void loadFilterList(const QString &filePath);
    void addCustomRule(const QString &rule);

signals:
    void requestBlocked(const QUrl &url);

private:
    void parseRule(const QString &line);
    void addDomainRule(const QString &domain, bool isException);

    QSet<QString> m_blockedDomains;
    QSet<QString> m_allowedDomains;
    QVector<FilterRule> m_rules;
    mutable int m_blockedCount = 0;

    // Known tracker domains (built-in fallback)
    static const QSet<QString> s_builtinTrackers;
};
