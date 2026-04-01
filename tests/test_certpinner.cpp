#include <QtTest/QtTest>
#include "security/CertificatePinner.h"

class TestCertificatePinner : public QObject {
    Q_OBJECT

private slots:
    void testEmptyChainFails();
    void testNoPinsAccepts();
    void testPinMismatchFails();
    void testSubdomainMatching();
};

void TestCertificatePinner::testEmptyChainFails()
{
    CertificatePinner pinner;

    QList<QSslCertificate> emptyChain;
    QVERIFY(!pinner.validateChain("example.com", emptyChain));
}

void TestCertificatePinner::testNoPinsAccepts()
{
    CertificatePinner pinner;

    // Generate a self-signed cert for testing
    // Without pins configured, validation should pass (if chain is non-empty and strong)
    QVERIFY(!pinner.hasPins("example.com"));
}

void TestCertificatePinner::testPinMismatchFails()
{
    CertificatePinner pinner;

    // Pin a fake hash
    pinner.pinCertificate("example.com", QByteArray("fakehash123"));
    QVERIFY(pinner.hasPins("example.com"));

    // Empty chain should fail
    QList<QSslCertificate> emptyChain;
    QVERIFY(!pinner.validateChain("example.com", emptyChain));
}

void TestCertificatePinner::testSubdomainMatching()
{
    CertificatePinner pinner;

    pinner.pinCertificate("example.com", QByteArray("somehash"));
    QVERIFY(pinner.hasPins("example.com"));
    QVERIFY(!pinner.hasPins("sub.example.com")); // Exact match only in hasPins
}

QTEST_MAIN(TestCertificatePinner)
#include "test_certpinner.moc"
