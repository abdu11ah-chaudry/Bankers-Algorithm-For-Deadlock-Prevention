#pragma once
#include <QWidget>
#include <QTimer>
#include <vector>
#include <QString>

struct ProcessNode {
    int id;
    QString label;
    QRectF  rect;
    int     state;   // 0=idle, 1=running, 2=done, 3=blocked
    qreal   alpha;
};

class VisualizerWidget : public QWidget {
    Q_OBJECT
public:
    explicit VisualizerWidget(QWidget *parent = nullptr);

    void setSafeSequence(const std::vector<int> &seq, int total);
    void setBlockedProcesses(const std::vector<int> &blocked, int total);
    void clearAll();
    void animateStep(int step);   // highlight step-th node in sequence
    void showUnsafe();
    void showSafe();

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void onPulse();

private:
    void layoutNodes();
    void drawArrow(QPainter &p, QPointF from, QPointF to, QColor color);

    std::vector<int> m_safeSeq;
    std::vector<int> m_blocked;
    int  m_total        = 0;
    int  m_currentStep  = -1;
    bool m_unsafe       = false;
    bool m_safe         = false;
    qreal m_pulse       = 0.0;
    int   m_pulseDir    = 1;

    QTimer *m_timer;
    std::vector<ProcessNode> m_nodes;

    // colours
    static constexpr int NODE_W = 72;
    static constexpr int NODE_H = 48;
};
