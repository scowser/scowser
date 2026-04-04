#include "ui/AboutDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About scowser");
    setFixedSize(420, 360);

    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);
    layout->setContentsMargins(32, 32, 32, 32);

    // App icon
    auto *iconLabel = new QLabel(this);
    QPixmap icon(":/icons/scowser.png");
    iconLabel->setPixmap(icon.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    // App name
    auto *nameLabel = new QLabel("scowser", this);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(nameLabel);

    // Version
    auto *versionLabel = new QLabel(
        QString("Version %1").arg(QApplication::applicationVersion()), this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("font-size: 14px; color: #a6adc8;");
    layout->addWidget(versionLabel);

    // Description
    auto *descLabel = new QLabel("The Secure Browser", this);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("font-size: 14px; color: #a6adc8;");
    layout->addWidget(descLabel);

    layout->addSpacing(8);

    // Feature summary
    auto *featuresLabel = new QLabel(
        "Ad & tracker blocking \u00b7 DNS-over-HTTPS\n"
        "Zero telemetry \u00b7 Strict TLS\n"
        "CSP enforcement \u00b7 Ephemeral sessions",
        this);
    featuresLabel->setAlignment(Qt::AlignCenter);
    featuresLabel->setStyleSheet("font-size: 12px; color: #6c7086; line-height: 1.4;");
    layout->addWidget(featuresLabel);

    layout->addStretch();
}
