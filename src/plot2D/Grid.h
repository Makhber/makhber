/***************************************************************************
    File                 : Grid.h
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
#ifndef GRID_H
#define GRID_H

#include "core/MakhberDefs.h"

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>

#include <QPen>

//! 2D Grid class
class MAKHBER_EXPORT Grid : public QwtPlotGrid
{
public:
    Grid();

    void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
              const QRectF &rect) const;
    void drawLines(QPainter *painter, const QRectF &rect, Qt::Orientation orientation,
                   const QwtScaleMap &map, const QList<double> &values) const;

    bool xZeroLineEnabled() { return (mrkX >= 0) ? true : false; };
    void enableZeroLineX(bool enable = true);
    bool yZeroLineEnabled() { return (mrkY >= 0) ? true : false; };
    void enableZeroLineY(bool enable = true);

    void setMajPenX(const QPen &p) { setMajorPen(p); };
    const QPen &majPenX() const { return majorPen(); };

    void setMinPenX(const QPen &p) { setMinorPen(p); };
    const QPen &minPenX() const { return minorPen(); };

    void setMajPenY(const QPen &p)
    {
        if (d_maj_pen_y != p)
            d_maj_pen_y = p;
    };
    const QPen &majPenY() const { return d_maj_pen_y; };

    void setMinPenY(const QPen &p)
    {
        if (d_min_pen_y != p)
            d_min_pen_y = p;
    };
    const QPen &minPenY() const { return d_min_pen_y; };

    void load(const QStringList &);
    void copy(Grid *);
    void saveToJson(QJsonObject *);

private:
    QPen d_maj_pen_y;
    QPen d_min_pen_y;

    long mrkX, mrkY; // x=0 et y=0 line markers keys
};

#endif
