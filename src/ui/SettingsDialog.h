#pragma once

#include <QDialog>

class Settings;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QTabWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(Settings *settings, QWidget *parent = nullptr);

private:
    void setupPrivacyTab(QTabWidget *tabs);
    void setupSecurityTab(QTabWidget *tabs);
    void loadFromSettings();
    void connectWidgets();

    Settings *m_settings;

    // Privacy tab
    QComboBox *m_dnsProviderCombo;
    QLineEdit *m_customDnsEdit;
    QComboBox *m_searchEngineCombo;
    QCheckBox *m_ephemeralCheck;
    QCheckBox *m_dntCheck;

    // Security tab
    QCheckBox *m_adBlockCheck;
    QCheckBox *m_jsCheck;
};
