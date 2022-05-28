/***************************************************************************
    File                 : Grid.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : 2D Grid class

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
#include "Grid.h"

#include "plot2D/Plot.h"
#include "plot2D/Graph.h"
#include "core/ColorButton.h"

#include <qwt_plot_canvas.h>
#include <qwt_painter.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>

#include <QPainter>
#include <QJsonObject>

Grid::Grid()
    : QwtPlotGrid(),
      d_maj_pen_y(QPen(Qt::blue, 0, Qt::SolidLine)),
      d_min_pen_y(QPen(Qt::gray, 0, Qt::DotLine)),
      mrkX(-1),
      mrkY(-1)
{
}

/*!
  \brief Draw the grid

  The grid is drawn into the bounding rectangle such that
  gridlines begin and end at the rectangle's borders. The X and Y
  maps are used to map the scale divisions into the drawing region
  screen.
  \param painter  Painter
  \param mx X axis map
  \param my Y axis
  \param r Contents rect of the plot canvas
  */
void Grid::draw(QPainter *painter, const QwtScaleMap &mx, const QwtScaleMap &my,
                const QRectF &r) const
{
    //  draw minor X gridlines
    painter->setPen(minorPen());

    if (xMinEnabled()) {
        drawLines(painter, r, Qt::Vertical, mx, xScaleDiv().ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Vertical, mx, xScaleDiv().ticks(QwtScaleDiv::MediumTick));
    }

    //  draw minor Y gridlines
    painter->setPen(d_min_pen_y);

    if (yMinEnabled()) {
        drawLines(painter, r, Qt::Horizontal, my, yScaleDiv().ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Horizontal, my, yScaleDiv().ticks(QwtScaleDiv::MediumTick));
    }

    //  draw major X gridlines
    painter->setPen(majorPen());

    if (xEnabled()) {
        drawLines(painter, r, Qt::Vertical, mx, xScaleDiv().ticks(QwtScaleDiv::MajorTick));
    }

    //  draw major Y gridlines
    painter->setPen(d_maj_pen_y);

    if (yEnabled()) {
        drawLines(painter, r, Qt::Horizontal, my, yScaleDiv().ticks(QwtScaleDiv::MajorTick));
    }
}

void Grid::drawLines(QPainter *painter, const QRectF &rect, Qt::Orientation orientation,
                     const QwtScaleMap &map, const QList<double> &values) const
{
    const int x1 = rect.left();
    const int x2 = rect.right() + 1;
    const int y1 = rect.top();
    const int y2 = rect.bottom() + 1;
    const int margin = 10;

    for (uint i = 0; i < (uint)values.count(); i++) {
        const int value = map.transform(values[i]);
        if (orientation == Qt::Horizontal) {
            if ((value >= y1 + margin) && (value <= y2 - margin))
                QwtPainter::drawLine(painter, x1, value, x2, value);
        } else {
            if ((value >= x1 + margin) && (value <= x2 - margin))
                QwtPainter::drawLine(painter, value, y1, value, y2);
        }
    }
}

void Grid::load(QJsonObject *jsGrid)
{
    Plot *d_plot = dynamic_cast<Plot *>(plot());
    if (!d_plot)
        return;

    enableZeroLineX(jsGrid->value("xZeroLineEnabled").toBool());
    enableZeroLineY(jsGrid->value("yZeroLineEnabled").toBool());

    QJsonObject jsMajPenX = jsGrid->value("majPenX").toObject();
    setMajPenX(QPen(QColor(COLORVALUE(jsMajPenX.value("color").toString())),
                    jsMajPenX.value("width").toInt(),
                    Graph::getPenStyle(jsMajPenX.value("style").toInt())));

    QJsonObject jsMinPenX = jsGrid->value("minPenX").toObject();
    setMinPenX(QPen(QColor(COLORVALUE(jsMinPenX.value("color").toString())),
                    jsMinPenX.value("width").toInt(),
                    Graph::getPenStyle(jsMinPenX.value("style").toInt())));

    QJsonObject jsMajPenY = jsGrid->value("majPenY").toObject();
    setMajPenY(QPen(QColor(COLORVALUE(jsMajPenY.value("color").toString())),
                    jsMajPenY.value("width").toInt(),
                    Graph::getPenStyle(jsMajPenY.value("style").toInt())));

    QJsonObject jsMinPenY = jsGrid->value("minPenY").toObject();
    setMinPenY(QPen(QColor(COLORVALUE(jsMinPenY.value("color").toString())),
                    jsMinPenY.value("width").toInt(),
                    Graph::getPenStyle(jsMinPenY.value("style").toInt())));

    enableX(jsGrid->value("xEnabled").toBool());
    enableXMin(jsGrid->value("xMinEnabled").toBool());
    enableY(jsGrid->value("yEnabled").toBool());
    enableYMin(jsGrid->value("yMinEnabled").toBool());

    setAxes(jsGrid->value("xAxis").toInt(), jsGrid->value("yAxis").toInt());
}

void Grid::enableZeroLineX(bool enable)
{
    Plot *d_plot = dynamic_cast<Plot *>(plot());
    if (!d_plot)
        return;

    if (mrkX < 0 && enable) {
        auto *m = new QwtPlotMarker();
        mrkX = d_plot->insertMarker(m);
        m->setRenderHint(QwtPlotItem::RenderAntialiased, false);
        m->setAxes(xAxis(), yAxis());
        m->setLineStyle(QwtPlotMarker::VLine);
        m->setValue(0.0, 0.0);

        int width = 1;
        /*if (d_plot->canvas()->lineWidth())
            width = d_plot->canvas()->lineWidth();
        else*/
        if (d_plot->axisEnabled(QwtPlot::yLeft) || d_plot->axisEnabled(QwtPlot::yRight))
            width = d_plot->axesLinewidth();

        m->setLinePen(QPen(Qt::black, width, Qt::SolidLine));
    } else if (mrkX >= 0 && !enable) {
        d_plot->removeMarker(mrkX);
        mrkX = -1;
    }
}

void Grid::enableZeroLineY(bool enable)
{
    Plot *d_plot = dynamic_cast<Plot *>(plot());
    if (!d_plot)
        return;

    if (mrkY < 0 && enable) {
        auto *m = new QwtPlotMarker();
        mrkY = d_plot->insertMarker(m);
        m->setRenderHint(QwtPlotItem::RenderAntialiased, false);
        m->setAxes(xAxis(), yAxis());
        m->setLineStyle(QwtPlotMarker::HLine);
        m->setValue(0.0, 0.0);

        int width = 1;
        /*if (d_plot->canvas()->lineWidth())
            width = d_plot->canvas()->lineWidth();
        else*/
        if (d_plot->axisEnabled(QwtPlot::xBottom) || d_plot->axisEnabled(QwtPlot::xTop))
            width = d_plot->axesLinewidth();

        m->setLinePen(QPen(Qt::black, width, Qt::SolidLine));
    } else if (mrkY >= 0 && !enable) {
        d_plot->removeMarker(mrkY);
        mrkY = -1;
    }
}

void Grid::copy(Grid *grid)
{
    if (!grid)
        return;

    setMajPenX(grid->majPenX());
    setMinPenX(grid->minPenX());
    setMajPenY(grid->majPenY());
    setMinPenY(grid->minPenY());

    enableX(grid->xEnabled());
    enableXMin(grid->xMinEnabled());
    enableY(grid->yEnabled());
    enableYMin(grid->yMinEnabled());

    setAxes(grid->xAxis(), grid->yAxis());

    enableZeroLineX(grid->xZeroLineEnabled());
    enableZeroLineY(grid->yZeroLineEnabled());
}

void Grid::saveToJson(QJsonObject *jsObject)
{
    jsObject->insert("xEnabled", xEnabled());
    jsObject->insert("xMinEnabled", xMinEnabled());
    jsObject->insert("yEnabled", yEnabled());
    jsObject->insert("yMinEnabled", yMinEnabled());

    QJsonObject jsMajPenX {};
    jsMajPenX.insert("color", majPenX().color().name());
    jsMajPenX.insert("style", majPenX().style() - 1);
    jsMajPenX.insert("width", majPenX().width());
    jsObject->insert("majPenX", jsMajPenX);

    QJsonObject jsMinPenX {};
    jsMinPenX.insert("color", minPenX().color().name());
    jsMinPenX.insert("style", minPenX().style() - 1);
    jsMinPenX.insert("width", minPenX().width());
    jsObject->insert("minPenX", jsMinPenX);

    QJsonObject jsMajPenY {};
    jsMajPenY.insert("color", majPenY().color().name());
    jsMajPenY.insert("style", majPenY().style() - 1);
    jsMajPenY.insert("width", majPenY().width());
    jsObject->insert("majPenY", jsMajPenY);

    QJsonObject jsMinPenY {};
    jsMinPenY.insert("color", minPenY().color().name());
    jsMinPenY.insert("style", minPenY().style() - 1);
    jsMinPenY.insert("width", minPenY().width());
    jsObject->insert("minPenY", jsMinPenY);

    jsObject->insert("xZeroLineEnabled", xZeroLineEnabled());
    jsObject->insert("yZeroLineEnabled", yZeroLineEnabled());
    jsObject->insert("xAxis", xAxis());
    jsObject->insert("yAxis", yAxis());
}
