#include "app/Application.h"
#include "security/SessionManager.h"
#include "security/AdBlocker.h"
#include "security/DnsOverHttps.h"
#include "security/CertificatePinner.h"
#include "security/CSPEnforcer.h"
#include "sandbox/ProcessSandbox.h"
#include "network/RequestInterceptor.h"
#include "network/NetworkManager.h"

#include <QIcon>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineUrlScheme>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName("scowser");
    setApplicationVersion("0.0.13");
    setOrganizationName("scowser");
    setWindowIcon(QIcon(":/icons/scowser.png"));

    disableTelemetry();
    initSecurity();
    configureWebEngine();
}

Application::~Application() = default;

Application *Application::instance()
{
    return qobject_cast<Application *>(QCoreApplication::instance());
}

void Application::initSecurity()
{
    m_sessionManager = std::make_unique<SessionManager>(this);
    m_adBlocker = std::make_unique<AdBlocker>(this);
    m_dnsResolver = std::make_unique<DnsOverHttps>(this);
    m_certPinner = std::make_unique<CertificatePinner>(this);
    m_cspEnforcer = std::make_unique<CSPEnforcer>(this);
    m_sandbox = std::make_unique<ProcessSandbox>(this);
    m_requestInterceptor = std::make_unique<RequestInterceptor>(m_adBlocker.get(), this);
    m_networkManager = std::make_unique<NetworkManager>(m_certPinner.get(), this);

    m_sandbox->applySandbox();
}

void Application::configureWebEngine()
{
    // Use off-the-record profile for ephemeral sessions
    auto *profile = QWebEngineProfile::defaultProfile();

    // Disable persistent cookies and cache (the default profile is already
    // off-the-record). Do NOT clear setPersistentStoragePath — Chromium's
    // certificate trust store needs a valid data directory.
    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

    // Disable spell checking (sends data externally on some configs)
    profile->setSpellCheckEnabled(false);

    // Install request interceptor for ad blocking and security headers
    profile->setUrlRequestInterceptor(m_requestInterceptor.get());

    // Configure default settings
    auto *settings = profile->settings();
    settings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
}

void Application::disableTelemetry()
{
    // Disable Chromium telemetry and reporting
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
        "--disable-background-networking "
        "--disable-client-side-phishing-detection "
        "--disable-default-apps "
        "--disable-extensions "
        "--disable-sync "
        "--disable-translate "
        "--disable-domain-reliability "
        "--disable-breakpad "
        "--disable-component-update "
        "--no-pings "
        "--metrics-recording-only "
        "--safebrowsing-disable-auto-update "
        "--disable-features=AutofillServerCommunication,NetworkTimeServiceQuerying");
}
