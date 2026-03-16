#include <QApplication>
#include "proxy_bridge.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Intercepto");
    app.setApplicationVersion("1.0.0");

    // Registra ProxyEvent para uso em conexoes Qt entre threads
    qRegisterMetaType<ProxyEvent>("ProxyEvent");

    ProxyBridge bridge;
    MainWindow  window(&bridge);

    // Conecta bridge -> mainwindow (bridge emite na thread Qt, conexao direta e segura)
    QObject::connect(&bridge, &ProxyBridge::requestCaptured,
                     &window, &MainWindow::onRequestCaptured,
                     Qt::QueuedConnection);

    window.show();
    return app.exec();
}
