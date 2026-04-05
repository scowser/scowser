#include "ui/AddressBar.h"

#include <QIcon>
#include <QLabel>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QStyle>

AddressBar::AddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText("Enter URL or search...");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(400);

    // Manual icon label — gives us full control over vertical centering
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(IconSize, IconSize);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setPixmap(QIcon(":/icons/lock-insecure.svg").pixmap(IconSize, IconSize));

    // Reserve space on the left for the icon
    setTextMargins(IconSize + IconMargin, 0, 0, 0);

    connect(this, &QLineEdit::returnPressed, this, &AddressBar::onReturnPressed);
}

void AddressBar::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    positionIcon();
}

void AddressBar::positionIcon()
{
    int x = IconMargin / 2 + 2;
    int y = (height() - IconSize) / 2;
    m_iconLabel->move(x, y);
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
        m_iconLabel->setPixmap(QIcon(":/icons/lock-secure.svg").pixmap(IconSize, IconSize));
        m_iconLabel->setToolTip("Connection is secure (HTTPS)");
    } else {
        m_iconLabel->setPixmap(QIcon(":/icons/lock-insecure.svg").pixmap(IconSize, IconSize));
        m_iconLabel->setToolTip("Connection is not secure");
    }

    // Dynamic property drives QSS styling (AddressBar[secure="true"] / "false")
    setProperty("secure", secure);
    style()->unpolish(this);
    style()->polish(this);
}

bool AddressBar::isSecure() const
{
    return m_secure;
}

void AddressBar::setSearchEngineUrl(const QString &url)
{
    m_searchEngineUrl = url;
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

    // Treat as a search query — use configured search engine (default: DuckDuckGo)
    return QUrl(m_searchEngineUrl.arg(QString::fromUtf8(QUrl::toPercentEncoding(text))));
}
