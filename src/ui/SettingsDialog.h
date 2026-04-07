#pragma once

#include <QDialog>

class Settings;
class QComboBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QTabWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(Settings *settings, QWidget *parent = nullptr);

private:
    void setupGeneralTab(QTabWidget *tabs);
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

    // General tab
    QCheckBox *m_useDefaultHomepage;
    QLineEdit *m_homepageEdit;
    QLabel *m_downloadDirLabel;

    // Security tab
    QCheckBox *m_adBlockCheck;
    QCheckBox *m_jsCheck;
};
