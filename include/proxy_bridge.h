#pragma once

#include <QObject>
#include "proxy_event.h"

Q_DECLARE_METATYPE(ProxyEvent)

// Ponto de cruzamento thread-safe entre as threads Boost e a thread Qt.
// post() pode ser chamado de qualquer thread; requestCaptured e emitido na thread Qt.
class ProxyBridge : public QObject {
    Q_OBJECT
public:
    explicit ProxyBridge(QObject* parent = nullptr);

    void post(ProxyEvent event);

signals:
    void requestCaptured(ProxyEvent event);
};
