#include "visualizer.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QResizeEvent>
#include <cmath>

VisualizerWidget::VisualizerWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: transparent;");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VisualizerWidget::onPulse);
    m_timer->start(40);
}

void VisualizerWidget::setSafeSequence(const std::vector<int> &seq, int total) {
    m_safeSeq    = seq;
    m_blocked.clear();
    m_total      = total;
    m_currentStep = -1;
    m_unsafe     = false;
    m_safe       = false;
    layoutNodes();
    update();
}

void VisualizerWidget::setBlockedProcesses(const std::vector<int> &blocked, int total) {
    m_blocked    = blocked;
    m_safeSeq.clear();
    m_total      = total;
    m_currentStep = -1;
    layoutNodes();
    update();
}

void VisualizerWidget::clearAll() {
    m_safeSeq.clear();
    m_blocked.clear();
    m_total      = 0;
    m_currentStep = -1;
    m_unsafe     = false;
    m_safe       = false;
    m_nodes.clear();
    update();
}

void VisualizerWidget::animateStep(int step) {
    m_currentStep = step;
    update();
}

void VisualizerWidget::showUnsafe() {
    m_unsafe = true;
    m_safe   = false;
    update();
}

void VisualizerWidget::showSafe() {
    m_safe   = true;
    m_unsafe = false;
    update();
}

void VisualizerWidget::onPulse() {
    m_pulse += 0.05 * m_pulseDir;
    if (m_pulse >= 1.0) { m_pulse = 1.0; m_pulseDir = -1; }
    if (m_pulse <= 0.0) { m_pulse = 0.0; m_pulseDir =  1; }
    update();
}

void VisualizerWidget::layoutNodes() {
    m_nodes.clear();
    if (m_total <= 0) return;

    int cols = m_total;
    int w    = width();
    int h    = height();
    int padX = 20, padY = 20;
    int spacing = (cols > 1) ? (w - 2*padX - NODE_W) / (cols - 1) : 0;

    for (int idx = 0; idx < (int)m_safeSeq.size(); ++idx) {
        int pid = m_safeSeq[idx];
        ProcessNode nd;
        nd.id    = pid;
        nd.label = QString("P%1").arg(pid);
        nd.state = 0;
        nd.alpha = 1.0;
        qreal x  = padX + idx * spacing;
        qreal y  = h/2 - NODE_H/2;
        nd.rect  = QRectF(x, y, NODE_W, NODE_H);
        m_nodes.push_back(nd);
    }
    // blocked
    if (!m_blocked.empty() && m_safeSeq.empty()) {
        int allPids = m_total;
        spacing = (allPids > 1) ? (w - 2*padX - NODE_W) / (allPids - 1) : 0;
        for (int i = 0; i < allPids; ++i) {
            ProcessNode nd;
            nd.id    = i;
            nd.label = QString("P%1").arg(i);
            bool isBlk = false;
            for (int b : m_blocked) if (b == i) { isBlk = true; break; }
            nd.state = isBlk ? 3 : 2;
            nd.alpha = 1.0;
            qreal x  = padX + i * spacing;
            qreal y  = h/2 - NODE_H/2;
            nd.rect  = QRectF(x, y, NODE_W, NODE_H);
            m_nodes.push_back(nd);
        }
    }
    (void)padY;
}

void VisualizerWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    layoutNodes();
}

void VisualizerWidget::drawArrow(QPainter &p, QPointF from, QPointF to, QColor color) {
    p.setPen(QPen(color, 2.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(from, to);

    // Arrowhead
    QPointF dir = to - from;
    double len  = std::sqrt(dir.x()*dir.x() + dir.y()*dir.y());
    if (len < 1) return;
    dir /= len;
    QPointF perp(-dir.y(), dir.x());
    double arrowLen = 10.0, arrowW = 5.0;
    QPointF tip  = to;
    QPointF left = tip - dir*arrowLen + perp*arrowW;
    QPointF right= tip - dir*arrowLen - perp*arrowW;
    QPolygonF head;
    head << tip << left << right;
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawPolygon(head);
}

void VisualizerWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int W = width(), H = height();

    // Background gradient
    QLinearGradient bg(0,0,0,H);
    bg.setColorAt(0, QColor(18,18,32));
    bg.setColorAt(1, QColor(12,12,22));
    p.fillRect(rect(), bg);

    if (m_nodes.empty()) {
        p.setPen(QColor(100,110,140));
        p.setFont(QFont("Segoe UI", 12));
        p.drawText(rect(), Qt::AlignCenter,
                   "Run the algorithm to see\nthe safe sequence here");
        return;
    }

    // Safety/Unsafe banner
    if (m_safe) {
        qreal glow = 0.5 + 0.5*m_pulse;
        QColor bannerCol(30, int(160*glow), 80, 60);
        p.fillRect(0, 0, W, 32, bannerCol);
        p.setPen(QColor(80, 255, 130));
        QFont f("Segoe UI", 10, QFont::Bold);
        p.setFont(f);
        p.drawText(QRect(0,0,W,32), Qt::AlignCenter, "✔  SAFE STATE — Safe Sequence Found");
    } else if (m_unsafe) {
        qreal glow = 0.5 + 0.5*m_pulse;
        QColor bannerCol(int(160*glow), 30, 40, 80);
        p.fillRect(0, 0, W, 32, bannerCol);
        p.setPen(QColor(255, 80, 90));
        QFont f("Segoe UI", 10, QFont::Bold);
        p.setFont(f);
        p.drawText(QRect(0,0,W,32), Qt::AlignCenter, "⚠  UNSAFE STATE DETECTED — Deadlock Risk!");
    }

    // Draw arrows between consecutive nodes (only for safe seq)
    if (!m_safeSeq.empty() && m_nodes.size() > 1) {
        for (int i = 0; i+1 < (int)m_nodes.size(); ++i) {
            QPointF from = m_nodes[i].rect.center()   + QPointF(NODE_W/2, 0);
            QPointF to   = m_nodes[i+1].rect.center() - QPointF(NODE_W/2, 0);
            bool active  = (i < m_currentStep);
            QColor arrowCol = active ? QColor(80,220,120) : QColor(80,90,120);
            drawArrow(p, from, to, arrowCol);
        }
    }

    // Draw nodes
    for (int idx = 0; idx < (int)m_nodes.size(); ++idx) {
        const ProcessNode &nd = m_nodes[idx];
        QRectF r = nd.rect;

        // Determine state
        int state = nd.state;
        if (!m_safeSeq.empty()) {
            if (idx <  m_currentStep) state = 2;       // done
            else if (idx == m_currentStep) state = 1;   // running
            else state = 0;                             // idle
        }

        // Glow for running/done
        QColor baseCol, glowCol, textCol;
        switch(state){
            case 1: // running — teal/cyan
                baseCol = QColor(0, 180, 200);
                glowCol = QColor(0, int(200+55*m_pulse), int(220+35*m_pulse), 180);
                textCol = Qt::white;
                break;
            case 2: // done — green
                baseCol = QColor(40, 180, 90);
                glowCol = QColor(60, 200, 110, 120);
                textCol = Qt::white;
                break;
            case 3: // blocked — red
                baseCol = QColor(200, 50, 60);
                glowCol = QColor(int(220+35*m_pulse), 60, 70, 160);
                textCol = Qt::white;
                break;
            default: // idle — dark blue-grey
                baseCol = QColor(45, 55, 90);
                glowCol = QColor(60, 75, 120, 80);
                textCol = QColor(160,175,210);
        }

        // Glow circle
        if (state == 1 || state == 3) {
            qreal glowR = (NODE_W/2 + 12 + 6*m_pulse);
            QRadialGradient rg(r.center(), glowR);
            rg.setColorAt(0, glowCol);
            rg.setColorAt(1, Qt::transparent);
            p.setBrush(rg);
            p.setPen(Qt::NoPen);
            p.drawEllipse(r.center(), glowR, glowR);
        }

        // Node box with rounded corners
        QLinearGradient grad(r.topLeft(), r.bottomRight());
        grad.setColorAt(0, baseCol.lighter(140));
        grad.setColorAt(1, baseCol);
        p.setBrush(grad);

        QColor borderCol = (state == 1) ? QColor(0,230,255)
                         : (state == 2) ? QColor(80,255,140)
                         : (state == 3) ? QColor(255,80,100)
                         : QColor(80,95,140);
        p.setPen(QPen(borderCol, 2));
        p.drawRoundedRect(r, 12, 12);

        // Step badge for safe sequence
        if (!m_safeSeq.empty() && state != 0) {
            QRectF badge(r.right()-18, r.top()-10, 20, 20);
            p.setBrush(QColor(30,30,50));
            p.setPen(QPen(borderCol, 1.5));
            p.drawEllipse(badge);
            p.setPen(borderCol);
            p.setFont(QFont("Segoe UI", 7, QFont::Bold));
            p.drawText(badge, Qt::AlignCenter, QString::number(idx+1));
        }

        // Process label
        p.setPen(textCol);
        p.setFont(QFont("Segoe UI", 11, QFont::Bold));
        p.drawText(r, Qt::AlignCenter, nd.label);

        // State sub-label
        QString sub;
        if (state==1) sub="Running";
        else if(state==2) sub="Done";
        else if(state==3) sub="Blocked";

        if (!sub.isEmpty()) {
            QRectF sr(r.x(), r.bottom()+4, r.width(), 16);
            p.setFont(QFont("Segoe UI", 7));
            p.setPen(borderCol);
            p.drawText(sr, Qt::AlignCenter, sub);
        }
    }
}
