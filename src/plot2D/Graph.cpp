/***************************************************************************
    File                 : Graph.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Graph widget

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

#include "Graph.h"

#include "plot2D/Grid.h"
#include "plot2D/CanvasPicker.h"
#include "plot2D/QwtErrorPlotCurve.h"
#include "plot2D/Legend.h"
#include "plot2D/ArrowMarker.h"
#include "plot2D/ScalePicker.h"
#include "plot2D/TitlePicker.h"
#include "plot2D/QwtPieCurve.h"
#include "plot2D/ImageMarker.h"
#include "plot2D/QwtBarCurve.h"
#include "plot2D/BoxCurve.h"
#include "plot2D/QwtHistogram.h"
#include "plot2D/VectorCurve.h"
#include "plot2D/ScaleDraw.h"
#include "plot2D/FunctionCurve.h"
#include "plot2D/Spectrogram.h"
#include "plot2D/SelectionMoveResizer.h"
#include "plot2D/RangeSelectorTool.h"
#include "plot2D/PlotCurve.h"
#include "plot2D/SymbolBox.h"
#include "core/ApplicationWindow.h"
#include "core/ColorButton.h"
#include "core/PatternBox.h"
#include "aspects/column/Column.h"

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>
#include <qwt_text_label.h>
#include <qwt_color_map.h>

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QMenu>
#include <QTextStream>
#include <QLocale>
#include <QPrintDialog>
#include <QImageWriter>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVarLengthArray>
#include <QSvgGenerator>

#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstddef>

Graph::Graph(QWidget *parent, QString name, Qt::WindowFlags f) : QWidget(parent, f)
{
    if (name.isEmpty())
        setObjectName("graph");
    else
        setObjectName(name);

    n_curves = 0;
    d_active_tool = nullptr;
    widthLine = 1;
    selectedMarker = -1;
    drawTextOn = false;
    drawLineOn = false;
    drawArrowOn = false;
    ignoreResize = true;
    drawAxesBackbone = true;
    m_autoscale = true;
    autoScaleFonts = false;
    hidden_size = QSize();
    d_antialiasing = true;
    d_scale_on_print = true;
    d_print_cropmarks = false;

    defaultArrowLineWidth = 1;
    defaultArrowColor = QColor(Qt::black);
    defaultArrowLineStyle = Qt::SolidLine;
    defaultArrowHeadLength = 4;
    defaultArrowHeadAngle = 45;
    defaultArrowHeadFill = true;

    defaultMarkerFont = QFont();
    defaultMarkerFrame = 1;
    defaultTextMarkerColor = QColor(Qt::black);
    defaultTextMarkerBackground = QColor(Qt::white);

    d_user_step = QVector<double>(QwtPlot::axisCnt);
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        axisType << AxisType::Numeric;
        axesFormatInfo << QString();
        axesFormulas << QString();
        d_user_step[i] = 0.0;
    }

    d_plot = new Plot(this);
    cp = new CanvasPicker(this);

    titlePicker = new TitlePicker(d_plot);
    scalePicker = new ScalePicker(d_plot);

    d_zoomer[0] = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, d_plot->canvas());
    d_zoomer[0]->setRubberBandPen(QPen(Qt::black));
    d_zoomer[1] = new QwtPlotZoomer(QwtPlot::xTop, QwtPlot::yRight, d_plot->canvas());
    zoom(false);

    setGeometry(0, 0, 500, 400);
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d_plot);
    setMouseTracking(true);

    legendMarkerID = -1; // no legend for an empty graph
    d_texts = QVector<int>();
    c_type = QVector<int>();
    c_keys = QVector<int>();

    connect(cp, SIGNAL(selectPlot()), this, SLOT(activateGraph()));
    connect(cp, SIGNAL(drawTextOff()), this, SIGNAL(drawTextOff()));
    connect(cp, SIGNAL(viewImageDialog()), this, SIGNAL(viewImageDialog()));
    connect(cp, SIGNAL(viewTextDialog()), this, SIGNAL(viewTextDialog()));
    connect(cp, SIGNAL(viewLineDialog()), this, SIGNAL(viewLineDialog()));
    connect(cp, SIGNAL(showPlotDialog(int)), this, SIGNAL(showPlotDialog(int)));
    connect(cp, SIGNAL(showMarkerPopupMenu()), this, SIGNAL(showMarkerPopupMenu()));
    connect(cp, SIGNAL(modified()), this, SIGNAL(modifiedGraph()));

    connect(titlePicker, SIGNAL(showTitleMenu()), this, SLOT(showTitleContextMenu()));
    connect(titlePicker, SIGNAL(doubleClicked()), this, SIGNAL(viewTitleDialog()));
    connect(titlePicker, SIGNAL(removeTitle()), this, SLOT(removeTitle()));
    connect(titlePicker, SIGNAL(clicked()), this, SLOT(selectTitle()));

    connect(scalePicker, SIGNAL(clicked()), this, SLOT(activateGraph()));
    connect(scalePicker, SIGNAL(clicked()), this, SLOT(deselectMarker()));
    connect(scalePicker, SIGNAL(axisDblClicked(int)), this, SIGNAL(axisDblClicked(int)));
    connect(scalePicker, SIGNAL(axisTitleRightClicked(int)), this, SLOT(showAxisTitleMenu(int)));
    connect(scalePicker, SIGNAL(axisRightClicked(int)), this, SLOT(showAxisContextMenu(int)));
    connect(scalePicker, SIGNAL(xAxisTitleDblClicked()), this, SIGNAL(xAxisTitleDblClicked()));
    connect(scalePicker, SIGNAL(yAxisTitleDblClicked()), this, SIGNAL(yAxisTitleDblClicked()));
    connect(scalePicker, SIGNAL(rightAxisTitleDblClicked()), this,
            SIGNAL(rightAxisTitleDblClicked()));
    connect(scalePicker, SIGNAL(topAxisTitleDblClicked()), this, SIGNAL(topAxisTitleDblClicked()));

    connect(d_zoomer[0], SIGNAL(zoomed(const QRectF &)), this, SLOT(zoomed(const QRectF &)));
}

void Graph::notifyChanges()
{
    Q_EMIT modifiedGraph();
}

void Graph::activateGraph()
{
    Q_EMIT selectedGraph(this);
    setFocus();
}

void Graph::deselectMarker()
{
    selectedMarker = -1;
    if (d_markers_selector)
        delete d_markers_selector;
}

long Graph::selectedMarkerKey()
{
    return selectedMarker;
}

QwtPlotMarker *Graph::selectedMarkerPtr()
{
    return d_plot->marker(selectedMarker);
}

void Graph::setSelectedMarker(long mrk, bool add)
{
    selectedMarker = mrk;
    if (add) {
        if (d_markers_selector) {
            if (d_texts.contains(mrk))
                d_markers_selector->add(dynamic_cast<Legend *>(d_plot->marker(mrk)));
            else if (d_lines.contains(mrk))
                d_markers_selector->add(dynamic_cast<ArrowMarker *>(d_plot->marker(mrk)));
            else if (d_images.contains(mrk))
                d_markers_selector->add(dynamic_cast<ImageMarker *>(d_plot->marker(mrk)));
        } else {
            if (d_texts.contains(mrk))
                d_markers_selector =
                        new SelectionMoveResizer(dynamic_cast<Legend *>(d_plot->marker(mrk)));
            else if (d_lines.contains(mrk))
                d_markers_selector =
                        new SelectionMoveResizer(dynamic_cast<ArrowMarker *>(d_plot->marker(mrk)));
            else if (d_images.contains(mrk))
                d_markers_selector =
                        new SelectionMoveResizer(dynamic_cast<ImageMarker *>(d_plot->marker(mrk)));
            else
                return;
            connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
        }
    } else {
        if (d_texts.contains(mrk)) {
            if (d_markers_selector) {
                if (d_markers_selector->contains(dynamic_cast<Legend *>(d_plot->marker(mrk))))
                    return;
                delete d_markers_selector;
            }
            d_markers_selector =
                    new SelectionMoveResizer(dynamic_cast<Legend *>(d_plot->marker(mrk)));
        } else if (d_lines.contains(mrk)) {
            if (d_markers_selector) {
                if (d_markers_selector->contains(dynamic_cast<ArrowMarker *>(d_plot->marker(mrk))))
                    return;
                delete d_markers_selector;
            }
            d_markers_selector =
                    new SelectionMoveResizer(dynamic_cast<ArrowMarker *>(d_plot->marker(mrk)));
        } else if (d_images.contains(mrk)) {
            if (d_markers_selector) {
                if (d_markers_selector->contains(dynamic_cast<ImageMarker *>(d_plot->marker(mrk))))
                    return;
                delete d_markers_selector;
            }
            d_markers_selector =
                    new SelectionMoveResizer(dynamic_cast<ImageMarker *>(d_plot->marker(mrk)));
        } else
            return;
        connect(d_markers_selector, SIGNAL(targetsChanged()), this, SIGNAL(modifiedGraph()));
    }
}

void Graph::initFonts(const QFont &scaleTitleFnt, const QFont &numbersFnt)
{
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        d_plot->setAxisFont(i, numbersFnt);
        QwtText t = d_plot->axisTitle(i);
        t.setFont(scaleTitleFnt);
        d_plot->setAxisTitle(i, t);
    }
}

void Graph::setAxisFont(int axis, const QFont &fnt)
{
    d_plot->setAxisFont(axis, fnt);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

QFont Graph::axisFont(int axis)
{
    return d_plot->axisFont(axis);
}

void Graph::enableAxis(int axis, bool on)
{
    d_plot->enableAxis(axis, on);
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    if (scale)
        scale->setMargin(0);

    scalePicker->refresh();
}

void Graph::enableAxes(const QStringList &list)
{
    int i = 0;
    for (i = 0; i < QwtPlot::axisCnt; i++) {
        bool ok = list[i + 1].toInt();
        d_plot->enableAxis(i, ok);
    }

    for (i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            scale->setMargin(0);
    }
    scalePicker->refresh();
}

void Graph::enableAxes(QVector<bool> axesOn)
{
    for (int i = 0; i < QwtPlot::axisCnt; i++)
        d_plot->enableAxis(i, axesOn[i]);

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            scale->setMargin(0);
    }
    scalePicker->refresh();
}

QVector<bool> Graph::enabledAxes()
{
    QVector<bool> axesOn(4);
    for (int i = 0; i < QwtPlot::axisCnt; i++)
        axesOn[i] = d_plot->axisEnabled(i);
    return axesOn;
}

QList<int> Graph::axesBaseline()
{
    QList<int> baselineDist;
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            baselineDist << scale->margin();
        else
            baselineDist << 0;
    }
    return baselineDist;
}

void Graph::setAxesBaseline(const QList<int> &lst)
{
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            scale->setMargin(lst[i]);
    }
}

void Graph::setAxesBaseline(QStringList &lst)
{
    lst.removeAll(lst.first());
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            scale->setMargin((lst[i]).toInt());
    }
}

QList<Graph::AxisType> Graph::axesType()
{
    return axisType;
}

void Graph::setLabelsNumericFormat(int axis, int format, int prec, const QString &formula)
{
    axisType[axis] = AxisType::Numeric;
    axesFormulas[axis] = formula;

    auto *sd_old = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    const QwtScaleDiv div = sd_old->scaleDiv();

    if (format == Plot::Superscripts) {
        auto *sd = new QwtSupersciptsScaleDraw(
                *dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                formula.toUtf8().constData());
        sd->setLabelFormat('s', prec);
        sd->setScaleDiv(div);
        d_plot->setAxisScaleDraw(axis, sd);
    } else {
        auto *sd = new ScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                 formula.toUtf8().constData());
        sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
        sd->setScaleDiv(div);

        if (format == Plot::Automatic)
            sd->setLabelFormat('g', prec);
        else if (format == Plot::Scientific)
            sd->setLabelFormat('e', prec);
        else if (format == Plot::Decimal)
            sd->setLabelFormat('f', prec);

        d_plot->setAxisScaleDraw(axis, sd);
    }
}

void Graph::setLabelsNumericFormat(int axis, const QStringList &l)
{
    QwtScaleDraw *sd = d_plot->axisScaleDraw(axis);
    if (!sd->hasComponent(QwtAbstractScaleDraw::Labels) || axisType[axis] != AxisType::Numeric)
        return;

    int format = l[2 * axis].toInt();
    int prec = l[2 * axis + 1].toInt();
    setLabelsNumericFormat(axis, format, prec, axesFormulas[axis]);
}

void Graph::setLabelsNumericFormat(QJsonObject *jsObject)
{
    QJsonArray jsFormats = jsObject->value("formats").toArray();
    QJsonArray jsPrecisions = jsObject->value("precisions").toArray();
    for (int axis = 0; axis < 4; axis++) {
        int format = jsFormats.at(axis).toInt();
        int prec = jsPrecisions.at(axis).toInt();
        setLabelsNumericFormat(axis, format, prec, axesFormulas[axis]);
    }
}

QJsonObject Graph::saveAxesLabelsType()
{
    QJsonObject jsAxesLabelsType {};

    QJsonArray jsTypes {}, jsFormatInfos {};
    for (int i = 0; i < 4; i++) {
        auto type = axisType[i];
        jsTypes.append(static_cast<int>(type));
        if (type == AxisType::Time || type == AxisType::Date || type == AxisType::DateTime
            || type == AxisType::Txt || type == AxisType::ColHeader || type == AxisType::Day
            || type == AxisType::Month)
            jsFormatInfos.append(axesFormatInfo[i]);
        else
            jsFormatInfos.append("");
    };
    jsAxesLabelsType.insert("types", jsTypes);
    jsAxesLabelsType.insert("formatInfo", jsFormatInfos);

    return jsAxesLabelsType;
}

QJsonObject Graph::saveTicksType()
{
    QJsonObject jsTicksType {};

    QJsonArray jsMajorTicksType {};
    QList<int> ticksTypeList = d_plot->getMajorTicksType();
    for (int i = 0; i < 4; i++)
        jsMajorTicksType.append(ticksTypeList[i]);
    jsTicksType.insert("majorTicks", jsMajorTicksType);

    QJsonArray jsMinortTicksType {};
    ticksTypeList = d_plot->getMinorTicksType();
    for (int i = 0; i < 4; i++)
        jsMinortTicksType.append(ticksTypeList[i]);
    jsTicksType.insert("minorTicks", jsMinortTicksType);

    return jsTicksType;
}

QStringList Graph::enabledTickLabels()
{
    QStringList lst;
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        const QwtScaleDraw *sd = d_plot->axisScaleDraw(axis);
        lst << QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels));
    }
    return lst;
}

QJsonArray Graph::saveEnabledTickLabels()
{
    QJsonArray jsEnabledTickLabels {};
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        const QwtScaleDraw *sd = d_plot->axisScaleDraw(axis);
        jsEnabledTickLabels.append(QString::number(sd->hasComponent(QwtAbstractScaleDraw::Labels)));
    }
    return jsEnabledTickLabels;
}

QJsonObject Graph::saveLabelsFormat()
{
    QJsonObject jsLabelsFormat {};

    QJsonArray jsFormats {}, jsPrecisions {};
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        jsFormats.append(d_plot->axisLabelFormat(axis));
        jsPrecisions.append(d_plot->axisLabelPrecision(axis));
    }
    jsLabelsFormat.insert("formats", jsFormats);
    jsLabelsFormat.insert("precisions", jsPrecisions);

    return jsLabelsFormat;
}

QJsonArray Graph::saveAxesBaseline()
{
    QJsonArray jsAxesbaseline {};
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale)
            jsAxesbaseline.append(scale->margin());
        else
            jsAxesbaseline.append(0);
    }
    return jsAxesbaseline;
}

QJsonObject Graph::saveLabelsRotation()
{
    QJsonObject jsLabelsRotation {};
    jsLabelsRotation.insert("xBottom", labelsRotation(QwtPlot::xBottom));
    jsLabelsRotation.insert("xTop", (QwtPlot::xTop));
    return jsLabelsRotation;
}

void Graph::setEnabledTickLabels(const QStringList &labelsOn)
{
    for (int axis = 0; axis < QwtPlot::axisCnt; axis++) {
        QwtScaleWidget *sc = d_plot->axisWidget(axis);
        if (sc) {
            QwtScaleDraw *sd = d_plot->axisScaleDraw(axis);
            sd->enableComponent(QwtAbstractScaleDraw::Labels, labelsOn[axis] == "1");
        }
    }
}

void Graph::setMajorTicksType(const QList<int> &lst)
{
    if (d_plot->getMajorTicksType() == lst)
        return;

    for (int i = 0; i < (int)lst.count(); i++) {
        auto *sd = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(i));
        if (lst[i] == ScaleDraw::None || lst[i] == ScaleDraw::In)
            sd->enableComponent(QwtAbstractScaleDraw::Ticks, false);
        else {
            sd->enableComponent(QwtAbstractScaleDraw::Ticks);
            sd->setTickLength(QwtScaleDiv::MinorTick, d_plot->minorTickLength());
            sd->setTickLength(QwtScaleDiv::MediumTick, d_plot->minorTickLength());
            sd->setTickLength(QwtScaleDiv::MajorTick, d_plot->majorTickLength());
        }
        sd->setMajorTicksStyle((ScaleDraw::TicksStyle)lst[i]);
    }
}

void Graph::setMajorTicksType(const QStringList &lst)
{
    for (int i = 0; i < (int)lst.count(); i++)
        d_plot->setMajorTicksType(i, lst[i].toInt());
}

void Graph::setMinorTicksType(const QList<int> &lst)
{
    if (d_plot->getMinorTicksType() == lst)
        return;

    for (int i = 0; i < (int)lst.count(); i++)
        d_plot->setMinorTicksType(i, lst[i]);
}

void Graph::setMinorTicksType(const QStringList &lst)
{
    for (int i = 0; i < (int)lst.count(); i++)
        d_plot->setMinorTicksType(i, lst[i].toInt());
}

int Graph::minorTickLength()
{
    return d_plot->minorTickLength();
}

int Graph::majorTickLength()
{
    return d_plot->majorTickLength();
}

void Graph::setAxisTicksLength(int axis, int majTicksType, int minTicksType, int minLength,
                               int majLength)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    if (!scale)
        return;

    d_plot->setTickLength(minLength, majLength);

    auto *sd = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    sd->setMajorTicksStyle((ScaleDraw::TicksStyle)majTicksType);
    sd->setMinorTicksStyle((ScaleDraw::TicksStyle)minTicksType);

    if (majTicksType == ScaleDraw::None && minTicksType == ScaleDraw::None)
        sd->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    else
        sd->enableComponent(QwtAbstractScaleDraw::Ticks);

    if (majTicksType == ScaleDraw::None || majTicksType == ScaleDraw::In)
        majLength = minLength;
    if (minTicksType == ScaleDraw::None || minTicksType == ScaleDraw::In)
        minLength = 0;

    sd->setTickLength(QwtScaleDiv::MinorTick, minLength);
    sd->setTickLength(QwtScaleDiv::MediumTick, minLength);
    sd->setTickLength(QwtScaleDiv::MajorTick, majLength);
}

void Graph::setTicksLength(int minLength, int majLength)
{
    QList<int> majTicksType = d_plot->getMajorTicksType();
    QList<int> minTicksType = d_plot->getMinorTicksType();

    for (int i = 0; i < 4; i++)
        setAxisTicksLength(i, majTicksType[i], minTicksType[i], minLength, majLength);
}

void Graph::changeTicksLength(int minLength, int majLength)
{
    if (d_plot->minorTickLength() == minLength && d_plot->majorTickLength() == majLength)
        return;

    setTicksLength(minLength, majLength);

    d_plot->hide();
    for (int i = 0; i < 4; i++) {
        if (d_plot->axisEnabled(i)) {
            d_plot->enableAxis(i, false);
            d_plot->enableAxis(i, true);
        }
    }
    d_plot->replot();
    d_plot->show();

    Q_EMIT modifiedGraph();
}

void Graph::showAxis(int axis, AxisType type, const QString &formatInfo, Table *table, bool axisOn,
                     int majTicksType, int minTicksType, bool labelsOn, const QColor &c, int format,
                     int prec, int rotation, int baselineDist, const QString &formula,
                     const QColor &labelsColor)
{
    d_plot->enableAxis(axis, axisOn);
    if (!axisOn)
        return;

    QList<int> majTicksTypeList = d_plot->getMajorTicksType();
    QList<int> minTicksTypeList = d_plot->getMinorTicksType();

    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    auto *sclDraw = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));

    if (d_plot->axisEnabled(axis) == axisOn && majTicksTypeList[axis] == majTicksType
        && minTicksTypeList[axis] == minTicksType && axesColors()[axis] == COLORNAME(c)
        && axesNumColors()[axis] == COLORNAME(labelsColor)
        && prec == d_plot->axisLabelPrecision(axis) && format == d_plot->axisLabelFormat(axis)
        && labelsRotation(axis) == rotation && axisType[axis] == type
        && axesFormatInfo[axis] == formatInfo && axesFormulas[axis] == formula
        && scale->margin() == baselineDist
        && sclDraw->hasComponent(QwtAbstractScaleDraw::Labels) == labelsOn)
        return;

    scale->setMargin(baselineDist);
    QPalette pal = scale->palette();
    if (pal.color(QPalette::Active, QPalette::WindowText) != c)
        pal.setColor(QPalette::WindowText, c);
    if (pal.color(QPalette::Active, QPalette::Text) != labelsColor)
        pal.setColor(QPalette::Text, labelsColor);
    scale->setPalette(pal);

    if (!labelsOn)
        sclDraw->enableComponent(QwtAbstractScaleDraw::Labels, false);
    else {
        switch (type) {
        case AxisType::Numeric:
            setLabelsNumericFormat(axis, format, prec, formula);
            break;
        case AxisType::Day:
            setLabelsDayFormat(axis, format);
            break;
        case AxisType::Month:
            setLabelsMonthFormat(axis, format);
            break;
        case AxisType::Time:
        case AxisType::Date:
        case AxisType::DateTime:
            setLabelsDateTimeFormat(axis, type, formatInfo);
            break;
        case AxisType::Txt:
            setLabelsTextFormat(axis, table, formatInfo);
            break;
        case AxisType::ColHeader:
            setLabelsColHeaderFormat(axis, table);
            break;
        }

        setAxisLabelRotation(axis, rotation);
    }

    sclDraw = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    sclDraw->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);

    setAxisTicksLength(axis, majTicksType, minTicksType, d_plot->minorTickLength(),
                       d_plot->majorTickLength());

    if (axisOn && (axis == QwtPlot::xTop || axis == QwtPlot::yRight))
        updateSecondaryAxis(axis); // synchronize scale divisions

    scalePicker->refresh();
    d_plot->updateLayout(); // This is necessary in order to enable/disable tick labels
    scale->repaint();
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setLabelsDayFormat(int axis, int format)
{
    axisType[axis] = AxisType::Day;
    axesFormatInfo[axis] = QString::number(format);

    auto *sd_old = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    const QwtScaleDiv div = sd_old->scaleDiv();

    auto *sd = new WeekDayScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                    (WeekDayScaleDraw::NameFormat)format);
    sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
    sd->setScaleDiv(div);
    d_plot->setAxisScaleDraw(axis, sd);
}

void Graph::setLabelsMonthFormat(int axis, int format)
{
    axisType[axis] = AxisType::Month;
    axesFormatInfo[axis] = QString::number(format);

    auto *sd_old = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    const QwtScaleDiv div = sd_old->scaleDiv();

    auto *sd = new MonthScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                  (MonthScaleDraw::NameFormat)format);
    sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
    sd->setScaleDiv(div);
    d_plot->setAxisScaleDraw(axis, sd);
}

void Graph::setLabelsTextFormat(int axis, const Column *column, int startRow, int endRow)
{
    // TODO: The whole text labels functionality is very limited. Specifying
    // only one column for the labels will always mean that the labels
    // correspond to 1, 2, 3, 4, etc.. Other label mappings such as
    // a -> 1.5, b -> 4.5, c -> 16.5 (with step set to 0.5) or similar
    // require to have an additional column with numeric values.
    // This should be supported in our new plotting framework.
    if (!column)
        return;
    auto *table = qobject_cast<future::Table *>(column->parentAspect());
    if (!table)
        return;
    if (axis < 0 || axis > 3)
        return;

    axisType[axis] = AxisType::Txt;
    axesFormatInfo[axis] = table->name() + "_" + column->name();

    QMap<int, QString> list;
    for (int row = startRow; row <= endRow; row++)
        if (!column->isInvalid(row))
            list.insert(row + 1, column->textAt(row));
    auto *sd = new QwtTextScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                    list);
    sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
    d_plot->setAxisScaleDraw(axis, sd);
}

void Graph::setLabelsTextFormat(int axis, Table *table, const QString &columnName)
{
    Column *col = nullptr;
    if (!table || !(col = table->column(columnName))) {
        QMessageBox::critical(
                this, tr("Internal Error"),
                tr("<html>Failed to set axis labels on Graph %1. Maybe you're trying to open a "
                   "corrupted"
                   " project file; or there's some problem within Makhber. Please report this"
                   " as a bug (together with detailed instructions how to reproduce this message or"
                   " the corrupted file).<p>"
                   "<a href=\"https://github.com/Makhber/makhber/issues>\">"
                   "bug tracker: "
                   "https://github.com/Makhber/makhber/issues</a></html>")
                        .arg(objectName()));
        return;
    }
    setLabelsTextFormat(axis, col, 0, col->rowCount() - 1);
}

void Graph::setLabelsColHeaderFormat(int axis, Table *table)
{
    if (!table)
        return;

    axisType[axis] = AxisType::ColHeader;
    axesFormatInfo[axis] = table->name();

    QMap<int, QString> list;
    for (int col = 0; col < table->columnCount(); col++)
        if (table->colPlotDesignation(col) == Makhber::Y)
            list.insert(col, table->colLabel(col));
    auto *sd = new QwtTextScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                    list);
    sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
    d_plot->setAxisScaleDraw(axis, sd);
}

void Graph::setLabelsDateTimeFormat(int axis, AxisType type, const QString &formatInfo)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList list = formatInfo.split(";", Qt::KeepEmptyParts);
#else
    QStringList list = formatInfo.split(";", QString::KeepEmptyParts);
#endif
    if ((int)list.count() < 2 || list[1].isEmpty()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Couldn't change the axis type to the requested format!"));
        return;
    }

    switch (type) {
    case AxisType::Time: {
        auto *sd = new TimeScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                     list[0].isEmpty() ? QTime(12, 0, 0, 0)
                                                       : QTime::fromString(list[0].right(8)),
                                     list[1]);
        sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
        sd->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
        d_plot->setAxisScaleDraw(axis, sd);
        break;
    }
    case AxisType::Date: {
        auto *sd = new DateScaleDraw(*dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                                     QDate::fromString(list[0].left(10), "yyyy-MM-dd"), list[1]);
        sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
        sd->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
        d_plot->setAxisScaleDraw(axis, sd);
        break;
    }
    case AxisType::DateTime: {
        auto *sd = new DateTimeScaleDraw(
                *dynamic_cast<const ScaleDraw *>(d_plot->axisScaleDraw(axis)),
                QDateTime::fromString(list[0], "yyyy-MM-ddThh:mm:ss"), list[1]);
        sd->enableComponent(QwtAbstractScaleDraw::Labels, true);
        sd->enableComponent(QwtAbstractScaleDraw::Backbone, drawAxesBackbone);
        d_plot->setAxisScaleDraw(axis, sd);
        break;
    }
    default:
        // safeguard - such an argument is invalid for this method
        return;
    }

    axisType[axis] = type;
    axesFormatInfo[axis] = formatInfo;
}

void Graph::setAxisLabelRotation(int axis, int rotation)
{
    if (axis == QwtPlot::xBottom) {
        if (rotation > 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignRight | Qt::AlignVCenter);
        else if (rotation < 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignLeft | Qt::AlignVCenter);
        else if (rotation == 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignHCenter | Qt::AlignBottom);
    } else if (axis == QwtPlot::xTop) {
        if (rotation > 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignLeft | Qt::AlignVCenter);
        else if (rotation < 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignRight | Qt::AlignVCenter);
        else if (rotation == 0)
            d_plot->setAxisLabelAlignment(axis, Qt::AlignHCenter | Qt::AlignTop);
    }
    d_plot->setAxisLabelRotation(axis, (double)rotation);
}

int Graph::labelsRotation(int axis)
{
    auto *sclDraw = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(axis));
    return (int)sclDraw->labelRotation();
}

void Graph::setYAxisTitleFont(const QFont &fnt)
{
    QwtText t = d_plot->axisTitle(QwtPlot::yLeft);
    t.setFont(fnt);
    d_plot->setAxisTitle(QwtPlot::yLeft, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setXAxisTitleFont(const QFont &fnt)
{
    QwtText t = d_plot->axisTitle(QwtPlot::xBottom);
    t.setFont(fnt);
    d_plot->setAxisTitle(QwtPlot::xBottom, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setRightAxisTitleFont(const QFont &fnt)
{
    QwtText t = d_plot->axisTitle(QwtPlot::yRight);
    t.setFont(fnt);
    d_plot->setAxisTitle(QwtPlot::yRight, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setTopAxisTitleFont(const QFont &fnt)
{
    QwtText t = d_plot->axisTitle(QwtPlot::xTop);
    t.setFont(fnt);
    d_plot->setAxisTitle(QwtPlot::xTop, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setAxisTitleFont(int axis, const QFont &fnt)
{
    QwtText t = d_plot->axisTitle(axis);
    t.setFont(fnt);
    d_plot->setAxisTitle(axis, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

QFont Graph::axisTitleFont(int axis)
{
    return d_plot->axisTitle(axis).font();
}

QColor Graph::axisTitleColor(int axis)
{
    QColor c;
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    if (scale)
        c = scale->title().color();
    return c;
}

void Graph::setAxesNumColors(const QStringList &colors)
{
    for (int i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            QPalette pal = scale->palette();
            pal.setColor(QPalette::Text, QColor(COLORVALUE(colors[i])));
            scale->setPalette(pal);
        }
    }
}

void Graph::setAxesColors(const QStringList &colors)
{
    for (int i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            QPalette pal = scale->palette();
            pal.setColor(QPalette::WindowText, QColor(COLORVALUE(colors[i])));
            scale->setPalette(pal);
        }
    }
}

QJsonObject Graph::saveAxesColors()
{
    QJsonObject jsAxesColors {};

    QStringList colors, numColors;
    int i = 0;
    for (i = 0; i < 4; i++) {
        colors << COLORNAME(QColor(Qt::black));
        numColors << COLORNAME(QColor(Qt::black));
    }

    QPalette pal;
    for (i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            pal = scale->palette();
            colors[i] = COLORNAME(pal.color(QPalette::Active, QPalette::WindowText));
            numColors[i] = COLORNAME(pal.color(QPalette::Active, QPalette::Text));
        }
    }

    QJsonArray jsColors {}, jsNumColors {};
    for (int i = 0; i < 4; i++) {
        jsColors.append(colors[i]);
        jsNumColors.append(numColors[i]);
    }
    jsAxesColors.insert("colors", jsColors);
    jsAxesColors.insert("numberColors", jsNumColors);

    return jsAxesColors;
}

QStringList Graph::axesColors()
{
    QStringList colors;
    QPalette pal;
    int i = 0;
    for (i = 0; i < 4; i++)
        colors << COLORNAME(QColor(Qt::black));

    for (i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            pal = scale->palette();
            colors[i] = COLORNAME(pal.color(QPalette::Active, QPalette::WindowText));
        }
    }
    return colors;
}

QColor Graph::axisColor(int axis)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    if (scale)
        return scale->palette().color(QPalette::Active, QPalette::WindowText);
    else
        return QColor(Qt::black);
}

QColor Graph::axisNumbersColor(int axis)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(axis);
    if (scale)
        return scale->palette().color(QPalette::Active, QPalette::Text);
    else
        return QColor(Qt::black);
}

QStringList Graph::axesNumColors()
{
    QStringList colors;
    QPalette pal;
    int i = 0;
    for (i = 0; i < 4; i++)
        colors << COLORNAME(QColor(Qt::black));

    for (i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            pal = scale->palette();
            colors[i] = COLORNAME(pal.color(QPalette::Active, QPalette::Text));
        }
    }
    return colors;
}

void Graph::setTitleColor(const QColor &c)
{
    QwtText t = d_plot->title();
    t.setColor(c);
    d_plot->setTitle(t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setTitleAlignment(int align)
{
    QwtText t = d_plot->title();
    t.setRenderFlags(align);
    d_plot->setTitle(t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setTitleFont(const QFont &fnt)
{
    QwtText t = d_plot->title();
    t.setFont(fnt);
    d_plot->setTitle(t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setYAxisTitle(const QString &text)
{
    d_plot->setAxisTitle(QwtPlot::yLeft, text);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setXAxisTitle(const QString &text)
{
    d_plot->setAxisTitle(QwtPlot::xBottom, text);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setRightAxisTitle(const QString &text)
{
    d_plot->setAxisTitle(QwtPlot::yRight, text);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setTopAxisTitle(const QString &text)
{
    d_plot->setAxisTitle(QwtPlot::xTop, text);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

int Graph::axisTitleAlignment(int axis)
{
    return d_plot->axisTitle(axis).renderFlags();
}

void Graph::setAxesTitlesAlignment(QJsonArray *jsRenderFlags)
{
    for (int i = 0; i < 4; i++) {
        QwtText t = d_plot->axisTitle(i);
        t.setRenderFlags(jsRenderFlags->at(i).toInt());
        d_plot->setAxisTitle(i, t);
    }
}

void Graph::setXAxisTitleAlignment(int align)
{
    QwtText t = d_plot->axisTitle(QwtPlot::xBottom);
    t.setRenderFlags(align);
    d_plot->setAxisTitle(QwtPlot::xBottom, t);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setYAxisTitleAlignment(int align)
{
    QwtText t = d_plot->axisTitle(QwtPlot::yLeft);
    t.setRenderFlags(align);
    d_plot->setAxisTitle(QwtPlot::yLeft, t);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setTopAxisTitleAlignment(int align)
{
    QwtText t = d_plot->axisTitle(QwtPlot::xTop);
    t.setRenderFlags(align);
    d_plot->setAxisTitle(QwtPlot::xTop, t);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setRightAxisTitleAlignment(int align)
{
    QwtText t = d_plot->axisTitle(QwtPlot::yRight);
    t.setRenderFlags(align);
    d_plot->setAxisTitle(QwtPlot::yRight, t);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::setAxisTitle(int axis, const QString &text)
{
    d_plot->setAxisTitle(axis, text);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

QStringList Graph::scalesTitles()
{
    QStringList scaleTitles;
    int axis = 0;
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        switch (i) {
        case 0:
            axis = 2;
            scaleTitles << d_plot->axisTitle(axis).text();
            break;

        case 1:
            axis = 0;
            scaleTitles << d_plot->axisTitle(axis).text();
            break;

        case 2:
            axis = 3;
            scaleTitles << d_plot->axisTitle(axis).text();
            break;

        case 3:
            axis = 1;
            scaleTitles << d_plot->axisTitle(axis).text();
            break;
        }
    }
    return scaleTitles;
}

void Graph::updateSecondaryAxis(int axis)
{
    for (int i = 0; i < n_curves; i++) {
        QwtPlotItem *it = plotItem(i);
        if (!it)
            continue;

        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) {
            auto *sp = dynamic_cast<Spectrogram *>(it);
            if (sp->colorScaleAxis() == axis)
                return;
        }

        if ((axis == QwtPlot::yRight && it->yAxis() == QwtPlot::yRight)
            || (axis == QwtPlot::xTop && it->xAxis() == QwtPlot::xTop))
            return;
    }

    int a = QwtPlot::xBottom;
    if (axis == QwtPlot::yRight)
        a = QwtPlot::yLeft;

    if (!d_plot->axisEnabled(a))
        return;

    QwtScaleEngine *se = d_plot->axisScaleEngine(a);
    const QwtScaleDiv sd = d_plot->axisScaleDiv(a);

    QwtScaleEngine *sc_engine = nullptr;
    /* if (se->transformation()->type() == QwtScaleTransformation::Log10)
        sc_engine = new QwtLogScaleEngine();
    else if (se->transformation()->type() == QwtScaleTransformation::Linear)
        sc_engine = new QwtLinearScaleEngine();*/

    if (sc_engine && se->testAttribute(QwtScaleEngine::Inverted))
        sc_engine->setAttribute(QwtScaleEngine::Inverted);

    d_plot->setAxisScaleEngine(axis, sc_engine);
    d_plot->setAxisScaleDiv(axis, sd);

    d_user_step[axis] = d_user_step[a];

    QwtScaleWidget *scale = d_plot->axisWidget(a);
    int start = scale->startBorderDist();
    int end = scale->endBorderDist();

    scale = d_plot->axisWidget(axis);
    scale->setMinBorderDist(start, end);
}

void Graph::setAutoScale()
{
    for (int i = 0; i < QwtPlot::axisCnt; i++)
        d_plot->setAxisAutoScale(i);

    d_plot->replot();
    d_zoomer[0]->setZoomBase();
    d_zoomer[1]->setZoomBase();
    updateScale();

    Q_EMIT modifiedGraph();
}

void Graph::setScale(int axis, double start, double end, double step, int majorTicks,
                     int minorTicks, int type, bool inverted)
{
    QwtScaleEngine *sc_engine = nullptr;
    if (type)
        sc_engine = new QwtLogScaleEngine();
    else
        sc_engine = new QwtLinearScaleEngine();

    int max_min_intervals = minorTicks;
    if (minorTicks == 1)
        max_min_intervals = 3;
    if (minorTicks > 1)
        max_min_intervals = minorTicks + 1;

    QwtScaleDiv div = sc_engine->divideScale(qMin(start, end), qMax(start, end), majorTicks,
                                             max_min_intervals, step);
    d_plot->setAxisMaxMajor(axis, majorTicks);
    d_plot->setAxisMaxMinor(axis, minorTicks);

    if (inverted) {
        sc_engine->setAttribute(QwtScaleEngine::Inverted);
        div.invert();
    }

    d_plot->setAxisScaleEngine(axis, sc_engine);
    d_plot->setAxisScaleDiv(axis, div);

    d_zoomer[0]->setZoomBase();
    d_zoomer[1]->setZoomBase();

    d_user_step[axis] = step;

    if (axis == QwtPlot::xBottom || axis == QwtPlot::yLeft) {
        updateSecondaryAxis(QwtPlot::xTop);
        updateSecondaryAxis(QwtPlot::yRight);
    }

    d_plot->replot();
    // keep markers on canvas area
    updateMarkersBoundingRect();
    d_plot->replot();
}

QStringList Graph::analysableCurvesList()
{
    QStringList cList;
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotCurve *c = d_plot->curve(keys[i]);
        if (c && c_type[i] != ErrorBars)
            cList << c->title().text();
    }
    return cList;
}

QStringList Graph::curvesList()
{
    QStringList cList;
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotCurve *c = d_plot->curve(keys[i]);
        if (c)
            cList << c->title().text();
    }
    return cList;
}

QStringList Graph::plotItemsList() const
{
    QStringList cList;
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (it)
            cList << it->title().text();
    }
    return cList;
}

void Graph::copyImage()
{
    QImage image(d_plot->size(), QImage::Format_ARGB32);
    exportPainter(image);
    QApplication::clipboard()->setImage(image);
}

void Graph::exportToFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please provide a valid file name!"));
        return;
    }

    if (fileName.contains(".eps") || fileName.contains(".pdf") || fileName.contains(".ps")) {
        exportVector(fileName);
        return;
    } else if (fileName.contains(".svg")) {
        exportSVG(fileName);
        return;
    } else {
        QList<QByteArray> list = QImageWriter::supportedImageFormats();
        for (int i = 0; i < list.count(); i++) {
            if (fileName.contains("." + list[i].toLower())) {
                exportImage(fileName);
                return;
            }
        }
        QMessageBox::critical(this, tr("Error"), tr("File format not handled, operation aborted!"));
    }
}

void Graph::exportImage(const QString &fileName, int quality)
{
    QImage image(size(), QImage::Format_ARGB32);
    exportPainter(image);
    image.save(fileName, nullptr, quality);
}

void Graph::exportVector(const QString &fileName, int, bool color, bool keepAspect,
                         QPageSize pageSize, QPageLayout::Orientation orientation)
{
    if (fileName.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please provide a valid file name!"));
        return;
    }

    QPrinter printer;
    printer.setCreator("Makhber");
    printer.setFullPage(true);

    printer.setOutputFileName(fileName);
    if (fileName.contains(".eps")) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Output in postscript format is not available for Qt5, using PDF"));
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName + ".pdf");
    }

    if (color)
        printer.setColorMode(QPrinter::Color);
    else
        printer.setColorMode(QPrinter::GrayScale);

    if (pageSize == QPageSize(QPageSize::Custom))
        printer.setPageSize(QPageSize(size(), QPageSize::Point));
    else {
        printer.setPageOrientation(orientation);
        printer.setPageSize(pageSize);
    }

    exportPainter(printer, keepAspect);
}

void Graph::print()
{
    QPrinter printer;
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(true);

    // printing should preserve plot aspect ratio, if possible
    double aspect = double(d_plot->width()) / double(d_plot->height());
    if (aspect < 1)
        printer.setPageOrientation(QPageLayout::Portrait);
    else
        printer.setPageOrientation(QPageLayout::Landscape);

    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {
        QRect plotRect = d_plot->rect();
        QRect paperRect = printer.pageLayout().fullRectPixels(printer.resolution());
        if (d_scale_on_print) {
            int dpiy = printer.logicalDpiY();
            int margin = (int)((2 / 2.54) * dpiy); // 2 cm margins

            int width = qRound(aspect * printer.height()) - 2 * margin;
            int x = qRound(abs(printer.width() - width) * 0.5);

            plotRect = QRect(x, margin, width, printer.height() - 2 * margin);
            if (x < margin) {
                plotRect.setLeft(margin);
                plotRect.setWidth(printer.width() - 2 * margin);
            }
        } else {
            int x_margin = (paperRect.width() - plotRect.width()) / 2;
            int y_margin = (paperRect.height() - plotRect.height()) / 2;
            plotRect.moveTo(x_margin, y_margin);
        }

        QPainter paint(&printer);
        if (d_print_cropmarks) {
            QRect cr = plotRect; // cropmarks rectangle
            cr.adjust(-1, -1, 2, 2);
            paint.save();
            paint.setPen(QPen(QColor(Qt::black), 0.5, Qt::DashLine));
            paint.drawLine(paperRect.left(), cr.top(), paperRect.right(), cr.top());
            paint.drawLine(paperRect.left(), cr.bottom(), paperRect.right(), cr.bottom());
            paint.drawLine(cr.left(), paperRect.top(), cr.left(), paperRect.bottom());
            paint.drawLine(cr.right(), paperRect.top(), cr.right(), paperRect.bottom());
            paint.restore();
        }

        print(&paint, plotRect);
    }
}

void Graph::exportSVG(const QString &fname)
{
    QSvgGenerator svg;
    svg.setFileName(fname);
    svg.setSize(d_plot->size());
    svg.setViewBox(d_plot->rect());
    svg.setResolution(96); // FIXME hardcored
    svg.setTitle(this->objectName());
    exportPainter(svg);
}

void Graph::exportPainter(QPaintDevice &paintDevice, bool keepAspect, QRect rect)
{
    QPainter p(&paintDevice);
    exportPainter(p, keepAspect, rect, QSize(paintDevice.width(), paintDevice.height()));
}

void Graph::exportPainter(QPainter &painter, bool keepAspect, QRect rect, QSize size)
{
    if (size == QSize())
        size = d_plot->size();
    if (rect == QRect())
        rect = QRect(QPoint(0, 0), size);
    if (keepAspect) {
        QSize scaled = rect.size();
        scaled.scale(size, Qt::KeepAspectRatio);
        size = scaled;
    }

    painter.scale((double)size.width() / (double)rect.width(),
                  (double)size.height() / (double)rect.height());

    print(&painter, rect);
}

int Graph::selectedCurveID()
{
    if (d_range_selector)
        return curveKey(curveIndex(d_range_selector->selectedCurve()));
    else
        return -1;
}

QString Graph::selectedCurveTitle()
{
    if (d_range_selector)
        return d_range_selector->selectedCurve()->title().text();
    else
        return {};
}

bool Graph::markerSelected()
{
    return (selectedMarker >= 0);
}

void Graph::removeMarker()
{
    if (selectedMarker >= 0) {
        if (d_markers_selector) {
            if (d_texts.contains(selectedMarker))
                d_markers_selector->removeAll(
                        dynamic_cast<Legend *>(d_plot->marker(selectedMarker)));
            else if (d_lines.contains(selectedMarker))
                d_markers_selector->removeAll(
                        dynamic_cast<ArrowMarker *>(d_plot->marker(selectedMarker)));
            else if (d_images.contains(selectedMarker))
                d_markers_selector->removeAll(
                        dynamic_cast<ImageMarker *>(d_plot->marker(selectedMarker)));
        }
        d_plot->removeMarker(selectedMarker);
        d_plot->replot();
        Q_EMIT modifiedGraph();

        if (selectedMarker == legendMarkerID)
            legendMarkerID = -1;

        if (d_lines.contains(selectedMarker)) {
            int index = d_lines.indexOf(selectedMarker);
            int last_line_marker = (int)d_lines.size() - 1;
            for (int i = index; i < last_line_marker; i++)
                d_lines[i] = d_lines[i + 1];
            d_lines.resize(last_line_marker);
        } else if (d_texts.contains(selectedMarker)) {
            int index = d_texts.indexOf(selectedMarker);
            int last_text_marker = d_texts.size() - 1;
            for (int i = index; i < last_text_marker; i++)
                d_texts[i] = d_texts[i + 1];
            d_texts.resize(last_text_marker);
        } else if (d_images.contains(selectedMarker)) {
            int index = d_images.indexOf(selectedMarker);
            int last_image_marker = d_images.size() - 1;
            for (int i = index; i < last_image_marker; i++)
                d_images[i] = d_images[i + 1];
            d_images.resize(last_image_marker);
        }
        selectedMarker = -1;
    }
}

void Graph::cutMarker()
{
    copyMarker();
    removeMarker();
}

bool Graph::arrowMarkerSelected()
{
    return (d_lines.contains(selectedMarker));
}

bool Graph::imageMarkerSelected()
{
    return (d_images.contains(selectedMarker));
}

void Graph::copyMarker()
{
    if (selectedMarker < 0) {
        selectedMarkerType = None;
        return;
    }

    if (d_lines.contains(selectedMarker)) {
        auto *mrkL = dynamic_cast<ArrowMarker *>(d_plot->marker(selectedMarker));
        auxMrkStart = mrkL->startPoint();
        auxMrkEnd = mrkL->endPoint();
        selectedMarkerType = Arrow;
    } else if (d_images.contains(selectedMarker)) {
        auto *mrkI = dynamic_cast<ImageMarker *>(d_plot->marker(selectedMarker));
        auxMrkStart = mrkI->origin();
        QRect rect = mrkI->rect();
        auxMrkEnd = rect.bottomRight();
        auxMrkFileName = mrkI->fileName();
        selectedMarkerType = Image;
    } else
        selectedMarkerType = Text;
}

void Graph::pasteMarker()
{
    if (selectedMarkerType == Arrow) {
        auto *mrkL = new ArrowMarker();
        int linesOnPlot = (int)d_lines.size();
        d_lines.resize(++linesOnPlot);
        d_lines[linesOnPlot - 1] = d_plot->insertMarker(mrkL);

        mrkL->setColor(auxMrkColor);
        mrkL->setWidth(auxMrkWidth);
        mrkL->setStyle(auxMrkStyle);
        mrkL->setStartPoint(QPoint(auxMrkStart.x() + 10, auxMrkStart.y() + 10));
        mrkL->setEndPoint(QPoint(auxMrkEnd.x() + 10, auxMrkEnd.y() + 10));
        mrkL->drawStartArrow(startArrowOn);
        mrkL->drawEndArrow(endArrowOn);
        mrkL->setHeadLength(auxArrowHeadLength);
        mrkL->setHeadAngle(auxArrowHeadAngle);
        mrkL->fillArrowHead(auxFilledArrowHead);
    } else if (selectedMarkerType == Image) {
        auto *mrk = new ImageMarker(auxMrkFileName);
        int imagesOnPlot = d_images.size();
        d_images.resize(++imagesOnPlot);
        d_images[imagesOnPlot - 1] = d_plot->insertMarker(mrk);

        QPoint o = d_plot->canvas()->mapFromGlobal(QCursor::pos());
        if (!d_plot->canvas()->contentsRect().contains(o))
            o = QPoint(auxMrkStart.x() + 20, auxMrkStart.y() + 20);
        mrk->setOrigin(o);
        mrk->setSize(QRect(auxMrkStart, auxMrkEnd).size());
    } else {
        auto *mrk = new Legend(d_plot);
        int texts = d_texts.size();
        d_texts.resize(++texts);
        d_texts[texts - 1] = d_plot->insertMarker(mrk);

        QPoint o = d_plot->canvas()->mapFromGlobal(QCursor::pos());
        if (!d_plot->canvas()->contentsRect().contains(o))
            o = QPoint(auxMrkStart.x() + 20, auxMrkStart.y() + 20);
        mrk->setOrigin(o);
        mrk->setAngle(auxMrkAngle);
        mrk->setFrameStyle(auxMrkBkg);
        mrk->setFont(auxMrkFont);
        mrk->setText(auxMrkText);
        mrk->setTextColor(auxMrkColor);
        mrk->setBackgroundColor(auxMrkBkgColor);
    }

    d_plot->replot();
    deselectMarker();
}

void Graph::setCopiedMarkerEnds(const QPoint &start, const QPoint &end)
{
    auxMrkStart = start;
    auxMrkEnd = end;
}

void Graph::setCopiedTextOptions(int bkg, const QString &text, const QFont &font,
                                 const QColor &color, const QColor &bkgColor)
{
    auxMrkBkg = bkg;
    auxMrkText = text;
    auxMrkFont = font;
    auxMrkColor = color;
    auxMrkBkgColor = bkgColor;
}

void Graph::setCopiedArrowOptions(int width, Qt::PenStyle style, const QColor &color, bool start,
                                  bool end, int headLength, int headAngle, bool filledHead)
{
    auxMrkWidth = width;
    auxMrkStyle = style;
    auxMrkColor = color;
    startArrowOn = start;
    endArrowOn = end;
    auxArrowHeadLength = headLength;
    auxArrowHeadAngle = headAngle;
    auxFilledArrowHead = filledHead;
}

bool Graph::titleSelected()
{
    return d_plot->titleLabel()->hasFocus();
}

void Graph::selectTitle()
{
    if (!d_plot->hasFocus()) {
        Q_EMIT selectedGraph(this);
        QwtTextLabel *title = d_plot->titleLabel();
        title->setFocus();
    }

    deselectMarker();
}

void Graph::setTitle(const QString &t)
{
    d_plot->setTitle(t);
    Q_EMIT modifiedGraph();
}

void Graph::removeTitle()
{
    if (d_plot->titleLabel()->hasFocus()) {
        d_plot->setTitle(" ");
        Q_EMIT modifiedGraph();
    }
}

void Graph::initTitle(bool on, const QFont &fnt)
{
    if (on) {
        QwtText t = d_plot->title();
        t.setFont(fnt);
        t.setText(tr("Title"));
        d_plot->setTitle(t);
    }
}

void Graph::removeLegend()
{
    if (legendMarkerID >= 0) {
        int index = d_texts.indexOf(legendMarkerID);
        int texts = d_texts.size();
        for (int i = index; i < texts - 1; i++)
            d_texts[i] = d_texts[i + 1];
        d_texts.resize(--texts);

        d_plot->removeMarker(legendMarkerID);
        legendMarkerID = -1;
    }
}

void Graph::updateImageMarker(int x, int y, int w, int h)
{
    auto *mrk = dynamic_cast<ImageMarker *>(d_plot->marker(selectedMarker));
    mrk->setRect(x, y, w, h);
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::updateTextMarker(const QString &text, int angle, int bkg, const QFont &fnt,
                             const QColor &textColor, const QColor &backgroundColor)
{
    auto *mrkL = dynamic_cast<Legend *>(d_plot->marker(selectedMarker));
    mrkL->setText(text);
    mrkL->setAngle(angle);
    mrkL->setTextColor(textColor);
    mrkL->setBackgroundColor(backgroundColor);
    mrkL->setFont(fnt);
    mrkL->setFrameStyle(bkg);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

Legend *Graph::legend()
{
    if (legendMarkerID >= 0)
        return dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
    else
        return nullptr;
}

QString Graph::legendText()
{
    QString text = "";
    for (int i = 0; i < n_curves; i++) {
        const QwtPlotCurve *c = curve(i);
        if (c && c->rtti() != QwtPlotItem::Rtti_PlotSpectrogram && c_type[i] != ErrorBars) {
            text += "\\c{";
            text += QString::number(i + 1);
            text += "}";
            const auto *fc = dynamic_cast<const FunctionCurve *>(c);
            if (fc)
                text += fc->legend();
            else
                text += c->title().text();
            text += "\n";
        }
    }
    return text;
}

QString Graph::pieLegendText()
{
    QString text = "";
    QList<int> keys = d_plot->curveKeys();
    const QwtPlotCurve *curve = (QwtPlotCurve *)d_plot->curve(keys[0]);
    if (curve) {
        for (int i = 0; i < int(curve->dataSize()); i++) {
            text += "\\p{";
            text += QString::number(i + 1);
            text += "} ";
            text += QString::number(i + 1);
            text += "\n";
        }
    }
    return text;
}

void Graph::updateCurvesData(Table *w, const QString &colName)
{
    QList<int> keys = d_plot->curveKeys();
    int updated_curves = 0;
    QList<PlotCurve *> to_remove; // curves which changed too much to be kept
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if (dynamic_cast<PlotCurve *>(it)->type() == Function)
            continue;
        auto *c = dynamic_cast<DataCurve *>(it);
        if (c->table() == w && (c->xColumnName() == colName || c->yColumnName() == colName)) {
            auto colType = w->column(colName)->columnMode();
            AxisType atype;
            if (c->xColumnName() == colName) {
                if (c->type() == HorizontalBars)
                    atype = (AxisType)axisType[QwtPlot::yLeft];
                else
                    atype = (AxisType)axisType[QwtPlot::xBottom];
            } else {
                if (c->type() == HorizontalBars)
                    atype = (AxisType)axisType[QwtPlot::xBottom];
                else
                    atype = (AxisType)axisType[QwtPlot::yLeft];
            }
            // compare setLabels*Format() calls in insertCurve()
            if ((colType == Makhber::ColumnMode::Text && atype != AxisType::Txt)
                || (colType == Makhber::ColumnMode::DateTime
                    && ((atype != AxisType::Time) && (atype != AxisType::Date)
                        && (atype != AxisType::DateTime))))
                to_remove << c;
            else if (c->updateData(w, colName))
                updated_curves++;
        } else if ((dynamic_cast<DataCurve *>(it))->updateData(w, colName))
            updated_curves++;
    }
    for (PlotCurve *c : to_remove) {
        removeCurve(curveIndex(c));
        updated_curves++;
    }
    if (updated_curves) {
        if (isPiePlot())
            updatePlot();
        else {
            if (m_autoscale) {
                for (int i = 0; i < QwtPlot::axisCnt; i++)
                    d_plot->setAxisAutoScale(i);
            }
            d_plot->replot();
        }
    }
}

QJsonArray Graph::saveEnabledAxes()
{
    QJsonArray jsEnabledAxes {};
    for (int i = 0; i < QwtPlot::axisCnt; i++)
        jsEnabledAxes.append(d_plot->axisEnabled(i));

    return jsEnabledAxes;
}

bool Graph::framed()
{
    return d_plot->lineWidth() > 0;
}

QColor Graph::canvasFrameColor()
{
    auto *canvas = (QwtPlotCanvas *)d_plot->canvas();
    QPalette pal = canvas->palette();
    return pal.color(QPalette::Active, QPalette::WindowText);
}

int Graph::canvasFrameWidth()
{
    auto *canvas = (QwtPlotCanvas *)d_plot->canvas();
    return canvas->lineWidth();
}

void Graph::drawCanvasFrame(const QStringList &frame)
{
    d_plot->setLineWidth((frame[1]).toInt());
    if ((frame[1]).toInt())
        d_plot->setFrameStyle(QFrame::Box);

    QPalette pal = d_plot->palette();
    pal.setColor(QPalette::WindowText, QColor(COLORVALUE(frame[2])));
    d_plot->setPalette(pal);
}

void Graph::drawCanvasFrame(bool frameOn, int width, const QColor &color)
{
    QPalette pal = d_plot->palette();

    if (frameOn && d_plot->lineWidth() == width
        && pal.color(QPalette::Active, QPalette::WindowText) == color)
        return;

    if (frameOn) {
        d_plot->setLineWidth(width);
        pal.setColor(QPalette::WindowText, color);
        d_plot->setPalette(pal);
    } else {
        d_plot->setLineWidth(0);
        pal.setColor(QPalette::WindowText, QColor(Qt::black));
        d_plot->setPalette(pal);
    }
    Q_EMIT modifiedGraph();
}

void Graph::drawCanvasFrame(bool frameOn, int width)
{
    if (frameOn) {
        d_plot->setLineWidth(width);
    }
}

void Graph::drawAxesBackbones(bool yes)
{
    if (drawAxesBackbone == yes)
        return;

    drawAxesBackbone = yes;

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            auto *sclDraw = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(i));
            sclDraw->enableComponent(QwtAbstractScaleDraw::Backbone, yes);
            scale->repaint();
        }
    }

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::loadAxesOptions(const QString &s)
{
    if (s == "1")
        return;

    drawAxesBackbone = false;

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            auto *sclDraw = dynamic_cast<ScaleDraw *>(d_plot->axisScaleDraw(i));
            sclDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
            scale->repaint();
        }
    }
}

void Graph::setAxesLinewidth(int width)
{
    if (d_plot->axesLinewidth() == width)
        return;

    d_plot->setAxesLinewidth(); // width);

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            // scale->setPenWidth(width);
            scale->repaint();
        }
    }

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::loadAxesLinewidth() // int width)
{
    d_plot->setAxesLinewidth(); // width);
}

QJsonObject Graph::saveCanvas()
{
    QJsonObject jsCanvas {};
    /*int w = d_plot->canvas()->lineWidth();
    if (w > 0) {
        s += "CanvasFrame\t" + QString::number(w) + "\t";
        s += COLORNAME(canvasFrameColor()) + "\n";
    }*/
    jsCanvas.insert("canvasBackground", COLORNAME(d_plot->canvasBackground().color()));
    jsCanvas.insert("alpha", d_plot->canvasBackground().color().alpha());
    return jsCanvas;
}

QJsonObject Graph::saveFonts()
{
    QJsonObject jsFonts {};
    QFont f;

    QJsonObject jsTitle {};
    f = d_plot->title().font();
    jsTitle.insert("family", f.family());
    jsTitle.insert("pointSize", f.pointSize());
    jsTitle.insert("weight", f.weight());
    jsTitle.insert("italic", f.italic());
    jsTitle.insert("underline", f.underline());
    jsTitle.insert("strikeout", f.strikeOut());
    jsFonts.insert("titleFont", jsTitle);

    QJsonArray jsScaleFonts {};
    for (int i = 0; i < d_plot->axisCnt; i++) {
        f = d_plot->axisTitle(i).font();
        QJsonObject jsScale {};
        jsScale.insert("family", f.family());
        jsScale.insert("pointSize", f.pointSize());
        jsScale.insert("weight", f.weight());
        jsScale.insert("italic", f.italic());
        jsScale.insert("underline", f.underline());
        jsScale.insert("strikeout", f.strikeOut());
        jsScaleFonts.append(jsScale);
    }
    jsFonts.insert("scaleFonts", jsScaleFonts);

    QJsonArray jsAxesFonts {};
    for (int i = 0; i < d_plot->axisCnt; i++) {
        f = d_plot->axisFont(i);
        QJsonObject jsAxes {};
        jsAxes.insert("family", f.family());
        jsAxes.insert("pointSize", f.pointSize());
        jsAxes.insert("weight", f.weight());
        jsAxes.insert("italic", f.italic());
        jsAxes.insert("underline", f.underline());
        jsAxes.insert("strikeout", f.strikeOut());
        jsAxesFonts.append(jsAxes);
    }
    jsFonts.insert("axesFonts", jsAxesFonts);

    return jsFonts;
}

QJsonArray Graph::saveAxesFormulas()
{
    QJsonArray jsAxesFormulas {};
    for (int i = 0; i < 4; i++)
        jsAxesFormulas.append(axesFormulas[i]);
    return jsAxesFormulas;
}

QJsonArray Graph::saveScale()
{
    QJsonArray jsSaveScale {};
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        QJsonObject jsScale {};

        jsScale.insert("axis", i);

        const QwtScaleDiv scDiv = d_plot->axisScaleDiv(i);

        jsScale.insert("minBound", qMin(scDiv.lowerBound(), scDiv.upperBound()));
        jsScale.insert("maxBound", qMax(scDiv.lowerBound(), scDiv.upperBound()));
        jsScale.insert("step", d_user_step[i]);
        jsScale.insert("maxMajor", d_plot->axisMaxMajor(i));
        jsScale.insert("maxMinor", d_plot->axisMaxMinor(i));

        const QwtScaleEngine *sc_eng = d_plot->axisScaleEngine(i);
        // QwtTransform *tr = sc_eng->transformation();
        // s += QString::number((int)tr->type()) + "\t";
        jsScale.insert("transformation", 0);
        jsScale.insert("inverted", sc_eng->testAttribute(QwtScaleEngine::Inverted));
        jsSaveScale.append(jsScale);
    }

    return jsSaveScale;
}

void Graph::setXAxisTitleColor(const QColor &c)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(QwtPlot::xBottom);
    if (scale) {
        QwtText t = scale->title();
        t.setColor(c);
        scale->setTitle(t);
        Q_EMIT modifiedGraph();
    }
}

void Graph::setYAxisTitleColor(const QColor &c)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(QwtPlot::yLeft);
    if (scale) {
        QwtText t = scale->title();
        t.setColor(c);
        scale->setTitle(t);
        Q_EMIT modifiedGraph();
    }
}

void Graph::setRightAxisTitleColor(const QColor &c)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(QwtPlot::yRight);
    if (scale) {
        QwtText t = scale->title();
        t.setColor(c);
        scale->setTitle(t);
        Q_EMIT modifiedGraph();
    }
}

void Graph::setTopAxisTitleColor(const QColor &c)
{
    auto *scale = (QwtScaleWidget *)d_plot->axisWidget(QwtPlot::xTop);
    if (scale) {
        QwtText t = scale->title();
        t.setColor(c);
        scale->setTitle(t);
        Q_EMIT modifiedGraph();
    }
}

void Graph::setAxesTitleColor(QJsonArray *jsColors)
{
    for (int i = 0; i < jsColors->size(); i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        if (scale) {
            QwtText title = scale->title();
            title.setColor(QColor(COLORVALUE(jsColors->at(i).toString())));
            scale->setTitle(title);
        }
    }
}

QJsonArray Graph::saveAxesTitleColors()
{
    QJsonArray jsColors {};
    for (int i = 0; i < 4; i++) {
        auto *scale = (QwtScaleWidget *)d_plot->axisWidget(i);
        QColor c;
        if (scale)
            c = scale->title().color();
        else
            c = QColor(Qt::black);

        jsColors.append(COLORNAME(c));
    }
    return jsColors;
}

QJsonObject Graph::saveTitle()
{
    QJsonObject jsTitle {};
    jsTitle.insert("text", d_plot->title().text());
    jsTitle.insert("color", COLORNAME(d_plot->title().color()));
    jsTitle.insert("renderFlags", d_plot->title().renderFlags());
    return jsTitle;
}

QJsonArray Graph::saveScaleTitles()
{
    QJsonArray jsScaleTitles {};
    int a {};
    for (int i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            a = 2;
            break;
        case 1:
            a = 0;
            break;
        case 2:
            a = 3;
            break;
        case 3:
            a = 1;
            break;
        }
        QString title = d_plot->axisTitle(a).text();
        jsScaleTitles.append(title);
    }
    return jsScaleTitles;
}

QJsonObject Graph::saveAxesTitleAlignement()
{
    QJsonObject jsAlignment {};

    QJsonArray jsAlignHCenter {};
    QStringList axes;
    for (int i = 0; i < 4; i++)
        jsAlignHCenter.append(Qt::AlignHCenter);
    jsAlignment.insert("alignHCenter", jsAlignHCenter);

    QJsonArray jsFlags {};
    for (int i = 0; i < 4; i++) {
        if (d_plot->axisEnabled(i)) {
            jsFlags.append(d_plot->axisTitle(i).renderFlags());
        } else {
            jsFlags.append(0);
        }
    }
    jsAlignment.insert("renderFlags", jsFlags);

    return jsAlignment;
}

QJsonObject Graph::savePieCurveLayout()
{
    QJsonObject jsPieCurve {};
    jsPieCurve.insert("curveType", "Pie");

    auto *pieCurve = dynamic_cast<QwtPieCurve *>(curve(0));
    jsPieCurve.insert("title", pieCurve->title().text());

    QJsonObject jsPen {};
    QPen pen = pieCurve->pen();
    jsPen.insert("width", pen.width());
    jsPen.insert("color", COLORNAME(pen.color()));
    jsPen.insert("styleName", penStyleName(pen.style()));
    jsPieCurve.insert("pen", jsPen);

    Qt::BrushStyle pattern = pieCurve->pattern();
    int index = 0;
    if (pattern == Qt::SolidPattern)
        index = 0;
    else if (pattern == Qt::HorPattern)
        index = 1;
    else if (pattern == Qt::VerPattern)
        index = 2;
    else if (pattern == Qt::CrossPattern)
        index = 3;
    else if (pattern == Qt::BDiagPattern)
        index = 4;
    else if (pattern == Qt::FDiagPattern)
        index = 5;
    else if (pattern == Qt::DiagCrossPattern)
        index = 6;
    else if (pattern == Qt::Dense1Pattern)
        index = 7;
    else if (pattern == Qt::Dense2Pattern)
        index = 8;
    else if (pattern == Qt::Dense3Pattern)
        index = 9;
    else if (pattern == Qt::Dense4Pattern)
        index = 10;
    else if (pattern == Qt::Dense5Pattern)
        index = 11;
    else if (pattern == Qt::Dense6Pattern)
        index = 12;
    else if (pattern == Qt::Dense7Pattern)
        index = 13;
    else
        throw std::runtime_error("pattern out of range");

    jsPieCurve.insert("pattern", index);
    jsPieCurve.insert("ray", pieCurve->ray());
    jsPieCurve.insert("firstColor", pieCurve->firstColor());
    jsPieCurve.insert("startRow", pieCurve->startRow());
    jsPieCurve.insert("endRow", pieCurve->endRow());
    jsPieCurve.insert("visible", pieCurve->isVisible());

    return jsPieCurve;
}

QJsonObject Graph::saveCurveLayout(int index)
{
    QJsonObject jsCurveLayout {};
    int style = c_type[index];
    auto *c = (QwtPlotCurve *)curve(index);
    if (!c)
        return jsCurveLayout;

    jsCurveLayout.insert("style", style);
    if (style == Spline)
        jsCurveLayout.insert("curveStyle", 4);
    else if (style == VerticalSteps)
        jsCurveLayout.insert("curveStyle", 5);
    else
        jsCurveLayout.insert("curveStyle", c->style());
    QJsonObject jsPen {};
    jsPen.insert("color", COLORNAME(c->pen().color()));
    jsPen.insert("style", c->pen().style() - 1);
    jsPen.insert("width", c->pen().width());
    jsCurveLayout.insert("pen", jsPen);

    const QwtSymbol *symbol = c->symbol();
    QJsonObject jsSymbol {};
    jsSymbol.insert("width", symbol->size().width());
    jsSymbol.insert("style", SymbolBox::symbolIndex(symbol->style()));
    jsSymbol.insert("penColor", COLORNAME(symbol->pen().color()));
    if (symbol->brush().style() != Qt::NoBrush)
        jsSymbol.insert("brushColor", COLORNAME(symbol->brush().color()));
    else
        jsSymbol.insert("brushColor", "");
    jsCurveLayout.insert("symbol", jsSymbol);

    bool filled = c->brush().style() == Qt::NoBrush ? false : true;
    jsCurveLayout.insert("filled", filled);

    jsCurveLayout.insert("brushColor", COLORNAME(c->brush().color()));
    jsCurveLayout.insert("brushStyle", PatternBox::patternIndex(c->brush().style()));
    if (style <= LineSymbols || style == Box)
        jsCurveLayout.insert("penWidth", symbol->pen().width());
    // extension for custom dash pattern
    jsCurveLayout.insert("capStyle", static_cast<int>(c->pen().capStyle()));
    jsCurveLayout.insert("joinStyle", static_cast<int>(c->pen().joinStyle()));
    QJsonArray jsCustomDash {};
    for (auto v : c->pen().dashPattern())
        jsCustomDash.append(v);
    jsCurveLayout.insert("customDash", jsCustomDash);

    if (style == VerticalBars || style == HorizontalBars || style == Histogram) {
        auto *b = dynamic_cast<QwtBarCurve *>(c);
        jsCurveLayout.insert("gap", b->gap());
        jsCurveLayout.insert("offset", b->offset());
    }

    if (style == Histogram) {
        auto *h = dynamic_cast<QwtHistogram *>(c);
        jsCurveLayout.insert("autoBinning", h->autoBinning());
        jsCurveLayout.insert("binSize", h->binSize());
        jsCurveLayout.insert("begin", h->begin());
        jsCurveLayout.insert("end", h->end());
    } else if (style == VectXYXY || style == VectXYAM) {
        auto *v = dynamic_cast<VectorCurve *>(c);
        jsCurveLayout.insert("vectorColor", COLORNAME(v->color()));
        jsCurveLayout.insert("vectorWidth", v->width());
        jsCurveLayout.insert("headLength", v->headLength());
        jsCurveLayout.insert("headAngle", v->headAngle());
        jsCurveLayout.insert("filledArrowHead", v->filledArrowHead());

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList colsList = v->plotAssociation().split(",", Qt::SkipEmptyParts);
#else
        QStringList colsList = v->plotAssociation().split(",", QString::SkipEmptyParts);
#endif
        jsCurveLayout.insert("X_A", colsList[2].remove("(X)").remove("(A)"));
        jsCurveLayout.insert("Y_M", colsList[3].remove("(Y)").remove("(M)"));
        if (style == VectXYAM)
            jsCurveLayout.insert("position", v->position());
    } else if (style == Box) {
        auto *b = dynamic_cast<BoxCurve *>(c);
        jsCurveLayout.insert("xValue", b->sample(0).x());
        jsCurveLayout.insert("maxStyle", SymbolBox::symbolIndex(b->maxStyle()));
        jsCurveLayout.insert("p99Style", SymbolBox::symbolIndex(b->p99Style()));
        jsCurveLayout.insert("meanStyle", SymbolBox::symbolIndex(b->meanStyle()));
        jsCurveLayout.insert("p1Style", SymbolBox::symbolIndex(b->p1Style()));
        jsCurveLayout.insert("minStyle", SymbolBox::symbolIndex(b->minStyle()));
        jsCurveLayout.insert("boxStyle", b->boxStyle());
        jsCurveLayout.insert("boxWidth", b->boxWidth());
        jsCurveLayout.insert("boxRangeType", b->boxRangeType());
        jsCurveLayout.insert("boxRange", b->boxRange());
        jsCurveLayout.insert("whiskersRangeType", b->whiskersRangeType());
        jsCurveLayout.insert("whiskersRange", b->whiskersRange());
    }

    return jsCurveLayout;
}

void Graph::saveCurves(QJsonObject *jsObject)
{
    QJsonArray jsCurves {};
    if (isPiePlot())
        jsCurves.append(savePieCurveLayout());
    else {
        for (int i = 0; i < n_curves; i++) {
            QwtPlotItem *it = plotItem(i);
            QJsonObject jsCurve {};
            if (!it)
                continue;

            if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) {
                (dynamic_cast<Spectrogram *>(it))->saveToJson(&jsCurve);
                continue;
            }

            auto *c = dynamic_cast<PlotCurve *>(it);
            if (c->type() != ErrorBars) {
                jsCurve.insert("title", c->title().text());
                if (c->type() == Function) {
                    jsCurve.insert("curveType", "Function");
                    jsCurve.insert("functionType",
                                   dynamic_cast<FunctionCurve *>(c)->functionType());
                    jsCurve.insert("variable", dynamic_cast<FunctionCurve *>(c)->variable());
                    jsCurve.insert("startRange", dynamic_cast<FunctionCurve *>(c)->startRange());
                    jsCurve.insert("endRange", dynamic_cast<FunctionCurve *>(c)->endRange());
                    jsCurve.insert("dataSize",
                                   static_cast<int>(dynamic_cast<FunctionCurve *>(c)->dataSize()));
                    jsCurve.insert("formulas",
                                   QJsonArray::fromStringList(
                                           dynamic_cast<FunctionCurve *>(c)->formulas()));
                } else if (c->type() == Box) {
                    jsCurve.insert("curveType", "Box");
                    jsCurve.insert("x", c->data()->sample(0).x());
                } else {
                    jsCurve.insert("curveType", "Data");
                    jsCurve.insert("xColumnName", dynamic_cast<DataCurve *>(c)->xColumnName());
                    jsCurve.insert("startRow", dynamic_cast<DataCurve *>(c)->startRow());
                    jsCurve.insert("endRow", dynamic_cast<DataCurve *>(c)->endRow());
                }

                jsCurve.insert("curveLayout", saveCurveLayout(i));
                jsCurve.insert("xAxis", c->xAxis());
                jsCurve.insert("yAxis", c->yAxis());
                jsCurve.insert("visible", c->isVisible());
            } else if (c->type() == ErrorBars) {
                auto *er = dynamic_cast<QwtErrorPlotCurve *>(it);
                jsCurve.insert("curveType", "ErrorBars");
                jsCurve.insert("direction", er->direction());
                jsCurve.insert("masterXCoulumn", er->masterCurve()->xColumnName());
                jsCurve.insert("masterTitle", er->masterCurve()->title().text());
                jsCurve.insert("title", er->title().text());
                jsCurve.insert("width", er->width());
                jsCurve.insert("capLength", er->capLength());
                jsCurve.insert("color", COLORNAME(er->color()));
                jsCurve.insert("throughSymbol", er->throughSymbol());
                jsCurve.insert("plusSide", er->plusSide());
                jsCurve.insert("minusSide", er->minusSide());
            }
            jsCurves.append(jsCurve);
        }
    }
    jsObject->insert("curves", jsCurves);
}

Legend *Graph::newLegend()
{
    auto *mrk = new Legend(d_plot);
    mrk->setOrigin(QPoint(10, 10));

    if (isPiePlot())
        mrk->setText(pieLegendText());
    else
        mrk->setText(legendText());

    mrk->setFrameStyle(defaultMarkerFrame);
    mrk->setFont(defaultMarkerFont);
    mrk->setTextColor(defaultTextMarkerColor);
    mrk->setBackgroundColor(defaultTextMarkerBackground);

    legendMarkerID = d_plot->insertMarker(mrk);
    int texts = d_texts.size();
    d_texts.resize(++texts);
    d_texts[texts - 1] = legendMarkerID;

    Q_EMIT modifiedGraph();
    d_plot->replot();
    return mrk;
}

void Graph::addTimeStamp()
{
    Legend *mrk = newLegend(QLocale().toString(QDateTime::currentDateTime()));
    mrk->setOrigin(QPoint(d_plot->canvas()->width() / 2, 10));
    Q_EMIT modifiedGraph();
    d_plot->replot();
}

Legend *Graph::newLegend(const QString &text)
{
    auto *mrk = new Legend(d_plot);
    selectedMarker = d_plot->insertMarker(mrk);
    if (d_markers_selector)
        delete d_markers_selector;

    int texts = d_texts.size();
    d_texts.resize(++texts);
    d_texts[texts - 1] = selectedMarker;

    mrk->setOrigin(QPoint(5, 5));
    mrk->setText(text);
    mrk->setFrameStyle(defaultMarkerFrame);
    mrk->setFont(defaultMarkerFont);
    mrk->setTextColor(defaultTextMarkerColor);
    mrk->setBackgroundColor(defaultTextMarkerBackground);
    return mrk;
}

void Graph::insertLegend(QJsonObject *jsLegend)
{
    legendMarkerID = insertTextMarker(jsLegend);
}

long Graph::insertTextMarker(QJsonObject *jsText)
{
    auto *mrk = new Legend(d_plot);
    long key = d_plot->insertMarker(mrk);

    int texts = d_texts.size();
    d_texts.resize(++texts);
    d_texts[texts - 1] = key;

    mrk->setOriginCoord(jsText->value("xValue").toDouble(), jsText->value("yValue").toDouble());

    QJsonObject jsFont = jsText->value("font").toObject();
    QFont fnt = QFont(jsFont.value("family").toString(), jsFont.value("pointSize").toInt(),
                      jsFont.value("weight").toInt(), jsFont.value("italic").toInt());
    fnt.setUnderline(jsFont.value("underline").toInt());
    fnt.setStrikeOut(jsFont.value("strikeout").toInt());
    mrk->setFont(fnt);

    mrk->setAngle(jsText->value("angle").toInt());

    mrk->setTextColor(QColor(COLORVALUE(jsText->value("textColor").toString())));
    mrk->setFrameStyle(jsText->value("frameStyle").toInt());
    QColor c = QColor(COLORVALUE(jsText->value("backgroundColor").toString()));
    c.setAlpha(jsText->value("alpha").toInt());
    mrk->setBackgroundColor(c);

    QJsonArray jsTextList = jsText->value("textList").toArray();
    QString text = QString();
    for (int i = 0; i < jsTextList.size(); i++)
        text += jsTextList.at(i).toString() + "\n";
    mrk->setText(text.trimmed());
    return key;
}

void Graph::addArrow(QJsonObject *jsArrow)
{
    auto *mrk = new ArrowMarker();
    long mrkID = d_plot->insertMarker(mrk);
    int linesOnPlot = (int)d_lines.size();
    d_lines.resize(++linesOnPlot);
    d_lines[linesOnPlot - 1] = mrkID;

    mrk->setBoundingRect(jsArrow->value("xStart").toDouble(), jsArrow->value("yStart").toDouble(),
                         jsArrow->value("xEnd").toDouble(), jsArrow->value("yENd").toDouble());

    mrk->setWidth(jsArrow->value("width").toInt());
    mrk->setColor(QColor(COLORVALUE(jsArrow->value("color").toString())));
    mrk->setStyle(getPenStyle(jsArrow->value("style").toString()));
    mrk->drawEndArrow(jsArrow->value("hasEndArrow").toBool());
    mrk->drawStartArrow(jsArrow->value("hasStartArrow").toBool());
    mrk->setHeadLength(jsArrow->value("headLength").toInt());
    mrk->setHeadAngle(jsArrow->value("headAngle").toInt());
    mrk->fillArrowHead(jsArrow->value("filledArrowHead").toBool());
    mrk->setCapStyle(static_cast<Qt::PenCapStyle>(jsArrow->value("capStyle").toInt()));
    mrk->setJoinStyle(static_cast<Qt::PenJoinStyle>(jsArrow->value("joinStyle").toInt()));
    QJsonArray jsCustomDash = jsArrow->value("customDash").toArray();
    QVector<qreal> customDash;
    for (int i = 0; i < jsCustomDash.size(); i++)
        customDash << jsCustomDash.at(i).toDouble();
    QPen pen = mrk->linePen();
    if (mrk->style() == Qt::CustomDashLine)
        pen.setDashPattern(customDash);
    mrk->setLinePen(pen);
}

void Graph::addArrow(ArrowMarker *mrk)
{
    auto *aux = new ArrowMarker();
    int linesOnPlot = (int)d_lines.size();
    d_lines.resize(++linesOnPlot);
    d_lines[linesOnPlot - 1] = d_plot->insertMarker(aux);

    aux->setBoundingRect(mrk->startPointCoord().x(), mrk->startPointCoord().y(),
                         mrk->endPointCoord().x(), mrk->endPointCoord().y());
    aux->setWidth(mrk->width());
    aux->setColor(mrk->color());
    aux->setStyle(mrk->style());
    aux->drawEndArrow(mrk->hasEndArrow());
    aux->drawStartArrow(mrk->hasStartArrow());
    aux->setHeadLength(mrk->headLength());
    aux->setHeadAngle(mrk->headAngle());
    aux->fillArrowHead(mrk->filledArrowHead());
}

ArrowMarker *Graph::arrow(long id)
{
    return dynamic_cast<ArrowMarker *>(d_plot->marker(id));
}

ImageMarker *Graph::imageMarker(long id)
{
    return dynamic_cast<ImageMarker *>(d_plot->marker(id));
}

Legend *Graph::textMarker(long id)
{
    return dynamic_cast<Legend *>(d_plot->marker(id));
}

long Graph::insertTextMarker(Legend *mrk)
{
    auto *aux = new Legend(d_plot);
    selectedMarker = d_plot->insertMarker(aux);
    if (d_markers_selector)
        delete d_markers_selector;

    int texts = d_texts.size();
    d_texts.resize(++texts);
    d_texts[texts - 1] = selectedMarker;

    aux->setTextColor(mrk->textColor());
    aux->setBackgroundColor(mrk->backgroundColor());
    aux->setOriginCoord(mrk->xValue(), mrk->yValue());
    aux->setFont(mrk->font());
    aux->setFrameStyle(mrk->frameStyle());
    aux->setAngle(mrk->angle());
    aux->setText(mrk->text());
    return selectedMarker;
}

QJsonObject Graph::saveMarkers()
{
    QJsonObject jsMarkers {};

    QJsonArray jsImages {};
    for (int i = 0; i < d_images.size(); i++) {
        auto *mrkI = dynamic_cast<ImageMarker *>(d_plot->marker(d_images[i]));
        QJsonObject jsImage {};
        jsImage.insert("filename", mrkI->fileName());
        jsImage.insert("xValue", mrkI->xValue());
        jsImage.insert("yValue", mrkI->yValue());
        jsImage.insert("right", mrkI->right());
        jsImage.insert("bottom", mrkI->bottom());
        jsImages.append(jsImage);
    }
    jsMarkers.insert("images", jsImages);

    QJsonArray jsLines {};
    for (int i = 0; i < d_lines.size(); i++) {
        auto *mrkL = dynamic_cast<ArrowMarker *>(d_plot->marker(d_lines[i]));
        QJsonObject jsLine {};

        QPointF sp = mrkL->startPointCoord();
        jsLine.insert("xStart", sp.x());
        jsLine.insert("yStart", sp.y());

        QPointF ep = mrkL->endPointCoord();
        jsLine.insert("xEnd", ep.x());
        jsLine.insert("yEnd", ep.y());

        jsLine.insert("width", mrkL->width());
        jsLine.insert("color", COLORNAME(mrkL->color()));
        jsLine.insert("style", penStyleName(mrkL->style()));
        jsLine.insert("hasEndArrow", mrkL->hasEndArrow());
        jsLine.insert("hasStartArrow", mrkL->hasStartArrow());
        jsLine.insert("headLength", mrkL->headLength());
        jsLine.insert("headAngle", mrkL->headAngle());
        jsLine.insert("filledArrowHead", mrkL->filledArrowHead());
        jsLine.insert("capStyle", static_cast<int>(mrkL->capStyle()));
        jsLine.insert("joinStyle", static_cast<int>(mrkL->joinStyle()));
        QJsonArray jsCustomDash;
        for (auto v : mrkL->linePen().dashPattern())
            jsCustomDash.append(v);
        jsLine.insert("customDash", jsCustomDash);
        jsLines.append(jsLine);
    }
    jsMarkers.insert("lines", jsLines);

    QJsonArray jsTexts {};
    for (int i = 0; i < d_texts.size(); i++) {
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(d_texts[i]));
        QJsonObject jsText {};
        if (d_texts[i] != legendMarkerID)
            jsText.insert("type", "text");
        else
            jsText.insert("type", "legend");

        jsText.insert("xValue", mrk->xValue());
        jsText.insert("yValue", mrk->yValue());

        QFont f = mrk->font();
        QJsonObject jsFont {};
        jsFont.insert("family", f.family());
        jsFont.insert("pointSize", f.pointSize());
        jsFont.insert("weight", f.weight());
        jsFont.insert("italic", f.italic());
        jsFont.insert("underline", f.underline());
        jsFont.insert("strikeout", f.strikeOut());
        jsText.insert("font", jsFont);
        jsText.insert("textColor", COLORNAME(mrk->textColor()));
        jsText.insert("fromeStyle", mrk->frameStyle());
        jsText.insert("angle", mrk->angle());
        jsText.insert("backgroundColor", COLORNAME(mrk->backgroundColor()));
        jsText.insert("alpha", mrk->backgroundColor().alpha());
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList textList = mrk->text().split("\n", Qt::KeepEmptyParts);
#else
        QStringList textList = mrk->text().split("\n", QString::KeepEmptyParts);
#endif
        jsText.insert("textList", QJsonArray::fromStringList(textList));
        jsTexts.append(jsText);
    }
    jsMarkers.insert("texts", jsTexts);

    return jsMarkers;
}

double Graph::selectedXStartValue()
{
    if (d_range_selector)
        return d_range_selector->minXValue();
    else
        return 0;
}

double Graph::selectedXEndValue()
{
    if (d_range_selector)
        return d_range_selector->maxXValue();
    else
        return 0;
}

QwtPlotItem *Graph::plotItem(int index)
{
    if (!n_curves || index >= n_curves || index < 0)
        return nullptr;

    return d_plot->plotItem(c_keys[index]);
}

int Graph::plotItemIndex(QwtPlotItem *it) const
{
    for (int i = 0; i < n_curves; i++) {
        if (d_plot->plotItem(c_keys[i]) == it)
            return i;
    }
    return -1;
}

QwtPlotCurve *Graph::curve(int index) const
{
    if (!n_curves || index >= n_curves || index < 0)
        return nullptr;

    return d_plot->curve(c_keys[index]);
}

int Graph::curveIndex(QwtPlotCurve *c) const
{
    return plotItemIndex(c);
}

int Graph::range(int index, double *start, double *end)
{
    if (d_range_selector && d_range_selector->selectedCurve() == curve(index)) {
        *start = d_range_selector->minXValue();
        *end = d_range_selector->maxXValue();
        return d_range_selector->dataSize();
    } else {
        QwtPlotCurve *c = curve(index);
        if (!c)
            return 0;

        *start = c->data()->sample(0).x();
        *end = c->data()->sample(c->dataSize() - 1).x();
        return static_cast<int>(c->dataSize());
    }
}

CurveLayout Graph::initCurveLayout()
{
    CurveLayout cl;
    cl.connectType = 0;
    cl.lStyle = 0;
    cl.lWidth = 1;
    cl.sSize = 7;
    cl.sType = 0;
    cl.filledArea = 0;
    cl.aCol = 0;
    cl.aStyle = 0;
    cl.lCol = 0;
    cl.penWidth = 1;
    cl.symCol = 0;
    cl.fillCol = 0;
    return cl;
}

CurveLayout Graph::initCurveLayout(int style, int curves)
{
    int i = n_curves - 1;

    CurveLayout cl = initCurveLayout();
    int color = 0;
    guessUniqueCurveLayout(color, cl.sType);

    cl.lCol = color;
    cl.symCol = color;
    cl.fillCol = color;

    if (style == Graph::Line) {
        cl.connectType = QwtPlotCurve::Lines;
        cl.sType = 0;
    } else if (style == Graph::LineSymbols)
        cl.connectType = QwtPlotCurve::Lines;
    else if (style == Graph::Scatter)
        cl.connectType = QwtPlotCurve::NoCurve;
    else if (style == Graph::VerticalDropLines)
        cl.connectType = QwtPlotCurve::Sticks;
    else if (style == Graph::HorizontalSteps || style == Graph::VerticalSteps) {
        cl.connectType = QwtPlotCurve::Steps;
        cl.sType = 0;
    } else if (style == Graph::Spline)
        cl.connectType = 4;
    else if (curves && (style == Graph::VerticalBars || style == Graph::HorizontalBars)) {
        cl.connectType = QwtPlotCurve::UserCurve;
        cl.filledArea = 1;
        cl.lCol = 0; // black color pen
        cl.aCol = i + 1;
        cl.sType = 0;
        if (style == Graph::VerticalBars || style == Graph::HorizontalBars) {
            auto *b = dynamic_cast<QwtBarCurve *>(curve(i));
            if (b) {
                b->setGap(qRound(100 * (1 - 1.0 / (double)curves)));
                b->setOffset(-50 * (curves - 1) + i * 100);
            }
        }
    } else if (style == Graph::Histogram) {
        cl.filledArea = 1;
        cl.lCol = i + 1; // start with red color pen
        cl.aCol = i + 1; // start with red fill color
        cl.aStyle = 4;
        cl.sType = 0;
    } else if (style == Graph::Area) {
        cl.filledArea = 1;
        cl.aCol = color;
        cl.sType = 0;
    } else {
        cl.connectType = QwtPlotCurve::UserCurve;
        cl.sType = 0;
    }
    return cl;
}

bool Graph::canConvertTo(QwtPlotCurve *c, CurveType type)
{
    if (!c)
        return false;
    // conversion between VectXYXY and VectXYAM is possible
    if (dynamic_cast<VectorCurve *>(c))
        return type == VectXYXY || type == VectXYAM;
    // conversion between Pie, Histogram and Box should be possible (all of them take one input
    // column), but lots of special-casing in ApplicationWindow and Graph makes this very difficult
    if (dynamic_cast<QwtPieCurve *>(c) || dynamic_cast<QwtHistogram *>(c)
        || dynamic_cast<BoxCurve *>(c))
        return false;
    // converting error bars doesn't make sense
    if (dynamic_cast<QwtErrorPlotCurve *>(c))
        return false;
    // line/symbol, area and bar curves can be converted to each other
    if (dynamic_cast<DataCurve *>(c))
        return type == Line || type == Scatter || type == LineSymbols || type == VerticalBars
                || type == HorizontalBars || type == HorizontalSteps || type == VerticalSteps
                || type == Area || type == VerticalDropLines || type == Spline;
    return false;
}

void Graph::setCurveType(int curve_index, CurveType type, bool update)
{
    if (curve_index >= c_type.size())
        return;
    auto old_type = static_cast<CurveType>(c_type[curve_index]);
    // nothing changed
    if (type == old_type)
        return;
    // not all types can be modified cleanly
    if (type != Line && type != Scatter && type != LineSymbols && type != VerticalBars
        && type != Area && type != VerticalDropLines && type != Spline && type != HorizontalSteps
        && type != HorizontalBars && type != VerticalSteps && type != VectXYXY && type != VectXYAM)
        return;
    if (old_type != Line && old_type != Scatter && old_type != LineSymbols
        && old_type != VerticalBars && old_type != Area && old_type != VerticalDropLines
        && old_type != Spline && old_type != HorizontalSteps && old_type != HorizontalBars
        && old_type != VerticalSteps && old_type != VectXYXY && old_type != VectXYAM)
        return;

    if ((type == VerticalBars || type == HorizontalBars)
        != (old_type == VerticalBars || old_type == HorizontalBars)) {
        // bar curves are represented by a different class than the other curve types, which makes
        // changing from/to horizontal or vertical bars more difficult
        auto *old_curve = dynamic_cast<DataCurve *>(curve(curve_index));
        DataCurve *new_curve = nullptr;
        switch (type) {
        case Histogram:
        case Box:
        case Pie:
            return;
        case VerticalBars:
            new_curve = new QwtBarCurve(QwtBarCurve::Vertical, old_curve->table(),
                                        old_curve->xColumnName(), old_curve->yColumnName(),
                                        old_curve->startRow(), old_curve->endRow());
            new_curve->setStyle(QwtPlotCurve::UserCurve);
            break;
        case HorizontalBars:
            new_curve = new QwtBarCurve(QwtBarCurve::Horizontal, old_curve->table(),
                                        old_curve->xColumnName(), old_curve->yColumnName(),
                                        old_curve->startRow(), old_curve->endRow());
            new_curve->setStyle(QwtPlotCurve::UserCurve);
            break;
        default:
            new_curve = new DataCurve(old_curve->table(), old_curve->xColumnName(),
                                      old_curve->yColumnName(), old_curve->startRow(),
                                      old_curve->endRow());
            break;
        }

        old_curve->clearErrorBars();
        if (int i = d_fit_curves.indexOf(old_curve) >= 0)
            d_fit_curves.replace(i, new_curve);

        d_plot->removeCurve(c_keys[curve_index]);
        c_keys[curve_index] = d_plot->insertCurve(new_curve);
        new_curve->loadData();

        if (d_range_selector && old_curve == d_range_selector->selectedCurve())
            d_range_selector->setSelectedCurve(new_curve);
    }

    c_type[curve_index] = type;
    dynamic_cast<DataCurve *>(curve(curve_index))->setType(type);
    if (!update)
        return;

    CurveLayout cl = initCurveLayout(type, 1);
    updateCurveLayout(curve_index, &cl);
    updatePlot();
}

void Graph::updateCurveLayout(int index, const CurveLayout *cL)
{
    auto *c = dynamic_cast<DataCurve *>(this->curve(index));
    if (!c)
        return;
    if (c_type.isEmpty() || (int)c_type.size() < index)
        return;

    QPen pen = QPen(ColorButton::color(cL->symCol), cL->penWidth, Qt::SolidLine);
    if (cL->symbolFill)
        c->setSymbol(new QwtSymbol(SymbolBox::style(cL->sType),
                                   QBrush(ColorButton::color(cL->fillCol)), pen,
                                   QSize(cL->sSize, cL->sSize)));
    else
        c->setSymbol(new QwtSymbol(SymbolBox::style(cL->sType), QBrush(), pen,
                                   QSize(cL->sSize, cL->sSize)));

    // custom dash pattern
    pen = QPen(ColorButton::color(cL->lCol), cL->lWidth, getPenStyle(cL->lStyle));
    pen.setCapStyle(static_cast<Qt::PenCapStyle>(cL->lCapStyle));
    pen.setJoinStyle(static_cast<Qt::PenJoinStyle>(cL->lJoinStyle));
    QVector<qreal> customDash;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    for (auto v : (cL->lCustomDash).split(" ", Qt::SkipEmptyParts))
#else
    for (auto v : (cL->lCustomDash).split(" ", QString::SkipEmptyParts))
#endif
        customDash << v.toDouble();
    if (pen.style() == Qt::CustomDashLine)
        pen.setDashPattern(customDash);
    c->setPen(pen);

    int style = c_type[index];
    if (style == Scatter)
        c->setStyle(QwtPlotCurve::NoCurve);
    else if (style == Spline) {
        c->setStyle(QwtPlotCurve::Lines);
        c->setCurveAttribute(QwtPlotCurve::Fitted, true);
    } else if (style == VerticalSteps) {
        c->setStyle(QwtPlotCurve::Steps);
        c->setCurveAttribute(QwtPlotCurve::Inverted, true);
    } else
        c->setStyle((QwtPlotCurve::CurveStyle)cL->connectType);

    QBrush brush = QBrush(ColorButton::color(cL->aCol));
    if (cL->filledArea)
        brush.setStyle(getBrushStyle(cL->aStyle));
    else
        brush.setStyle(Qt::NoBrush);
    c->setBrush(brush);
}

void Graph::updateErrorBars(QwtErrorPlotCurve *er, bool xErr, int width, int cap, const QColor &c,
                            bool plus, bool minus, bool through)
{
    if (!er)
        return;

    if (er->width() == width && er->capLength() == cap && er->color() == c && er->plusSide() == plus
        && er->minusSide() == minus && er->throughSymbol() == through && er->xErrors() == xErr)
        return;

    er->setWidth(width);
    er->setCapLength(cap);
    er->setColor(c);
    er->setXErrors(xErr);
    er->drawThroughSymbol(through);
    er->drawPlusSide(plus);
    er->drawMinusSide(minus);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

bool Graph::addErrorBars(const QString &yColName, Table *errTable, const QString &errColName,
                         Qt::Orientation type, int width, int cap, const QColor &color,
                         bool through, bool minus, bool plus)
{
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < n_curves; i++) {
        auto *c = dynamic_cast<DataCurve *>(d_plot->curve(keys[i]));
        if (c && c->title().text() == yColName && c_type[i] != ErrorBars) {
            return addErrorBars(c->xColumnName(), yColName, errTable, errColName, type, width, cap,
                                color, through, minus, plus);
        }
    }
    return false;
}

bool Graph::addErrorBars(const QString &xColName, const QString &yColName, Table *errTable,
                         const QString &errColName, Qt::Orientation type, int width, int cap,
                         const QColor &color, bool through, bool minus, bool plus)
{
    DataCurve *master_curve = masterCurve(xColName, yColName);
    if (!master_curve)
        return false;

    auto *er = new QwtErrorPlotCurve(type, errTable, errColName);
    er->setMasterCurve(master_curve);
    er->setCapLength(cap);
    er->setColor(color);
    er->setWidth(width);
    er->drawPlusSide(plus);
    er->drawMinusSide(minus);
    er->drawThroughSymbol(through);

    c_type.resize(++n_curves);
    c_type[n_curves - 1] = ErrorBars;

    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(er);

    updatePlot();
    return true;
}

void Graph::plotPie(Table *w, const QString &name, const QPen &pen, int brush, int size,
                    int firstColor, int startRow, int endRow, bool visible)
{
    if (endRow < 0)
        endRow = w->numRows() - 1;

    auto *pieCurve = new QwtPieCurve(w, name, startRow, endRow);
    pieCurve->loadData();
    pieCurve->setPen(pen);
    pieCurve->setRay(size);
    pieCurve->setFirstColor(firstColor);
    pieCurve->setBrushStyle(getBrushStyle(brush));
    pieCurve->setVisible(visible);

    c_keys.resize(++n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(pieCurve);

    c_type.resize(n_curves);
    c_type[n_curves - 1] = Pie;
}

void Graph::plotPie(Table *w, const QString &name, int startRow, int endRow)
{
    int ycol = w->colIndex(name);
    int size = 0;
    double sum = 0.0;

    Column *y_col_ptr = w->column(ycol);
    auto yColType = w->columnType(ycol);

    if (endRow < 0)
        endRow = w->numRows() - 1;

    for (int i = 0; i < QwtPlot::axisCnt; i++)
        d_plot->enableAxis(i, false);
    scalePicker->refresh();

    d_plot->setTitle(QString());

    static_cast<QwtPlotCanvas *>(d_plot->canvas())->setLineWidth(1);

    QVarLengthArray<double> Y(abs(endRow - startRow) + 1);
    for (int row = startRow; row <= endRow && row < y_col_ptr->rowCount(); row++) {
        if (!y_col_ptr->isInvalid(row)) {
            if (yColType == Makhber::ColumnMode::Text) {
                QString yval = y_col_ptr->textAt(row);
                bool valid_data = true;
                Y[size] = QLocale().toDouble(yval, &valid_data);
                if (!valid_data)
                    continue;
            } else
                Y[size] = y_col_ptr->valueAt(row);

            sum += Y[size];
            size++;
        }
    }
    if (!size)
        return;
    Y.resize(size);

    auto *pieCurve = new QwtPieCurve(w, name, startRow, endRow);
    pieCurve->setSamples(Y.data(), Y.data(), size);

    c_keys.resize(++n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(pieCurve);

    c_type.resize(n_curves);
    c_type[n_curves - 1] = Pie;

    // This has to be synced with QwtPieCurve::drawPie() for now... until we have a clean solution.
    QRectF canvas_rect = d_plot->plotLayout()->canvasRect();
    float radius = 0.45 * qMin(canvas_rect.width(), canvas_rect.height());

    double PI = 4 * atan(1.0);
    double angle = 90;

    for (int i = 0; i < size; i++) {
        const double value = Y[i] / sum * 360;
        double alabel = (angle - value * 0.5) * PI / 180.0;

        auto *aux = new Legend(d_plot);
        aux->setFrameStyle(0);
        aux->setText(QString::number(Y[i] / sum * 100, 'g', 2) + "%");

        int texts = d_texts.size();
        d_texts.resize(++texts);
        d_texts[texts - 1] = d_plot->insertMarker(aux);

        aux->setOrigin(canvas_rect.center().toPoint()
                       + QPoint(int(radius * cos(alabel) - 0.5 * aux->rect().width()),
                                int(-radius * sin(alabel) - 0.5 * aux->rect().height())));

        angle -= value;
    }

    if (legendMarkerID >= 0) {
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
        if (mrk) {
            QString text = "";
            for (int i = 0; i < size; i++) {
                text += "\\p{";
                text += QString::number(i + 1);
                text += "} ";
                text += QString::number(i + 1);
                text += "\n";
            }
            mrk->setText(text);
        }
    }

    d_plot->replot();
    updateScale();
}

bool Graph::plotHistogram(Table *w, QStringList names, int startRow, int endRow)
{
    if (!w)
        return false;
    if (endRow < 0 || endRow >= w->numRows())
        endRow = w->numRows() - 1;

    bool success = false;
    for (QString col : names) {
        Column *col_ptr = w->column(col);
        if (!col_ptr || col_ptr->columnMode() != Makhber::ColumnMode::Numeric)
            continue;

        auto *c = new QwtHistogram(w, col, startRow, endRow);
        c->loadData();
        c->setStyle(QwtPlotCurve::UserCurve);

        c_type.resize(++n_curves);
        c_type[n_curves - 1] = Histogram;
        c_keys.resize(n_curves);
        c_keys[n_curves - 1] = d_plot->insertCurve(c);

        CurveLayout cl = initCurveLayout(Histogram, names.size());
        updateCurveLayout(n_curves - 1, &cl);

        addLegendItem(col);

        success = true;
    }

    return success;
}

void Graph::insertPlotItem(QwtPlotItem *i, int type)
{
    c_type.resize(++n_curves);
    c_type[n_curves - 1] = type;

    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(i);

    if (i->rtti() != QwtPlotItem::Rtti_PlotSpectrogram)
        addLegendItem(i->title().text());
}

bool Graph::insertCurvesList(Table *w, const QStringList &names, int style, int lWidth, int sSize,
                             int startRow, int endRow)
{
    if (style == Graph::Pie)
        plotPie(w, names[0], startRow, endRow);
    else if (style == Box)
        plotBoxDiagram(w, names, startRow, endRow);
    else if (style == Histogram)
        plotHistogram(w, names, startRow, endRow);
    else if (style == Graph::VectXYXY || style == Graph::VectXYAM)
        plotVectorCurve(w, names, style, startRow, endRow);
    else {
        QStringList errorCurves, otherCurves;
        for (QString col : names) {
            int colIndex = w->colIndex(col);
            if (colIndex < 0)
                continue;
            switch (w->colPlotDesignation(colIndex)) {
            case Makhber::xErr:
            case Makhber::yErr:
                errorCurves << col;
                break;
            default:
                otherCurves << col;
                break;
            }
        }
        QStringList lst = otherCurves + errorCurves;

        for (int i = 0; i < lst.size(); i++) {
            CurveType type_of_i {};
            int j = w->colIndex(lst[i]);
            if (j < 0)
                continue;
            bool ok = false;
            if (i >= otherCurves.size()) {
                type_of_i = ErrorBars;
                int ycol = -1;
                for (int k = j - 1; k >= 0; k--) {
                    if (w->colPlotDesignation(k) == Makhber::Y) {
                        ycol = k;
                        break;
                    }
                }
                if (ycol < 0)
                    ycol = w->colY(w->colIndex(lst[i]));
                if (ycol < 0)
                    return false;

                if (w->colPlotDesignation(j) == Makhber::xErr)
                    ok = addErrorBars(w->colName(ycol), w, lst[i], Qt::Horizontal);
                else
                    ok = addErrorBars(w->colName(ycol), w, lst[i]);
            } else {
                type_of_i = (CurveType)style;
                ok = insertCurve(w, lst[i], style, startRow, endRow);
            }

            if (ok) {
                CurveLayout cl = initCurveLayout(type_of_i, otherCurves.size());
                cl.sSize = sSize;
                cl.lWidth = lWidth;
                updateCurveLayout(i, &cl);
            } else
                return false;
        }
    }
    updatePlot();
    return true;
}

bool Graph::insertCurve(Table *w, const QString &name, int style, int startRow, int endRow)
{ // provided for convenience
    int ycol = w->colIndex(name);
    int xcol = w->colX(ycol);

    bool succes = insertCurve(w, w->colName(xcol), w->colName(ycol), style, startRow, endRow);
    if (succes)
        Q_EMIT modifiedGraph();
    return succes;
}

bool Graph::insertCurve(Table *w, int xcol, const QString &name, int style)
{
    return insertCurve(w, w->colName(xcol), w->colName(w->colIndex(name)), style);
}

bool Graph::insertCurve(Table *w, const QString &xColName, const QString &yColName, int style,
                        int startRow, int endRow)
{
    if (!w)
        return false;
    DataCurve *c = nullptr;

    switch (style) {
    case Histogram:
    case Box:
    case Pie:
        return false;
    case VerticalBars:
        c = new QwtBarCurve(QwtBarCurve::Vertical, w, xColName, yColName, startRow, endRow);
        c->setStyle(QwtPlotCurve::UserCurve);
        break;
    case HorizontalBars:
        c = new QwtBarCurve(QwtBarCurve::Horizontal, w, xColName, yColName, startRow, endRow);
        c->setStyle(QwtPlotCurve::UserCurve);
        break;
    default:
        c = new DataCurve(w, xColName, yColName, startRow, endRow);
        break;
    };

    c_type.resize(++n_curves);
    c_type[n_curves - 1] = style;
    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(c);

    c->setPen(QPen(Qt::black, widthLine));
    c->setSymbol(new QwtSymbol());

    if (!c->loadData())
        return false;

    addLegendItem(yColName);
    updatePlot();

    return true;
}

void Graph::plotVectorCurve(Table *w, const QStringList &colList, int style, int startRow,
                            int endRow)
{
    if (colList.count() != 4)
        return;

    if (endRow < 0)
        endRow = w->numRows() - 1;

    VectorCurve *v = nullptr;
    if (style == VectXYAM)
        v = new VectorCurve(VectorCurve::XYAM, w, colList[0], colList[1], colList[2], colList[3],
                            startRow, endRow);
    else
        v = new VectorCurve(VectorCurve::XYXY, w, colList[0], colList[1], colList[2], colList[3],
                            startRow, endRow);

    if (!v)
        return;

    v->loadData();
    v->setStyle(QwtPlotCurve::NoCurve);

    c_type.resize(++n_curves);
    c_type[n_curves - 1] = style;

    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(v);

    addLegendItem(colList[1]);
    updatePlot();
}

void Graph::updateVectorsLayout(int curve, const QColor &color, int width, int arrowLength,
                                int arrowAngle, bool filled, int position,
                                const QString &xEndColName, const QString &yEndColName)
{
    auto *vect = dynamic_cast<VectorCurve *>(this->curve(curve));
    if (!vect)
        return;

    vect->setColor(color);
    vect->setWidth(width);
    vect->setHeadLength(arrowLength);
    vect->setHeadAngle(arrowAngle);
    vect->fillArrowHead(filled);
    vect->setPosition(position);

    if (!xEndColName.isEmpty() && !yEndColName.isEmpty())
        vect->setVectorEnd(xEndColName, yEndColName);

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::updatePlot()
{
    if (m_autoscale && !zoomOn() && d_active_tool == nullptr) {
        for (int i = 0; i < QwtPlot::axisCnt; i++)
            d_plot->setAxisAutoScale(i);
    }

    d_plot->replot();
    updateMarkersBoundingRect();
    updateSecondaryAxis(QwtPlot::xTop);
    updateSecondaryAxis(QwtPlot::yRight);

    if (isPiePlot()) {
        auto *c = dynamic_cast<QwtPieCurve *>(curve(0));
        c->updateBoundingRect();
    }

    d_plot->replot();
    d_zoomer[0]->setZoomBase();
    d_zoomer[1]->setZoomBase();
}

void Graph::updateScale()
{
    QwtScaleDiv scDiv = d_plot->axisScaleDiv(QwtPlot::xBottom);
    QList<double> lst = scDiv.ticks(QwtScaleDiv::MajorTick);

    double step = fabs(lst[1] - lst[0]);

    if (!m_autoscale)
#if QWT_VERSION >= 0x050200
        d_plot->setAxisScale(QwtPlot::xBottom, scDiv.lowerBound(), scDiv.upperBound(), step);
#else
        d_plot->setAxisScale(QwtPlot::xBottom, scDiv->lBound(), scDiv->hBound(), step);
#endif

    scDiv = d_plot->axisScaleDiv(QwtPlot::yLeft);
    lst = scDiv.ticks(QwtScaleDiv::MajorTick);

    step = fabs(lst[1] - lst[0]);

    if (!m_autoscale)
#if QWT_VERSION >= 0x050200
        d_plot->setAxisScale(QwtPlot::yLeft, scDiv.lowerBound(), scDiv.upperBound(), step);
#else
        d_plot->setAxisScale(QwtPlot::yLeft, scDiv->lBound(), scDiv->hBound(), step);
#endif

    d_plot->replot();
    updateMarkersBoundingRect();
    updateSecondaryAxis(QwtPlot::xTop);
    updateSecondaryAxis(QwtPlot::yRight);

    d_plot->replot(); // TODO: avoid 2nd replot!
}

void Graph::setBarsGap(int curve, int gapPercent, int offset)
{
    auto *bars = dynamic_cast<QwtBarCurve *>(this->curve(curve));
    if (!bars)
        return;

    if (bars->gap() == gapPercent && bars->offset() == offset)
        return;

    bars->setGap(gapPercent);
    bars->setOffset(offset);
}

void Graph::removePie()
{
    if (legendMarkerID >= 0) {
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
        if (mrk)
            mrk->setText({});
    }

    d_plot->removeCurve(c_keys[0]);
    d_plot->replot();

    c_keys.resize(0);
    c_type.resize(0);
    n_curves = 0;
    Q_EMIT modifiedGraph();
}

void Graph::removeCurves(const QString &s)
{
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (!it)
            continue;

        if (it->title().text() == s) {
            removeCurve(i);
            continue;
        }

        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;
        if ((dynamic_cast<PlotCurve *>(it))->type() == Function)
            continue;

        if ((dynamic_cast<DataCurve *>(it))
                    ->plotAssociation()
                    .contains(QRegularExpression("\b" + s + "\b")))
            removeCurve(i);
    }
    d_plot->replot();
}

void Graph::removeCurve(const QString &s)
{
    removeCurve(plotItemsList().indexOf(s));
}

void Graph::removeCurve(int index)
{
    if (index < 0 || index >= n_curves)
        return;

    QwtPlotItem *it = plotItem(index);
    if (!it)
        return;

    removeLegendItem(index);

    if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
        if ((dynamic_cast<PlotCurve *>(it))->type() == ErrorBars)
            (dynamic_cast<QwtErrorPlotCurve *>(it))->detachFromMasterCurve();
        else if ((dynamic_cast<PlotCurve *>(it))->type() != Function)
            (dynamic_cast<DataCurve *>(it))->clearErrorBars();

        if (d_fit_curves.contains(dynamic_cast<QwtPlotCurve *>(it))) {
            int i = d_fit_curves.indexOf(dynamic_cast<QwtPlotCurve *>(it));
            if (i >= 0 && i < d_fit_curves.size())
                d_fit_curves.removeAt(i);
        }
    }

    if (d_range_selector && curve(index) == d_range_selector->selectedCurve()) {
        if (n_curves > 1 && (index - 1) >= 0)
            d_range_selector->setSelectedCurve(curve(index - 1));
        else if (n_curves > 1 && index + 1 < n_curves)
            d_range_selector->setSelectedCurve(curve(index + 1));
        else
            delete d_range_selector;
    }

    d_plot->removeCurve(c_keys[index]);
    n_curves--;

    for (int i = index; i < n_curves; i++) {
        c_type[i] = c_type[i + 1];
        c_keys[i] = c_keys[i + 1];
    }
    c_type.resize(n_curves);
    c_keys.resize(n_curves);
    Q_EMIT modifiedGraph();
}

void Graph::removeLegendItem(int index)
{
    if (legendMarkerID < 0 || c_type[index] == ErrorBars)
        return;

    auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
    if (!mrk)
        return;

    if (isPiePlot()) {
        mrk->setText({});
        return;
    }

    QString text = mrk->text();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList items = text.split("\n", Qt::SkipEmptyParts);
#else
    QStringList items = text.split("\n", QString::SkipEmptyParts);
#endif

    if (index >= (int)items.count())
        return;

    QStringList l = items.filter("\\c{" + QString::number(index + 1) + "}");
    if (!l.isEmpty())
        items.removeAll(l[0]); // remove the corresponding legend string
    text = items.join("\n") + "\n";

    QRegularExpression itemCmd(R"(\\c\{(\d+)\})");
    auto match = itemCmd.match(text);
    int pos = match.capturedStart();
    while (pos != -1) {
        int nr = match.captured(1).toInt();
        if (nr > index) {
            QString subst = QString("\\c{") + QString::number(nr - 1) + "}";
            text.replace(pos, match.capturedLength(), subst);
            pos += subst.length();
        } else
            pos += match.capturedLength();
        match = itemCmd.match(text, pos);
        pos = match.capturedStart();
    }

    mrk->setText(text);
}

void Graph::addLegendItem(const QString &colName)
{
    if (legendMarkerID >= 0) {
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
        if (mrk) {
            QString text = mrk->text();
            if (text.endsWith("\n", Qt::CaseSensitive))
                text.append("\\c{" + QString::number(curves()) + "}" + colName + "\n");
            else
                text.append("\n\\c{" + QString::number(curves()) + "}" + colName + "\n");

            mrk->setText(text);
        }
    }
}

void Graph::contextMenuEvent(QContextMenuEvent *e)
{
    if (selectedMarker >= 0) {
        Q_EMIT showMarkerPopupMenu();
        return;
    }

    QPoint pos = d_plot->canvas()->mapFrom(d_plot, e->pos());
    int dist = 0, point = 0;
    const long curve = d_plot->closestCurve(pos.x(), pos.y(), dist, point);
    const QwtPlotCurve *c = (QwtPlotCurve *)d_plot->curve(curve);

    if (c && dist < 10) // 10 pixels tolerance
        Q_EMIT showCurveContextMenu(curve);
    else
        Q_EMIT showContextMenu();

    e->accept();
}

void Graph::closeEvent(QCloseEvent *e)
{
    Q_EMIT closedGraph();
    e->accept();
}

bool Graph::zoomOn()
{
    return (d_zoomer[0]->isEnabled() || d_zoomer[1]->isEnabled());
}

void Graph::zoomed(const QRectF &)
{
    Q_EMIT modifiedGraph();
}

void Graph::zoom(bool on)
{
    d_zoomer[0]->setEnabled(on);
    d_zoomer[1]->setEnabled(on);
    for (int i = 0; i < n_curves; i++) {
        auto *sp = (Spectrogram *)this->curve(i);
        if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) {
            if (sp->colorScaleAxis() == QwtPlot::xBottom || sp->colorScaleAxis() == QwtPlot::yLeft)
                d_zoomer[0]->setEnabled(false);
            else
                d_zoomer[1]->setEnabled(false);
        }
    }

    QCursor cursor = QCursor(QPixmap(":/lens.xpm"), -1, -1);
    if (on)
        d_plot->canvas()->setCursor(cursor);
    else
        d_plot->canvas()->setCursor(Qt::ArrowCursor);
}

void Graph::zoomOut()
{
    d_zoomer[0]->zoom(-1);
    d_zoomer[1]->zoom(-1);

    updateSecondaryAxis(QwtPlot::xTop);
    updateSecondaryAxis(QwtPlot::yRight);
}

void Graph::drawText(bool on)
{
    QCursor c = QCursor(Qt::IBeamCursor);
    if (on)
        d_plot->canvas()->setCursor(c);
    else
        d_plot->canvas()->setCursor(Qt::ArrowCursor);

    drawTextOn = on;
}

ImageMarker *Graph::addImage(ImageMarker *mrk)
{
    if (!mrk)
        return nullptr;

    auto *mrk2 = new ImageMarker(mrk->fileName());

    int imagesOnPlot = d_images.size();
    d_images.resize(++imagesOnPlot);
    d_images[imagesOnPlot - 1] = d_plot->insertMarker(mrk2);

    mrk2->setBoundingRect(mrk->xValue(), mrk->yValue(), mrk->right(), mrk->bottom());
    return mrk;
}

ImageMarker *Graph::addImage(const QString &fileName)
{
    if (fileName.isEmpty() || !QFile::exists(fileName)) {
        QMessageBox::warning(
                nullptr, tr("File open error"),
                tr("Image file: <p><b> %1 </b><p>does not exist anymore!").arg(fileName));
        return nullptr;
    }

    auto *mrk = new ImageMarker(fileName);
    int imagesOnPlot = d_images.size();
    d_images.resize(++imagesOnPlot);
    d_images[imagesOnPlot - 1] = d_plot->insertMarker(mrk);

    QSize picSize = mrk->pixmap().size();
    int w = d_plot->canvas()->width();
    if (picSize.width() > w)
        picSize.setWidth(w);

    int h = d_plot->canvas()->height();
    if (picSize.height() > h)
        picSize.setHeight(h);

    mrk->setSize(picSize);
    d_plot->replot();

    Q_EMIT modifiedGraph();
    return mrk;
}

void Graph::insertImageMarker(QJsonObject *jsImage)
{
    QString fn = jsImage->value("filename").toString();
    if (!QFile::exists(fn)) {
        QMessageBox::warning(nullptr, tr("File open error"),
                             tr("Image file: <p><b> %1 </b><p>does not exist anymore!").arg(fn));
    } else {
        auto *mrk = new ImageMarker(fn);
        if (!mrk)
            return;

        int imagesOnPlot = d_images.size();
        d_images.resize(++imagesOnPlot);
        d_images[imagesOnPlot - 1] = d_plot->insertMarker(mrk);

        mrk->setBoundingRect(
                jsImage->value("xValue").toDouble(), jsImage->value("yValue").toDouble(),
                jsImage->value("right").toDouble(), jsImage->value("bottom").toDouble());
    }
}

void Graph::drawLine(bool on, bool arrow)
{
    drawLineOn = on;
    drawArrowOn = arrow;
    if (!on)
        Q_EMIT drawLineEnded(true);
}

bool Graph::modifyFunctionCurve(ApplicationWindow *parent, int curve, int type,
                                const QStringList &formulas, const QString &var,
                                QList<double> &ranges, int points)
{
    auto *c = dynamic_cast<FunctionCurve *>(this->curve(curve));
    if (!c)
        return false;

    if (c->functionType() == type && c->variable() == var && c->formulas() == formulas
        && c->startRange() == ranges[0] && c->endRange() == ranges[1]
        && static_cast<int>(c->dataSize()) == points)
        return true;

    FunctionCurve backup(parent);
    backup.copy(c);

    c->setFunctionType((FunctionCurve::FunctionType)type);
    c->setRange(ranges[0], ranges[1]);
    c->setFormulas(formulas);
    c->setVariable(var);
    if (!c->loadData(points)) {
        c->copy(&backup);
        c->loadData(points);
        return false;
    }

    if (legendMarkerID >= 0) { // update the legend marker
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
        if (mrk) {
            QString text = (mrk->text()).replace(backup.legend(), c->legend());
            mrk->setText(text);
        }
    }
    updatePlot();
    Q_EMIT modifiedGraph();
    return true;
}

QString Graph::generateFunctionName(const QString &name)
{
    int index = 1;
    QString newName = name + QString::number(index);

    QStringList lst;
    for (int i = 0; i < n_curves; i++) {
        auto *c = dynamic_cast<PlotCurve *>(this->curve(i));
        if (!c)
            continue;

        if (c->type() == Function)
            lst << c->title().text();
    }

    while (lst.contains(newName)) {
        newName = name + QString::number(++index);
    }
    return newName;
}

bool Graph::addFunctionCurve(ApplicationWindow *parent, int type, const QStringList &formulas,
                             const QString &var, QList<double> &ranges, int points,
                             const QString &title)
{
    QString name;
    if (!title.isEmpty())
        name = title;
    else
        name = generateFunctionName();

    auto *c = new FunctionCurve(parent, (FunctionCurve::FunctionType)type, name);
    c->setPen(QPen(QColor(Qt::black), widthLine));
    c->setRange(ranges[0], ranges[1]);
    c->setFormulas(formulas);
    c->setVariable(var);
    if (!c->loadData(points)) {
        delete c;
        return false;
    }

    c_type.resize(++n_curves);
    c_type[n_curves - 1] = Line;

    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(c);

    addLegendItem(c->legend());
    updatePlot();

    Q_EMIT modifiedGraph();
    return true;
}

bool Graph::insertFunctionCurve(ApplicationWindow *parent, const QStringList &func_spec, int points)
{
    int type = FunctionCurve::Normal;
    QStringList formulas;
    QString var, name;
    QList<double> ranges;

    QStringList curve = func_spec[0].split(",");
    type = curve[0].toInt();
    name = curve[1];
    var = curve[2];
    ranges += curve[3].toDouble();
    ranges += curve[4].toDouble();

    formulas << func_spec[1];
    if (type != FunctionCurve::Normal)
        formulas << func_spec[2];

    return addFunctionCurve(parent, type, formulas, var, ranges, points, name);
}

void Graph::createTable(const QString &curveName)
{
    if (curveName.isEmpty())
        return;

    const QwtPlotCurve *cv = curve(curveName);
    if (!cv)
        return;

    createTable(cv);
}

void Graph::createTable(const QwtPlotCurve *curve)
{
    if (!curve)
        return;

    int size = static_cast<int>(curve->dataSize());

    auto *xCol =
            new Column(tr("1", "curve data table x column name"), Makhber::ColumnMode::Numeric);
    auto *yCol =
            new Column(tr("2", "curve data table y column name"), Makhber::ColumnMode::Numeric);
    xCol->setPlotDesignation(Makhber::X);
    yCol->setPlotDesignation(Makhber::Y);
    for (int i = 0; i < size; i++) {
        xCol->setValueAt(i, curve->data()->sample(i).x());
        yCol->setValueAt(i, curve->data()->sample(i).y());
    }
    QString legend = tr("Data set generated from curve") + ": " + curve->title().text();
    Q_EMIT createTable(tr("Curve data %1").arg(1), legend, QList<Column *>() << xCol << yCol);
}

void Graph::saveToJson(QJsonObject *jsObject, bool saveAsTemplate)
{
    QJsonObject jsGeometry {};
    jsGeometry.insert("x", this->pos().x());
    jsGeometry.insert("y", this->pos().y());
    jsGeometry.insert("width", this->frameGeometry().width());
    jsGeometry.insert("height", this->frameGeometry().height());
    jsObject->insert("geometry", jsGeometry);

    jsObject->insert("plotTitle", saveTitle());
    jsObject->insert("antialiasing", d_antialiasing);
    jsObject->insert("background", COLORNAME(d_plot->paletteBackgroundColor()));
    jsObject->insert("alpha", d_plot->paletteBackgroundColor().alpha());
    // s += "Margin\t" + QString::number(d_plot->margin()) + "\n";

    QJsonObject jsBorder {};
    jsBorder.insert("lineWidth", d_plot->lineWidth());
    jsBorder.insert("frameColor", COLORNAME(d_plot->frameColor()));
    jsObject->insert("border", jsBorder);

    QJsonObject jsGrid {};
    grid()->saveToJson(&jsGrid);
    jsObject->insert("grid", jsGrid);
    jsObject->insert("enabledAxes", saveEnabledAxes());

    QJsonObject jsAxesTitles {};
    jsAxesTitles.insert("scale", saveScaleTitles());
    jsAxesTitles.insert("colors", saveAxesTitleColors());
    jsAxesTitles.insert("alignment", saveAxesTitleAlignement());
    jsObject->insert("axesTitles", jsAxesTitles);

    jsObject->insert("fonts", saveFonts());
    jsObject->insert("enabledTickLabels", saveEnabledTickLabels());
    jsObject->insert("axesColors", saveAxesColors());
    jsObject->insert("axesBaseline", saveAxesBaseline());
    jsObject->insert("canvas", saveCanvas());

    if (!saveAsTemplate)
        saveCurves(jsObject);

    jsObject->insert("scale", saveScale());
    jsObject->insert("axesFormulas", saveAxesFormulas());
    jsObject->insert("labelsFormat", saveLabelsFormat());
    jsObject->insert("axesLabelsType", saveAxesLabelsType());
    jsObject->insert("ticksType", saveTicksType());
    jsObject->insert("minorTickLength", minorTickLength());
    jsObject->insert("majorTickLength", majorTickLength());
    jsObject->insert("drawAxesBackbone", drawAxesBackbone);
    jsObject->insert("axesLineWidth", d_plot->axesLinewidth());
    jsObject->insert("labelsRotation", saveLabelsRotation());
    jsObject->insert("markers", saveMarkers());

    if (!saveAsTemplate)
        jsObject->insert("type", "Graph");
    else
        jsObject->insert("templateType", "Graph");
}

void Graph::showIntensityTable()
{
    auto *mrk = dynamic_cast<ImageMarker *>(d_plot->marker(selectedMarker));
    if (!mrk)
        return;

    Q_EMIT createIntensityTable(mrk->fileName());
}

void Graph::updateMarkersBoundingRect()
{
    for (int d_line : d_lines) {
        auto *mrkL = dynamic_cast<ArrowMarker *>(d_plot->marker(d_line));
        if (mrkL)
            mrkL->updateBoundingRect();
    }
    for (int d_text : d_texts) {
        auto *mrkT = dynamic_cast<Legend *>(d_plot->marker(d_text));
        if (mrkT)
            mrkT->updateOrigin();
    }

    for (int d_image : d_images) {
        auto *mrk = dynamic_cast<ImageMarker *>(d_plot->marker(d_image));
        if (mrk)
            mrk->updateBoundingRect();
    }
    d_plot->replot();
}
void Graph::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    hidden_size = size();
}

void Graph::resizeEvent(QResizeEvent *e)
{
    if (ignoreResize)
        return;
    QSize oldSize = e->oldSize();
    if (!this->isVisible() || !oldSize.isValid())
        oldSize = hidden_size;
    if (!oldSize.isValid())
        return;

    if (autoScaleFonts) {
        QSize size = e->size();

        double ratio = (double)size.height() / (double)oldSize.height();
        scaleFonts(ratio);
    }

    d_plot->resize(e->size());
}

void Graph::scaleFonts(double factor)
{
    for (int d_text : d_texts) {
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(d_text));
        QFont font = mrk->font();
        font.setPointSizeF(factor * font.pointSizeF());
        mrk->setFont(font);
    }
    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        QFont font = axisFont(i);
        font.setPointSizeF(factor * font.pointSizeF());
        d_plot->setAxisFont(i, font);

        QwtText title = d_plot->axisTitle(i);
        font = title.font();
        font.setPointSizeF(factor * font.pointSizeF());
        title.setFont(font);
        d_plot->setAxisTitle(i, title);
    }

    QwtText title = d_plot->title();
    QFont font = title.font();
    font.setPointSizeF(factor * font.pointSizeF());
    title.setFont(font);
    d_plot->setTitle(title);

    d_plot->replot();
}
/*
void Graph::setMargin(int d)
{
    if (d_plot->margin() == d)
        return;

    d_plot->setMargin(d);
    Q_EMIT modifiedGraph();
}
*/
void Graph::setFrame(int width, const QColor &color)
{
    if (d_plot->frameColor() == color && width == d_plot->lineWidth())
        return;

    QPalette pal = d_plot->palette();
    pal.setColor(QPalette::WindowText, color);
    d_plot->setPalette(pal);

    d_plot->setLineWidth(width);
}

void Graph::setBackgroundColor(const QColor &color)
{
    QPalette p = d_plot->palette();
    p.setColor(QPalette::Window, color);
    d_plot->setPalette(p);

    d_plot->setAutoFillBackground(true);
    Q_EMIT modifiedGraph();
}

void Graph::setCanvasBackground(const QColor &color)
{
    d_plot->setCanvasBackground(color);
    Q_EMIT modifiedGraph();
}

Qt::BrushStyle Graph::getBrushStyle(int style)
{
    Qt::BrushStyle brushStyle {};
    switch (style) {
    case 0:
        brushStyle = Qt::SolidPattern;
        break;
    case 1:
        brushStyle = Qt::HorPattern;
        break;
    case 2:
        brushStyle = Qt::VerPattern;
        break;
    case 3:
        brushStyle = Qt::CrossPattern;
        break;
    case 4:
        brushStyle = Qt::BDiagPattern;
        break;
    case 5:
        brushStyle = Qt::FDiagPattern;
        break;
    case 6:
        brushStyle = Qt::DiagCrossPattern;
        break;
    case 7:
        brushStyle = Qt::Dense1Pattern;
        break;
    case 8:
        brushStyle = Qt::Dense2Pattern;
        break;
    case 9:
        brushStyle = Qt::Dense3Pattern;
        break;
    case 10:
        brushStyle = Qt::Dense4Pattern;
        break;
    case 11:
        brushStyle = Qt::Dense5Pattern;
        break;
    case 12:
        brushStyle = Qt::Dense6Pattern;
        break;
    case 13:
        brushStyle = Qt::Dense7Pattern;
        break;
    default:
        throw std::runtime_error("invalid brush style");
    }
    return brushStyle;
}

QString Graph::penStyleName(Qt::PenStyle style)
{
    if (style == Qt::SolidLine)
        return "SolidLine";
    else if (style == Qt::DashLine)
        return "DashLine";
    else if (style == Qt::DotLine)
        return "DotLine";
    else if (style == Qt::DashDotLine)
        return "DashDotLine";
    else if (style == Qt::DashDotDotLine)
        return "DashDotDotLine";
    else if (style == Qt::CustomDashLine)
        return "CustomDashLine";
    else
        return "SolidLine";
}

Qt::PenStyle Graph::getPenStyle(int style)
{
    Qt::PenStyle linePen {};
    switch (style) {
    case 0:
        linePen = Qt::SolidLine;
        break;
    case 1:
        linePen = Qt::DashLine;
        break;
    case 2:
        linePen = Qt::DotLine;
        break;
    case 3:
        linePen = Qt::DashDotLine;
        break;
    case 4:
        linePen = Qt::DashDotDotLine;
        break;
    case 5:
        linePen = Qt::CustomDashLine;
        break;
    default:
        throw std::runtime_error("invalid pen style");
    }
    return linePen;
}

Qt::PenStyle Graph::getPenStyle(const QString &s)
{
    Qt::PenStyle style = Qt::SolidLine;
    if (s == "SolidLine")
        style = Qt::SolidLine;
    else if (s == "DashLine")
        style = Qt::DashLine;
    else if (s == "DotLine")
        style = Qt::DotLine;
    else if (s == "DashDotLine")
        style = Qt::DashDotLine;
    else if (s == "DashDotDotLine")
        style = Qt::DashDotDotLine;
    else if (s == "CustomDashLine")
        style = Qt::CustomDashLine;
    return style;
}

int Graph::obsoleteSymbolStyle(int type)
{
    if (type <= 4)
        return type + 1;
    else
        return type + 2;
}

int Graph::curveType(int curveIndex)
{
    if (curveIndex < (int)c_type.size() && curveIndex >= 0)
        return c_type[curveIndex];
    else
        return -1;
}

void Graph::showPlotErrorMessage(QWidget *parent, const QStringList &emptyColumns)
{
    QApplication::restoreOverrideCursor();

    int n = (int)emptyColumns.count();
    if (n > 1) {
        QString columns;
        for (int i = 0; i < n; i++)
            columns += "<p><b>" + emptyColumns[i] + "</b></p>";

        QMessageBox::warning(parent, tr("Warning"),
                             tr("The columns") + ": " + columns
                                     + tr("are empty and will not be added to the plot!"));
    } else if (n == 1)
        QMessageBox::warning(parent, tr("Warning"),
                             tr("The column") + " <b>" + emptyColumns[0] + "</b> "
                                     + tr("is empty and will not be added to the plot!"));
}

void Graph::showTitleContextMenu()
{
    QMenu titleMenu(this);
    titleMenu.addAction(QPixmap(":/cut.xpm"), tr("&Cut"), this, SLOT(cutTitle()));
    titleMenu.addAction(QPixmap(":/copy.xpm"), tr("&Copy"), this, SLOT(copyTitle()));
    titleMenu.addAction(tr("&Delete"), this, SLOT(removeTitle()));
    titleMenu.addSeparator();
    titleMenu.addAction(tr("&Properties..."), this, SIGNAL(viewTitleDialog()));
    titleMenu.exec(QCursor::pos());
}

void Graph::cutTitle()
{
    QApplication::clipboard()->setText(d_plot->title().text(), QClipboard::Clipboard);
    removeTitle();
}

void Graph::copyTitle()
{
    QApplication::clipboard()->setText(d_plot->title().text(), QClipboard::Clipboard);
}

void Graph::removeAxisTitle()
{
    int axis = (selectedAxis + 2) % 4; // inconsistent notation in Qwt enumerations between
    // QwtScaleDraw::alignment and QwtPlot::Axis
    d_plot->setAxisTitle(axis, QString());
    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::cutAxisTitle()
{
    copyAxisTitle();
    removeAxisTitle();
}

void Graph::copyAxisTitle()
{
    int axis = (selectedAxis + 2) % 4; // unconsistent notation in Qwt enumerations between
    // QwtScaleDraw::alignment and QwtPlot::Axis
    QApplication::clipboard()->setText(d_plot->axisTitle(axis).text(), QClipboard::Clipboard);
}

void Graph::showAxisTitleMenu(int axis)
{
    selectedAxis = axis;

    QMenu titleMenu(this);
    titleMenu.addAction(QPixmap(":/cut.xpm"), tr("&Cut"), this, SLOT(cutAxisTitle()));
    titleMenu.addAction(QPixmap(":/copy.xpm"), tr("&Copy"), this, SLOT(copyAxisTitle()));
    titleMenu.addAction(tr("&Delete"), this, SLOT(removeAxisTitle()));
    titleMenu.addSeparator();
    switch (axis) {
    case QwtScaleDraw::BottomScale:
        titleMenu.addAction(tr("&Properties..."), this, SIGNAL(xAxisTitleDblClicked()));
        break;

    case QwtScaleDraw::LeftScale:
        titleMenu.addAction(tr("&Properties..."), this, SIGNAL(yAxisTitleDblClicked()));
        break;

    case QwtScaleDraw::TopScale:
        titleMenu.addAction(tr("&Properties..."), this, SIGNAL(topAxisTitleDblClicked()));
        break;

    case QwtScaleDraw::RightScale:
        titleMenu.addAction(tr("&Properties..."), this, SIGNAL(rightAxisTitleDblClicked()));
        break;
    }

    titleMenu.exec(QCursor::pos());
}

void Graph::showAxisContextMenu(int axis)
{
    selectedAxis = axis;

    QMenu menu(this);
    menu.addAction(QPixmap(":/unzoom.xpm"), tr("&Rescale to show all"), this, SLOT(setAutoScale()),
                   tr("Ctrl+Shift+R"));
    menu.addSeparator();
    menu.addAction(tr("&Hide axis"), this, SLOT(hideSelectedAxis()));

    QAction *gridsID = menu.addAction(tr("&Show grids"), this, SLOT(showGrids()));
    if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale) {
        if (d_plot->grid()->yEnabled())
            gridsID->setChecked(true);
    } else {
        if (d_plot->grid()->xEnabled())
            gridsID->setChecked(true);
    }

    menu.addSeparator();
    menu.addAction(tr("&Scale..."), this, SLOT(showScaleDialog()));
    menu.addAction(tr("&Properties..."), this, SLOT(showAxisDialog()));
    menu.exec(QCursor::pos());
}

void Graph::showAxisDialog()
{
    Q_EMIT showSelectedAxisDialog(selectedAxis);
}

void Graph::showScaleDialog()
{
    Q_EMIT axisDblClicked(selectedAxis);
}

void Graph::hideSelectedAxis()
{
    int axis = -1;
    if (selectedAxis == QwtScaleDraw::LeftScale || selectedAxis == QwtScaleDraw::RightScale)
        axis = selectedAxis - 2;
    else
        axis = selectedAxis + 2;

    d_plot->enableAxis(axis, false);
    scalePicker->refresh();
    Q_EMIT modifiedGraph();
}

void Graph::showGrids()
{
    showGrid(selectedAxis);
}

void Graph::showGrid()
{
    showGrid(QwtScaleDraw::LeftScale);
    showGrid(QwtScaleDraw::BottomScale);
}

void Graph::showGrid(int axis)
{
    Grid *grid = d_plot->grid();
    if (!grid)
        return;

    if (axis == QwtScaleDraw::LeftScale || axis == QwtScaleDraw::RightScale) {
        grid->enableY(!grid->yEnabled());
        grid->enableYMin(!grid->yMinEnabled());
    } else if (axis == QwtScaleDraw::BottomScale || axis == QwtScaleDraw::TopScale) {
        grid->enableX(!grid->xEnabled());
        grid->enableXMin(!grid->xMinEnabled());
    } else
        return;

    d_plot->replot();
    Q_EMIT modifiedGraph();
}

void Graph::copy(ApplicationWindow *parent, Graph *g)
{
    int i = 0;
    Plot *plot = g->plotWidget();
    // d_plot->setMargin(plot->margin());

    setAntialiasing(g->antialiasing());
    autoScaleFonts = g->autoscaleFonts();

    setBackgroundColor(plot->paletteBackgroundColor());
    setFrame(plot->lineWidth(), plot->frameColor());
    setCanvasBackground(plot->canvasBackground().color());

    enableAxes(g->enabledAxes());
    setAxesColors(g->axesColors());
    setAxesNumColors(g->axesNumColors());
    setAxesBaseline(g->axesBaseline());

    grid()->copy(g->grid());

    d_plot->setTitle(g->plotWidget()->title());

    drawCanvasFrame(g->framed(), g->canvasFrameWidth(), g->canvasFrameColor());

    for (i = 0; i < 4; i++) {
        setAxisTitle(i, g->axisTitle(i));
        setAxisFont(i, g->axisFont(i));
    }

    setXAxisTitleColor(g->axisTitleColor(2));
    setXAxisTitleFont(g->axisTitleFont(2));
    setXAxisTitleAlignment(g->axisTitleAlignment(2));

    setYAxisTitleColor(g->axisTitleColor(0));
    setYAxisTitleFont(g->axisTitleFont(0));
    setYAxisTitleAlignment(g->axisTitleAlignment(0));

    setTopAxisTitleColor(g->axisTitleColor(3));
    setTopAxisTitleFont(g->axisTitleFont(3));
    setTopAxisTitleAlignment(g->axisTitleAlignment(3));

    setRightAxisTitleColor(g->axisTitleColor(1));
    setRightAxisTitleFont(g->axisTitleFont(1));
    setRightAxisTitleAlignment(g->axisTitleAlignment(1));

    setAxesLinewidth(plot->axesLinewidth());
    removeLegend();

    for (i = 0; i < g->curves(); i++) {
        auto *it = (QwtPlotItem *)g->plotItem(i);
        if (it->rtti() == QwtPlotItem::Rtti_PlotCurve) {
            auto *cv = dynamic_cast<DataCurve *>(it);
            int n = static_cast<int>(cv->dataSize());
            int style = (dynamic_cast<PlotCurve *>(it))->type();
            QVector<double> x(n);
            QVector<double> y(n);
            for (int j = 0; j < n; j++) {
                x[j] = cv->data()->sample(j).x();
                y[j] = cv->data()->sample(j).y();
            }

            PlotCurve *c = nullptr;
            if (style == Pie) {
                c = new QwtPieCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
                (dynamic_cast<QwtPieCurve *>(c))->setRay((dynamic_cast<QwtPieCurve *>(cv))->ray());
                (dynamic_cast<QwtPieCurve *>(c))
                        ->setFirstColor((dynamic_cast<QwtPieCurve *>(cv))->firstColor());
            } else if (style == Function) {
                c = new FunctionCurve(parent, cv->title().text());
                dynamic_cast<FunctionCurve *>(c)->copy(dynamic_cast<FunctionCurve *>(it));
            } else if (style == VerticalBars || style == HorizontalBars) {
                c = new QwtBarCurve((dynamic_cast<QwtBarCurve *>(cv))->orientation(), cv->table(),
                                    cv->xColumnName(), cv->title().text(), cv->startRow(),
                                    cv->endRow());
                (dynamic_cast<QwtBarCurve *>(c))->copy(dynamic_cast<const QwtBarCurve *>(cv));
            } else if (style == ErrorBars) {
                auto *er = dynamic_cast<QwtErrorPlotCurve *>(cv);
                DataCurve *master_curve = masterCurve(er);
                if (master_curve) {
                    c = new QwtErrorPlotCurve(cv->table(), cv->title().text());
                    (dynamic_cast<QwtErrorPlotCurve *>(c))->copy(er);
                    (dynamic_cast<QwtErrorPlotCurve *>(c))->setMasterCurve(master_curve);
                }
            } else if (style == Histogram) {
                c = new QwtHistogram(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
                (dynamic_cast<QwtHistogram *>(c))->copy(dynamic_cast<const QwtHistogram *>(cv));
            } else if (style == VectXYXY || style == VectXYAM) {
                VectorCurve::VectorStyle vs = VectorCurve::XYXY;
                if (style == VectXYAM)
                    vs = VectorCurve::XYAM;
                c = new VectorCurve(vs, cv->table(), cv->xColumnName(), cv->title().text(),
                                    (dynamic_cast<VectorCurve *>(cv))->vectorEndXAColName(),
                                    (dynamic_cast<VectorCurve *>(cv))->vectorEndYMColName(),
                                    cv->startRow(), cv->endRow());
                (dynamic_cast<VectorCurve *>(c))->copy(dynamic_cast<const VectorCurve *>(cv));
            } else if (style == Box) {
                c = new BoxCurve(cv->table(), cv->title().text(), cv->startRow(), cv->endRow());
                (dynamic_cast<BoxCurve *>(c))->copy(dynamic_cast<const BoxCurve *>(cv));
                c->setSamples(x, y);
            } else
                c = new DataCurve(cv->table(), cv->xColumnName(), cv->title().text(),
                                  cv->startRow(), cv->endRow());

            c_keys.resize(++n_curves);
            c_keys[i] = d_plot->insertCurve(c);

            c_type.resize(n_curves);
            c_type[i] = g->curveType(i);

            if (c_type[i] != Box && c_type[i] != ErrorBars)
                c->setSamples(x.data(), y.data(), n);

            c->setPen(cv->pen());
            c->setBrush(cv->brush());
            c->setStyle(cv->style());
            c->setSymbol(new QwtSymbol(cv->symbol()->style()));

            if (cv->testCurveAttribute(QwtPlotCurve::Fitted))
                c->setCurveAttribute(QwtPlotCurve::Fitted, true);
            else if (cv->testCurveAttribute(QwtPlotCurve::Inverted))
                c->setCurveAttribute(QwtPlotCurve::Inverted, true);

            c->setAxes(cv->xAxis(), cv->yAxis());
            c->setVisible(cv->isVisible());

            QList<QwtPlotCurve *> lst = g->fitCurvesList();
            if (lst.contains(dynamic_cast<QwtPlotCurve *>(it)))
                d_fit_curves << c;
        } else if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram) {
            Spectrogram *sp = (dynamic_cast<Spectrogram *>(it))->copy();
            c_keys.resize(++n_curves);
            c_keys[i] = d_plot->insertCurve(sp);

            sp->showColorScale((dynamic_cast<Spectrogram *>(it))->colorScaleAxis(),
                               (dynamic_cast<Spectrogram *>(it))->hasColorScale());
            sp->setColorBarWidth((dynamic_cast<Spectrogram *>(it))->colorBarWidth());
            sp->setVisible(it->isVisible());

            c_type.resize(n_curves);
            c_type[i] = g->curveType(i);
        }
    }

    axesFormulas = g->axesFormulas;
    axisType = g->axisType;
    axesFormatInfo = g->axesFormatInfo;
    axisType = g->axisType;

    for (i = 0; i < QwtPlot::axisCnt; i++) {
        QwtScaleWidget *sc = g->plotWidget()->axisWidget(i);
        if (!sc)
            continue;

        QwtScaleDraw *sd = g->plotWidget()->axisScaleDraw(i);
        if (sd->hasComponent(QwtAbstractScaleDraw::Labels)) {
            if (axisType[i] == AxisType::Numeric)
                setLabelsNumericFormat(i, plot->axisLabelFormat(i), plot->axisLabelPrecision(i),
                                       axesFormulas[i]);
            else if (axisType[i] == AxisType::Day)
                setLabelsDayFormat(i, axesFormatInfo[i].toInt());
            else if (axisType[i] == AxisType::Month)
                setLabelsMonthFormat(i, axesFormatInfo[i].toInt());
            else if (axisType[i] == AxisType::Time || axisType[i] == AxisType::Date
                     || axisType[i] == AxisType::DateTime)
                setLabelsDateTimeFormat(i, axisType[i], axesFormatInfo[i]);
            else {
                auto *sd = dynamic_cast<QwtTextScaleDraw *>(plot->axisScaleDraw(i));
                d_plot->setAxisScaleDraw(i, new QwtTextScaleDraw(sd->labelsMap()));
            }
        } else {
            sd = d_plot->axisScaleDraw(i);
            sd->enableComponent(QwtAbstractScaleDraw::Labels, false);
        }
    }
    for (i = 0; i < QwtPlot::axisCnt; i++) { // set same scales
        const QwtScaleEngine *se = plot->axisScaleEngine(i);
        if (!se)
            continue;

        QwtScaleEngine *sc_engine = nullptr;
        /*if (se->transformation()->type() == QwtScaleTransformation::Log10)
            sc_engine = new QwtLogScaleEngine();
        else if (se->transformation()->type() == QwtScaleTransformation::Linear)*/
        sc_engine = new QwtLinearScaleEngine();

        int majorTicks = plot->axisMaxMajor(i);
        int minorTicks = plot->axisMaxMinor(i);
        d_plot->setAxisMaxMajor(i, majorTicks);
        d_plot->setAxisMaxMinor(i, minorTicks);

        const QwtScaleDiv sd = plot->axisScaleDiv(i);

        d_user_step[i] = g->axisStep(i);

#if QWT_VERSION >= 0x050200
        QwtScaleDiv div = sc_engine->divideScale(qMin(sd.lowerBound(), sd.upperBound()),
                                                 qMax(sd.lowerBound(), sd.upperBound()), majorTicks,
                                                 minorTicks, d_user_step[i]);
#else
        QwtScaleDiv div = sc_engine->divideScale(qMin(sd->lBound(), sd->hBound()),
                                                 qMax(sd->lBound(), sd->hBound()), majorTicks,
                                                 minorTicks, d_user_step[i]);
#endif

        if (se->testAttribute(QwtScaleEngine::Inverted)) {
            sc_engine->setAttribute(QwtScaleEngine::Inverted);
            div.invert();
        }

        d_plot->setAxisScaleEngine(i, sc_engine);
        d_plot->setAxisScaleDiv(i, div);
    }

    drawAxesBackbones(g->drawAxesBackbone);
    setMajorTicksType(g->plotWidget()->getMajorTicksType());
    setMinorTicksType(g->plotWidget()->getMinorTicksType());
    setTicksLength(g->minorTickLength(), g->majorTickLength());

    setAxisLabelRotation(QwtPlot::xBottom, g->labelsRotation(QwtPlot::xBottom));
    setAxisLabelRotation(QwtPlot::xTop, g->labelsRotation(QwtPlot::xTop));

    QVector<int> imag = g->imageMarkerKeys();
    for (i = 0; i < (int)imag.size(); i++)
        addImage((ImageMarker *)g->imageMarker(imag[i]));

    QVector<int> txtMrkKeys = g->textMarkerKeys();
    for (i = 0; i < (int)txtMrkKeys.size(); i++) {
        auto *mrk = (Legend *)g->textMarker(txtMrkKeys[i]);
        if (!mrk)
            continue;

        if (txtMrkKeys[i] == g->legendMarkerID)
            legendMarkerID = insertTextMarker(mrk);
        else
            insertTextMarker(mrk);
    }

    QVector<int> l = g->lineMarkerKeys();
    for (i = 0; i < (int)l.size(); i++) {
        auto *lmrk = (ArrowMarker *)g->arrow(l[i]);
        if (lmrk)
            addArrow(lmrk);
    }
    d_plot->replot();

    if (isPiePlot()) {
        auto *c = dynamic_cast<QwtPieCurve *>(curve(0));
        c->updateBoundingRect();
    }
}

void Graph::plotBoxDiagram(Table *w, const QStringList &names, int startRow, int endRow)
{
    if (endRow < 0)
        endRow = w->numRows() - 1;

    for (int j = 0; j < (int)names.count(); j++) {
        auto *c = new BoxCurve(w, names[j], startRow, endRow);
        c->setSamples(QVector<double>(1, double(j + 1)), QVector<double>());
        c->loadData();

        c_keys.resize(++n_curves);
        c_keys[n_curves - 1] = d_plot->insertCurve(c);
        c_type.resize(n_curves);
        c_type[n_curves - 1] = Box;

        c->setPen(QPen(ColorButton::color(j), 1));
        c->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol, QBrush(), QPen(ColorButton::color(j), 1),
                                   QSize(7, 7)));
    }

    auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
    if (mrk)
        mrk->setText(legendText());

    axisType[QwtPlot::xBottom] = AxisType::ColHeader;
    d_plot->setAxisScaleDraw(QwtPlot::xBottom, new QwtTextScaleDraw(w->selectedYLabels()));
    d_plot->setAxisMaxMajor(QwtPlot::xBottom, names.count() + 1);
    d_plot->setAxisMaxMinor(QwtPlot::xBottom, 0);

    axisType[QwtPlot::xTop] = AxisType::ColHeader;
    d_plot->setAxisScaleDraw(QwtPlot::xTop, new QwtTextScaleDraw(w->selectedYLabels()));
    d_plot->setAxisMaxMajor(QwtPlot::xTop, names.count() + 1);
    d_plot->setAxisMaxMinor(QwtPlot::xTop, 0);

    axesFormatInfo[QwtPlot::xBottom] = w->name();
    axesFormatInfo[QwtPlot::xTop] = w->name();
}

void Graph::setCurveStyle(int index, int s)
{
    QwtPlotCurve *c = curve(index);
    if (!c)
        return;

    if (s == 4) // ancient spline style in Qwt 4.2.0
    {
        s = QwtPlotCurve::Lines;
        c->setCurveAttribute(QwtPlotCurve::Fitted, true);
        c_type[index] = Spline;
    } else if (s == QwtPlotCurve::Lines)
        c->setCurveAttribute(QwtPlotCurve::Fitted, false);
    else if (s == 5) // Vertical Steps
    {
        s = QwtPlotCurve::Steps;
        c->setCurveAttribute(QwtPlotCurve::Inverted, true);
        c_type[index] = VerticalSteps;
    } else if (s == QwtPlotCurve::Steps) // Horizontal Steps
    {
        c->setCurveAttribute(QwtPlotCurve::Inverted, false);
        c_type[index] = HorizontalSteps;
    }
    c->setStyle((QwtPlotCurve::CurveStyle)s);
}

void Graph::setCurveSymbol(int index, QwtSymbol *s)
{
    QwtPlotCurve *c = curve(index);
    if (!c)
        return;

    c->setSymbol(s);
}

void Graph::setCurvePen(int index, const QPen &p)
{
    QwtPlotCurve *c = curve(index);
    if (!c)
        return;

    c->setPen(p);
}

void Graph::setCurveBrush(int index, const QBrush &b)
{
    QwtPlotCurve *c = curve(index);
    if (!c)
        return;

    c->setBrush(b);
}

void Graph::openBoxDiagram(Table *w, QJsonObject *jsCurve)
{
    if (!w)
        return;

    QJsonObject jsLayout = jsCurve->value("curveLayout").toObject();

    int startRow = jsCurve->value("startRow").toInt();
    int endRow = jsCurve->value("endRow").toInt();

    auto *c = new BoxCurve(w, jsCurve->value("title").toString(), startRow, endRow);
    c->setSamples(QVector<double>(1, jsCurve->value("xValue").toDouble()), QVector<double>());
    c->loadData();

    c_keys.resize(++n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(c);
    c_type.resize(n_curves);
    c_type[n_curves - 1] = Box;

    c->setMaxStyle(SymbolBox::style(jsLayout.value("maxStyle").toInt()));
    c->setP99Style(SymbolBox::style(jsLayout.value("p99Style").toInt()));
    c->setMeanStyle(SymbolBox::style(jsLayout.value("meanStyle").toInt()));
    c->setP1Style(SymbolBox::style(jsLayout.value("p1Style").toInt()));
    c->setMinStyle(SymbolBox::style(jsLayout.value("minStyle").toInt()));

    c->setBoxStyle(jsLayout.value("boxStyle").toInt());
    c->setBoxWidth(jsLayout.value("boxWidth").toInt());
    c->setBoxRange(jsLayout.value("boxRangeType").toInt(), jsLayout.value("boxRange").toDouble());
    c->setWhiskersRange(jsLayout.value("whiskersRangeType").toInt(),
                        jsLayout.value("whiskersRange").toDouble());
}

void Graph::setActiveTool(PlotToolInterface *tool)
{
    if (d_active_tool)
        delete d_active_tool;
    d_active_tool = tool;
    if (d_range_selector) {
        if (tool)
            // we want to use tool now, so disable range selection
            d_range_selector->setEnabled(false);
        else
            // re-enable range selection
            d_range_selector->setEnabled(true);
    }
}

void Graph::disableTools()
{
    if (zoomOn())
        zoom(false);
    if (drawLineActive())
        drawLine(false);
    setActiveTool(nullptr);
    if (d_range_selector)
        delete d_range_selector;
}

bool Graph::enableRangeSelectors(const QObject *status_target, const char *status_slot)
{
    // disable other tools, otherwise it's undefined what tool should handle mouse clicks
    disableTools();
    d_range_selector = new RangeSelectorTool(this, status_target, status_slot);
    connect(d_range_selector, SIGNAL(rangeSelectorChanged()), this, SIGNAL(dataRangeChanged()));
    return true;
}

void Graph::setTextMarkerDefaults(int f, const QFont &font, const QColor &textCol,
                                  const QColor &backgroundCol)
{
    defaultMarkerFont = font;
    defaultMarkerFrame = f;
    defaultTextMarkerColor = textCol;
    defaultTextMarkerBackground = backgroundCol;
}

void Graph::setArrowDefaults(int lineWidth, const QColor &c, Qt::PenStyle style, int headLength,
                             int headAngle, bool fillHead)
{
    defaultArrowLineWidth = lineWidth;
    defaultArrowColor = c;
    defaultArrowLineStyle = style;
    defaultArrowHeadLength = headLength;
    defaultArrowHeadAngle = headAngle;
    defaultArrowHeadFill = fillHead;
}

QString Graph::parentPlotName()
{
    auto *w = dynamic_cast<QWidget *>(parent()->parent()->parent());
    return QString(w->objectName());
}

void Graph::guessUniqueCurveLayout(int &colorIndex, int &symbolIndex)
{
    colorIndex = 0;
    symbolIndex = 0;

    int curve_index = n_curves - 1;
    if (curve_index >= 0
        && c_type[curve_index] == ErrorBars) { // find out the pen color of the master curve
        auto *er = dynamic_cast<QwtErrorPlotCurve *>(d_plot->curve(c_keys[curve_index]));
        DataCurve *master_curve = er->masterCurve();
        if (master_curve) {
            colorIndex = ColorButton::colorIndex(master_curve->pen().color());
            return;
        }
    }

    for (int i = 0; i < n_curves; i++) {
        const QwtPlotCurve *c = curve(i);
        if (c && c->rtti() == QwtPlotItem::Rtti_PlotCurve) {
            int index = ColorButton::colorIndex(c->pen().color());
            if (index > colorIndex)
                colorIndex = index;

            index = SymbolBox::symbolIndex(c->symbol()->style());
            if (index > symbolIndex)
                symbolIndex = index;
        }
    }
    if (n_curves > 1)
        colorIndex = (colorIndex + 1) % 16;
    if (colorIndex == 13) // avoid white invisible curves
        colorIndex = 0;

    symbolIndex = (symbolIndex + 1) % 15;
    if (!symbolIndex)
        symbolIndex = 1;
}

void Graph::addFitCurve(QwtPlotCurve *c)
{
    if (c)
        d_fit_curves << c;
}

void Graph::deleteFitCurves()
{
    for (QwtPlotCurve *c : d_fit_curves)
        removeCurve(curveIndex(c));

    d_plot->replot();
}

void Graph::plotSpectrogram(Matrix *m, CurveType type)
{
    if (type != GrayMap && type != ColorMap && type != ContourMap)
        return;

    auto *d_spectrogram = new Spectrogram(m);
    if (type == GrayMap)
        d_spectrogram->setGrayScale();
    else if (type == ContourMap) {
        d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, false);
        d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    } else if (type == ColorMap) {
        d_spectrogram->setDefaultColorMap();
        d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    }

    c_keys.resize(++n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(d_spectrogram);

    c_type.resize(n_curves);
    c_type[n_curves - 1] = type;

    QwtScaleWidget *rightAxis = d_plot->axisWidget(QwtPlot::yRight);
    rightAxis->setColorBarEnabled(type != ContourMap);
    // rightAxis->setColorMap(d_spectrogram->data().range(), d_spectrogram->colorMap());

    d_plot->setAxisScale(QwtPlot::xBottom, m->xStart(), m->xEnd());
    d_plot->setAxisScale(QwtPlot::yLeft, m->yStart(), m->yEnd());

    // d_plot->setAxisScale(QwtPlot::yRight, d_spectrogram->data().range().minValue(),
    //                     d_spectrogram->data().range().maxValue());
    d_plot->enableAxis(QwtPlot::yRight, type != ContourMap);

    d_plot->replot();
}

void Graph::restoreSpectrogram(ApplicationWindow *app, QJsonObject *jsCurve)
{
    Matrix *m = app->matrix(jsCurve->value("matrix").toString());
    if (!m)
        return;

    auto *sp = new Spectrogram(m);

    c_type.resize(++n_curves);
    c_type[n_curves - 1] = Graph::ColorMap;
    c_keys.resize(n_curves);
    c_keys[n_curves - 1] = d_plot->insertCurve(sp);

    int color_policy = jsCurve->value("colorPolicy").toInt();
    if (color_policy == Spectrogram::GrayScale)
        sp->setGrayScale();
    else if (color_policy == Spectrogram::Default)
        sp->setDefaultColorMap();
    int mode = jsCurve->value("mode").toInt();
    QColor color1 = jsCurve->value("minColor").toString();
    QColor color2 = jsCurve->value("maxColor").toString();

    QwtLinearColorMap colorMap = QwtLinearColorMap(color1, color2);
    colorMap.setMode((QwtLinearColorMap::Mode)mode);

    // int stops = jsCurve->value("colorStops").toInt();
    QJsonArray jsColorStops = jsCurve->value("stops").toArray();
    QJsonArray jsRGBStops = jsCurve->value("colorNames").toArray();
    for (int i = 0; i < jsColorStops.size(); i++) {
        colorMap.addColorStop(jsColorStops.at(i).toDouble(), QColor(jsRGBStops.at(i).toString()));
    }
    sp->setCustomColorMap(); // colorMap);
    sp->setDisplayMode(QwtPlotSpectrogram::ImageMode, jsCurve->value("Image").toInt());
    int contours = jsCurve->value("counterLines").toInt();
    sp->setDisplayMode(QwtPlotSpectrogram::ContourMode, contours);
    if (contours) {
        sp->setLevelsNumber(); // levels);
        int defaultPen = jsCurve->value("defaultPen").toInt();
        if (!defaultPen)
            sp->setDefaultContourPen(QPen(Qt::NoPen));
        else {
            QColor c = QColor(COLORVALUE(jsCurve->value("penColor").toString()));
            int width = jsCurve->value("penWidth").toInt();
            int style = jsCurve->value("penStyle").toInt();
            sp->setDefaultContourPen(QPen(c, width, Graph::getPenStyle(style)));
        }
    }
    QJsonObject jsColorBar = jsCurve->value("colorBar").toObject();
    int color_axis = jsColorBar.value("axis").toInt();
    int width = jsColorBar.value("width").toInt();

    QwtScaleWidget *colorAxis = d_plot->axisWidget(color_axis);
    if (colorAxis) {
        colorAxis->setColorBarWidth(width);
        colorAxis->setColorBarEnabled(true);
    }
    sp->setVisible(jsCurve->value("visible").toBool());
}

bool Graph::validCurvesDataSize() const
{
    if (!n_curves) {
        QMessageBox::warning(const_cast<Graph *>(this), tr("Warning"),
                             tr("There are no curves available on this plot!"));
        return false;
    } else {
        for (int i = 0; i < n_curves; i++) {
            QwtPlotItem *item = curve(i);
            if (item && item->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
                auto *c = dynamic_cast<QwtPlotCurve *>(item);
                if (c->dataSize() >= 2)
                    return true;
            }
        }
        QMessageBox::warning(const_cast<Graph *>(this), tr("Error"),
                             tr("There are no curves with more than two points on this plot. "
                                "Operation aborted!"));
        return false;
    }
}

Graph::~Graph()
{
    setActiveTool(nullptr);
    if (d_range_selector)
        delete d_range_selector;
    delete titlePicker;
    delete scalePicker;
    delete cp;
    delete d_plot;
}

void Graph::setAntialiasing(bool on, bool update)
{
    if (d_antialiasing == on)
        return;

    d_antialiasing = on;

    if (update) {
        QList<int> curve_keys = d_plot->curveKeys();
        for (int i = 0; i < (int)curve_keys.count(); i++) {
            QwtPlotItem *c = d_plot->curve(curve_keys[i]);
            if (c)
                c->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
        }
        QList<int> marker_keys = d_plot->markerKeys();
        for (int i = 0; i < (int)marker_keys.count(); i++) {
            QwtPlotMarker *m = d_plot->marker(marker_keys[i]);
            if (m)
                m->setRenderHint(QwtPlotItem::RenderAntialiased, d_antialiasing);
        }
        d_plot->replot();
    }
}

bool Graph::focusNextPrevChild(bool)
{
    QList<int> mrkKeys = d_plot->markerKeys();
    int n = mrkKeys.size();
    if (n < 2)
        return false;

    int min_key = mrkKeys[0], max_key = mrkKeys[0];
    for (int i = 0; i < n; i++) {
        if (mrkKeys[i] >= max_key)
            max_key = mrkKeys[i];
        if (mrkKeys[i] <= min_key)
            min_key = mrkKeys[i];
    }

    int key = selectedMarker;
    if (key >= 0) {
        key++;
        if (key > max_key)
            key = min_key;
    } else
        key = min_key;

    cp->disableEditing();

    setSelectedMarker(key);
    return true;
}

QString Graph::axisFormatInfo(int axis)
{
    if (axis < 0 || axis >= QwtPlot::axisCnt)
        return QString();
    else
        return axesFormatInfo[axis];
}

void Graph::updateCurveNames(const QString &oldName, const QString &newName, bool updateTableName)
{
    // update plotted curves list
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (!it)
            continue;
        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;

        auto *c = dynamic_cast<DataCurve *>(it);
        if (c && c->plotAssociation().contains(oldName))
            c->updateColumnNames(oldName, newName, updateTableName);
    }

    if (legendMarkerID >= 0) { // update legend
        auto *mrk = dynamic_cast<Legend *>(d_plot->marker(legendMarkerID));
        if (mrk) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList lst = mrk->text().split("\n", Qt::SkipEmptyParts);
#else
            QStringList lst = mrk->text().split("\n", QString::SkipEmptyParts);
#endif
            lst.replaceInStrings(oldName, newName);
            mrk->setText(lst.join("\n"));
            d_plot->replot();
        }
    }
}

void Graph::setCurveFullRange(int curveIndex)
{
    auto *c = dynamic_cast<DataCurve *>(curve(curveIndex));
    if (c) {
        c->setFullRange();
        updatePlot();
        Q_EMIT modifiedGraph();
    }
}

DataCurve *Graph::masterCurve(QwtErrorPlotCurve *er)
{
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if ((dynamic_cast<PlotCurve *>(it))->type() == Function)
            continue;

        if ((dynamic_cast<DataCurve *>(it))->plotAssociation()
            == er->masterCurve()->plotAssociation())
            return dynamic_cast<DataCurve *>(it);
    }
    return nullptr;
}

DataCurve *Graph::masterCurve(const QString &xColName, const QString &yColName)
{
    QString master_curve = xColName + "(X)," + yColName + "(Y)";
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (!it)
            continue;
        if (it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram)
            continue;
        if ((dynamic_cast<PlotCurve *>(it))->type() == Function)
            continue;

        if ((dynamic_cast<DataCurve *>(it))->plotAssociation() == master_curve)
            return dynamic_cast<DataCurve *>(it);
    }
    return nullptr;
}

void Graph::showCurve(int index, bool visible)
{
    QwtPlotItem *it = plotItem(index);
    if (it)
        it->setVisible(visible);

    Q_EMIT modifiedGraph();
}

int Graph::visibleCurves()
{
    int c = 0;
    QList<int> keys = d_plot->curveKeys();
    for (int i = 0; i < (int)keys.count(); i++) {
        QwtPlotItem *it = d_plot->plotItem(keys[i]);
        if (it && it->isVisible())
            c++;
    }
    return c;
}

QPageSize::PageSizeId Graph::minPageSize(const QPrinter &printer, const QRect &r)
{
    double x_margin = 0.2 / 2.54 * printer.logicalDpiX(); // 2 mm margins
    double y_margin = 0.2 / 2.54 * printer.logicalDpiY();
    double w_mm = 2 * x_margin + (double)(r.width()) / (double)printer.logicalDpiX() * 25.4;
    double h_mm = 2 * y_margin + (double)(r.height()) / (double)printer.logicalDpiY() * 25.4;

    int w = 0, h = 0;
    if (w_mm / h_mm > 1) {
        w = (int)ceil(w_mm);
        h = (int)ceil(h_mm);
    } else {
        h = (int)ceil(w_mm);
        w = (int)ceil(h_mm);
    }

    QPageSize::PageSizeId size = QPageSize::A5;
    if (w < 45 && h < 32)
        size = QPageSize::B10;
    else if (w < 52 && h < 37)
        size = QPageSize::A9;
    else if (w < 64 && h < 45)
        size = QPageSize::B9;
    else if (w < 74 && h < 52)
        size = QPageSize::A8;
    else if (w < 91 && h < 64)
        size = QPageSize::B8;
    else if (w < 105 && h < 74)
        size = QPageSize::A7;
    else if (w < 128 && h < 91)
        size = QPageSize::B7;
    else if (w < 148 && h < 105)
        size = QPageSize::A6;
    else if (w < 182 && h < 128)
        size = QPageSize::B6;
    else if (w < 210 && h < 148)
        size = QPageSize::A5;
    else if (w < 220 && h < 110)
        size = QPageSize::DLE;
    else if (w < 229 && h < 163)
        size = QPageSize::C5E;
    else if (w < 241 && h < 105)
        size = QPageSize::Comm10E;
    else if (w < 257 && h < 182)
        size = QPageSize::B5;
    else if (w < 279 && h < 216)
        size = QPageSize::Letter;
    else if (w < 297 && h < 210)
        size = QPageSize::A4;
    else if (w < 330 && h < 210)
        size = QPageSize::Folio;
    else if (w < 356 && h < 216)
        size = QPageSize::Legal;
    else if (w < 364 && h < 257)
        size = QPageSize::B4;
    else if (w < 420 && h < 297)
        size = QPageSize::A3;
    else if (w < 515 && h < 364)
        size = QPageSize::B3;
    else if (w < 594 && h < 420)
        size = QPageSize::A2;
    else if (w < 728 && h < 515)
        size = QPageSize::B2;
    else if (w < 841 && h < 594)
        size = QPageSize::A1;
    else if (w < 1030 && h < 728)
        size = QPageSize::B1;
    else if (w < 1189 && h < 841)
        size = QPageSize::A0;
    else if (w < 1456 && h < 1030)
        size = QPageSize::B0;

    return size;
}

int Graph::mapToQwtAxis(int axis)
{
    int a = -1;
    switch (axis) {
    case 0:
        a = QwtPlot::xBottom;
        break;
    case 1:
        a = QwtPlot::yLeft;
        break;
    case 2:
        a = QwtPlot::xTop;
        break;
    case 3:
        a = QwtPlot::yRight;
        break;
    }
    return a;
}

void Graph::print(QPainter *painter, const QRect &plotRect)
{
    d_plot->print(painter, plotRect);
}

void Graph::deselect()
{
    deselectMarker();
    scalePicker->deselect();
    titlePicker->setSelected(false);
}
