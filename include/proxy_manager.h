#pragma once

#include <QObject>
#include <map>
#include <memory>

#include "proxy.h"
#include "proxy_bridge.h"

struct PortEntry {
    std::unique_ptr<Proxy> proxy;
    uint64_t               req_count{0};
};

class ProxyManager : public QObject {
    Q_OBJECT
public:
    explicit ProxyManager(ProxyBridge* bridge, QObject* parent = nullptr);
    ~ProxyManager() override;

    // Inicia proxy na porta. Retorna false se a porta ja estiver ativa ou falhar.
    bool   startPort(int port);

    // Para e remove o proxy da porta.
    void   stopPort(int port);

    // Para todos os proxies ativos.
    void   stopAll();

    bool   isRunning(int port) const;
    uint64_t reqCount(int port) const;

signals:
    void portStarted(int port);
    void portStopped(int port);
    void portError(int port, QString message);
    void reqCountChanged(int port, uint64_t count);

private:
    ProxyBridge*          bridge_;
    std::map<int, PortEntry> ports_;
};
