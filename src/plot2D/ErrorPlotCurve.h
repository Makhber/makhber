/***************************************************************************
    File                 : ErrorPlotCurve.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Error bars curve

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
#ifndef ERRORBARS_H
#define ERRORBARS_H

#include "plot2D/PlotCurve.h"

#include <qwt_plot.h>

//! Error bars curve
class MAKHBER_EXPORT ErrorPlotCurve : public DataCurve
{
public:
    ErrorPlotCurve(Qt::Orientation orientation, Table *t, const QString &name);
    ErrorPlotCurve(Table *t, const QString &name);

    void copy(const ErrorPlotCurve *e);

    QRectF boundingRect() const override;

    double errorValue(int i) const;
    QVector<double> errors() const { return err; };
    void setErrors(const QVector<double> &data) { err = data; };

    int capLength() const { return cap; };
    void setCapLength(int t) { cap = t; };

    int width() const { return pen().width(); };
    void setWidth(int w);

    QColor color() const { return pen().color(); };
    void setColor(const QColor &c);

    Qt::Orientation direction() const { return type; };
    void setDirection(Qt::Orientation o) { type = o; };

    bool xErrors() const;
    void setXErrors(bool yes);

    bool throughSymbol() const { return through; };
    void drawThroughSymbol(bool yes) { through = yes; };

    bool plusSide() const { return plus; };
    void drawPlusSide(bool yes) { plus = yes; };

    bool minusSide() const { return minus; };
    void drawMinusSide(bool yes) { minus = yes; };

    //! Returns the master curve to which this error bars curve is attached.
    DataCurve *masterCurve() const { return d_master_curve; };
    void setMasterCurve(DataCurve *c);

    //! Causes the master curve to delete this curve from its managed error bars list.
    void detachFromMasterCurve() { d_master_curve->removeErrorBars(this); };

    QString plotAssociation() const override;

    bool updateData(Table *t, const QString &colName) override;
    virtual bool loadData() override;

private:
    virtual void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                      const QRectF &canvasRect) const override;

    void drawErrorBars(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                       int from, int to) const;

    //! Stores the error bar values
    QVector<double> err;

    //! Orientation of the bars: Horizontal or Vertical
    Qt::Orientation type;

    //! Length of the bar cap decoration
    int cap;

    bool plus, minus, through;

    //! Reference to the master curve to which this error bars curve is attached.
    DataCurve *d_master_curve;
};

#endif
