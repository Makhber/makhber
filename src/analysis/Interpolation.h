/***************************************************************************
    File                 : Interpolation.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical interpolation of data sets

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
#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "analysis/Filter.h"

class QwtPlotCurve;
class MAKHBER_EXPORT Interpolation : public Filter
{
    Q_OBJECT

public:
    enum InterpolationMethod { Linear, Cubic, Akima };

    Interpolation(ApplicationWindow *parent, Graph *g, const QString &curveTitle, int m = 0);
    Interpolation(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
                  double end, int m = 0);

    int method() { return d_method; };
    void setMethod(int m);
    void setMethod(InterpolationMethod m) { setMethod((int)m); };

protected:
    virtual bool isDataAcceptable();

private:
    void init(int m);
    void calculateOutputData(double *x, double *y);

    //! the interpolation method
    int d_method {};
};

#endif
