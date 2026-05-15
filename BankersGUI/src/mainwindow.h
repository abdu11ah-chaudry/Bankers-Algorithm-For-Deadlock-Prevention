#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QSplitter>
#include <QStackedWidget>
#include <vector>
#include "banker.h"
#include "visualizer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    // Buttons
    void onGenerateTables();
    void onReset();
    void onCalculateNeed();
    void onRunAlgorithm();
    void onNextStep();
    void onSimulateRequest();
    void onSaveScenario();
    void onLoadScenario();
    void onToggleTheme();
    void onLoadDemo();

    // Table edits
    void onAllocationChanged(int row, int col);
    void onMaximumChanged(int row, int col);

    // Step timer
    void onStepTimer();

private:
    // Build UI
    void buildUI();
    QWidget* buildLeftPanel();
    QWidget* buildCenterPanel();
    QWidget* buildRightPanel();
    QWidget* buildBottomPanel();
    QPushButton* makeButton(const QString &text, const QString &color,
                            const QString &hoverColor = QString());

    // Table helpers
    void buildMatrixTables(int procs, int res);
    void refreshNeedTable();
    void refreshAvailableRow();
    void highlightRow(QTableWidget *tbl, int row, QColor color);
    void clearHighlights(QTableWidget *tbl);
    void setTableHeaderStyle(QTableWidget *tbl, const QString &accent);
    QTableWidgetItem* makeItem(const QString &text, Qt::Alignment align = Qt::AlignCenter);

    // Logging
    void log(const QString &msg, const QString &color = "#c8d0e0");
    void logSeparator();

    // State
    void readMatricesFromUI();
    void applyDarkTheme();
    void applyLightTheme();
    bool m_darkTheme = true;

    // Algorithm backend
    BankersAlgorithm m_banker;

    // Step execution state
    std::vector<StepInfo>  m_steps;
    std::vector<int>       m_safeSeq;
    int                    m_stepIndex = 0;
    bool                   m_stepping  = false;
    QTimer                *m_stepTimer = nullptr;

    // Widgets – Left panel
    QSpinBox   *m_spnProcesses;
    QSpinBox   *m_spnResources;
    QPushButton *m_btnGenerate;
    QPushButton *m_btnReset;
    QPushButton *m_btnCalcNeed;
    QPushButton *m_btnRun;
    QPushButton *m_btnStep;
    QPushButton *m_btnSimReq;
    QPushButton *m_btnSave;
    QPushButton *m_btnLoad;
    QPushButton *m_btnTheme;
    QPushButton *m_btnDemo;
    QLabel      *m_lblStatus;

    // Widgets – Center panel
    QTableWidget *m_tblAlloc;
    QTableWidget *m_tblMax;
    QTableWidget *m_tblNeed;
    QTableWidget *m_tblAvail;
    QLabel       *m_lblAllocTitle;
    QLabel       *m_lblMaxTitle;
    QLabel       *m_lblNeedTitle;
    QLabel       *m_lblAvailTitle;

    // Widgets – Right panel
    VisualizerWidget *m_visualizer;
    QLabel           *m_lblSafeSeq;

    // Simulate request widgets
    QComboBox   *m_cmbProcess;
    QTableWidget*m_tblRequest;

    // Widgets – Bottom
    QTextEdit   *m_console;

    // Busy guard
    bool m_updatingTables  = false;

    // Demo flash animation counter
    int  m_demoFlashCount  = 0;
};
