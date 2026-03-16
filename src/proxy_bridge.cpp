#include "proxy_bridge.h"

ProxyBridge::ProxyBridge(QObject* parent) : QObject(parent) {}

void ProxyBridge::post(ProxyEvent event) {
    // Marshalla a emissao do signal para a thread principal do Qt.
    QMetaObject::invokeMethod(this, [this, e = std::move(event)]() mutable {
        emit requestCaptured(std::move(e));
    }, Qt::QueuedConnection);
}
