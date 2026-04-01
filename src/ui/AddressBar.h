#pragma once

#include <QLineEdit>
#include <QUrl>

class AddressBar : public QLineEdit {
    Q_OBJECT

public:
    explicit AddressBar(QWidget *parent = nullptr);

    void setUrl(const QUrl &url);
    void setSecurityIndicator(bool secure);

signals:
    void urlEntered(const QUrl &url);

private slots:
    void onReturnPressed();

private:
    QUrl sanitizeInput(const QString &text) const;
};
