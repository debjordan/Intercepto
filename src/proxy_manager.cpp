#include "proxy_manager.h"

ProxyManager::ProxyManager(ProxyBridge* bridge, QObject* parent)
    : QObject(parent), bridge_(bridge)
{}

ProxyManager::~ProxyManager() {
    stopAll();
}

bool ProxyManager::startPort(int port) {
    if (ports_.count(port)) return false;

    try {
        PortEntry entry;
        entry.proxy = std::make_unique<Proxy>(
            "0.0.0.0", static_cast<uint16_t>(port),
            [this, port](ProxyEvent e) {
                e.port = port;
                bridge_->post(std::move(e));
                auto it = ports_.find(port);
                if (it != ports_.end()) {
                    it->second.req_count++;
                    emit reqCountChanged(port, it->second.req_count);
                }
            }
        );
        entry.proxy->start();
        ports_[port] = std::move(entry);
        emit portStarted(port);
        return true;

    } catch (const std::exception& ex) {
        emit portError(port, QString::fromStdString(ex.what()));
        return false;
    }
}

void ProxyManager::stopPort(int port) {
    auto it = ports_.find(port);
    if (it == ports_.end()) return;
    it->second.proxy->stop();
    ports_.erase(it);
    emit portStopped(port);
}

void ProxyManager::stopAll() {
    for (auto& [port, entry] : ports_) {
        entry.proxy->stop();
    }
    ports_.clear();
}

bool ProxyManager::isRunning(int port) const {
    return ports_.count(port) > 0;
}

uint64_t ProxyManager::reqCount(int port) const {
    auto it = ports_.find(port);
    return it != ports_.end() ? it->second.req_count : 0;
}
