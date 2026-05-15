// mainwindow_slots.cpp  — appended via separate TU trick; included by mainwindow.cpp
// This file contains all slot implementations.
// Include it from mainwindow.cpp by adding:  #include "mainwindow_slots.cpp"

#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QHeaderView>
#include <QScrollBar>

// ─── Logging ──────────────────────────────────────────────────────────────────

void MainWindow::log(const QString &msg, const QString &color) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_console->append(
        "<span style='color:#45475a;'>[" + ts + "]</span> "
        "<span style='color:" + color + ";'>" + msg.toHtmlEscaped() + "</span>");
    m_console->verticalScrollBar()->setValue(
        m_console->verticalScrollBar()->maximum());
}

void MainWindow::logSeparator() {
    m_console->append("<hr style='border:1px solid #313244; margin:4px 0;'/>");
}

// ─── Table helpers ────────────────────────────────────────────────────────────

void MainWindow::highlightRow(QTableWidget *tbl, int row, QColor color) {
    for (int j = 0; j < tbl->columnCount(); ++j) {
        auto *it = tbl->item(row, j);
        if (it) it->setBackground(color);
    }
}

void MainWindow::clearHighlights(QTableWidget *tbl) {
    for (int i = 0; i < tbl->rowCount(); ++i)
        for (int j = 0; j < tbl->columnCount(); ++j) {
            auto *it = tbl->item(i, j);
            if (it) it->setBackground(QColor("#1e1e2e"));
        }
}

// ─── Build / Generate Tables ─────────────────────────────────────────────────

void MainWindow::buildMatrixTables(int procs, int res) {
    m_updatingTables = true;

    // Column headers: R0, R1, …
    QStringList colHdrs;
    for (int j = 0; j < res; ++j) colHdrs << QString("R%1").arg(j);

    // Row headers: P0, P1, …
    QStringList rowHdrs;
    for (int i = 0; i < procs; ++i) rowHdrs << QString("P%1").arg(i);

    auto initTable = [&](QTableWidget *tbl, bool editable) {
        tbl->setRowCount(procs);
        tbl->setColumnCount(res);
        tbl->setHorizontalHeaderLabels(colHdrs);
        tbl->setVerticalHeaderLabels(rowHdrs);
        for (int i = 0; i < procs; ++i)
            for (int j = 0; j < res; ++j) {
                auto *it = new QTableWidgetItem("0");
                it->setTextAlignment(Qt::AlignCenter);
                if (!editable) it->setFlags(it->flags() & ~Qt::ItemIsEditable);
                tbl->setItem(i, j, it);
            }
    };

    initTable(m_tblAlloc, true);
    initTable(m_tblMax,   true);
    initTable(m_tblNeed,  false);

    // Available: single row
    m_tblAvail->setRowCount(1);
    m_tblAvail->setColumnCount(res);
    m_tblAvail->setHorizontalHeaderLabels(colHdrs);
    m_tblAvail->setVerticalHeaderLabels({"Avail"});
    for (int j = 0; j < res; ++j)
        m_tblAvail->setItem(0, j, new QTableWidgetItem("0"));

    // Request table columns
    m_tblRequest->setColumnCount(res);
    m_tblRequest->setHorizontalHeaderLabels(colHdrs);
    m_tblRequest->setRowCount(1);
    for (int j = 0; j < res; ++j)
        m_tblRequest->setItem(0, j, new QTableWidgetItem("0"));

    // Process combo
    m_cmbProcess->clear();
    for (int i = 0; i < procs; ++i)
        m_cmbProcess->addItem(QString("P%1").arg(i));

    m_updatingTables = false;
}

void MainWindow::refreshNeedTable() {
    m_updatingTables = true;
    int procs = m_banker.processCount();
    int res   = m_banker.resourceCount();
    for (int i = 0; i < procs; ++i)
        for (int j = 0; j < res; ++j) {
            int val = m_banker.getNeed(i, j);
            auto *it = m_tblNeed->item(i, j);
            if (it) {
                it->setText(QString::number(val));
                it->setForeground(val < 0 ? QColor("#f38ba8") : QColor("#cba6f7"));
            }
        }
    m_updatingTables = false;
}

void MainWindow::refreshAvailableRow() {
    m_updatingTables = true;
    int res = m_banker.resourceCount();
    for (int j = 0; j < res; ++j) {
        auto *it = m_tblAvail->item(0, j);
        if (it) it->setText(QString::number(m_banker.getAvailable(j)));
    }
    m_updatingTables = false;
}

void MainWindow::readMatricesFromUI() {
    int procs = m_tblAlloc->rowCount();
    int res   = m_tblAlloc->columnCount();
    m_banker.setProcessCount(procs);
    m_banker.setResourceCount(res);

    for (int i = 0; i < procs; ++i)
        for (int j = 0; j < res; ++j) {
            auto a = m_tblAlloc->item(i, j);
            auto x = m_tblMax->item(i, j);
            m_banker.setAllocation(i, j, a ? a->text().toInt() : 0);
            m_banker.setMaximum   (i, j, x ? x->text().toInt() : 0);
        }
    for (int j = 0; j < res; ++j) {
        auto av = m_tblAvail->item(0, j);
        m_banker.setAvailable(j, av ? av->text().toInt() : 0);
    }
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void MainWindow::onLoadDemo() {
    // ── 1. Pick a random demo scenario ────────────────────────────────────────
    int idx = m_banker.loadRandomDemo();
    const DemoScenario &sc = m_banker.demoScenarios()[idx];

    int procs = m_banker.processCount();
    int res   = m_banker.resourceCount();

    // ── 2. Resize spinboxes and rebuild tables ─────────────────────────────────
    m_spnProcesses->setValue(procs);
    m_spnResources->setValue(res);
    buildMatrixTables(procs, res);

    // ── 3. Generate generic column headers R0..Rn ─────────────────────────────
    m_updatingTables = true;
    QStringList colHdrs;
    for (int j = 0; j < res; ++j) colHdrs << QString("R%1").arg(j);
    m_tblAlloc->setHorizontalHeaderLabels(colHdrs);
    m_tblMax->setHorizontalHeaderLabels(colHdrs);
    m_tblNeed->setHorizontalHeaderLabels(colHdrs);
    m_tblAvail->setHorizontalHeaderLabels(colHdrs);
    m_tblRequest->setHorizontalHeaderLabels(colHdrs);

    // ── 4. Populate all matrix cells ──────────────────────────────────────────
    for (int i = 0; i < procs; ++i)
        for (int j = 0; j < res; ++j) {
            auto setCell = [](QTableWidgetItem *it, int val, const QColor &bg) {
                if (!it) return;
                it->setText(QString::number(val));
                it->setBackground(bg);
                it->setForeground(QColor("#ffffff"));
            };
            setCell(m_tblAlloc->item(i,j), m_banker.getAllocation(i,j), QColor("#1a2a4a"));
            setCell(m_tblMax->item(i,j),   m_banker.getMaximum(i,j),   QColor("#2a1a3a"));
            setCell(m_tblNeed->item(i,j),  m_banker.getNeed(i,j),      QColor("#1a1a3a"));
        }
    for (int j = 0; j < res; ++j) {
        auto *it = m_tblAvail->item(0, j);
        if (it) {
            it->setText(QString::number(m_banker.getAvailable(j)));
            it->setBackground(QColor("#0d2a1a"));
            it->setForeground(QColor("#a6e3a1"));
        }
    }
    m_updatingTables = false;

    // ── 5. Animate: flash newly loaded rows with a highlight, then fade ────────
    //    Use a single-shot sequence driven by QTimer lambdas (3 pulses).
    auto flashTables = [this, procs, res]() {
        const QColor flashOn("#29324a");
        const QColor flashOff("#1e1e2e");

        auto applyFlash = [this, procs, res](const QColor &c) {
            for (int i = 0; i < procs; ++i) {
                for (int j = 0; j < res; ++j) {
                    if (auto *it = m_tblAlloc->item(i,j)) it->setBackground(c);
                    if (auto *it = m_tblMax->item(i,j))   it->setBackground(c);
                    if (auto *it = m_tblNeed->item(i,j))  it->setBackground(c);
                }
            }
        };

        applyFlash(flashOn);
        QTimer::singleShot(300, this, [applyFlash, flashOff]() { applyFlash(flashOff); });
        QTimer::singleShot(600, this, [applyFlash, flashOn]()  { applyFlash(flashOn);  });
        QTimer::singleShot(900, this, [applyFlash, flashOff]() { applyFlash(flashOff); });
    };
    flashTables();

    // ── 6. Clear previous state ────────────────────────────────────────────────
    m_visualizer->clearAll();
    m_stepping = false;
    m_stepTimer->stop();
    m_steps.clear();
    m_safeSeq.clear();
    m_stepIndex = 0;
    m_console->clear();

    // ── 7. Run safety algorithm automatically ──────────────────────────────────
    std::vector<StepInfo> steps;
    std::vector<int>      safeSeq;
    bool safe = m_banker.runSafetyAlgorithm(safeSeq, steps);

    // ── 8. Console: scenario header ───────────────────────────────────────────
    log("┌─────────────────────────────────────────────────────────┐", "#45475a");
    log("│  🎲  RANDOM DEMO SCENARIO LOADED                        │", "#89b4fa");
    log("│  " + QString::fromStdString(sc.name).leftJustified(55) + "│", "#cba6f7");
    log("└─────────────────────────────────────────────────────────┘", "#45475a");
    log("📋 " + QString::fromStdString(sc.description), "#a6adc8");
    logSeparator();

    // ── 9. Log Available vector ───────────────────────────────────────────────
    QString availStr = "📦 Available: [";
    for (int j = 0; j < res; ++j)
        availStr += (j ? ", " : "") + QString("R%1=%2").arg(j).arg(m_banker.getAvailable(j));
    availStr += "]";
    log(availStr, "#a6e3a1");

    // ── 10. Log Need matrix ───────────────────────────────────────────────────
    log("📊 Need Matrix (Max − Allocation):", "#cba6f7");
    for (int i = 0; i < procs; ++i) {
        QString row = QString("   P%1 → [").arg(i);
        for (int j = 0; j < res; ++j)
            row += (j ? ", " : "") + QString::number(m_banker.getNeed(i,j));
        row += "]";
        log(row, "#cdd6f4");
    }
    logSeparator();

    // ── 11. Log safety check trace ────────────────────────────────────────────
    log("🔍 Running Safety Algorithm...", "#89dceb");
    for (auto &s : steps) {
        QString col = s.canExecute ? "#a6e3a1" : "#f38ba8";
        log("  " + QString::fromStdString(s.message), col);
    }
    logSeparator();

    // ── 12. Visual state feedback ─────────────────────────────────────────────
    if (safe) {
        // ── SAFE banner ───────────────────────────────────────────────────────
        QString seq;
        for (int i = 0; i < (int)safeSeq.size(); ++i)
            seq += (i ? " → " : "") + QString("P%1").arg(safeSeq[i]);

        m_lblSafeSeq->setText("✔  SAFE STATE  |  Sequence: " + seq);
        m_lblSafeSeq->setStyleSheet(
            "color:#a6e3a1; font-size:12px; font-weight:700; padding:8px;"
            " background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "  stop:0 #0d2a1a, stop:1 #1a3a1a);"
            " border-radius:8px; border:2px solid #2d6a3a;"
            " letter-spacing:0.5px;");

        m_lblStatus->setText("✔ SAFE STATE");
        m_lblStatus->setStyleSheet(
            "color:#a6e3a1; font-size:11px; font-weight:700; padding:6px;"
            " background:#0d2a1a; border-radius:6px; border:1px solid #2d6a3a;");

        // Green-highlight rows in the order they execute
        clearHighlights(m_tblAlloc);
        clearHighlights(m_tblNeed);
        for (int pid : safeSeq) {
            highlightRow(m_tblAlloc, pid, QColor("#1a3a1a"));
            highlightRow(m_tblNeed,  pid, QColor("#1a3a1a"));
        }

        // Feed visualizer
        m_visualizer->setSafeSequence(safeSeq, procs);
        m_visualizer->showSafe();
        for (int i = 0; i < (int)safeSeq.size(); ++i)
            m_visualizer->animateStep(i + 1);

        // Log result
        log("✅ SAFE STATE — Safe sequence found:", "#a6e3a1");
        log("   " + seq, "#a6e3a1");
        log("   All processes can complete without deadlock.", "#a6adc8");

        // Store for step execution
        m_safeSeq = safeSeq;
        m_steps   = steps;

    } else {
        // ── UNSAFE banner ─────────────────────────────────────────────────────
        m_lblSafeSeq->setText("⚠  UNSAFE STATE  —  No safe sequence exists!");
        m_lblSafeSeq->setStyleSheet(
            "color:#f38ba8; font-size:12px; font-weight:700; padding:8px;"
            " background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "  stop:0 #2d0d0d, stop:1 #3a1a1a);"
            " border-radius:8px; border:2px solid #6a2d2d;"
            " letter-spacing:0.5px;");

        m_lblStatus->setText("⚠ UNSAFE STATE");
        m_lblStatus->setStyleSheet(
            "color:#f38ba8; font-size:11px; font-weight:700; padding:6px;"
            " background:#2d0d0d; border-radius:6px; border:1px solid #6a2d2d;");

        // Find and highlight blocked processes in red, completed ones in yellow
        std::vector<int> blocked;
        for (int i = 0; i < procs; ++i) {
            bool inSeq = false;
            for (int s : safeSeq) if (s == i) { inSeq = true; break; }
            if (!inSeq) blocked.push_back(i);
        }

        clearHighlights(m_tblAlloc);
        clearHighlights(m_tblNeed);
        // Partially-executed: yellow
        for (int pid : safeSeq) {
            highlightRow(m_tblAlloc, pid, QColor("#2a2a0a"));
            highlightRow(m_tblNeed,  pid, QColor("#2a2a0a"));
        }
        // Blocked: red
        for (int b : blocked) {
            highlightRow(m_tblAlloc, b, QColor("#3a0d0d"));
            highlightRow(m_tblNeed,  b, QColor("#3a0d0d"));
        }

        // Feed visualizer
        m_visualizer->setBlockedProcesses(blocked, procs);
        m_visualizer->showUnsafe();

        // Log result
        log("🚫 UNSAFE STATE — No safe sequence exists!", "#f38ba8");
        if (!safeSeq.empty()) {
            QString partial;
            for (int i = 0; i < (int)safeSeq.size(); ++i)
                partial += (i ? " → " : "") + QString("P%1").arg(safeSeq[i]);
            log("   Partial sequence before deadlock: " + partial, "#f9e2af");
        }
        QString blkStr = "   🔴 Blocked processes: ";
        for (int i = 0; i < (int)blocked.size(); ++i)
            blkStr += (i ? ", " : "") + QString("P%1").arg(blocked[i]);
        log(blkStr, "#f38ba8");
        log("   These processes cannot obtain all needed resources.", "#a6adc8");
        log("   ⚠ Deadlock risk — system cannot guarantee completion.", "#f38ba8");
    }

    logSeparator();
    log(QString("🎲 Scenario %1/%2 — Click 📂 Load Demo again for a different example.")
            .arg(idx + 1).arg(m_banker.demoScenarios().size()), "#6c7086");
    logSeparator();
}

void MainWindow::onGenerateTables() {
    int procs = m_spnProcesses->value();
    int res   = m_spnResources->value();
    m_banker.setProcessCount(procs);
    m_banker.setResourceCount(res);
    buildMatrixTables(procs, res);
    m_visualizer->clearAll();
    m_lblSafeSeq->setText("Tables generated. Fill in values and run.");
    m_lblStatus->setText("Tables ready");
    log(QString("⚡ Generated tables for %1 processes, %2 resource types").arg(procs).arg(res), "#89b4fa");
    logSeparator();
}

void MainWindow::onReset() {
    m_banker.reset();
    m_tblAlloc->setRowCount(0); m_tblAlloc->setColumnCount(0);
    m_tblMax->setRowCount(0);   m_tblMax->setColumnCount(0);
    m_tblNeed->setRowCount(0);  m_tblNeed->setColumnCount(0);
    m_tblAvail->setRowCount(0); m_tblAvail->setColumnCount(0);
    m_tblRequest->setColumnCount(0);
    m_cmbProcess->clear();
    m_visualizer->clearAll();
    m_lblSafeSeq->setText("No sequence computed yet.");
    m_lblStatus->setText("Reset");
    m_stepping = false; m_stepTimer->stop();
    log("↺  System reset.", "#f38ba8");
    logSeparator();
}

void MainWindow::onCalculateNeed() {
    if (m_tblAlloc->rowCount() == 0) {
        QMessageBox::warning(this, "No Tables", "Generate tables first."); return;
    }
    readMatricesFromUI();
    std::string err;
    if (!m_banker.validateState(err)) {
        log("❌ Validation error: " + QString::fromStdString(err), "#f38ba8");
        QMessageBox::warning(this, "Validation Error", QString::fromStdString(err));
        return;
    }
    m_banker.computeNeed();
    refreshNeedTable();
    log("∑  Need matrix computed (Need = Maximum − Allocation)", "#cba6f7");
    m_lblStatus->setText("Need calculated");
}

void MainWindow::onAllocationChanged(int row, int col) {
    if (m_updatingTables) return;
    auto *it = m_tblAlloc->item(row, col);
    if (!it) return;
    int val = it->text().toInt();
    m_banker.setAllocation(row, col, val);
    m_banker.setProcessCount(m_tblAlloc->rowCount());
    m_banker.setResourceCount(m_tblAlloc->columnCount());
    // auto-update need if max is set
    int mx = m_tblMax->item(row, col) ? m_tblMax->item(row, col)->text().toInt() : 0;
    m_banker.setMaximum(row, col, mx);
    m_banker.computeNeed();
    refreshNeedTable();
}

void MainWindow::onMaximumChanged(int row, int col) {
    if (m_updatingTables) return;
    auto *it = m_tblMax->item(row, col);
    if (!it) return;
    int val = it->text().toInt();
    m_banker.setMaximum(row, col, val);
    m_banker.setProcessCount(m_tblAlloc->rowCount());
    m_banker.setResourceCount(m_tblAlloc->columnCount());
    int al = m_tblAlloc->item(row, col) ? m_tblAlloc->item(row, col)->text().toInt() : 0;
    m_banker.setAllocation(row, col, al);
    m_banker.computeNeed();
    refreshNeedTable();
}

void MainWindow::onRunAlgorithm() {
    if (m_tblAlloc->rowCount() == 0) {
        QMessageBox::warning(this, "No Data", "Generate tables and enter data first."); return;
    }
    m_stepTimer->stop();
    m_stepping = false;

    readMatricesFromUI();
    std::string err;
    if (!m_banker.validateState(err)) {
        log("❌ " + QString::fromStdString(err), "#f38ba8");
        QMessageBox::warning(this, "Validation Error", QString::fromStdString(err));
        return;
    }
    m_banker.computeNeed();
    refreshNeedTable();

    m_steps.clear(); m_safeSeq.clear();
    bool safe = m_banker.runSafetyAlgorithm(m_safeSeq, m_steps);

    log("", "#cdd6f4");
    log("══════════════ Running Safety Algorithm ══════════════", "#89dceb");

    for (auto &s : m_steps) {
        QString col = s.canExecute ? "#a6e3a1" : "#f38ba8";
        log("  " + QString::fromStdString(s.message), col);
        if (s.canExecute) highlightRow(m_tblAlloc, s.processId, QColor("#1a3a1a"));
        else              highlightRow(m_tblAlloc, s.processId, QColor("#3a1a1a"));
    }

    if (safe) {
        QString seq;
        for (int i = 0; i < (int)m_safeSeq.size(); ++i)
            seq += (i ? " → " : "") + QString("P%1").arg(m_safeSeq[i]);
        log("", "#cdd6f4");
        log("✔  SAFE STATE — Safe Sequence: " + seq, "#a6e3a1");
        m_lblSafeSeq->setText("Safe Sequence: " + seq);
        m_lblSafeSeq->setStyleSheet(
            "color:#a6e3a1; font-size:12px; font-weight:700; padding:6px;"
            " background:#1e2d1e; border-radius:6px; border:1px solid #2d4a2d;");
        m_visualizer->setSafeSequence(m_safeSeq, m_banker.processCount());
        m_visualizer->showSafe();
        // animate all steps
        for (int i = 0; i < (int)m_safeSeq.size(); ++i)
            m_visualizer->animateStep(i+1);
        m_lblStatus->setText("SAFE ✔");
    } else {
        log("", "#cdd6f4");
        log("⚠  UNSAFE STATE — No safe sequence exists! Deadlock risk!", "#f38ba8");
        m_lblSafeSeq->setText("⚠ UNSAFE STATE DETECTED");
        m_lblSafeSeq->setStyleSheet(
            "color:#f38ba8; font-size:12px; font-weight:700; padding:6px;"
            " background:#2d1e1e; border-radius:6px; border:1px solid #4a2d2d;");

        // find blocked processes
        std::vector<int> blocked;
        for (int i = 0; i < m_banker.processCount(); ++i) {
            bool inSeq = false;
            for (int s : m_safeSeq) if (s == i) { inSeq = true; break; }
            if (!inSeq) blocked.push_back(i);
        }
        m_visualizer->setBlockedProcesses(blocked, m_banker.processCount());
        m_visualizer->showUnsafe();
        for (int b : blocked) highlightRow(m_tblAlloc, b, QColor("#3a1a1a"));
        m_lblStatus->setText("UNSAFE ⚠");
    }
    logSeparator();
    m_stepIndex = 0;
}

void MainWindow::onNextStep() {
    if (m_tblAlloc->rowCount() == 0) {
        QMessageBox::warning(this, "No Data", "Generate tables first."); return;
    }
    if (!m_stepping) {
        // fresh start
        readMatricesFromUI();
        std::string err;
        if (!m_banker.validateState(err)) {
            log("❌ " + QString::fromStdString(err), "#f38ba8");
            return;
        }
        m_banker.computeNeed();
        refreshNeedTable();
        m_steps.clear(); m_safeSeq.clear();
        m_banker.runSafetyAlgorithm(m_safeSeq, m_steps);
        m_stepIndex = 0;
        m_stepping  = true;
        clearHighlights(m_tblAlloc);
        m_visualizer->setSafeSequence(m_safeSeq, m_banker.processCount());
        log("⏭  Step-by-step execution started", "#89dceb");
        logSeparator();
        m_stepTimer->start();
        return;
    }
    if (m_stepIndex >= (int)m_steps.size()) {
        m_stepTimer->stop();
        m_stepping = false;
        log("⏹  Step execution complete.", "#89dceb");
        logSeparator();
        return;
    }
    onStepTimer();
}

void MainWindow::onStepTimer() {
    if (m_stepIndex >= (int)m_steps.size()) {
        m_stepTimer->stop();
        m_stepping = false;
        log("⏹  All steps done.", "#89dceb");
        logSeparator();
        return;
    }
    const StepInfo &s = m_steps[m_stepIndex];
    QString col = s.canExecute ? "#a6e3a1" : "#f38ba8";
    log(QString("Step %1: ").arg(m_stepIndex+1) + QString::fromStdString(s.message), col);

    clearHighlights(m_tblAlloc);
    clearHighlights(m_tblNeed);
    if (s.canExecute) {
        highlightRow(m_tblAlloc, s.processId, QColor("#1a3a1a"));
        highlightRow(m_tblNeed,  s.processId, QColor("#1a3a1a"));
    } else {
        highlightRow(m_tblAlloc, s.processId, QColor("#3a1a1a"));
        highlightRow(m_tblNeed,  s.processId, QColor("#3a1a1a"));
    }

    // count how many done
    int done = 0;
    for (int k = 0; k <= m_stepIndex; ++k) if (m_steps[k].canExecute) ++done;
    m_visualizer->animateStep(done);

    // Update available display after execution
    if (s.canExecute) {
        QString work = "Work after: [";
        for (int j = 0; j < (int)s.workAfter.size(); ++j)
            work += (j ? ", " : "") + QString::number(s.workAfter[j]);
        work += "]";
        log("  Available → " + work, "#a6e3a1");
    }
    ++m_stepIndex;
}

void MainWindow::onSimulateRequest() {
    if (m_tblAlloc->rowCount() == 0) {
        QMessageBox::warning(this, "No Data", "Generate tables first."); return;
    }
    readMatricesFromUI();
    m_banker.computeNeed();

    int pid = m_cmbProcess->currentIndex();
    int res = m_banker.resourceCount();
    std::vector<int> request(res);
    for (int j = 0; j < res; ++j) {
        auto *it = m_tblRequest->item(0, j);
        request[j] = it ? it->text().toInt() : 0;
    }

    QString reqStr = "[";
    for (int j = 0; j < res; ++j) reqStr += (j?", ":"") + QString::number(request[j]);
    reqStr += "]";

    log("", "#cdd6f4");
    log(QString("📨 Simulating request by P%1: %2").arg(pid).arg(reqStr), "#fab387");

    m_banker.saveSnapshot();
    RequestResult result = m_banker.processRequest(pid, request);

    QString col = result.granted ? "#a6e3a1"
                : result.wouldWait ? "#f9e2af"
                : "#f38ba8";
    log("   " + QString::fromStdString(result.reason), col);

    if (result.granted) {
        QString seq;
        for (int i = 0; i < (int)result.safeSeq.size(); ++i)
            seq += (i ? " → " : "") + QString("P%1").arg(result.safeSeq[i]);
        log("   New safe sequence: " + seq, "#a6e3a1");
        // Update tables from banker state
        m_updatingTables = true;
        for (int i = 0; i < m_banker.processCount(); ++i)
            for (int j = 0; j < res; ++j)
                if (m_tblAlloc->item(i,j))
                    m_tblAlloc->item(i,j)->setText(
                        QString::number(m_banker.getAllocation(i,j)));
        m_updatingTables = false;
        refreshNeedTable();
        refreshAvailableRow();
        m_visualizer->setSafeSequence(result.safeSeq, m_banker.processCount());
        m_visualizer->showSafe();
    } else if (!result.wouldWait) {
        // rolled back — restore snapshot just to be safe
        m_banker.restoreSnapshot();
    }
    logSeparator();
}

void MainWindow::onSaveScenario() {
    if (m_banker.processCount() == 0) {
        QMessageBox::warning(this, "No Data", "Nothing to save."); return;
    }
    readMatricesFromUI();
    m_banker.computeNeed();
    QString path = QFileDialog::getSaveFileName(
        this, "Save Scenario", QDir::homePath(), "Banker Scenario (*.bkr);;All Files (*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".bkr")) path += ".bkr";
    if (m_banker.saveToFile(path.toStdString()))
        log("💾 Scenario saved to: " + path, "#a6e3a1");
    else
        log("❌ Failed to save scenario.", "#f38ba8");
}

void MainWindow::onLoadScenario() {
    QString path = QFileDialog::getOpenFileName(
        this, "Load Scenario", QDir::homePath(), "Banker Scenario (*.bkr);;All Files (*)");
    if (path.isEmpty()) return;
    std::string err;
    if (!m_banker.loadFromFile(path.toStdString(), err)) {
        log("❌ Load failed: " + QString::fromStdString(err), "#f38ba8");
        QMessageBox::warning(this, "Load Error", QString::fromStdString(err));
        return;
    }
    int procs = m_banker.processCount(), res = m_banker.resourceCount();
    m_spnProcesses->setValue(procs);
    m_spnResources->setValue(res);
    buildMatrixTables(procs, res);
    m_updatingTables = true;
    for (int i = 0; i < procs; ++i)
        for (int j = 0; j < res; ++j) {
            m_tblAlloc->item(i,j)->setText(QString::number(m_banker.getAllocation(i,j)));
            m_tblMax->item(i,j)->setText(QString::number(m_banker.getMaximum(i,j)));
            m_tblNeed->item(i,j)->setText(QString::number(m_banker.getNeed(i,j)));
        }
    for (int j = 0; j < res; ++j)
        m_tblAvail->item(0,j)->setText(QString::number(m_banker.getAvailable(j)));
    m_updatingTables = false;
    m_visualizer->clearAll();
    log("📁 Scenario loaded from: " + path, "#89b4fa");
    logSeparator();
}

void MainWindow::onToggleTheme() {
    m_darkTheme = !m_darkTheme;
    if (m_darkTheme) { applyDarkTheme(); m_btnTheme->setText("☀ Light"); }
    else             { applyLightTheme(); m_btnTheme->setText("🌙 Dark"); }
}

void MainWindow::applyDarkTheme() {
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget { background:#1e1e2e; color:#cdd6f4;
            font-family:'Segoe UI','Noto Sans',sans-serif; font-size:12px; }
        QToolTip { background:#313244; color:#cdd6f4; border:1px solid #45475a;
            border-radius:4px; padding:4px; }
        QScrollBar:vertical { background:#1e1e2e; width:8px; border-radius:4px; }
        QScrollBar::handle:vertical { background:#45475a; border-radius:4px; min-height:20px; }
        QScrollBar:horizontal { background:#1e1e2e; height:8px; border-radius:4px; }
        QScrollBar::handle:horizontal { background:#45475a; border-radius:4px; }
    )");
}

void MainWindow::applyLightTheme() {
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget { background:#eff1f5; color:#4c4f69;
            font-family:'Segoe UI','Noto Sans',sans-serif; font-size:12px; }
        QGroupBox { color:#7287fd; border:1px solid #bcc0cc; border-radius:8px; }
        QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 4px; }
        QTableWidget { background:#ffffff; color:#4c4f69; gridline-color:#bcc0cc; }
        QToolTip { background:#ffffff; color:#4c4f69; border:1px solid #bcc0cc; padding:4px; }
        QTextEdit { background:#ffffff; color:#4c4f69; }
        QScrollBar:vertical { background:#eff1f5; width:8px; border-radius:4px; }
        QScrollBar::handle:vertical { background:#bcc0cc; border-radius:4px; }
    )");
}

void MainWindow::closeEvent(QCloseEvent *e) {
    m_stepTimer->stop();
    QMainWindow::closeEvent(e);
}
