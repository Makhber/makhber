/***************************************************************************
    File                 : PolynomialFit.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Polynomial Fit and Linear Fit classes

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
#ifndef POLYNOMIALFIT_H
#define POLYNOMIALFIT_H

#include "analysis/Fit.h"

class MAKHBER_EXPORT PolynomialFit : public Fit
{
    Q_OBJECT

public:
    PolynomialFit(ApplicationWindow *parent, Graph *g, int order = 2, bool legend = false);
    PolynomialFit(ApplicationWindow *parent, Graph *g, QString &curveTitle, int order = 2,
                  bool legend = false);
    PolynomialFit(ApplicationWindow *parent, Graph *g, QString &curveTitle, double start,
                  double end, int order = 2, bool legend = false);

    QString legendInfo() override;
    void fit() override;

    static QString generateFormula(int order);
    static QStringList generateParameterList(int order);

private:
    void init();
    void calculateFitCurveData(const std::vector<double> &, double *, double *) override;

    int d_order;
    bool show_legend;
};

class MAKHBER_EXPORT LinearFit : public Fit
{
    Q_OBJECT

public:
    LinearFit(ApplicationWindow *parent, Graph *g);
    LinearFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle);
    LinearFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
              double end);

    void fit() override;

private:
    void init();
    void calculateFitCurveData(const std::vector<double> &, double *, double *) override;
};
#endif
