/***************************************************************************
    File                 : fitclasses.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : MultiPeakFit module with Lorentz and Gauss peak shapes

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
#include "MultiPeakFit.h"

#include "analysis/fit_gsl.h"
#include "plot2D/PlotCurve.h"
#include "plot2D/FunctionCurve.h"
#include "core/ColorButton.h"
#include "aspects/column/Column.h"

#include <QLocale>
#include <QMessageBox>

#include <cmath>

MultiPeakFit::MultiPeakFit(ApplicationWindow *parent, Graph *g, PeakProfile profile, int peaks)
    : Fit(parent, g), d_profile(profile)
{
    setObjectName(tr("MultiPeak"));

    if (profile == Gauss) {
        d_f = gauss_multi_peak_f;
        d_df = gauss_multi_peak_df;
        d_fdf = gauss_multi_peak_fdf;
        d_fsimplex = gauss_multi_peak_d;
    } else {
        d_f = lorentz_multi_peak_f;
        d_df = lorentz_multi_peak_df;
        d_fdf = lorentz_multi_peak_fdf;
        d_fsimplex = lorentz_multi_peak_d;
    }

    d_param_init = nullptr;
    covar = nullptr;

    setNumPeaks(peaks);

    generate_peak_curves = true;
    d_peaks_color = 2; // green
}

void MultiPeakFit::setNumPeaks(int n)
{
    d_peaks = n;
    if (d_profile == Gauss)
        d_explanation = tr("Gauss Fit");
    else
        d_explanation = tr("Lorentz Fit");
    if (d_peaks > 1)
        d_explanation += "(" + QString::number(d_peaks) + ") " + tr("multi-peak");

    d_p = 3 * d_peaks + 1;
    d_min_points = d_p;

    if (d_param_init)
        gsl_vector_free(d_param_init);
    d_param_init = gsl_vector_alloc(d_p);
    gsl_vector_set_all(d_param_init, 1.0);

    if (covar)
        gsl_matrix_free(covar);
    covar = gsl_matrix_alloc(d_p, d_p);
    d_results.resize(d_p);

    d_param_names = generateParameterList(d_peaks);
    d_param_explain = generateExplanationList(d_peaks);
    d_formula = generateFormula(d_peaks, d_profile);
}

QStringList MultiPeakFit::generateParameterList(int peaks)
{
    if (peaks == 1)
        return QStringList() << "A"
                             << "xc"
                             << "w"
                             << "y0";

    QStringList lst;
    for (int i = 0; i < peaks; i++) {
        QString index = QString::number(i + 1);
        lst << "A" + index;
        lst << "xc" + index;
        lst << "w" + index;
    }
    lst << "y0";
    return lst;
}

QStringList MultiPeakFit::generateExplanationList(int peaks)
{
    if (peaks == 1)
        return QStringList() << tr("(amplitude)") << tr("(center)") << tr("(width)")
                             << tr("(offset)");

    QStringList lst;
    for (int i = 0; i < peaks; i++) {
        QString index = QString::number(i + 1);
        lst << tr("(amplitude %1)").arg(index);
        lst << tr("(center %1)").arg(index);
        lst << tr("(width %1)").arg(index);
    }
    lst << tr("(offset)");
    return lst;
}

QString MultiPeakFit::generateFormula(int peaks, PeakProfile profile)
{
    if (peaks == 1)
        switch (profile) {
        case Gauss:
            return "y0+A*sqrt(2/PI)/w*exp(-2*((x-xc)/w)^2)";
            break;

        case Lorentz:
            return "y0+2*A/PI*w/(4*(x-xc)^2+w^2)";
            break;
        }

    QString formula = "y0+";
    for (int i = 0; i < peaks; i++) {
        formula += peakFormula(i + 1, profile);
        if (i < peaks - 1)
            formula += "+";
    }
    return formula;
}

QString MultiPeakFit::peakFormula(int peakIndex, PeakProfile profile)
{
    QString formula;
    QString index = QString::number(peakIndex);
    switch (profile) {
    case Gauss:
        formula += "sqrt(2/PI)*A" + index + "/w" + index;
        formula += "*exp(-2*(x-xc" + index + ")^2/w" + index + "^2)";
        break;
    case Lorentz:
        formula += "2*A" + index + "/PI*w" + index + "/(4*(x-xc" + index + ")^2+w" + index + "^2)";
        break;
    }
    return formula;
}

void MultiPeakFit::guessInitialValues()
{
    if (d_peaks > 1)
        return;

    gsl_vector_view x = gsl_vector_view_array(d_x, d_n);
    gsl_vector_view y = gsl_vector_view_array(d_y, d_n);

    double min_out = NAN, max_out = NAN;
    gsl_vector_minmax(&y.vector, &min_out, &max_out);

    if (d_profile == Gauss)
        gsl_vector_set(d_param_init, 0, sqrt(M_2_PI) * (max_out - min_out));
    else if (d_profile == Lorentz)
        gsl_vector_set(d_param_init, 0, 1.0);

    gsl_vector_set(d_param_init, 1, gsl_vector_get(&x.vector, gsl_vector_max_index(&y.vector)));
    gsl_vector_set(d_param_init, 2, 1.0);
    gsl_vector_set(d_param_init, 3, min_out);
}

void MultiPeakFit::storeCustomFitResults(const std::vector<double> &par)
{
    d_results = par;

    if (d_profile == Lorentz) {
        for (int j = 0; j < d_peaks && unsigned(j) < d_results.size(); j++)
            d_results[3 * j] = M_PI_2 * d_results[3 * j];
    }
}

void MultiPeakFit::insertPeakFunctionCurve(double *x, double *y, int peak)
{
    QStringList curves = d_graph->curvesList();
    int index = 0;
    for (int i = 0; i < (int)curves.count(); i++) {
        if (curves[i].startsWith(tr("Peak")))
            index++;
    }
    QString title = tr("Peak") + QString::number(++index);

    auto *c = new FunctionCurve(dynamic_cast<ApplicationWindow *>(parent()), FunctionCurve::Normal,
                                title);
    c->setPen(QPen(d_peaks_color, 1));
    c->setSamples(x, y, d_points);
    c->setRange(d_x[0], d_x[d_n - 1]);

    QString formula;
    for (int j = 0; j < 3; j++) {
        int p = 3 * peak + j;
        formula += QString("%1=%2\n").arg(d_param_names[p]).arg(d_results[p], 0, 'g', d_prec);
    }
    formula += QString("%1=%2\n\ny0+%3")
                       .arg(d_param_names[d_p - 1])
                       .arg(d_results[d_p - 1], 1, 'g', d_prec)
                       .arg(peakFormula(peak + 1, d_profile));
    c->setFormula(formula);
    d_graph->insertPlotItem(c, Graph::Line);
    d_graph->addFitCurve(c);
}

void MultiPeakFit::generateFitCurve(const std::vector<double> &par)
{
    auto *app = dynamic_cast<ApplicationWindow *>(parent());
    if (!d_gen_function)
        d_points = d_n;

    gsl_matrix *m = gsl_matrix_alloc(d_points, d_peaks);
    if (!m) {
        QMessageBox::warning(app, tr("Fit Error"),
                             tr("Could not allocate enough memory for the fit curves!"));
        return;
    }

    auto *X = new double[d_points];
    auto *Y = new double[d_points];
    int i = 0, j = 0;
    int peaks_aux = d_peaks;
    if (d_peaks == 1)
        peaks_aux--;

    if (d_gen_function) {
        double step = (d_x[d_n - 1] - d_x[0]) / (d_points - 1);
        for (i = 0; i < d_points; i++) {
            X[i] = d_x[0] + i * step;
            double yi = 0;
            for (j = 0; j < d_peaks; j++) {
                double diff = X[i] - par[3 * j + 1];
                double w = par[3 * j + 2];
                double y_aux = 0;
                if (d_profile == Gauss)
                    y_aux += sqrt(M_2_PI) * par[3 * j] / w * exp(-2 * diff * diff / (w * w));
                else
                    y_aux += par[3 * j] * w / (4 * diff * diff + w * w);

                yi += y_aux;
                y_aux += par[d_p - 1];
                gsl_matrix_set(m, i, j, y_aux);
            }
            Y[i] = yi + par[d_p - 1]; // add offset
        }

        if (d_peaks > 1)
            insertFitFunctionCurve(objectName() + tr("Fit"), X, Y, 2);
        else
            insertFitFunctionCurve(objectName() + tr("Fit"), X, Y);

        if (generate_peak_curves) {
            for (i = 0; i < peaks_aux; i++) { // add the peak curves
                for (j = 0; j < d_points; j++)
                    Y[j] = gsl_matrix_get(m, j, i);

                insertPeakFunctionCurve(X, Y, i);
            }
        }
    } else {
        QString tableName = app->generateUniqueName(tr("Fit"));
        QString label = d_explanation + " " + tr("fit of") + " " + d_curve->title().text();

        QList<Column *> columns;
        columns << new Column(tr("1", "multipeak fit table first column name"),
                              Makhber::ColumnMode::Numeric);
        for (i = 0; i < peaks_aux; i++)
            columns << new Column(tr("peak%1").arg(QString::number(i + 1)),
                                  Makhber::ColumnMode::Numeric);
        columns << new Column(tr("2", "multipeak fit table last column name"),
                              Makhber::ColumnMode::Numeric);
        Table *t = app->newHiddenTable(tableName, label, columns);

        for (i = 0; i < d_points; i++) {
            X[i] = d_x[i];
            columns.at(0)->setValueAt(i, X[i]);

            double yi = 0;
            for (j = 0; j < d_peaks; j++) {
                double diff = X[i] - par[3 * j + 1];
                double w = par[3 * j + 2];
                double y_aux = 0;
                if (d_profile == Gauss)
                    y_aux += sqrt(M_2_PI) * par[3 * j] / w * exp(-2 * diff * diff / (w * w));
                else
                    y_aux += par[3 * j] * w / (4 * diff * diff + w * w);

                yi += y_aux;
                y_aux += par[d_p - 1];
                columns.at(j + 1)->setValueAt(i, y_aux);
                gsl_matrix_set(m, i, j, y_aux);
            }
            Y[i] = yi + par[d_p - 1]; // add offset
            if (d_peaks > 1)
                columns.at(d_peaks + 1)->setValueAt(i, Y[i]);
        }

        label = tableName + "_2";
        auto *c = new DataCurve(t, tableName + "_" + columns.at(0)->name(), label);
        if (d_peaks > 1)
            c->setPen(QPen(d_curveColor, 2));
        else
            c->setPen(QPen(d_curveColor, 1));
        c->setSamples(X, Y, d_points);
        d_graph->insertPlotItem(c, Graph::Line);
        d_graph->addFitCurve(c);

        if (generate_peak_curves) {
            for (i = 0; i < peaks_aux; i++) { // add the peak curves
                for (j = 0; j < d_points; j++)
                    Y[j] = gsl_matrix_get(m, j, i);

                label = tableName + "_" + tr("peak") + QString::number(i + 1);
                c = new DataCurve(t, tableName + "_" + columns.at(0)->name(), label);
                c->setPen(QPen(d_peaks_color, 1));
                c->setSamples(X, Y, d_points);
                d_graph->insertPlotItem(c, Graph::Line);
                d_graph->addFitCurve(c);
            }
        }
    }
    d_graph->replot();

    delete[] X;
    delete[] Y;
    gsl_matrix_free(m);
}

QString MultiPeakFit::logFitInfo(const std::vector<double> &par, int iterations, int status,
                                 const QString &plotName)
{
    QString info = Fit::logFitInfo(par, iterations, status, plotName);
    if (d_peaks == 1)
        return info;

    info += tr("Peak") + "\t" + tr("Area") + "\t";
    info += tr("Center") + "\t" + tr("Width") + "\t" + tr("Height") + "\n";
    info += "--------------------------------------------------------------------------------------"
            "-\n";
    for (int j = 0; j < d_peaks; j++) {
        info += QString::number(j + 1) + "\t";
        info += QLocale().toString(par[3 * j], 'g', d_prec) + "\t";
        info += QLocale().toString(par[3 * j + 1], 'g', d_prec) + "\t";
        info += QLocale().toString(par[3 * j + 2], 'g', d_prec) + "\t";

        if (d_profile == Lorentz)
            info += QLocale().toString(M_2_PI * par[3 * j] / par[3 * j + 2], 'g', d_prec) + "\n";
        else
            info += QLocale().toString(sqrt(M_2_PI) * par[3 * j] / par[3 * j + 2], 'g', d_prec)
                    + "\n";
    }
    info += "--------------------------------------------------------------------------------------"
            "-\n";
    return info;
}

/*****************************************************************************
 *
 * Class LorentzFit
 *
 *****************************************************************************/

LorentzFit::LorentzFit(ApplicationWindow *parent, Graph *g)
    : MultiPeakFit(parent, g, MultiPeakFit::Lorentz, 1)
{
    init();
}

LorentzFit::LorentzFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle)
    : MultiPeakFit(parent, g, MultiPeakFit::Lorentz, 1)
{
    init();
    setDataFromCurve(curveTitle);
}

LorentzFit::LorentzFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
                       double end)
    : MultiPeakFit(parent, g, MultiPeakFit::Lorentz, 1)
{
    init();
    setDataFromCurve(curveTitle, start, end);
}

void LorentzFit::init()
{
    setObjectName("Lorentz");
    d_explanation = tr("Lorentz");
    d_param_explain << tr("(area)") << tr("(center)") << tr("(width)") << tr("(offset)");
}

/*****************************************************************************
 *
 * Class GaussFit
 *
 *****************************************************************************/

GaussFit::GaussFit(ApplicationWindow *parent, Graph *g)
    : MultiPeakFit(parent, g, MultiPeakFit::Gauss, 1)
{
    setObjectName("Gauss");
    d_explanation = tr("Gauss");
    d_param_explain << tr("(area)") << tr("(center)") << tr("(width)") << tr("(offset)");
}

GaussFit::GaussFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle)
    : MultiPeakFit(parent, g, MultiPeakFit::Gauss, 1)
{
    init();
    setDataFromCurve(curveTitle);
}

GaussFit::GaussFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
                   double end)
    : MultiPeakFit(parent, g, MultiPeakFit::Gauss, 1)
{
    init();
    setDataFromCurve(curveTitle, start, end);
}

void GaussFit::init()
{
    setObjectName("Gauss");
    d_explanation = tr("Gauss");
    d_param_explain << tr("(area)") << tr("(center)") << tr("(width)") << tr("(offset)");
}

/*****************************************************************************
 *
 * Class GaussAmpFit
 *
 *****************************************************************************/

GaussAmpFit::GaussAmpFit(ApplicationWindow *parent, Graph *g) : Fit(parent, g)
{
    init();
}

GaussAmpFit::GaussAmpFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle)
    : Fit(parent, g)
{
    init();
    setDataFromCurve(curveTitle);
}

GaussAmpFit::GaussAmpFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle,
                         double start, double end)
    : Fit(parent, g)
{
    init();
    setDataFromCurve(curveTitle, start, end);
}

void GaussAmpFit::init()
{
    setObjectName("GaussAmp");
    d_f = gauss_f;
    d_df = gauss_df;
    d_fdf = gauss_fdf;
    d_fsimplex = gauss_d;
    d_p = 4;
    d_min_points = d_p;
    d_param_init = gsl_vector_alloc(d_p);
    gsl_vector_set_all(d_param_init, 1.0);
    covar = gsl_matrix_alloc(d_p, d_p);
    d_results.resize(d_p);
    d_param_explain << tr("(offset)") << tr("(height)") << tr("(center)") << tr("(width)");
    d_param_names << "y0"
                  << "A"
                  << "xc"
                  << "w";
    d_explanation = tr("GaussAmp Fit");
    d_formula = "y0+A*exp(-(x-xc)^2/(2*w^2))";
}

void GaussAmpFit::calculateFitCurveData(const std::vector<double> &par, double *X, double *Y)
{
    double w2 = par[3] * par[3];
    if (d_gen_function) {
        double X0 = d_x[0];
        double step = (d_x[d_n - 1] - X0) / (d_points - 1);
        for (int i = 0; i < d_points; i++) {
            X[i] = X0 + i * step;
            double diff = X[i] - par[2];
            Y[i] = par[1] * exp(-0.5 * diff * diff / w2) + par[0];
        }
    } else {
        for (int i = 0; i < d_points; i++) {
            X[i] = d_x[i];
            double diff = X[i] - par[2];
            Y[i] = par[1] * exp(-0.5 * diff * diff / w2) + par[0];
        }
    }
}
