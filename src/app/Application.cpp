#include "app/Application.h"
#include "app/Settings.h"
#include "security/SessionManager.h"
#include "security/AdBlocker.h"
#include "security/DnsOverHttps.h"
#include "security/CertificatePinner.h"
#include "security/CSPEnforcer.h"
#include "sandbox/ProcessSandbox.h"
#include "network/RequestInterceptor.h"
#include "network/NetworkManager.h"
#include "app/DownloadManager.h"

#include <QIcon>
#include <QFile>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QtWebEngineCore/QWebEngineUrlScheme>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName("scowser");
    setApplicationVersion("0.0.26");
    setOrganizationName("scowser");
    setWindowIcon(QIcon(":/icons/scowser.png"));

    m_settings = std::make_unique<Settings>(this);

    loadStyleSheet();
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
    m_requestInterceptor = std::make_unique<RequestInterceptor>(m_adBlocker.get(), m_dnsResolver.get(), this);
    m_networkManager = std::make_unique<NetworkManager>(m_certPinner.get(), this);
    m_downloadManager = std::make_unique<DownloadManager>(this);

    m_sandbox->applySandbox();

    applySettings();
    connectSettings();
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
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, m_settings->javaScriptEnabled());
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);

    // Inject CSP enforcement script into every page at document creation
    QWebEngineScript cspScript;
    cspScript.setName("scowser-csp-enforcer");
    cspScript.setSourceCode(m_cspEnforcer->enforcementScript());
    cspScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    cspScript.setWorldId(QWebEngineScript::ApplicationWorld);
    cspScript.setRunsOnSubFrames(true);
    profile->scripts()->insert(cspScript);

    // Connect download requests from the profile to the download manager
    connect(profile, &QWebEngineProfile::downloadRequested,
            m_downloadManager.get(), &DownloadManager::onDownloadRequested);
}

void Application::loadStyleSheet()
{
    QFile qss(":/style/scowser.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(qss.readAll());
        qss.close();
    }
}

void Application::disableTelemetry()
{
    // Disable Chromium telemetry and reporting, enable DNS-over-HTTPS
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
        // Enable Chromium's built-in Secure DNS (DoH) for all WebEngine requests.
        // Mode "secure" means only DoH is used — no fallback to plaintext DNS.
        "--dns-over-https-mode=secure "
        "--dns-over-https-templates=https://cloudflare-dns.com/dns-query "
        "--disable-features=AutofillServerCommunication,NetworkTimeServiceQuerying");
}

void Application::applySettings()
{
    m_dnsResolver->setProvider(m_settings->dnsProvider());
    if (m_settings->dnsProvider() == DnsOverHttps::Custom)
        m_dnsResolver->setCustomProvider(m_settings->customDnsUrl());

    m_sessionManager->setEphemeral(m_settings->ephemeralSessions());
    m_adBlocker->setEnabled(m_settings->adBlockingEnabled());
    m_requestInterceptor->setDoNotTrack(m_settings->doNotTrack());
    m_downloadManager->setDownloadDirectory(m_settings->downloadDirectory());
}

void Application::connectSettings()
{
    connect(m_settings.get(), &Settings::dnsProviderChanged,
            m_dnsResolver.get(), &DnsOverHttps::setProvider);

    connect(m_settings.get(), &Settings::customDnsUrlChanged,
            m_dnsResolver.get(), &DnsOverHttps::setCustomProvider);

    connect(m_settings.get(), &Settings::ephemeralSessionsChanged,
            m_sessionManager.get(), &SessionManager::setEphemeral);

    connect(m_settings.get(), &Settings::adBlockingEnabledChanged,
            m_adBlocker.get(), &AdBlocker::setEnabled);

    connect(m_settings.get(), &Settings::doNotTrackChanged,
            m_requestInterceptor.get(), &RequestInterceptor::setDoNotTrack);

    connect(m_settings.get(), &Settings::downloadDirectoryChanged,
            m_downloadManager.get(), &DownloadManager::setDownloadDirectory);

    connect(m_settings.get(), &Settings::javaScriptEnabledChanged, this, [](bool enabled) {
        auto *profile = QWebEngineProfile::defaultProfile();
        profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, enabled);
    });
}
