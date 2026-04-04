#pragma once

#include <QApplication>
#include <memory>

class SessionManager;
class AdBlocker;
class DnsOverHttps;
class CertificatePinner;
class CSPEnforcer;
class ProcessSandbox;
class RequestInterceptor;
class NetworkManager;

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() override;

    static Application *instance();

    SessionManager *sessionManager() const { return m_sessionManager.get(); }
    AdBlocker *adBlocker() const { return m_adBlocker.get(); }
    DnsOverHttps *dnsResolver() const { return m_dnsResolver.get(); }
    CertificatePinner *certPinner() const { return m_certPinner.get(); }
    ProcessSandbox *sandbox() const { return m_sandbox.get(); }
    CSPEnforcer *cspEnforcer() const { return m_cspEnforcer.get(); }
    RequestInterceptor *requestInterceptor() const { return m_requestInterceptor.get(); }

private:
    void loadStyleSheet();
    void initSecurity();
    void configureWebEngine();
    void disableTelemetry();

    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<AdBlocker> m_adBlocker;
    std::unique_ptr<DnsOverHttps> m_dnsResolver;
    std::unique_ptr<CertificatePinner> m_certPinner;
    std::unique_ptr<CSPEnforcer> m_cspEnforcer;
    std::unique_ptr<ProcessSandbox> m_sandbox;
    std::unique_ptr<RequestInterceptor> m_requestInterceptor;
    std::unique_ptr<NetworkManager> m_networkManager;
};
