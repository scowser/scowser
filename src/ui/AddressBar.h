#pragma once

#include <QLineEdit>
#include <QUrl>

class QLabel;

class AddressBar : public QLineEdit {
    Q_OBJECT

public:
    explicit AddressBar(QWidget *parent = nullptr);

    void setUrl(const QUrl &url);
    void setSecurityIndicator(bool secure);
    bool isSecure() const;

    void setSearchEngineUrl(const QString &url);

signals:
    void urlEntered(const QUrl &url);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onReturnPressed();

private:
    QUrl sanitizeInput(const QString &text) const;
    void positionIcon();

    QLabel *m_iconLabel;
    bool m_secure = false;
    QString m_searchEngineUrl = QStringLiteral("https://duckduckgo.com/?q=%1");

    static constexpr int IconSize = 16;
    static constexpr int IconMargin = 8;
};
