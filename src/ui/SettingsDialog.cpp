#include "ui/SettingsDialog.h"
#include "app/Settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle("Settings");
    setFixedSize(500, 400);

    auto *layout = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);

    setupPrivacyTab(tabs);
    setupSecurityTab(tabs);

    layout->addWidget(tabs);

    loadFromSettings();
    connectWidgets();
}

void SettingsDialog::setupPrivacyTab(QTabWidget *tabs)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    // DNS Provider group
    auto *dnsGroup = new QGroupBox("DNS Provider");
    auto *dnsLayout = new QVBoxLayout(dnsGroup);

    m_dnsProviderCombo = new QComboBox;
    m_dnsProviderCombo->addItem("Cloudflare (1.1.1.1)");
    m_dnsProviderCombo->addItem("Quad9 (9.9.9.9)");
    m_dnsProviderCombo->addItem("Custom");
    dnsLayout->addWidget(m_dnsProviderCombo);

    m_customDnsEdit = new QLineEdit;
    m_customDnsEdit->setPlaceholderText("https://dns.example.com/dns-query");
    m_customDnsEdit->setVisible(false);
    dnsLayout->addWidget(m_customDnsEdit);

    layout->addWidget(dnsGroup);

    // Search Engine group
    auto *searchGroup = new QGroupBox("Search Engine");
    auto *searchLayout = new QVBoxLayout(searchGroup);

    m_searchEngineCombo = new QComboBox;
    m_searchEngineCombo->addItem("DuckDuckGo", "https://duckduckgo.com/?q=%1");
    searchLayout->addWidget(m_searchEngineCombo);

    layout->addWidget(searchGroup);

    // Privacy toggles
    m_ephemeralCheck = new QCheckBox("Clear all browsing data on exit");
    layout->addWidget(m_ephemeralCheck);

    m_dntCheck = new QCheckBox("Send Do Not Track header");
    layout->addWidget(m_dntCheck);

    layout->addStretch();
    tabs->addTab(page, "Privacy");
}

void SettingsDialog::setupSecurityTab(QTabWidget *tabs)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    m_adBlockCheck = new QCheckBox("Enable ad and tracker blocking");
    layout->addWidget(m_adBlockCheck);

    m_jsCheck = new QCheckBox("Enable JavaScript");
    auto *jsNote = new QLabel("Changes take effect on next page load");
    jsNote->setObjectName("settingsNote");
    layout->addWidget(m_jsCheck);
    layout->addWidget(jsNote);

    layout->addStretch();
    tabs->addTab(page, "Security");
}

void SettingsDialog::loadFromSettings()
{
    m_dnsProviderCombo->setCurrentIndex(static_cast<int>(m_settings->dnsProvider()));
    m_customDnsEdit->setText(m_settings->customDnsUrl());
    m_customDnsEdit->setVisible(m_settings->dnsProvider() == DnsOverHttps::Custom);

    // Find the combo item matching current search engine URL
    int searchIdx = m_searchEngineCombo->findData(m_settings->searchEngineUrl());
    if (searchIdx >= 0)
        m_searchEngineCombo->setCurrentIndex(searchIdx);

    m_ephemeralCheck->setChecked(m_settings->ephemeralSessions());
    m_dntCheck->setChecked(m_settings->doNotTrack());
    m_adBlockCheck->setChecked(m_settings->adBlockingEnabled());
    m_jsCheck->setChecked(m_settings->javaScriptEnabled());
}

void SettingsDialog::connectWidgets()
{
    // DNS provider
    connect(m_dnsProviderCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        auto provider = static_cast<DnsOverHttps::Provider>(index);
        m_settings->setDnsProvider(provider);
        m_customDnsEdit->setVisible(provider == DnsOverHttps::Custom);
    });

    connect(m_customDnsEdit, &QLineEdit::editingFinished, this, [this]() {
        m_settings->setCustomDnsUrl(m_customDnsEdit->text().trimmed());
    });

    // Search engine
    connect(m_searchEngineCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        QString url = m_searchEngineCombo->itemData(index).toString();
        if (!url.isEmpty())
            m_settings->setSearchEngineUrl(url);
    });

    // Privacy toggles
    connect(m_ephemeralCheck, &QCheckBox::toggled, m_settings, &Settings::setEphemeralSessions);
    connect(m_dntCheck, &QCheckBox::toggled, m_settings, &Settings::setDoNotTrack);

    // Security toggles
    connect(m_adBlockCheck, &QCheckBox::toggled, m_settings, &Settings::setAdBlockingEnabled);
    connect(m_jsCheck, &QCheckBox::toggled, m_settings, &Settings::setJavaScriptEnabled);
}
