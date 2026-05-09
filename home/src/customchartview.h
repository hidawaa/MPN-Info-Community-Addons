#ifndef CUSTOMCHARTVIEW_H
#define CUSTOMCHARTVIEW_H

#include <QChartView>

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QBarSet;
QT_CHARTS_END_NAMESPACE

class ChartCalloutItem;

QT_CHARTS_USE_NAMESPACE

class CustomChartView : public QChartView
{
    Q_OBJECT
public:
    CustomChartView(QWidget *parent=0);

    void setChart(QChart *chart);

/*
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
*/

private slots:
    void onSeriesHovered(const QPointF &point, bool state);
    void onSeriesHovered(bool state, int index, QBarSet *barset);

private:
    QChart *mChart;
    ChartCalloutItem *mTooltips;
    bool mMousePress;
    bool mMouseMoved;
    QPoint mMouseMovePos;
};

#endif // CUSTOMCHARTVIEW_H
