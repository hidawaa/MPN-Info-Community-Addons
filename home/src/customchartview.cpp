#include "customchartview.h"

#include <QCursor>
#include <QXYSeries>
#include <QBarSeries>
#include <QBarSet>

#include "ui/chartcalloutitem.h"

CustomChartView::CustomChartView(QWidget *parent) :
    QChartView(parent),
    mChart(this->chart()),
    mTooltips(0),
    mMousePress(false),
    mMouseMoved(false)
{

}

void CustomChartView::setChart(QChart *chart)
{
    QChartView::setChart(chart);
    mChart = chart;
    mTooltips = new ChartCalloutItem(chart);

    foreach (QAbstractSeries *series, mChart->series()) {
        if (series->type() == QAbstractSeries::SeriesTypeLine) {
            QXYSeries *xySeries = static_cast<QXYSeries *>(series);
            connect(xySeries, SIGNAL(hovered(QPointF,bool)), SLOT(onSeriesHovered(QPointF,bool)));
        }
        else if (series->type() == QAbstractSeries::SeriesTypeBar) {
            QAbstractBarSeries *barSeries = static_cast<QAbstractBarSeries *>(series);
            connect(barSeries, SIGNAL(hovered(bool,int,QBarSet*)), SLOT(onSeriesHovered(bool,int,QBarSet*)));
        }
    }
}

/*
void CustomChartView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        mMousePress = true;
        mMouseMoved = false;
        mMouseMovePos = event->pos();
        mChart->setAnimationOptions(QChart::NoAnimation);
        break;
    default:
        break;
    }

    QChartView::mousePressEvent(event);
}

void CustomChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (mMousePress) {
        mMouseMoved = true;
        mChart->scroll(mMouseMovePos.x() - event->pos().x(), event->pos().y() - mMouseMovePos.y());
        mMouseMovePos = event->pos();
    }

    QChartView::mouseMoveEvent(event);
}

void CustomChartView::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        mMousePress = false;
        mChart->setAnimationOptions(QChart::SeriesAnimations);
        if (!mMouseMoved)
            mChart->zoomIn();
        break;
    case Qt::RightButton:
        mChart->zoomOut();
        break;
    default:
        break;
    }

    QChartView::mouseReleaseEvent(event);
}
*/

void CustomChartView::onSeriesHovered(const QPointF &point, bool state)
{
    if (!mTooltips)
        mTooltips = new ChartCalloutItem(mChart);

    if (state) {
        mTooltips->setText(QString("%L1").arg(point.y()));
        QXYSeries *series = static_cast<QXYSeries *>(sender());
        mTooltips->setPos(mChart->mapToPosition(point, series));
        mTooltips->setZValue(11);
        mTooltips->show();
    } else {
        mTooltips->hide();
    }
}

void CustomChartView::onSeriesHovered(bool state, int index, QBarSet *barset)
{
    if (!mTooltips)
        mTooltips = new ChartCalloutItem(mChart);

    QPoint pos = QCursor::pos();
    QPoint vPos = mapFromGlobal(pos);

    if (state) {
        mTooltips->setText(QString("%1").arg((*barset)[index]));
        mTooltips->setPos(mChart->mapFromParent(vPos));
        mTooltips->setZValue(11);
        mTooltips->show();
    } else {
        mTooltips->hide();
    }
}
