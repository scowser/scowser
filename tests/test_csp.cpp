#include <QtTest/QtTest>
#include "security/CSPEnforcer.h"

class TestCSPEnforcer : public QObject {
    Q_OBJECT

private slots:
    void testDefaultPolicyApplied();
    void testExistingPolicyStrengthened();
    void testBlocksJavascriptURI();
    void testBlocksDataURIInScripts();
    void testAllowsSameOrigin();
    void testAllowsHTTPS();
};

void TestCSPEnforcer::testDefaultPolicyApplied()
{
    CSPEnforcer enforcer;

    QUrl page("https://example.com");
    QString result = enforcer.enforcePolicy(page, "");

    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("default-src"));
    QVERIFY(result.contains("object-src 'none'"));
    QVERIFY(result.contains("upgrade-insecure-requests"));
}

void TestCSPEnforcer::testExistingPolicyStrengthened()
{
    CSPEnforcer enforcer;

    QUrl page("https://example.com");
    QString existing = "default-src 'self'; script-src 'self'";
    QString result = enforcer.enforcePolicy(page, existing);

    // Should add missing directives
    QVERIFY(result.contains("object-src 'none'"));
    QVERIFY(result.contains("base-uri 'self'"));
    QVERIFY(result.contains("frame-ancestors 'self'"));
    QVERIFY(result.contains("upgrade-insecure-requests"));
}

void TestCSPEnforcer::testBlocksJavascriptURI()
{
    CSPEnforcer enforcer;

    QUrl jsUri("javascript:alert(1)");
    QUrl page("https://example.com");

    QVERIFY(!enforcer.allowsResource("script-src", jsUri, page));
}

void TestCSPEnforcer::testBlocksDataURIInScripts()
{
    CSPEnforcer enforcer;

    QUrl dataUri("data:text/javascript,alert(1)");
    QUrl page("https://example.com");

    QVERIFY(!enforcer.allowsResource("script-src", dataUri, page));
}

void TestCSPEnforcer::testAllowsSameOrigin()
{
    CSPEnforcer enforcer;

    QUrl resource("https://example.com/script.js");
    QUrl page("https://example.com");

    QVERIFY(enforcer.allowsResource("script-src", resource, page));
}

void TestCSPEnforcer::testAllowsHTTPS()
{
    CSPEnforcer enforcer;

    QUrl resource("https://cdn.example.com/lib.js");
    QUrl page("https://example.com");

    QVERIFY(enforcer.allowsResource("script-src", resource, page));
}

QTEST_MAIN(TestCSPEnforcer)
#include "test_csp.moc"
