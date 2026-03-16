#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <unordered_map>

#include "proxy_event.h"
#include "proxy_manager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(ProxyBridge* bridge, QWidget* parent = nullptr);
    ~MainWindow() override;

public slots:
    void onRequestCaptured(ProxyEvent event);

private slots:
    void onStartClicked();
    void onStopPortClicked();
    void onClearClicked();
    void onRowSelected();
    void onPortCountChanged(int port, uint64_t count);

private:
    void setupUi();
    void setupConnections();
    void applyTheme();
    void updateStatusBar();
    QString buildDetailsHtml(const ProxyEvent& e) const;

    // Widgets
    QLineEdit*    portInput_;
    QPushButton*  btnStart_;
    QPushButton*  btnStop_;
    QPushButton*  btnClear_;
    QListWidget*  portsList_;
    QTableWidget* reqTable_;
    QTextEdit*    detailsText_;
    QLabel*       statusLabel_;

    // State
    ProxyManager*                       manager_;
    std::unordered_map<uint64_t, ProxyEvent> events_;
    uint64_t                            totalReqs_{0};
};
