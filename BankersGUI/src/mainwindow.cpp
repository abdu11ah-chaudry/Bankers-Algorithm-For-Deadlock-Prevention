#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QScrollArea>
#include <QLabel>
#include <QHeaderView>
#include <QScrollBar>
#include <QCloseEvent>
#include <QDateTime>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>

// ─── helpers ──────────────────────────────────────────────────────────────────

static QString accent(const QString &col, const QString &text) {
    return "<span style='color:" + col + ";'>" + text + "</span>";
}

QPushButton* MainWindow::makeButton(const QString &text,
                                    const QString &bg,
                                    const QString &hov)
{
    auto *btn = new QPushButton(text);
    QString hoverBg = hov.isEmpty() ? bg : hov;
    btn->setStyleSheet(QString(R"(
        QPushButton {
            background: %1; color: #ffffff;
            border: none; border-radius: 8px;
            padding: 8px 12px; font-weight: 600;
            font-size: 12px; letter-spacing: 0.3px;
        }
        QPushButton:hover { background: %2; }
        QPushButton:pressed { background: %1; opacity: 0.8; }
        QPushButton:disabled { background: #2a2d3e; color: #555; }
    )").arg(bg, hoverBg));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(34);
    return btn;
}

QTableWidgetItem* MainWindow::makeItem(const QString &text, Qt::Alignment align) {
    auto *it = new QTableWidgetItem(text);
    it->setTextAlignment(align);
    return it;
}

void MainWindow::setTableHeaderStyle(QTableWidget *tbl, const QString &accent) {
    tbl->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background:" + accent + "22;"
        " color:#cdd6f4; padding:4px; border:none; font-weight:600; }");
    tbl->verticalHeader()->setStyleSheet(
        "QHeaderView::section { background:#1e1e2e; color:#cdd6f4;"
        " padding:4px; border:none; font-weight:600; }");
}

// ─── constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Banker's Algorithm Simulator");
    setMinimumSize(1280, 820);
    resize(1440, 900);

    m_stepTimer = new QTimer(this);
    m_stepTimer->setInterval(900);
    connect(m_stepTimer, &QTimer::timeout, this, &MainWindow::onStepTimer);

    buildUI();
    applyDarkTheme();
    log("▶  Welcome to Banker's Algorithm Simulator", "#89dceb");
    log("   Load the demo or enter process/resource counts and click Generate Tables.", "#a6adc8");
    logSeparator();
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

void MainWindow::buildUI() {
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Header ──
    auto *header = new QFrame;
    header->setFixedHeight(72);
    header->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #1a1a2e, stop:0.4 #16213e, stop:1 #0f3460);"
        "border-bottom: 2px solid #313244;");
    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    auto *logoLbl = new QLabel("⚙");
    logoLbl->setStyleSheet("font-size:32px; color:#89b4fa;");
    auto *titleLbl = new QLabel("Banker's Algorithm Simulator");
    titleLbl->setStyleSheet("font-size:22px; font-weight:700; color:#cdd6f4; letter-spacing:1px;");
    auto *subLbl   = new QLabel("Deadlock Avoidance &amp; Safe State Detection");
    subLbl->setStyleSheet("font-size:11px; color:#6c7086; letter-spacing:0.5px;");

    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(2);
    titleBox->addWidget(titleLbl);
    titleBox->addWidget(subLbl);

    m_btnTheme = makeButton("☀ Light", "#313244", "#45475a");
    m_btnTheme->setFixedWidth(90);
    connect(m_btnTheme, &QPushButton::clicked, this, &MainWindow::onToggleTheme);

    hlay->addWidget(logoLbl);
    hlay->addSpacing(12);
    hlay->addLayout(titleBox);
    hlay->addStretch();
    hlay->addWidget(m_btnTheme);
    root->addWidget(header);

    // ── Main splitter ──
    auto *mainSplit = new QSplitter(Qt::Horizontal);
    mainSplit->setHandleWidth(4);
    mainSplit->setStyleSheet("QSplitter::handle { background:#313244; }");

    mainSplit->addWidget(buildLeftPanel());
    mainSplit->addWidget(buildCenterPanel());
    mainSplit->addWidget(buildRightPanel());
    mainSplit->setSizes({260, 680, 380});

    // ── Vertical split (main + console) ──
    auto *vSplit = new QSplitter(Qt::Vertical);
    vSplit->setHandleWidth(4);
    vSplit->setStyleSheet("QSplitter::handle { background:#313244; }");
    vSplit->addWidget(mainSplit);
    vSplit->addWidget(buildBottomPanel());
    vSplit->setSizes({620, 200});

    root->addWidget(vSplit);
}

// ─── Left panel ───────────────────────────────────────────────────────────────

QWidget* MainWindow::buildLeftPanel() {
    auto *w = new QWidget;
    w->setMinimumWidth(230);
    w->setMaximumWidth(280);
    w->setStyleSheet("background:#1e1e2e;");
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(14, 16, 14, 16);
    lay->setSpacing(10);

    // Section: Configuration
    auto *cfgBox = new QGroupBox("Configuration");
    cfgBox->setStyleSheet(
        "QGroupBox { color:#89b4fa; font-weight:700; font-size:12px;"
        " border:1px solid #313244; border-radius:10px; margin-top:8px; padding-top:8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 4px; }");
    auto *cfgLay = new QGridLayout(cfgBox);
    cfgLay->setSpacing(8);

    auto makeSpinLabel = [](const QString &txt) {
        auto *l = new QLabel(txt);
        l->setStyleSheet("color:#cdd6f4; font-size:12px;");
        return l;
    };
    auto makeSpinBox = [](int mn, int mx, int def) {
        auto *s = new QSpinBox;
        s->setRange(mn, mx); s->setValue(def);
        s->setStyleSheet(
            "QSpinBox { background:#313244; color:#cdd6f4; border:1px solid #45475a;"
            " border-radius:6px; padding:4px 6px; font-size:12px; }"
            "QSpinBox::up-button, QSpinBox::down-button { width:18px; background:#45475a; border-radius:3px; }");
        s->setMinimumHeight(30);
        return s;
    };

    m_spnProcesses = makeSpinBox(1, 20, 5);
    m_spnResources = makeSpinBox(1, 10, 3);
    m_spnProcesses->setToolTip("Number of processes (1–20)");
    m_spnResources->setToolTip("Number of resource types (1–10)");

    cfgLay->addWidget(makeSpinLabel("Processes:"), 0, 0);
    cfgLay->addWidget(m_spnProcesses, 0, 1);
    cfgLay->addWidget(makeSpinLabel("Resources:"), 1, 0);
    cfgLay->addWidget(m_spnResources, 1, 1);
    lay->addWidget(cfgBox);

    // Section: Actions
    auto *actBox = new QGroupBox("Actions");
    actBox->setStyleSheet(cfgBox->styleSheet());
    auto *actLay = new QVBoxLayout(actBox);
    actLay->setSpacing(7);

    m_btnDemo     = makeButton("📂 Load Demo",       "#5865f2", "#6870db");
    m_btnGenerate = makeButton("⚡ Generate Tables",  "#1e6fa0", "#2483bf");
    m_btnReset    = makeButton("↺  Reset",            "#45475a", "#585b70");
    m_btnCalcNeed = makeButton("∑  Calculate Need",   "#6c4f8a", "#8056a8");
    m_btnRun      = makeButton("▶  Run Algorithm",    "#1a7a4a", "#22a060");
    m_btnStep     = makeButton("⏭  Step Execution",   "#805000", "#a06800");
    m_btnSimReq   = makeButton("📨 Simulate Request", "#6b3030", "#8a3d3d");
    m_btnSave     = makeButton("💾 Save Scenario",    "#2d4a2d", "#3a6b3a");
    m_btnLoad     = makeButton("📁 Load Scenario",    "#2d3a4a", "#3a5060");

    m_btnDemo->setToolTip("Load the classic Silberschatz textbook example");
    m_btnGenerate->setToolTip("Generate editable tables for Allocation and Maximum matrices");
    m_btnReset->setToolTip("Clear all data and start over");
    m_btnCalcNeed->setToolTip("Compute Need = Maximum − Allocation");
    m_btnRun->setToolTip("Run the full Banker's Safety Algorithm");
    m_btnStep->setToolTip("Execute the algorithm one step at a time");
    m_btnSimReq->setToolTip("Simulate a resource request by a process");

    for (auto *b : {m_btnDemo, m_btnGenerate, m_btnReset,
                    m_btnCalcNeed, m_btnRun, m_btnStep,
                    m_btnSimReq, m_btnSave, m_btnLoad})
        actLay->addWidget(b);

    connect(m_btnDemo,     &QPushButton::clicked, this, &MainWindow::onLoadDemo);
    connect(m_btnGenerate, &QPushButton::clicked, this, &MainWindow::onGenerateTables);
    connect(m_btnReset,    &QPushButton::clicked, this, &MainWindow::onReset);
    connect(m_btnCalcNeed, &QPushButton::clicked, this, &MainWindow::onCalculateNeed);
    connect(m_btnRun,      &QPushButton::clicked, this, &MainWindow::onRunAlgorithm);
    connect(m_btnStep,     &QPushButton::clicked, this, &MainWindow::onNextStep);
    connect(m_btnSimReq,   &QPushButton::clicked, this, &MainWindow::onSimulateRequest);
    connect(m_btnSave,     &QPushButton::clicked, this, &MainWindow::onSaveScenario);
    connect(m_btnLoad,     &QPushButton::clicked, this, &MainWindow::onLoadScenario);

    lay->addWidget(actBox);

    // Status label
    m_lblStatus = new QLabel("Ready");
    m_lblStatus->setAlignment(Qt::AlignCenter);
    m_lblStatus->setWordWrap(true);
    m_lblStatus->setStyleSheet(
        "color:#a6e3a1; font-size:11px; padding:6px;"
        " background:#1e2d1e; border-radius:6px; border:1px solid #2d4a2d;");
    lay->addWidget(m_lblStatus);
    lay->addStretch();

    return w;
}

// ─── Center panel ─────────────────────────────────────────────────────────────

QWidget* MainWindow::buildCenterPanel() {
    auto *w = new QWidget;
    w->setStyleSheet("background:#181825;");
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(10);

    auto makeTitle = [](const QString &text, const QString &color) {
        auto *l = new QLabel(text);
        l->setStyleSheet(QString("color:%1; font-size:13px; font-weight:700;"
                                 " padding:2px 0;").arg(color));
        return l;
    };

    auto makeTable = [&](const QString &accent) -> QTableWidget* {
        auto *tbl = new QTableWidget(0, 0);
        tbl->setEditTriggers(QAbstractItemView::AllEditTriggers);
        tbl->setSelectionMode(QAbstractItemView::SingleSelection);
        tbl->setShowGrid(true);
        tbl->setGridStyle(Qt::SolidLine);
        tbl->setAlternatingRowColors(false);
        tbl->setStyleSheet(QString(R"(
            QTableWidget { background:#1e1e2e; color:#cdd6f4;
                gridline-color:#313244; border:1px solid %1;
                border-radius:8px; font-size:13px; }
            QTableWidget::item { padding:4px; }
            QTableWidget::item:selected { background:%1; color:#1e1e2e; }
        )").arg(accent));

        auto *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(15);
        effect->setOffset(0, 4);
        effect->setColor(QColor(0, 0, 0, 80));
        tbl->setGraphicsEffect(effect);

        tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tbl->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        tbl->verticalHeader()->setDefaultSectionSize(30);
        tbl->setMinimumHeight(100);
        return tbl;
    };

    // Allocation
    m_lblAllocTitle = makeTitle("▣  Allocation Matrix", "#89b4fa");
    m_tblAlloc = makeTable("#89b4fa");
    setTableHeaderStyle(m_tblAlloc, "#89b4fa");
    connect(m_tblAlloc, &QTableWidget::cellChanged,
            this, &MainWindow::onAllocationChanged);

    // Maximum
    m_lblMaxTitle = makeTitle("▣  Maximum Matrix", "#fab387");
    m_tblMax = makeTable("#fab387");
    setTableHeaderStyle(m_tblMax, "#fab387");
    connect(m_tblMax, &QTableWidget::cellChanged,
            this, &MainWindow::onMaximumChanged);

    // Need
    m_lblNeedTitle = makeTitle("▣  Need Matrix  (auto-calculated)", "#cba6f7");
    m_tblNeed = makeTable("#cba6f7");
    setTableHeaderStyle(m_tblNeed, "#cba6f7");
    m_tblNeed->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblNeed->setStyleSheet(m_tblNeed->styleSheet() +
        "QTableWidget { background:#1a1628; }");

    // Available
    m_lblAvailTitle = makeTitle("▣  Available Resources", "#a6e3a1");
    m_tblAvail = makeTable("#a6e3a1");
    setTableHeaderStyle(m_tblAvail, "#a6e3a1");
    m_tblAvail->setFixedHeight(68);

    lay->addWidget(m_lblAllocTitle);
    lay->addWidget(m_tblAlloc, 3);
    lay->addWidget(m_lblMaxTitle);
    lay->addWidget(m_tblMax, 3);

    auto *row2 = new QHBoxLayout;
    auto *needBox = new QVBoxLayout;
    needBox->addWidget(m_lblNeedTitle);
    needBox->addWidget(m_tblNeed);
    auto *availBox = new QVBoxLayout;
    availBox->addWidget(m_lblAvailTitle);
    availBox->addWidget(m_tblAvail);
    row2->addLayout(needBox, 3);
    row2->addLayout(availBox, 2);
    lay->addLayout(row2, 3);

    return w;
}

// ─── Right panel ──────────────────────────────────────────────────────────────

QWidget* MainWindow::buildRightPanel() {
    auto *w = new QWidget;
    w->setMinimumWidth(300);
    w->setStyleSheet("background:#1e1e2e;");
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(10);

    // Visualizer
    auto *vizTitle = new QLabel("◈  Safe Sequence Visualization");
    vizTitle->setStyleSheet("color:#89dceb; font-size:13px; font-weight:700;");
    m_visualizer = new VisualizerWidget;
    m_visualizer->setMinimumHeight(200);

    auto *vizEffect = new QGraphicsDropShadowEffect;
    vizEffect->setBlurRadius(20);
    vizEffect->setOffset(0, 6);
    vizEffect->setColor(QColor(0, 0, 0, 100));
    m_visualizer->setGraphicsEffect(vizEffect);

    m_lblSafeSeq = new QLabel("No sequence computed yet.");
    m_lblSafeSeq->setAlignment(Qt::AlignCenter);
    m_lblSafeSeq->setWordWrap(true);
    m_lblSafeSeq->setStyleSheet(
        "color:#a6adc8; font-size:12px; padding:6px;"
        " background:#181825; border-radius:6px;");

    // Request simulation
    auto *reqBox = new QGroupBox("Resource Request Simulation");
    reqBox->setStyleSheet(
        "QGroupBox { color:#f38ba8; font-weight:700; font-size:12px;"
        " border:1px solid #45475a; border-radius:10px; margin-top:8px; padding-top:8px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 4px; }");
    
    auto *reqEffect = new QGraphicsDropShadowEffect;
    reqEffect->setBlurRadius(15);
    reqEffect->setOffset(0, 4);
    reqEffect->setColor(QColor(0, 0, 0, 80));
    reqBox->setGraphicsEffect(reqEffect);

    auto *reqLay = new QVBoxLayout(reqBox);
    reqLay->setSpacing(6);

    auto *procRow = new QHBoxLayout;
    auto *procLbl = new QLabel("Process:");
    procLbl->setStyleSheet("color:#cdd6f4; font-size:12px;");
    m_cmbProcess = new QComboBox;
    m_cmbProcess->setStyleSheet(
        "QComboBox { background:#313244; color:#cdd6f4; border:1px solid #45475a;"
        " border-radius:6px; padding:4px 8px; }"
        "QComboBox::drop-down { border:none; }"
        "QComboBox QAbstractItemView { background:#313244; color:#cdd6f4; }");
    procRow->addWidget(procLbl);
    procRow->addWidget(m_cmbProcess, 1);

    auto *reqLbl = new QLabel("Request Vector:");
    reqLbl->setStyleSheet("color:#cdd6f4; font-size:12px;");

    m_tblRequest = new QTableWidget(1, 0);
    m_tblRequest->setFixedHeight(56);
    m_tblRequest->setStyleSheet(
        "QTableWidget { background:#1e1e2e; color:#cdd6f4; gridline-color:#313244;"
        " border:1px solid #45475a; border-radius:6px; }"
        "QTableWidget::item:selected { background:#f38ba822; }");
    m_tblRequest->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tblRequest->verticalHeader()->setVisible(false);

    reqLay->addLayout(procRow);
    reqLay->addWidget(reqLbl);
    reqLay->addWidget(m_tblRequest);

    lay->addWidget(vizTitle);
    lay->addWidget(m_visualizer, 3);
    lay->addWidget(m_lblSafeSeq);
    lay->addSpacing(4);
    lay->addWidget(reqBox);
    lay->addStretch();
    return w;
}

// ─── Bottom panel ─────────────────────────────────────────────────────────────

QWidget* MainWindow::buildBottomPanel() {
    auto *w = new QWidget;
    w->setStyleSheet("background:#11111b;");
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(12, 6, 12, 8);
    lay->setSpacing(4);

    auto *hdr = new QHBoxLayout;
    auto *consoleLbl = new QLabel("◈  Execution Console");
    consoleLbl->setStyleSheet("color:#89dceb; font-weight:700; font-size:12px;");
    auto *clearBtn = makeButton("Clear", "#313244", "#45475a");
    clearBtn->setFixedWidth(70);
    clearBtn->setFixedHeight(26);
    connect(clearBtn, &QPushButton::clicked, this, [this]{ m_console->clear(); });
    hdr->addWidget(consoleLbl);
    hdr->addStretch();
    hdr->addWidget(clearBtn);

    m_console = new QTextEdit;
    m_console->setReadOnly(true);
    m_console->setStyleSheet(
        "QTextEdit { background:#11111b; color:#cdd6f4; border:none;"
        " font-family:'Consolas','Courier New',monospace; font-size:12px; }"
        "QScrollBar:vertical { background:#1e1e2e; width:8px; border-radius:4px; }"
        "QScrollBar::handle:vertical { background:#45475a; border-radius:4px; }");

    lay->addLayout(hdr);
    lay->addWidget(m_console);
    return w;
}

// All slot implementations
#include "mainwindow_slots.cpp"
