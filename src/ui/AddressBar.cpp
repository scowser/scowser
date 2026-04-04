#include "ui/AddressBar.h"

#include <QAction>
#include <QIcon>
#include <QRegularExpression>

AddressBar::AddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText("Enter URL or search...");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(400);

    m_securityAction = addAction(QIcon(":/icons/lock-insecure.png"), QLineEdit::LeadingPosition);
    m_securityAction->setToolTip("Connection is not secure");

    connect(this, &QLineEdit::returnPressed, this, &AddressBar::onReturnPressed);
}

void AddressBar::setUrl(const QUrl &url)
{
    if (url.scheme() == "about" || url.isEmpty()) {
        clear();
        setSecurityIndicator(false);
        return;
    }
    setText(url.toDisplayString());
    setSecurityIndicator(url.scheme() == "https");
}

void AddressBar::setSecurityIndicator(bool secure)
{
    m_secure = secure;

    if (secure) {
        m_securityAction->setIcon(QIcon(":/icons/lock-secure.png"));
        m_securityAction->setToolTip("Connection is secure (HTTPS)");
        setStyleSheet("QLineEdit { border: 2px solid #4CAF50; padding: 4px 8px; border-radius: 4px; }");
    } else {
        m_securityAction->setIcon(QIcon(":/icons/lock-insecure.png"));
        m_securityAction->setToolTip("Connection is not secure");
        setStyleSheet("QLineEdit { border: 2px solid #f44336; padding: 4px 8px; border-radius: 4px; }");
    }
}

bool AddressBar::isSecure() const
{
    return m_secure;
}

void AddressBar::onReturnPressed()
{
    QUrl url = sanitizeInput(text().trimmed());
    if (url.isValid()) {
        emit urlEntered(url);
    }
}

QUrl AddressBar::sanitizeInput(const QString &text) const
{
    if (text.isEmpty()) {
        return QUrl();
    }

    // If it looks like a URL (has a scheme or dots suggesting a domain)
    static QRegularExpression urlPattern(R"(^(https?://|[a-zA-Z0-9][-a-zA-Z0-9]*\.[a-zA-Z]{2,}))");

    if (text.startsWith("http://") || text.startsWith("https://")) {
        return QUrl::fromUserInput(text);
    }

    if (urlPattern.match(text).hasMatch()) {
        // Default to HTTPS for security
        return QUrl::fromUserInput("https://" + text);
    }

    // Treat as a search query — use DuckDuckGo (privacy-respecting)
    return QUrl("https://duckduckgo.com/?q=" + QUrl::toPercentEncoding(text));
}
