/***************************************************************************
    File                 : Integration.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical integration of data sets

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
#include "Integration.h"

#include "plot2D/MultiLayer.h"
#include "plot2D/Legend.h"

#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_vector.h>

#include <QMessageBox>
#include <QDateTime>
#include <QLocale>

#include <stdexcept>
#include <cmath>

Integration::Integration(ApplicationWindow *parent, Graph *g) : Filter(parent, g)
{
    init();
}

Integration::Integration(ApplicationWindow *parent, Graph *g, const QString &curveTitle)
    : Filter(parent, g)
{
    init();
    setDataFromCurve(curveTitle);
}

Integration::Integration(ApplicationWindow *parent, Graph *g, const QString &curveTitle,
                         double start, double end)
    : Filter(parent, g)
{
    init();
    setDataFromCurve(curveTitle, start, end);
}

void Integration::init()
{
    setObjectName(tr("Integration"));
    d_method = Linear;
    d_sort_data = true;
    d_result = NAN;
}

double Integration::trapezoid()
{
    double sum = 0.0;
    std::vector<double> result_v;
    result_v.reserve(d_n);
    int size = d_n - 1;
    for (int i = 0; i < size; i++) {
        int j = i + 1;
        result_v.push_back(sum);
        sum += 0.5 * (d_y[j] + d_y[i]) * (d_x[j] - d_x[i]);
    }

    result_v.push_back(sum);
    d_points = d_n;
    addResultCurve(d_x, &result_v[0]);
    return sum;
}

bool Integration::isDataAcceptable()
{
    switch (d_method) {
    case Linear:
        break;
    case Cubic:
        break;
    case Akima:
        break;
    default:
        QMessageBox::critical(dynamic_cast<ApplicationWindow *>(parent()),
                              tr("Makhber") + " - " + tr("Error"),
                              tr("Unknown interpolation method. Valid values are: 0 - Linear, 1 - "
                                 "Cubic, 2 - Akima."));
        d_init_err = true;
        return true;
    }
    // GSL interpolation routines fail with division by zero on such data
    for (unsigned i = 1; i < d_n; i++)
        if (d_x[i - 1] == d_x[i]) {
            QMessageBox::critical(dynamic_cast<ApplicationWindow *>(parent()),
                                  tr("Makhber") + " - " + tr("Error"),
                                  tr("Several data points have the same x value causing divisions "
                                     "by zero, operation aborted!"));
            return false;
        }

    return Filter::isDataAcceptable();
}

QString Integration::logInfo()
{
    const gsl_interp_type *method_t = nullptr;
    QString method_name;
    switch (d_method) {
    case Linear:
        method_t = gsl_interp_linear;
        method_name = tr("Linear");
        break;
    case Cubic:
        method_t = gsl_interp_cspline;
        method_name = tr("Cubic");
        break;
    case Akima:
        method_t = gsl_interp_akima;
        method_name = tr("Akima");
        break;
    default:
        throw std::runtime_error("invalid method");
    }

    if (d_n < gsl_interp_type_min_size(method_t)) {
        QMessageBox::critical(dynamic_cast<ApplicationWindow *>(parent()),
                              tr("Makhber") + " - " + tr("Error"),
                              tr("You need at least %1 points in order to perform this operation!")
                                      .arg(gsl_interp_type_min_size(method_t)));
        d_init_err = true;
        return QString("");
    }

    gsl_interp *interpolation = gsl_interp_alloc(method_t, d_n);
    gsl_interp_init(interpolation, d_x, d_y, d_n);

    QString logInfo = "["
            + QLocale::c().toString(QDateTime::currentDateTime(), "dd-MM-yyyy hh:mm:ss:zzz") + "\t"
            + tr("Plot") + ": ''" + d_graph->parentPlotName() + "'']\n";
    logInfo += "\n" + tr("Numerical integration of") + ": " + d_curve->title().text()
            + tr(" using ") + method_name + tr("Interpolation") + "\n";

    auto *app = dynamic_cast<ApplicationWindow *>(parent());
    int prec = app->d_decimal_digits;
    logInfo += tr("Points") + ": " + QString::number(d_n) + " " + tr("from")
            + " x = " + QLocale().toString(d_from, 'g', prec) + " ";
    logInfo += tr("to") + " x = " + QLocale().toString(d_to, 'g', prec) + "\n";

    // using GSL to find maximum value of data set
    gsl_vector *aux = gsl_vector_alloc(d_n);
    for (unsigned i = 0; i < d_n; i++)
        gsl_vector_set(aux, i, d_y[i]);
    int maxID = static_cast<int>(gsl_vector_max_index(aux));
    gsl_vector_free(aux);

    // calculate result
    d_result = gsl_interp_eval_integ(interpolation, d_x, d_y, d_from, d_to, nullptr);

    logInfo += tr("Peak at") + " x = " + QLocale().toString(d_x[maxID], 'g', prec) + "\t";
    logInfo += "y = " + QLocale().toString(d_y[maxID], 'g', prec) + "\n";

    logInfo += tr("Area") + "=";
    logInfo += QLocale().toString(d_result, 'g', prec);
    logInfo += "\n-------------------------------------------------------------\n";
    // use trapezoid rule to get the integral curve and plot it
    trapezoid();

    gsl_interp_free(interpolation);

    return logInfo;
}
