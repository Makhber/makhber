/***************************************************************************
        File                 : Spectrogram.cpp
        Project              : Makhber
--------------------------------------------------------------------
        Copyright            : (C) 2006 by Ion Vasilief
        Email (use @ for *)  : ion_vasilief*yahoo.fr
        Description          : Makhber's Spectrogram Class
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

#include "Spectrogram.h"

#include "core/ColorButton.h"

#include <qwt_scale_widget.h>

#include <QPen>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>

#include <cmath>

Spectrogram::Spectrogram()
    : QwtPlotSpectrogram(),
      d_matrix(nullptr),
      color_axis(QwtPlot::yRight),
      color_map_policy(Default),
      color_map(new QwtLinearColorMap())
{
}

Spectrogram::Spectrogram(Matrix *m)
    : QwtPlotSpectrogram(QString(m->name())),
      d_matrix(m),
      color_axis(QwtPlot::yRight),
      color_map_policy(Default),
      color_map(new QwtLinearColorMap())
{
    setData(new MatrixData(m));
    double step =
            fabs(data()->interval(Qt::ZAxis).maxValue() - data()->interval(Qt::ZAxis).minValue())
            / 5;

    QList<double> contourLevels;
    for (size_t i = 1; i < 5; i++)
        contourLevels += data()->interval(Qt::ZAxis).minValue() + i * step;

    setContourLevels(contourLevels);
}

void Spectrogram::updateData(Matrix *m)
{
    if (!m)
        return;

    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    setData(new MatrixData(m));
    setLevelsNumber(levels());

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    if (colorAxis)
        colorAxis->setColorMap(data()->interval(Qt::ZAxis),
                               new QwtLinearColorMap(colorMap()->format()));

    plot->setAxisScale(color_axis, data()->interval(Qt::ZAxis).minValue(),
                       data()->interval(Qt::ZAxis).maxValue());
    plot->replot();
}

void Spectrogram::setLevelsNumber(int levels)
{
    double step =
            fabs(data()->interval(Qt::ZAxis).maxValue() - data()->interval(Qt::ZAxis).minValue())
            / levels;

    QList<double> contourLevels;
    for (size_t i = 1; i < static_cast<size_t>(levels); i++)
        contourLevels += data()->interval(Qt::ZAxis).minValue() + i * step;

    setContourLevels(contourLevels);
}

bool Spectrogram::hasColorScale()
{
    QwtPlot *plot = this->plot();
    if (!plot)
        return false;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    return colorAxis->isColorBarEnabled();
}

void Spectrogram::showColorScale(int axis, bool on)
{
    if (hasColorScale() == on && color_axis == axis)
        return;

    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    colorAxis->setColorBarEnabled(false);

    color_axis = axis;

    // We must switch main and the color scale axes and their respective scales
    int xAxis = this->xAxis();
    int yAxis = this->yAxis();
    int oldMainAxis = 0;
    if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop) {
        oldMainAxis = xAxis;
        xAxis = 5 - color_axis;
    } else if (axis == QwtPlot::yLeft || axis == QwtPlot::yRight) {
        oldMainAxis = yAxis;
        yAxis = 1 - color_axis;
    }

    // First we switch axes
    setAxes(xAxis, yAxis);

    // Next we switch axes scales
    QwtScaleDiv scDiv = plot->axisScaleDiv(oldMainAxis);
    if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
        plot->setAxisScale(xAxis, scDiv.lowerBound(), scDiv.upperBound());
    else if (axis == QwtPlot::yLeft || color_axis == QwtPlot::yRight)
        plot->setAxisScale(yAxis, scDiv.lowerBound(), scDiv.upperBound());

    colorAxis = plot->axisWidget(color_axis);
    plot->setAxisScale(color_axis, data()->interval(Qt::ZAxis).minValue(),
                       data()->interval(Qt::ZAxis).maxValue());
    colorAxis->setColorBarEnabled(on);
    colorAxis->setColorMap(data()->interval(Qt::ZAxis),
                           new QwtLinearColorMap(colorMap()->format()));
    if (!plot->axisEnabled(color_axis))
        plot->enableAxis(color_axis);
    colorAxis->show();
    plot->updateLayout();
}

int Spectrogram::colorBarWidth()
{
    QwtPlot *plot = this->plot();
    if (!plot)
        return 0;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    return colorAxis->colorBarWidth();
}

void Spectrogram::setColorBarWidth(int width)
{
    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    colorAxis->setColorBarWidth(width);
}

Spectrogram *Spectrogram::copy()
{
    auto *new_s = new Spectrogram(matrix());
    new_s->setDisplayMode(QwtPlotSpectrogram::ImageMode,
                          testDisplayMode(QwtPlotSpectrogram::ImageMode));
    new_s->setDisplayMode(QwtPlotSpectrogram::ContourMode,
                          testDisplayMode(QwtPlotSpectrogram::ContourMode));
    new_s->setColorMap(new QwtLinearColorMap(colorMap()->format()));
    new_s->setAxes(xAxis(), yAxis());
    new_s->setDefaultContourPen(defaultContourPen());
    new_s->setLevelsNumber(levels());
    new_s->color_map_policy = color_map_policy;
    return new_s;
}

void Spectrogram::setGrayScale()
{
    color_map->setColorInterval(Qt::black, Qt::white);
    setColorMap(color_map);
    color_map_policy = GrayScale;

    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    if (colorAxis)
        colorAxis->setColorMap(data()->interval(Qt::ZAxis),
                               new QwtLinearColorMap(colorMap()->format()));
}

void Spectrogram::setDefaultColorMap()
{
    color_map = defaultColorMap();
    setColorMap(color_map);
    color_map_policy = Default;

    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    if (colorAxis)
        colorAxis->setColorMap(this->data()->interval(Qt::ZAxis),
                               new QwtLinearColorMap(this->colorMap()->format()));
}

void Spectrogram::setCustomColorMap(QwtLinearColorMap *map)
{
    setColorMap(map);
    color_map = map;
    color_map_policy = Custom;

    QwtPlot *plot = this->plot();
    if (!plot)
        return;

    QwtScaleWidget *colorAxis = plot->axisWidget(color_axis);
    if (colorAxis)
        colorAxis->setColorMap(this->data()->interval(Qt::ZAxis),
                               new QwtLinearColorMap(this->colorMap()->format()));
}

QwtLinearColorMap *Spectrogram::defaultColorMap()
{
    QwtLinearColorMap *colorMap = new QwtLinearColorMap(Qt::blue, Qt::red);
    colorMap->addColorStop(0.25, Qt::cyan);
    colorMap->addColorStop(0.5, Qt::green);
    colorMap->addColorStop(0.75, Qt::yellow);
    return colorMap;
}

void Spectrogram::saveToJson(QJsonObject *jsObject)
{
    jsObject->insert("matrix", d_matrix->name());

    if (color_map_policy != Custom)
        jsObject->insert("colorPolicy", color_map_policy);
    else {
        QJsonObject jsColorMap {};
        jsColorMap.insert("mode", color_map->mode());
        jsColorMap.insert("minColor", COLORNAME(color_map->color1()));
        jsColorMap.insert("maxColor", COLORNAME(color_map->color2()));
        QVector<double> colors = color_map->colorStops();
        int stops = (int)colors.size();
        QJsonArray jsColorStops {};
        QJsonArray jsRGBStops {};
        for (int i = 1; i < stops - 1; i++) {
            jsColorStops.append(colors[i]);
            jsRGBStops.append(COLORNAME(QColor(color_map->rgb(QwtInterval(0, 1), colors[i]))));
        }
        jsColorMap.insert("colorStops", jsColorStops);
        jsColorMap.insert("colorNames", jsRGBStops);
        jsObject->insert("colorMap", jsColorMap);
    }
    jsObject->insert("image", testDisplayMode(QwtPlotSpectrogram::ImageMode));

    bool contourLines = testDisplayMode(QwtPlotSpectrogram::ContourMode);
    jsObject->insert("contourLines", contourLines);
    if (contourLines) {
        jsObject->insert("levels", levels());
        bool defaultPen = defaultContourPen().style() != Qt::NoPen;
        jsObject->insert("defaultPen", defaultPen);
        if (defaultPen) {
            jsObject->insert("penColor", COLORNAME(defaultContourPen().color()));
            jsObject->insert("penWidth", defaultContourPen().width());
            jsObject->insert("penStyle", defaultContourPen().style() - 1);
        }
    }
    QwtScaleWidget *colorAxis = plot()->axisWidget(color_axis);
    if (colorAxis && colorAxis->isColorBarEnabled()) {
        QJsonObject jsColorBar {};
        jsColorBar.insert("axis", color_axis);
        jsColorBar.insert("width", colorAxis->colorBarWidth());
        jsObject->insert("colorBar", jsColorBar);
    }
    jsObject->insert("visible", isVisible());
    jsObject->insert("type", "spectrogram");
}

MatrixData::MatrixData(Matrix *m) : d_matrix(m)
{
    double min_z = std::numeric_limits<double>::max();
    double max_z = -std::numeric_limits<double>::max();
    QVector<double> matrix_vect {};
    double matrix_element {};
    for (int i = 0; i < d_matrix->numRows(); i++) {
        for (int j = 0; j < d_matrix->numCols(); j++) {
            // replace NaNs and Infs with average of neighbours
            if (std::isfinite(d_matrix->cell(i, j)))
                matrix_element = d_matrix->cell(i, j);
            else {
                double av = 0;
                unsigned cnt = 0;
                for (int ii = -1; ii <= 1; ii += 2)
                    for (int jj = -1; jj <= 1; jj += 2)
                        if (std::isfinite(d_matrix->cell(i + ii, j + jj))) {
                            av += d_matrix->cell(i + ii, j + jj);
                            cnt++;
                        }
                if (cnt > 0)
                    av /= cnt;
                matrix_element = av;
            }
            matrix_vect << matrix_element;
            min_z = std::min(min_z, matrix_element);
            max_z = std::max(max_z, matrix_element);
        }
    }
    setValueMatrix(matrix_vect, d_matrix->numCols());

    setInterval(Qt::ZAxis, QwtInterval(min_z, max_z));

    setInterval(Qt::XAxis, QwtInterval(d_matrix->xStart(), d_matrix->xEnd()));

    setInterval(Qt::YAxis, QwtInterval(d_matrix->yStart(), d_matrix->yEnd()));
#if QWT_VERSION >= 0x060200
    setResampleMode(BicubicInterpolation);
#else
    setResampleMode(BilinearInterpolation);
#endif
}
