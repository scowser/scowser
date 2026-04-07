#pragma once

#include <QApplication>
#include <memory>

class Settings;
class SessionManager;
class AdBlocker;
class DnsOverHttps;
class CertificatePinner;
class CSPEnforcer;
class ProcessSandbox;
class RequestInterceptor;
class NetworkManager;
class DownloadManager;
class FavoritesManager;

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() override;

    static Application *instance();

    Settings *settings() const { return m_settings.get(); }
    SessionManager *sessionManager() const { return m_sessionManager.get(); }
    AdBlocker *adBlocker() const { return m_adBlocker.get(); }
    DnsOverHttps *dnsResolver() const { return m_dnsResolver.get(); }
    CertificatePinner *certPinner() const { return m_certPinner.get(); }
    ProcessSandbox *sandbox() const { return m_sandbox.get(); }
    CSPEnforcer *cspEnforcer() const { return m_cspEnforcer.get(); }
    RequestInterceptor *requestInterceptor() const { return m_requestInterceptor.get(); }
    DownloadManager *downloadManager() const { return m_downloadManager.get(); }
    FavoritesManager *favoritesManager() const { return m_favoritesManager.get(); }

private:
    void loadStyleSheet();
    void initSecurity();
    void configureWebEngine();
    void disableTelemetry();
    void applySettings();
    void connectSettings();

    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<AdBlocker> m_adBlocker;
    std::unique_ptr<DnsOverHttps> m_dnsResolver;
    std::unique_ptr<CertificatePinner> m_certPinner;
    std::unique_ptr<CSPEnforcer> m_cspEnforcer;
    std::unique_ptr<ProcessSandbox> m_sandbox;
    std::unique_ptr<RequestInterceptor> m_requestInterceptor;
    std::unique_ptr<NetworkManager> m_networkManager;
    std::unique_ptr<DownloadManager> m_downloadManager;
    std::unique_ptr<FavoritesManager> m_favoritesManager;
};
