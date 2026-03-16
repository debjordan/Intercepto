#include "mainwindow.h"

#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QVBoxLayout>

// Paleta Catppuccin Mocha
static const char* kStyle = R"(
QMainWindow, QWidget {
    background-color: #1e1e2e;
    color: #cdd6f4;
    font-size: 13px;
}
QGroupBox {
    border: 1px solid #313244;
    border-radius: 6px;
    margin-top: 10px;
    padding-top: 6px;
    font-weight: bold;
    color: #cdd6f4;
}
QGroupBox::title {
    color: #89b4fa;
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
}
QLineEdit {
    background-color: #181825;
    border: 1px solid #313244;
    border-radius: 4px;
    padding: 5px 8px;
    color: #cdd6f4;
}
QLineEdit:focus { border-color: #89b4fa; }
QPushButton {
    background-color: #313244;
    border: none;
    border-radius: 4px;
    padding: 6px 16px;
    color: #cdd6f4;
    font-weight: bold;
}
QPushButton:hover    { background-color: #45475a; }
QPushButton:pressed  { background-color: #585b70; }
QPushButton#btnStart { background-color: #a6e3a1; color: #1e1e2e; }
QPushButton#btnStart:hover { background-color: #94e090; }
QPushButton#btnStop  { background-color: #f38ba8; color: #1e1e2e; }
QPushButton#btnStop:hover  { background-color: #e07898; }
QTableWidget {
    background-color: #181825;
    alternate-background-color: #1e1e2e;
    gridline-color: #313244;
    selection-background-color: #313244;
    selection-color: #cdd6f4;
    border: none;
    border-radius: 4px;
}
QTableWidget::item { padding: 3px 8px; }
QHeaderView::section {
    background-color: #11111b;
    color: #89b4fa;
    padding: 6px 8px;
    border: none;
    border-bottom: 2px solid #313244;
    font-weight: bold;
}
QListWidget {
    background-color: #181825;
    border: none;
    border-radius: 4px;
}
QListWidget::item { padding: 6px 8px; border-radius: 4px; }
QListWidget::item:selected { background-color: #313244; }
QTextEdit {
    background-color: #181825;
    border: none;
    border-radius: 4px;
    color: #cdd6f4;
    font-family: "Courier New", monospace;
    font-size: 12px;
}
QScrollBar:vertical   { background: #181825; width: 6px; }
QScrollBar:horizontal { background: #181825; height: 6px; }
QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
    background: #45475a; border-radius: 3px; min-height: 20px;
}
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
    background: #585b70;
}
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QStatusBar { background-color: #11111b; color: #6c7086; font-size: 12px; }
QSplitter::handle { background-color: #313244; }
QSplitter::handle:horizontal { width: 2px; }
QSplitter::handle:vertical   { height: 2px; }
)";

// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(ProxyBridge* bridge, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Intercepto");
    setMinimumSize(1100, 700);
    setStyleSheet(kStyle);

    manager_ = new ProxyManager(bridge, this);

    setupUi();
    setupConnections();

    statusLabel_ = new QLabel("Pronto");
    statusBar()->addPermanentWidget(statusLabel_);
}

MainWindow::~MainWindow() {
    manager_->stopAll();
}

// ─── UI ───────────────────────────────────────────────────────────────────────

void MainWindow::setupUi() {
    auto* central = new QWidget;
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // Toolbar
    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("background-color: #181825; border-radius: 6px; padding: 2px;");
    auto* tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(8, 4, 8, 4);
    tl->setSpacing(8);

    tl->addWidget(new QLabel("Porta:"));
    portInput_ = new QLineEdit;
    portInput_->setPlaceholderText("ex: 8080");
    portInput_->setFixedWidth(90);
    tl->addWidget(portInput_);

    btnStart_ = new QPushButton("Iniciar");
    btnStart_->setObjectName("btnStart");
    tl->addWidget(btnStart_);

    tl->addStretch();

    btnClear_ = new QPushButton("Limpar Log");
    tl->addWidget(btnClear_);

    toolbar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    root->addWidget(toolbar);

    // Separador visual abaixo da toolbar
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: #313244;");
    root->addWidget(sep);

    // Splitter principal: esquerda (portas) | direita (tabela + detalhes)
    auto* mainSplit = new QSplitter(Qt::Horizontal);

    // Painel esquerdo: portas ativas
    auto* portsGroup = new QGroupBox("Portas Monitoradas");
    auto* pl = new QVBoxLayout(portsGroup);
    pl->setSpacing(4);

    portsList_ = new QListWidget;
    pl->addWidget(portsList_);

    btnStop_ = new QPushButton("Parar Porta");
    btnStop_->setObjectName("btnStop");
    btnStop_->setEnabled(false);
    pl->addWidget(btnStop_);

    mainSplit->addWidget(portsGroup);
    mainSplit->setStretchFactor(0, 0);

    // Splitter direito: tabela | detalhes
    auto* rightSplit = new QSplitter(Qt::Vertical);

    reqTable_ = new QTableWidget;
    reqTable_->setColumnCount(8);
    reqTable_->setHorizontalHeaderLabels(
        {"#", "Porta", "Hora", "Metodo", "Host", "URL", "Status", "Latencia"});
    reqTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    reqTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    reqTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reqTable_->setAlternatingRowColors(true);
    reqTable_->verticalHeader()->hide();
    reqTable_->setShowGrid(false);
    reqTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    reqTable_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    reqTable_->horizontalHeader()->setDefaultSectionSize(90);
    reqTable_->setColumnWidth(0, 50);
    reqTable_->setColumnWidth(1, 60);
    reqTable_->setColumnWidth(2, 95);
    reqTable_->setColumnWidth(3, 70);
    reqTable_->setColumnWidth(6, 80);
    reqTable_->setColumnWidth(7, 80);
    rightSplit->addWidget(reqTable_);

    auto* detailsGroup = new QGroupBox("Detalhes da Requisicao");
    auto* dl = new QVBoxLayout(detailsGroup);
    detailsText_ = new QTextEdit;
    detailsText_->setReadOnly(true);
    dl->addWidget(detailsText_);
    rightSplit->addWidget(detailsGroup);
    rightSplit->setStretchFactor(0, 3);
    rightSplit->setStretchFactor(1, 1);
    rightSplit->setSizes({500, 200});

    mainSplit->addWidget(rightSplit);
    mainSplit->setStretchFactor(1, 1);
    mainSplit->setSizes({200, 880});

    // stretch factor 1: o splitter expande para preencher todo o espaco restante
    root->addWidget(mainSplit, 1);
}

void MainWindow::setupConnections() {
    connect(btnStart_,  &QPushButton::clicked,     this, &MainWindow::onStartClicked);
    connect(btnStop_,   &QPushButton::clicked,     this, &MainWindow::onStopPortClicked);
    connect(btnClear_,  &QPushButton::clicked,     this, &MainWindow::onClearClicked);
    connect(portInput_, &QLineEdit::returnPressed, this, &MainWindow::onStartClicked);

    connect(portsList_, &QListWidget::itemSelectionChanged, this, [this]() {
        btnStop_->setEnabled(portsList_->currentItem() != nullptr);
    });

    connect(reqTable_, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onRowSelected);

    connect(manager_, &ProxyManager::portError, this,
            [this](int port, QString msg) {
                QMessageBox::warning(this, "Erro na Porta",
                    QString("Porta %1: %2").arg(port).arg(msg));
            });

    connect(manager_, &ProxyManager::reqCountChanged,
            this, &MainWindow::onPortCountChanged);
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void MainWindow::onStartClicked() {
    bool ok;
    int port = portInput_->text().toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        statusBar()->showMessage("Porta invalida (1-65535)", 3000);
        return;
    }
    if (manager_->isRunning(port)) {
        statusBar()->showMessage(QString("Porta %1 ja esta ativa").arg(port), 3000);
        return;
    }
    if (manager_->startPort(port)) {
        auto* item = new QListWidgetItem(QString(":%1   (0 req)").arg(port));
        item->setData(Qt::UserRole, port);
        item->setForeground(QColor("#a6e3a1"));
        portsList_->addItem(item);
        portInput_->clear();
        statusBar()->showMessage(QString("Proxy iniciado na porta %1").arg(port), 3000);
    }
}

void MainWindow::onStopPortClicked() {
    auto* item = portsList_->currentItem();
    if (!item) return;
    int port = item->data(Qt::UserRole).toInt();
    manager_->stopPort(port);
    delete portsList_->takeItem(portsList_->row(item));
    statusBar()->showMessage(QString("Porta %1 parada").arg(port), 3000);
}

void MainWindow::onClearClicked() {
    reqTable_->setRowCount(0);
    detailsText_->clear();
    events_.clear();
    totalReqs_ = 0;
    updateStatusBar();
}

void MainWindow::onPortCountChanged(int port, uint64_t count) {
    for (int i = 0; i < portsList_->count(); ++i) {
        auto* item = portsList_->item(i);
        if (item->data(Qt::UserRole).toInt() == port) {
            item->setText(QString(":%1   (%2 req)").arg(port).arg(count));
            break;
        }
    }
}

void MainWindow::onRequestCaptured(ProxyEvent event) {
    if (event.is_error) return;

    // Adiciona linha na tabela
    int row = reqTable_->rowCount();
    reqTable_->insertRow(row);

    auto mkItem = [](const QString& text) {
        auto* i = new QTableWidgetItem(text);
        i->setFlags(i->flags() & ~Qt::ItemIsEditable);
        return i;
    };

    QString method  = QString::fromStdString(event.method);
    QString status  = QString("%1 %2")
                          .arg(event.status_code)
                          .arg(QString::fromStdString(event.status_text));

    auto* idItem  = mkItem(QString::number(event.id));
    auto* portItem= mkItem(QString::number(event.port));
    auto* timeItem= mkItem(QString::fromStdString(event.timestamp));
    auto* methItem= mkItem(method);
    auto* hostItem= mkItem(QString::fromStdString(event.host));
    auto* urlItem = mkItem(QString::fromStdString(event.url));
    auto* statItem= mkItem(status);
    auto* latItem = mkItem(QString("%1ms").arg(event.latency_ms));

    // Cores por metodo
    QColor mc = [&]() -> QColor {
        if (method == "GET")    return QColor("#a6e3a1");
        if (method == "POST")   return QColor("#f9e2af");
        if (method == "PUT")    return QColor("#89b4fa");
        if (method == "DELETE") return QColor("#f38ba8");
        if (method == "PATCH")  return QColor("#89dceb");
        return QColor("#cdd6f4");
    }();
    methItem->setForeground(mc);

    // Cores por status
    QColor sc = [&]() -> QColor {
        unsigned c = event.status_code;
        if (c < 300) return QColor("#a6e3a1");
        if (c < 400) return QColor("#f9e2af");
        if (c < 500) return QColor("#f38ba8");
        return QColor("#f38ba8");
    }();
    statItem->setForeground(sc);

    idItem->setTextAlignment(Qt::AlignCenter);
    portItem->setTextAlignment(Qt::AlignCenter);
    statItem->setTextAlignment(Qt::AlignCenter);
    latItem->setTextAlignment(Qt::AlignCenter);

    reqTable_->setItem(row, 0, idItem);
    reqTable_->setItem(row, 1, portItem);
    reqTable_->setItem(row, 2, timeItem);
    reqTable_->setItem(row, 3, methItem);
    reqTable_->setItem(row, 4, hostItem);
    reqTable_->setItem(row, 5, urlItem);
    reqTable_->setItem(row, 6, statItem);
    reqTable_->setItem(row, 7, latItem);

    events_[event.id] = std::move(event);

    reqTable_->scrollToBottom();
    totalReqs_++;
    updateStatusBar();
}

void MainWindow::onRowSelected() {
    int row = reqTable_->currentRow();
    if (row < 0) return;

    auto* idItem = reqTable_->item(row, 0);
    if (!idItem) return;

    uint64_t id = idItem->text().toULongLong();
    auto it = events_.find(id);
    if (it == events_.end()) return;

    detailsText_->setHtml(buildDetailsHtml(it->second));
}

void MainWindow::updateStatusBar() {
    statusLabel_->setText(QString("Total: %1 requisicoes").arg(totalReqs_));
}

// ─── HTML de detalhes ─────────────────────────────────────────────────────────

QString MainWindow::buildDetailsHtml(const ProxyEvent& e) const {
    auto esc = [](const std::string& s) {
        return QString::fromStdString(s).toHtmlEscaped();
    };

    QString sc = e.status_code < 300 ? "#a6e3a1"
               : e.status_code < 400 ? "#f9e2af"
               : "#f38ba8";

    QString html;
    html += "<body style='background:#181825;color:#cdd6f4;font-family:monospace;margin:8px'>";

    html += QString("<span style='color:#89b4fa;font-weight:bold'>%1</span>"
                    " <span style='color:#cdd6f4'>%2</span>"
                    " &nbsp;<span style='color:#6c7086'>-></span>&nbsp;"
                    "<span style='color:#89dceb'>%3:%4</span>"
                    " &nbsp;<span style='color:#6c7086'>%5ms</span><br><br>")
                .arg(esc(e.method), esc(e.url))
                .arg(esc(e.host)).arg(e.port)
                .arg(e.latency_ms);

    html += QString("<span style='color:%1;font-weight:bold'>Resposta: %2 %3</span><br><br>")
                .arg(sc).arg(e.status_code).arg(esc(e.status_text));

    if (!e.req_headers.empty()) {
        html += "<span style='color:#cba6f7;font-weight:bold'>Requisicao - Headers</span><br>";
        html += QString("<pre style='margin:0 0 8px 0;color:#a6adc8'>%1</pre>").arg(esc(e.req_headers));
    }
    if (!e.req_body.empty()) {
        html += "<span style='color:#cba6f7;font-weight:bold'>Requisicao - Body</span><br>";
        html += QString("<pre style='margin:0 0 8px 0;color:#a6adc8'>%1</pre>")
                    .arg(esc(e.req_body.size() > 4096 ? e.req_body.substr(0, 4096) + "\n[truncado]" : e.req_body));
    }
    if (!e.res_headers.empty()) {
        html += "<span style='color:#cba6f7;font-weight:bold'>Resposta - Headers</span><br>";
        html += QString("<pre style='margin:0 0 8px 0;color:#a6adc8'>%1</pre>").arg(esc(e.res_headers));
    }
    if (!e.res_body.empty()) {
        html += "<span style='color:#cba6f7;font-weight:bold'>Resposta - Body</span><br>";
        html += QString("<pre style='margin:0 0 0 0;color:#a6adc8'>%1</pre>")
                    .arg(esc(e.res_body.size() > 4096 ? e.res_body.substr(0, 4096) + "\n[truncado]" : e.res_body));
    }

    html += "</body>";
    return html;
}
