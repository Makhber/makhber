/***************************************************************************
    File                 : HistogramCurve.h
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

#include "plot2D/BarCurve.h"

//! Histogram class
class MAKHBER_EXPORT HistogramCurve : public BarCurve
{
public:
    HistogramCurve(Table *t, const QString &name, int startRow, int endRow);

    void copy(const HistogramCurve *h);

    QRectF boundingRect() const override;

    void setBinning(bool autoBin, double size, double begin, double end);
    bool autoBinning() const { return d_autoBin; };
    double begin() const { return d_begin; };
    double end() const { return d_end; };
    double binSize() const { return d_bin_size; };

    virtual bool loadData() override;
    void initData(const QVector<double> &Y, int size);

    double mean() const { return d_mean; };
    double standardDeviation() const { return d_standard_deviation; };
    double minimum() const { return d_min; };
    double maximum() const { return d_max; };

private:
    void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from,
              int to) const override;

    bool d_autoBin;
    double d_bin_size {}, d_begin {}, d_end {};

    //! Variables storing statistical information
    double d_mean {}, d_standard_deviation {}, d_min {}, d_max {};
};
