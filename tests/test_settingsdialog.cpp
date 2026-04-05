#include <QtTest/QtTest>
#include <QCheckBox>
#include <QComboBox>
#include <QTemporaryFile>
#include "app/Settings.h"
#include "ui/SettingsDialog.h"

class TestSettingsDialog : public QObject {
    Q_OBJECT

private slots:
    void testDialogCreation();
    void testWidgetsReflectDefaults();
    void testWidgetsReflectCustomSettings();
    void testDnsComboUpdatesSettings();
    void testCheckboxTogglesUpdateSettings();
};

static QString tempSettingsPath()
{
    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    tmp.open();
    QString path = tmp.fileName();
    tmp.close();
    return path;
}

void TestSettingsDialog::testDialogCreation()
{
    QString path = tempSettingsPath();
    Settings settings(path);
    SettingsDialog dialog(&settings);

    QCOMPARE(dialog.windowTitle(), QString("Settings"));
    QCOMPARE(dialog.isVisible(), false);

    QFile::remove(path);
}

void TestSettingsDialog::testWidgetsReflectDefaults()
{
    QString path = tempSettingsPath();
    Settings settings(path);
    SettingsDialog dialog(&settings);

    // Find widgets
    auto *dnsCombo = dialog.findChild<QComboBox *>();
    auto *ephemeralCheck = dialog.findChild<QCheckBox *>(""); // find all checkboxes
    auto checkboxes = dialog.findChildren<QCheckBox *>();

    QVERIFY(dnsCombo != nullptr);
    QCOMPARE(dnsCombo->currentIndex(), 0); // Cloudflare

    // Should have 4 checkboxes: ephemeral, dnt, adblock, javascript
    QCOMPARE(checkboxes.size(), 4);

    // All defaults are true
    for (auto *cb : checkboxes) {
        QVERIFY(cb->isChecked());
    }

    QFile::remove(path);
}

void TestSettingsDialog::testWidgetsReflectCustomSettings()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    settings.setDnsProvider(DnsOverHttps::Quad9);
    settings.setAdBlockingEnabled(false);
    settings.setJavaScriptEnabled(false);

    SettingsDialog dialog(&settings);

    auto combos = dialog.findChildren<QComboBox *>();
    QVERIFY(!combos.isEmpty());
    QCOMPARE(combos.first()->currentIndex(), 1); // Quad9

    QFile::remove(path);
}

void TestSettingsDialog::testDnsComboUpdatesSettings()
{
    QString path = tempSettingsPath();
    Settings settings(path);
    SettingsDialog dialog(&settings);

    QSignalSpy spy(&settings, &Settings::dnsProviderChanged);

    auto *dnsCombo = dialog.findChildren<QComboBox *>().first();
    dnsCombo->setCurrentIndex(1); // Quad9

    QCOMPARE(spy.count(), 1);
    QCOMPARE(settings.dnsProvider(), DnsOverHttps::Quad9);

    QFile::remove(path);
}

void TestSettingsDialog::testCheckboxTogglesUpdateSettings()
{
    QString path = tempSettingsPath();
    Settings settings(path);
    SettingsDialog dialog(&settings);

    QSignalSpy adBlockSpy(&settings, &Settings::adBlockingEnabledChanged);
    QSignalSpy jsSpy(&settings, &Settings::javaScriptEnabledChanged);

    // Find checkboxes by text
    QCheckBox *adBlockCheck = nullptr;
    QCheckBox *jsCheck = nullptr;
    for (auto *cb : dialog.findChildren<QCheckBox *>()) {
        if (cb->text().contains("ad and tracker"))
            adBlockCheck = cb;
        if (cb->text().contains("JavaScript"))
            jsCheck = cb;
    }

    QVERIFY(adBlockCheck != nullptr);
    QVERIFY(jsCheck != nullptr);

    adBlockCheck->setChecked(false);
    QCOMPARE(adBlockSpy.count(), 1);
    QCOMPARE(settings.adBlockingEnabled(), false);

    jsCheck->setChecked(false);
    QCOMPARE(jsSpy.count(), 1);
    QCOMPARE(settings.javaScriptEnabled(), false);

    QFile::remove(path);
}

QTEST_MAIN(TestSettingsDialog)
#include "test_settingsdialog.moc"
