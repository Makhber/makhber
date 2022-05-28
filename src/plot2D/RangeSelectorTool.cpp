/***************************************************************************
    File                 : RangeSelectorTool.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Plot tool for selecting ranges on curves.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "RangeSelectorTool.h"

#include "plot2D/Graph.h"
#include "plot2D/Plot.h"
#include "plot2D/PlotCurve.h"

#include <qwt_symbol.h>
#include <QPoint>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLocale>

#include <cmath>

RangeSelectorTool::RangeSelectorTool(Graph *graph, const QObject *status_target,
                                     const char *status_slot)
    : QwtPlotPicker(graph->plotWidget()->canvas()), PlotToolInterface(graph)
{
    d_selected_curve = nullptr;
    for (int i = d_graph->curves(); i >= 0; --i) {
        d_selected_curve = d_graph->curve(i);
        if (d_selected_curve && d_selected_curve->rtti() == QwtPlotItem::Rtti_PlotCurve
            && d_selected_curve->dataSize() > 0)
            break;
        d_selected_curve = nullptr;
    }
    if (!d_selected_curve) {
        QMessageBox::critical(d_graph, tr("Warning"), tr("All the curves on this plot are empty!"));
        return;
    }

    d_active_point = 0;
    d_inactive_point = static_cast<int>(d_selected_curve->dataSize()) - 1;
    int marker_size = 20;
    /* If we want this, it should also be added setSelectedCurve and pointSelected
    if (d_selected_curve->symbol().style() != QwtSymbol::NoSymbol) {
            QSize ssize = d_selected_curve->symbol().size();
            marker_size += qMax(ssize.width(), ssize.height());
    }
    */
    d_active_marker.setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(QColor(255, 255, 0, 128)),
                                            QPen(Qt::red, 2), QSize(marker_size, marker_size)));
    d_active_marker.setLineStyle(QwtPlotMarker::VLine);
    d_active_marker.setLinePen(QPen(Qt::red, 1, Qt::DashLine));
    d_inactive_marker.setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(QColor(255, 255, 0, 128)),
                                              QPen(Qt::black, 2), QSize(marker_size, marker_size)));
    d_inactive_marker.setLineStyle(QwtPlotMarker::VLine);
    d_inactive_marker.setLinePen(QPen(Qt::black, 1, Qt::DashLine));
    d_active_marker.setValue(d_selected_curve->sample(d_active_point).x(),
                             d_selected_curve->sample(d_active_point).y());
    d_inactive_marker.setValue(d_selected_curve->sample(d_inactive_point).x(),
                               d_selected_curve->sample(d_inactive_point).y());
    d_active_marker.attach(d_graph->plotWidget());
    d_inactive_marker.attach(d_graph->plotWidget());

    setTrackerMode(QwtPicker::AlwaysOn);
    // setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
    d_graph->plotWidget()->canvas()->setCursor(QCursor(QPixmap(":/vizor.xpm"), -1, -1));
    d_graph->plotWidget()->canvas()->setFocus();
    d_graph->plotWidget()->replot();

    if (status_target)
        connect(this, SIGNAL(statusText(const QString &)), status_target, status_slot);
    Q_EMIT statusText(
            tr("Click or use Ctrl+arrow key to select range (arrows select active cursor)!"));
}

RangeSelectorTool::~RangeSelectorTool()
{
    d_active_marker.detach();
    d_inactive_marker.detach();
    d_graph->plotWidget()->canvas()->unsetCursor();
    d_graph->plotWidget()->replot();
}

void RangeSelectorTool::pointSelected(const QPoint &pos)
{
    int dist = 0, point = 0;
    const int curve_key = d_graph->plotWidget()->closestCurve(pos.x(), pos.y(), dist, point);
    if (curve_key < 0 || dist >= 5) // 5 pixels tolerance
        return;
    auto *curve = dynamic_cast<QwtPlotCurve *>(d_graph->plotWidget()->curve(curve_key));
    if (!curve)
        return;

    if (curve == d_selected_curve)
        setActivePoint(point);
    else {
        // try to be intelligent about selecting the inactive point
        double min_x = curve->data()->boundingRect().left();
        double max_x = curve->data()->boundingRect().right();
        int n = static_cast<int>(curve->dataSize());
        double second_x {};
        if (curve->sample(point).x() == min_x)
            second_x = max_x;
        else if (curve->sample(point).x() == max_x)
            second_x = min_x;
        else if (d_active_marker.xValue() < d_inactive_marker.xValue())
            second_x = max_x;
        else
            second_x = min_x;
        if (second_x == max_x) { // start at selected point and try larger indices first
            for (int i = 0; i < n; ++i)
                if (qFuzzyCompare(curve->sample((i + point) % n).x(), max_x))
                    d_inactive_point = (i + point) % n;
        } else { // start at selected point and try smaller indices first
            for (int i = n - 1; i >= 0; --i)
                if (qFuzzyCompare(curve->sample((i + point) % n).x(), max_x))
                    d_inactive_point = (i + point) % n;
        }
        d_selected_curve = curve;
        d_inactive_marker.setValue(curve->sample(d_inactive_point).x(),
                                   curve->sample(d_inactive_point).y());
        d_active_point = point;
        d_active_marker.setValue(d_selected_curve->sample(d_active_point).x(),
                                 d_selected_curve->sample(d_active_point).y());
        emitStatusText();
        Q_EMIT rangeSelectorChanged();
    }
    d_graph->plotWidget()->replot();
}

void RangeSelectorTool::setSelectedCurve(QwtPlotCurve *curve)
{
    if (!curve || d_selected_curve == curve)
        return;
    d_selected_curve = curve;
    d_active_point = 0;
    d_inactive_point = static_cast<int>(d_selected_curve->dataSize()) - 1;
    d_active_marker.setValue(d_selected_curve->sample(d_active_point).x(),
                             d_selected_curve->sample(d_active_point).y());
    d_inactive_marker.setValue(d_selected_curve->sample(d_inactive_point).x(),
                               d_selected_curve->sample(d_inactive_point).y());
    emitStatusText();
    Q_EMIT rangeSelectorChanged();
}

void RangeSelectorTool::setActivePoint(int point)
{
    if (point == d_active_point)
        return;
    d_active_point = point;
    d_active_marker.setValue(d_selected_curve->sample(d_active_point).x(),
                             d_selected_curve->sample(d_active_point).y());
    emitStatusText();
    Q_EMIT rangeSelectorChanged();
}

void RangeSelectorTool::emitStatusText()
{
    if ((dynamic_cast<PlotCurve *>(d_selected_curve))->type() == Graph::Function) {
        Q_EMIT statusText(
                QString("%1 <=> %2[%3]: x=%4; y=%5")
                        .arg(d_active_marker.xValue() > d_inactive_marker.xValue() ? tr("Right")
                                                                                   : tr("Left"),
                             d_selected_curve->title().text())
                        .arg(d_active_point + 1)
                        .arg(QLocale().toString(d_selected_curve->sample(d_active_point).x(), 'G',
                                                15),
                             QLocale().toString(d_selected_curve->sample(d_active_point).y(), 'G',
                                                15)));
    } else {
        Table *t = (dynamic_cast<DataCurve *>(d_selected_curve))->table();
        if (!t)
            return;

        int row = (dynamic_cast<DataCurve *>(d_selected_curve))->tableRow(d_active_point);

        Q_EMIT statusText(
                QString("%1 <=> %2[%3]: x=%4; y=%5")
                        .arg(d_active_marker.xValue() > d_inactive_marker.xValue() ? tr("Right")
                                                                                   : tr("Left"),
                             d_selected_curve->title().text())
                        .arg(row + 1)
                        .arg(t->text(row,
                                     t->colIndex((dynamic_cast<DataCurve *>(d_selected_curve))
                                                         ->xColumnName())),
                             t->text(row, t->colIndex(d_selected_curve->title().text()))));
    }
}

void RangeSelectorTool::switchActiveMarker()
{
    QPointF tmp = d_active_marker.value();
    d_active_marker.setValue(d_inactive_marker.value());
    d_inactive_marker.setValue(tmp);
    int tmp2 = d_active_point;
    d_active_point = d_inactive_point;
    d_inactive_point = tmp2;
    d_graph->plotWidget()->replot();

    emitStatusText();
}

bool RangeSelectorTool::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        if (keyEventFilter(dynamic_cast<QKeyEvent *>(event)))
            return true;
        break;
    default:
        break;
    }
    return QwtPlotPicker::eventFilter(obj, event);
}

bool RangeSelectorTool::keyEventFilter(QKeyEvent *ke)
{
    switch (ke->key()) {
    case Qt::Key_Up: {
        int n_curves = d_graph->curves();
        int start = d_graph->curveIndex(d_selected_curve) + 1;
        for (int i = start; i < start + n_curves; ++i)
            if (d_graph->curve(i % n_curves)->dataSize() > 0) {
                setSelectedCurve(d_graph->curve(i % n_curves));
                break;
            }
        d_graph->plotWidget()->replot();
        return true;
    }
    case Qt::Key_Down: {
        int n_curves = d_graph->curves();
        int start = d_graph->curveIndex(d_selected_curve) + n_curves - 1;
        for (int i = start; i > start - n_curves; --i)
            if (d_graph->curve(i % n_curves)->dataSize() > 0) {
                setSelectedCurve(d_graph->curve(i % n_curves));
                break;
            }
        d_graph->plotWidget()->replot();
        return true;
    }
    case Qt::Key_Right:
    case Qt::Key_Plus: {
        if (ke->modifiers() & Qt::ControlModifier) {
            int n_points = static_cast<int>(d_selected_curve->dataSize());
            setActivePoint((d_active_point + 1) % n_points);
            d_graph->plotWidget()->replot();
        } else
            switchActiveMarker();
        return true;
    }
    case Qt::Key_Left:
    case Qt::Key_Minus: {
        if (ke->modifiers() & Qt::ControlModifier) {
            int n_points = static_cast<int>(d_selected_curve->dataSize());
            setActivePoint((d_active_point - 1 + n_points) % n_points);
            d_graph->plotWidget()->replot();
        } else
            switchActiveMarker();
        return true;
    }
    default:
        break;
    }
    return false;
}
