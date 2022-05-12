/***************************************************************************
    File                 : VectorCurve.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Vector curve class

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
#include "VectorCurve.h"

#include <qwt_painter.h>
#include <qwt_scale_map.h>

#include <QPainter>
#include <QLocale>

#include <cmath>

VectorCurve::VectorCurve(VectorStyle style, Table *t, const QString &xColName, QString name,
                         const QString &endCol1, const QString &endCol2, int startRow, int endRow)
    : DataCurve(t, xColName, name, startRow, endRow)
{
    switch (style) {
    case XYXY:
        d_type = Graph::VectXYXY;
        break;
    case XYAM:
        d_type = Graph::VectXYAM;
        break;
    }
    pen = QPen(Qt::black, 1, Qt::SolidLine);
    filledArrow = true;
    d_headLength = 4;
    d_headAngle = 45;
    d_position = Tail;

    d_end_x_a = endCol1;
    d_end_y_m = endCol2;
}

void VectorCurve::copy(const VectorCurve *vc)
{
    d_type = vc->type();
    filledArrow = vc->filledArrow;
    d_headLength = vc->d_headLength;
    d_headAngle = vc->d_headAngle;
    d_position = vc->d_position;
    pen = vc->pen;
#if QWT_VERSION >= 0x060200
    vectorEnd = new QwtPointArrayData<double>(vc->vectorEnd->xData(), vc->vectorEnd->yData());
#else
    vectorEnd = new QwtPointArrayData(vc->vectorEnd->xData(), vc->vectorEnd->yData());
#endif
}

void VectorCurve::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                       const QRectF &canvasRect) const
{
    if (!painter || dataSize() == 0)
        return;

    QwtPlotCurve::draw(painter, xMap, yMap, canvasRect);
    painter->save();
    painter->setPen(pen);
    drawVector(painter, xMap, yMap, 0, static_cast<int>(dataSize() - 1));
    painter->restore();
}

void VectorCurve::drawVector(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                             int from, int to) const
{
    if (d_type == Graph::VectXYAM) {
        for (int i = from; i <= to; i++) {
            const double x0 = sample(i).x();
            const double y0 = sample(i).y();
            const double angle = vectorEnd->sample(i).x();
            const double mag = vectorEnd->sample(i).y();

            int xs = 0, ys = 0, xe = 0, ye = 0;
            switch (d_position) {
            case Tail:
                xs = xMap.transform(x0);
                ys = yMap.transform(y0);
                xe = xMap.transform(x0 + mag * cos(angle));
                ye = yMap.transform(y0 + mag * sin(angle));
                break;

            case Middle: {
                double dxh = 0.5 * mag * cos(angle);
                double dyh = 0.5 * mag * sin(angle);
                xs = xMap.transform(x0 - dxh);
                ys = yMap.transform(y0 - dyh);
                xe = xMap.transform(x0 + dxh);
                ye = yMap.transform(y0 + dyh);
            } break;

            case Head:
                xs = xMap.transform(x0 - mag * cos(angle));
                ys = yMap.transform(y0 - mag * sin(angle));
                xe = xMap.transform(x0);
                ye = yMap.transform(y0);
                break;
            }
            QwtPainter::drawLine(painter, xs, ys, xe, ye);
            drawArrowHead(painter, xs, ys, xe, ye);
        }
    } else {
        for (int i = from; i <= to; i++) {
            const int xs = xMap.transform(sample(i).x());
            const int ys = yMap.transform(sample(i).y());
            const int xe = xMap.transform(vectorEnd->sample(i).x());
            const int ye = yMap.transform(vectorEnd->sample(i).y());
            QwtPainter::drawLine(painter, xs, ys, xe, ye);
            drawArrowHead(painter, xs, ys, xe, ye);
        }
    }
}

void VectorCurve::drawArrowHead(QPainter *p, int xs, int ys, int xe, int ye) const
{
    p->save();
    p->translate(xe, ye);
    double t = theta(xs, ys, xe, ye);
    p->rotate(-t);

    double pi = 4 * atan(-1.0);
    int d = qRound(d_headLength * tan(pi * (double)d_headAngle / 180.0));

    QPolygon endArray(3);
    endArray[0] = QPoint(0, 0);
    endArray[1] = QPoint(-d_headLength, d);
    endArray[2] = QPoint(-d_headLength, -d);

    if (filledArrow)
        p->setBrush(QBrush(pen.color(), Qt::SolidPattern));

    QwtPainter::drawPolygon(p, endArray);
    p->restore();
}

double VectorCurve::theta(int x0, int y0, int x1, int y1) const
{
    double t = NAN, pi = 4 * atan(-1.0);
    if (x1 == x0) {
        if (y0 > y1)
            t = 90;
        else
            t = 270;
    } else {
        t = atan2((y1 - y0) * 1.0, (x1 - x0) * 1.0) * 180 / pi;
        if (t < 0)
            t = 360 + t;
    }
    return t;
}

void VectorCurve::setVectorEnd(const QString &xColName, const QString &yColName)
{
    if (d_end_x_a == xColName && d_end_y_m == yColName)
        return;

    d_end_x_a = xColName;
    d_end_y_m = yColName;

    loadData();
}

void VectorCurve::setVectorEnd(const QVector<double> &x, const QVector<double> &y)
{
#if QWT_VERSION >= 0x060200
    vectorEnd = new QwtPointArrayData<double>(x, y);
#else
    vectorEnd = new QwtPointArrayData(x, y);
#endif
}

int VectorCurve::width()
{
    return pen.width();
}

void VectorCurve::setWidth(int w)
{
    pen.setWidth(w);
}

QColor VectorCurve::color()
{
    return pen.color();
}

void VectorCurve::setColor(const QColor &c)
{
    if (pen.color() != c)
        pen.setColor(c);
}

void VectorCurve::setHeadLength(int l)
{
    if (d_headLength != l)
        d_headLength = l;
}

void VectorCurve::setHeadAngle(int a)
{
    if (d_headAngle != a)
        d_headAngle = a;
}

void VectorCurve::fillArrowHead(bool fill)
{
    if (filledArrow != fill)
        filledArrow = fill;
}

QRectF VectorCurve::boundingRect() const
{
    QRectF rect = QwtPlotCurve::boundingRect();

    if (d_type == Graph::VectXYXY)
        rect |= vectorEnd->boundingRect();
    else {
        int rows = abs(d_end_row - d_start_row) + 1;
        for (int i = 0; i < rows; i++) {
            double x_i = sample(i).x();
            double y_i = sample(i).y();
            double angle = vectorEnd->sample(i).x();
            double mag = vectorEnd->sample(i).y();
            switch (d_position) {
            case Tail:
                rect |= QRectF(x_i, y_i, mag * cos(angle), mag * sin(angle)).normalized();
                break;
            case Middle: {
                QRectF rect_i(0, 0, fabs(mag * cos(angle)), fabs(mag * sin(angle)));
                rect_i.moveCenter(QPointF(x_i, y_i));
                rect |= rect_i;
                break;
            }
            case Head:
                rect |= QRectF(x_i, y_i, -mag * cos(angle), -mag * sin(angle)).normalized();
                break;
            }
        }
    }
    return rect;
}

void VectorCurve::updateColumnNames(const QString &oldName, const QString &newName,
                                    bool updateTableName)
{
    if (updateTableName) {
        QString s = title().text();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = s.split("_", Qt::SkipEmptyParts);
        if (lst[0] == oldName)
            setTitle(newName + "_" + lst[1]);

        lst = d_x_column.split("_", Qt::SkipEmptyParts);
        if (lst[0] == oldName)
            d_x_column = newName + "_" + lst[1];

        lst = d_end_x_a.split("_", Qt::SkipEmptyParts);
        if (lst[0] == oldName)
            d_end_x_a = newName + "_" + lst[1];

        lst = d_end_y_m.split("_", Qt::SkipEmptyParts);
        if (lst[0] == oldName)
            d_end_y_m = newName + "_" + lst[1];
#else
        QStringList lst = s.split("_", QString::SkipEmptyParts);
        if (lst[0] == oldName)
            setTitle(newName + "_" + lst[1]);

        lst = d_x_column.split("_", QString::SkipEmptyParts);
        if (lst[0] == oldName)
            d_x_column = newName + "_" + lst[1];

        lst = d_end_x_a.split("_", QString::SkipEmptyParts);
        if (lst[0] == oldName)
            d_end_x_a = newName + "_" + lst[1];

        lst = d_end_y_m.split("_", QString::SkipEmptyParts);
        if (lst[0] == oldName)
            d_end_y_m = newName + "_" + lst[1];
#endif
    } else {
        if (title().text() == oldName)
            setTitle(newName);
        if (d_x_column == oldName)
            d_x_column = newName;
        if (d_end_x_a == oldName)
            d_end_x_a = newName;
        if (d_end_y_m == oldName)
            d_end_y_m = newName;
    }
}

QString VectorCurve::plotAssociation()
{
    QString base = d_x_column + "(X)," + title().text() + "(Y)," + d_end_x_a;
    if (d_type == Graph::VectXYAM)
        return base + "(A)," + d_end_y_m + "(M)";
    else
        return base + "(X)," + d_end_y_m + "(Y)";
}

bool VectorCurve::updateData(Table *t, const QString &colName)
{
    if (d_table != t
        || (colName != title().text() && d_x_column != colName && d_end_x_a != colName
            && d_end_y_m != colName))
        return false;

    return loadData();
}

bool VectorCurve::loadData()
{
    int xcol = d_table->colIndex(d_x_column);
    int ycol = d_table->colIndex(title().text());
    int endXCol = d_table->colIndex(d_end_x_a);
    int endYCol = d_table->colIndex(d_end_y_m);

    int rows = abs(d_end_row - d_start_row) + 1;
    QVector<double> X(rows), Y(rows), X2(rows), Y2(rows);
    int size = 0;
    for (int i = d_start_row; i <= d_end_row; i++) {
        QString xval = d_table->text(i, xcol);
        QString yval = d_table->text(i, ycol);
        QString xend = d_table->text(i, endXCol);
        QString yend = d_table->text(i, endYCol);
        if (!xval.isEmpty() && !yval.isEmpty() && !xend.isEmpty() && !yend.isEmpty()) {
            bool valid_data = true;
            X[size] = QLocale().toDouble(xval, &valid_data);
            if (!valid_data)
                continue;
            Y[size] = QLocale().toDouble(yval, &valid_data);
            if (!valid_data)
                continue;
            X2[size] = QLocale().toDouble(xend, &valid_data);
            if (!valid_data)
                continue;
            Y2[size] = QLocale().toDouble(yend, &valid_data);
            if (valid_data)
                size++;
        }
    }

    if (!size)
        return false;

    X.resize(size);
    Y.resize(size);
    X2.resize(size);
    Y2.resize(size);
    setSamples(X.data(), Y.data(), size);
    for (DataCurve *c : d_error_bars)
        c->setSamples(X.data(), Y.data(), size);
    setVectorEnd(X2, Y2);

    return true;
}

VectorCurve::~VectorCurve()
{
    // delete vectorEnd;
}
