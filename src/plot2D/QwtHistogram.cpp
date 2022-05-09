/***************************************************************************
    File                 : QwtHistogram.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Histogram class

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
#include "QwtHistogram.h"

#include "aspects/column/Column.h"

#include <qwt_scale_map.h>

#include <QPainter>
#include <QLocale>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_histogram.h>

#include <array>
#include <cmath>

QwtHistogram::QwtHistogram(Table *t, const QString &name, int startRow, int endRow)
    : QwtBarCurve(QwtBarCurve::Vertical, t, "dummy", name, startRow, endRow)
{
    d_autoBin = true;
}

void QwtHistogram::copy(const QwtHistogram *h)
{
    QwtBarCurve::copy((const QwtBarCurve *)h);

    d_autoBin = h->d_autoBin;
    d_bin_size = h->d_bin_size;
    d_begin = h->d_begin;
    d_end = h->d_end;
}

void QwtHistogram::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                        int from, int to) const
{
    if (!painter || dataSize() == 0)
        return;

    if (to < 0)
        to = static_cast<int>(dataSize()) - 1;

    painter->save();
    painter->setPen(QwtPlotCurve::pen());
    painter->setBrush(QwtPlotCurve::brush());

    const int ref = yMap.transform(baseline());
    const int dx = abs(xMap.transform(sample(from + 1).x()) - xMap.transform(sample(from).x()));
    const int bar_width = int(dx * (1 - gap() * 0.01));
    const int half_width = int(0.5 * (dx - bar_width));
    const int xOffset = int(0.01 * offset() * bar_width);

    for (int i = from; i <= to; i++) {
        const int px1 = xMap.transform(sample(i).x());
        const int py1 = yMap.transform(sample(i).y());
        painter->drawRect(px1 + half_width + xOffset, py1, bar_width + 1, (ref - py1 + 1));
    }

    painter->restore();
}

QRectF QwtHistogram::boundingRect() const
{
    QRectF rect = QwtPlotCurve::boundingRect();
    rect.setLeft(rect.left() - sample(1).x());
    rect.setRight(rect.right() + sample(static_cast<int>(dataSize()) - 1).x());
    rect.setTop(0);
    rect.setBottom(1.2 * rect.bottom());
    return rect;
}

void QwtHistogram::setBinning(bool autoBin, double size, double begin, double end)
{
    d_autoBin = autoBin;
    d_bin_size = size;
    d_begin = begin;
    d_end = end;
}

bool QwtHistogram::loadData()
{
    int r = abs(d_end_row - d_start_row) + 1;
    QVarLengthArray<double> Y(r);

    int ycol = d_table->colIndex(title().text());
    Column *y_col_ptr = d_table->column(ycol);
    auto yColType = d_table->columnType(ycol);
    int size = 0;
    for (int row = d_start_row; row <= d_end_row && row < y_col_ptr->rowCount(); row++) {
        if (!y_col_ptr->isInvalid(row)) {
            if (yColType == Makhber::ColumnMode::Text) {
                QString yval = y_col_ptr->textAt(row);
                bool valid_data = true;
                Y[size] = QLocale().toDouble(yval, &valid_data);
                if (!valid_data)
                    continue;
            } else
                Y[size] = y_col_ptr->valueAt(row);
            size++;
        }
    }

    if (size < 2 || (size == 2 && Y[0] == Y[1])) { // non valid histogram
        std::array<double, 2> X;
        Y.resize(2);
        for (int i = 0; i < 2; i++) {
            Y[i] = 0;
            X[i] = 0;
        }
        setSamples(X.data(), Y.data(), 2);
        return false;
    }

    int n = 0;
    gsl_histogram *h = nullptr;
    if (d_autoBin) {
        n = 10;
        h = gsl_histogram_alloc(n);
        if (!h)
            return false;

        gsl_vector *v = gsl_vector_alloc(size);
        for (int i = 0; i < size; i++)
            gsl_vector_set(v, i, Y[i]);

        double min = NAN, max = NAN;
        gsl_vector_minmax(v, &min, &max);
        gsl_vector_free(v);

        d_begin = floor(min);
        d_end = ceil(max);
        d_bin_size = (d_end - d_begin) / (double)n;

        gsl_histogram_set_ranges_uniform(h, floor(min), ceil(max));
    } else {
        n = int((d_end - d_begin) / d_bin_size + 1);
        h = gsl_histogram_alloc(n);
        if (!h)
            return false;

        auto *range = new double[n + 2];
        for (int i = 0; i <= n + 1; i++)
            range[i] = d_begin + i * d_bin_size;

        gsl_histogram_set_ranges(h, range, n + 1);
        delete[] range;
    }

    for (int i = 0; i < size; i++)
        gsl_histogram_increment(h, Y[i]);

    std::vector<double> X(n); // stores ranges (x) and bins (y)
    Y.resize(n);
    for (int i = 0; i < n; i++) {
        Y[i] = gsl_histogram_get(h, i);
        double lower = NAN, upper = NAN;
        gsl_histogram_get_range(h, i, &lower, &upper);
        X[i] = lower;
    }
    setSamples(X.data(), Y.data(), n);

    d_mean = gsl_histogram_mean(h);
    d_standard_deviation = gsl_histogram_sigma(h);
    d_min = gsl_histogram_min_val(h);
    d_max = gsl_histogram_max_val(h);

    gsl_histogram_free(h);

    return true;
}

void QwtHistogram::initData(const QVector<double> &Y, int size)
{
    if (size < 2 || (size == 2 && Y[0] == Y[1])) { // non valid histogram data
        std::array<double, 2> x, y;
        for (int i = 0; i < 2; i++) {
            y[i] = 0;
            x[i] = 0;
        }
        setSamples(x.data(), y.data(), 2);
        return;
    }

    const int n = 10; // default value
    std::array<double, n> x, y; // store ranges (x) and bins (y)
    gsl_histogram *h = gsl_histogram_alloc(n);
    if (!h)
        return;

    gsl_vector *v = nullptr;
    v = gsl_vector_alloc(size);
    for (int i = 0; i < size; i++)
        gsl_vector_set(v, i, Y[i]);

    double min = NAN, max = NAN;
    gsl_vector_minmax(v, &min, &max);
    gsl_vector_free(v);

    d_begin = floor(min);
    d_end = ceil(max);

    gsl_histogram_set_ranges_uniform(h, floor(min), ceil(max));

    for (int i = 0; i < size; i++)
        gsl_histogram_increment(h, Y[i]);

    for (int i = 0; i < n; i++) {
        y[i] = gsl_histogram_get(h, i);
        double lower = NAN, upper = NAN;
        gsl_histogram_get_range(h, i, &lower, &upper);
        x[i] = lower;
    }

    setSamples(x.data(), y.data(), n);

    d_bin_size = (d_end - d_begin) / (double)n;
    d_autoBin = true;
    d_mean = gsl_histogram_mean(h);
    d_standard_deviation = gsl_histogram_sigma(h);
    d_min = gsl_histogram_min_val(h);
    d_max = gsl_histogram_max_val(h);

    gsl_histogram_free(h);
}
