/***************************************************************************
    File                 : Plot.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Plot window class

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
#include "Plot.h"

#include "plot2D/Graph.h"
#include "plot2D/Grid.h"
#include "plot2D/ScaleDraw.h"
#include "plot2D/Spectrogram.h"
#include "plot2D/PlotCurve.h"

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_map.h>
#include <qwt_text_label.h>
#include <qwt_math.h>

#include <QPainter>

#include <iostream>

Plot::Plot(QWidget *parent, QString) : QwtPlot(parent)
{
    setAutoReplot(false);

    marker_key = 0;
    curve_key = 0;

    minTickLength = 5;
    majTickLength = 9;

    setGeometry(QRect(0, 0, 500, 400));
    setAxisTitle(QwtPlot::yLeft, tr("Y Axis Title"));
    setAxisTitle(QwtPlot::xBottom, tr("X Axis Title"));

    // grid
    d_grid = new Grid;
    d_grid->enableX(false);
    d_grid->enableY(false);
    d_grid->setMajPenX(QPen(Qt::blue, 0, Qt::SolidLine));
    d_grid->setMinPenX(QPen(Qt::gray, 0, Qt::DotLine));
    d_grid->attach(this);

    // custom scale
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)axisWidget(i);
        if (scale) {
            scale->setMargin(0);

            // the axis title color must be initialized...
            QwtText title = scale->title();
            title.setColor(Qt::black);
            scale->setTitle(title);

            //...same for axis color
            QPalette pal = scale->palette();
            pal.setColor(QPalette::WindowText, QColor(Qt::black));
            scale->setPalette(pal);

            auto *sd = new ScaleDraw();
            sd->setTickLength(QwtScaleDiv::MinorTick, minTickLength);
            sd->setTickLength(QwtScaleDiv::MediumTick, minTickLength);
            sd->setTickLength(QwtScaleDiv::MajorTick, majTickLength);

            setAxisScaleDraw(i, sd);
        }
    }

    QwtPlotLayout *pLayout = plotLayout();
    pLayout->setCanvasMargin(0);
    pLayout->setAlignCanvasToScales(true);

    QWidget *plCanvas = canvas();
    plCanvas->setFocusPolicy(Qt::StrongFocus);
    // plCanvas->setFocusIndicator(QwtPlotCanvas::ItemFocusIndicator);
    plCanvas->setFocus();
    // plCanvas->setFrameShadow(QwtPlot::Plain);
    plCanvas->setCursor(Qt::ArrowCursor);
    // plCanvas->setLineWidth(0);
    // plCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
    // plCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);

    auto background = QColor(Qt::white);
    background.setAlpha(255);

    QPalette cg;
    cg.setColor(QPalette::Window, background);
    setPalette(cg);

    setAutoFillBackground(true);

    setCanvasBackground(background);
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(plCanvas);
    setFrameShape(QFrame::Box);
    setLineWidth(0);
}

QColor Plot::frameColor()
{
    return palette().color(QPalette::Active, QPalette::WindowText);
}

void Plot::printFrame(QPainter *painter, const QRect &rect) const
{
    painter->save();

    int lw = lineWidth();
    if (lw) {
        QColor color = palette().color(QPalette::Active, QPalette::WindowText);
        painter->setPen(QPen(color, lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    } else
        painter->setPen(QPen(Qt::NoPen));

    painter->setBrush(paletteBackgroundColor());

    int lw2 = lw / 2;
    if (lw % 2)
        painter->drawRect(rect.adjusted(lw2, lw2, -(lw2 + 1), -(lw2 + 1)));
    else
        painter->drawRect(rect.adjusted(lw2, lw2, -lw2, -lw2));

    painter->restore();
}

void Plot::drawItems(QPainter *painter, const QRectF &rect, const QwtScaleMap map[axisCnt]) const
{
    QwtPlot::drawItems(painter, rect, map);

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        if (!axisEnabled(i))
            continue;

        auto *sd = (ScaleDraw *)axisScaleDraw(i);
        int majorTicksType = sd->majorTicksStyle();
        int minorTicksType = sd->minorTicksStyle();

        bool min = (minorTicksType == ScaleDraw::In || minorTicksType == ScaleDraw::Both);
        bool maj = (majorTicksType == ScaleDraw::In || majorTicksType == ScaleDraw::Both);

        if (min || maj)
            drawInwardTicks(painter, rect, map[i], i, min, maj);
    }
    if ((dynamic_cast<Graph *>(parent()))->axesBackbones()) {
        // int apw2 = axesLinewidth() / 2;
        bool clp = painter->hasClipping();
        painter->save();
        painter->setPen(QPen(canvas()->palette().color(QPalette::Active, QPalette::WindowText),
                             axesLinewidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setClipping(false);
        /*if (pfilter.options() & QwtPlotPrintFilter::PrintFrameWithScales)
            painter->drawRect(rect.adjusted(-(apw2 + 1), -(apw2 + 1), apw2, apw2));*/
        painter->restore();
        painter->setClipping(clp);
    }
}

void Plot::drawInwardTicks(QPainter *painter, const QRectF &rect, const QwtScaleMap &map, int axis,
                           bool min, bool maj) const
{
    double x1 = rect.left();
    double x2 = rect.right();
    double y1 = rect.top();
    double y2 = rect.bottom();

    QPalette pal = axisWidget(axis)->palette();
    QColor color = pal.color(QPalette::Active, QPalette::WindowText);

    painter->save();
    painter->setPen(QPen(color, axesLinewidth(), Qt::SolidLine));

    auto scDiv = axisScaleDiv(axis);
    const QList<double> minTickList = scDiv.ticks(QwtScaleDiv::MinorTick);
    int minTicks = (int)minTickList.count();

    const QList<double> medTickList = scDiv.ticks(QwtScaleDiv::MediumTick);
    int medTicks = (int)medTickList.count();

    const QList<double> majTickList = scDiv.ticks(QwtScaleDiv::MajorTick);
    int majTicks = (int)majTickList.count();

    int j = 0;
    double x = 0, y = 0, low = 0, high = 0;
    switch (axis) {
    case QwtPlot::yLeft:
        x = x1;
        low = y1;
        if (axisEnabled(QwtPlot::xTop))
            low += majTickLength;
        high = y2;
        if (axisEnabled(QwtPlot::xBottom))
            high -= majTickLength;
        if (min) {
            for (j = 0; j < minTicks; j++) {
                y = map.transform(minTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x, y, x + minTickLength, y);
            }
            for (j = 0; j < medTicks; j++) {
                y = map.transform(medTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x, y, x + minTickLength, y);
            }
        }

        if (maj) {
            for (j = 0; j < majTicks; j++) {
                y = map.transform(majTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x, y, x + majTickLength, y);
            }
        }
        break;

    case QwtPlot::yRight: {
        x = x2;
        low = y1;
        if (axisEnabled(QwtPlot::xTop))
            low += majTickLength;
        high = y2;
        if (axisEnabled(QwtPlot::xBottom))
            high -= majTickLength;
        if (min) {
            for (j = 0; j < minTicks; j++) {
                y = map.transform(minTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x + 1, y, x - minTickLength, y);
            }
            for (j = 0; j < medTicks; j++) {
                y = map.transform(medTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x + 1, y, x - minTickLength, y);
            }
        }

        if (maj) {
            for (j = 0; j < majTicks; j++) {
                y = map.transform(majTickList[j]);
                if (y >= low && y <= high)
                    QwtPainter::drawLine(painter, x + 1, y, x - majTickLength, y);
            }
        }
    } break;

    case QwtPlot::xBottom:
        y = y2;
        low = x1;
        if (axisEnabled(QwtPlot::yLeft))
            low += majTickLength;
        high = x2;
        if (axisEnabled(QwtPlot::yRight))
            high -= majTickLength;
        if (min) {
            for (j = 0; j < minTicks; j++) {
                x = map.transform(minTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y + 1, x, y - minTickLength);
            }
            for (j = 0; j < medTicks; j++) {
                x = map.transform(medTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y + 1, x, y - minTickLength);
            }
        }

        if (maj) {
            for (j = 0; j < majTicks; j++) {
                x = map.transform(majTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y + 1, x, y - majTickLength);
            }
        }
        break;

    case QwtPlot::xTop:
        y = y1;
        low = x1;
        if (axisEnabled(QwtPlot::yLeft))
            low += majTickLength;
        high = x2;
        if (axisEnabled(QwtPlot::yRight))
            high -= majTickLength;

        if (min) {
            for (j = 0; j < minTicks; j++) {
                x = map.transform(minTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y, x, y + minTickLength);
            }
            for (j = 0; j < medTicks; j++) {
                x = map.transform(medTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y, x, y + minTickLength);
            }
        }

        if (maj) {
            for (j = 0; j < majTicks; j++) {
                x = map.transform(majTickList[j]);
                if (x >= low && x <= high)
                    QwtPainter::drawLine(painter, x, y, x, y + majTickLength);
            }
        }
        break;
    }
    painter->restore();
}

void Plot::setAxesLinewidth() // int width)
{
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)this->axisWidget(i);
        if (scale) {
            // scale->setPenWidth(width);
            scale->repaint();
        }
    }
}

int Plot::axesLinewidth() const
{
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        /* const QwtScaleWidget *scale = this->axisWidget(axis);
        if (scale)
            return scale->penWidth();*/
    }
    return 0;
}

int Plot::minorTickLength() const
{
    return minTickLength;
}

int Plot::majorTickLength() const
{
    return majTickLength;
}

void Plot::setTickLength(int minLength, int majLength)
{
    if (majTickLength == majLength && minTickLength == minLength)
        return;

    majTickLength = majLength;
    minTickLength = minLength;
}

void Plot::print(QPainter *painter, const QRect &plotRect) const
{
    // QwtText t = title();
    printFrame(painter, plotRect);
    // QwtPlot::print(painter, plotRect);
    // setTitle(t);  WTF??
}

QwtPlotCurve *Plot::curve(int index)
{
    QwtPlotItem *it = d_curves.value(index);
    if (it && it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
        return dynamic_cast<QwtPlotCurve *>(it);
    else
        return nullptr;
}

int Plot::closestCurve(int xpos, int ypos, int &dist, int &point)
{
    std::array<QwtScaleMap, QwtPlot::axisCnt> map;
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++)
        map[axis] = canvasMap(axis);

    double dmin = 1.0e10;
    int key = -1;
    for (QMap<int, QwtPlotItem *>::iterator iter = d_curves.begin(); iter != d_curves.end();
         ++iter) {
        auto *item = (QwtPlotItem *)iter.value();
        if (!item)
            continue;

        if (item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
            auto *c = dynamic_cast<PlotCurve *>(item);
            for (int i = 0; i < static_cast<int>(c->dataSize()); i++) {
                double cx = map[c->xAxis()].transform(c->sample(i).x()) - double(xpos);
                double cy = map[c->yAxis()].transform(c->sample(i).y()) - double(ypos);
                double f = qwtSqr(cx) + qwtSqr(cy);
                if (f < dmin && c->type() != Graph::ErrorBars) {
                    dmin = f;
                    key = iter.key();
                    point = i;
                }
            }
        }
    }
    dist = int(sqrt(dmin));
    return key;
}

void Plot::removeMarker(int index)
{
    QwtPlotMarker *m = d_markers[index];
    if (!m)
        return;
    m->detach();
    d_markers.remove(index);
}

int Plot::insertMarker(QwtPlotMarker *m)
{
    marker_key++;
    if (!d_markers.contains(marker_key))
        d_markers.insert(marker_key, m);
    m->setRenderHint(QwtPlotItem::RenderAntialiased,
                     (dynamic_cast<Graph *>(parent()))->antialiasing());
    m->attach(((QwtPlot *)this));
    return marker_key;
}

int Plot::insertCurve(QwtPlotItem *c)
{
    curve_key++;
    if (!d_curves.contains(curve_key))
        d_curves.insert(curve_key, c);
    if (c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
        (dynamic_cast<QwtPlotCurve *>(c))->setPaintAttribute(QwtPlotCurve::FilterPoints);

    c->setRenderHint(QwtPlotItem::RenderAntialiased,
                     (dynamic_cast<Graph *>(parent()))->antialiasing());
    c->attach(this);
    return curve_key;
}

void Plot::removeCurve(int index)
{
    QwtPlotItem *c = d_curves[index];
    if (!c)
        return;

    if (c->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) {
        auto *sp = dynamic_cast<Spectrogram *>(c);
        QwtScaleWidget *colorAxis = axisWidget(sp->colorScaleAxis());
        if (colorAxis)
            colorAxis->setColorBarEnabled(false);
    }

    c->detach();
    d_curves.remove(index);
}

QList<int> Plot::getMajorTicksType()
{
    QList<int> majorTicksType;
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        if (axisEnabled(axis)) {
            auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
            majorTicksType << sd->majorTicksStyle();
        } else
            majorTicksType << ScaleDraw::Out;
    }
    return majorTicksType;
}

void Plot::setMajorTicksType(int axis, int type)
{
    auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
    if (sd)
        sd->setMajorTicksStyle((ScaleDraw::TicksStyle)type);
}

QList<int> Plot::getMinorTicksType()
{
    QList<int> minorTicksType;
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        if (axisEnabled(axis)) {
            auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
            minorTicksType << sd->minorTicksStyle();
        } else
            minorTicksType << ScaleDraw::Out;
    }
    return minorTicksType;
}

void Plot::setMinorTicksType(int axis, int type)
{
    auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
    if (sd)
        sd->setMinorTicksStyle((ScaleDraw::TicksStyle)type);
}

int Plot::axisLabelFormat(int axis)
{
#if QWT_VERSION >= 0x060200
    if (isAxisValid(axis)) {
#else
    if (axisValid(axis)) {
#endif
        int prec = 0;
        char format = 0;

        auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
        sd->labelFormat(format, prec);

        if (format == 'g')
            return Automatic;
        else if (format == 'e')
            return Scientific;
        else if (format == 'f')
            return Decimal;
        else
            return Superscripts;
    }

    return 0;
}

int Plot::axisLabelPrecision(int axis)
{
#if QWT_VERSION >= 0x060200
    if (isAxisValid(axis)) {
#else
    if (axisValid(axis)) {
#endif
        auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
        return sd->labelNumericPrecision();
    }

    // for a bad call we return the default values
    return 4;
}

/*!
  \return the number format for the major scale labels of a specified axis
  \param axis axis index
  \retval f format character
  \retval prec precision
  */
void Plot::axisLabelFormat(int axis, char &f, int &prec) const
{
#if QWT_VERSION >= 0x060200
    if (isAxisValid(axis)) {
#else
    if (axisValid(axis)) {
#endif
        auto *sd = (ScaleDraw *)axisScaleDraw(axis);
        sd->labelFormat(f, prec);
    } else {
        // for a bad call we return the default values
        f = 'g';
        prec = 4;
    }
}

/*!
  Change the number format for the major scale of a selected axis
  \param axis axis index
  \param f format
  \param prec precision
  */
void Plot::setAxisLabelFormat(int axis, char f, int prec)
{
#if QWT_VERSION >= 0x060200
    if (isAxisValid(axis)) {
#else
    if (axisValid(axis)) {
#endif
        auto *sd = dynamic_cast<ScaleDraw *>(axisScaleDraw(axis));
        sd->setLabelFormat(f, prec);
    }
}

const QColor &Plot::paletteBackgroundColor() const
{
    return palette().color(QPalette::Window);
}
