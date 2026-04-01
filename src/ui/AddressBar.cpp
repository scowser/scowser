#include "ui/AddressBar.h"

#include <QRegularExpression>

AddressBar::AddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText("Enter URL or search...");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(400);

    connect(this, &QLineEdit::returnPressed, this, &AddressBar::onReturnPressed);
}

void AddressBar::setUrl(const QUrl &url)
{
    if (url.scheme() == "about" || url.isEmpty()) {
        clear();
        return;
    }
    setText(url.toDisplayString());
}

void AddressBar::setSecurityIndicator(bool secure)
{
    if (secure) {
        setStyleSheet("QLineEdit { border: 2px solid #4CAF50; padding: 4px 8px; border-radius: 4px; }");
    } else {
        setStyleSheet("QLineEdit { border: 2px solid #f44336; padding: 4px 8px; border-radius: 4px; }");
    }
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
