/***************************************************************************
    File                 : FunctionCurve.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Function curve class

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
#include "FunctionCurve.h"

#include <QMessageBox>

#include <cmath>

// FIXME: While FunctionCurve itself supports arbitrary scripting
// interpreters now, fit curves assume muParser in many formulas (using
// "^" as power operator). Thus, simply having FunctionCurve honour the
// application-wide scripting environment would break fit curve plotting.
FunctionCurve::FunctionCurve(ApplicationWindow *parent, QString name)
    : PlotCurve(name), scripted(ScriptingLangManager::newEnv("muParser", parent))
{
    d_variable = "x";
    setType(Graph::Function);
}

FunctionCurve::FunctionCurve(ApplicationWindow *parent, const FunctionType &t, QString name)
    : PlotCurve(name),
      scripted(ScriptingLangManager::newEnv("muParser", parent)),
      d_function_type(t)
{
    d_variable = "x";
    setType(Graph::Function);
}

void FunctionCurve::setRange(double from, double to)
{
    d_from = from;
    d_to = to;
}

void FunctionCurve::copy(FunctionCurve *f)
{
    d_function_type = f->functionType();
    d_variable = f->variable();
    d_formulas = f->formulas();
    d_from = f->startRange();
    d_to = f->endRange();
}

QString FunctionCurve::legend() const
{
    QString label = title().text() + ": ";
    if (d_function_type == Normal)
        label += d_formulas[0];
    else if (d_function_type == Parametric) {
        label += "X(" + d_variable + ")=" + d_formulas[0];
        label += ", Y(" + d_variable + ")=" + d_formulas[1];
    } else if (d_function_type == Polar) {
        label += "R(" + d_variable + ")=" + d_formulas[0];
        label += ", Theta(" + d_variable + ")=" + d_formulas[1];
    }
    return label;
}

bool FunctionCurve::loadData(int points)
{
    if (!points)
        points = static_cast<int>(dataSize());

    auto *X = new double[points];
    auto *Y = new double[points];
    double step = (d_to - d_from) / (double)(points - 1);

    switch (d_function_type) {
    case Normal: {
        Script *script = scriptEnv->newScript(d_formulas[0], nullptr, title().text());
        QObject::connect(script, SIGNAL(error(const QString &, const QString &, int)), this,
                         SLOT(scriptError(const QString &, const QString &, int)));
        int i = 0;
        double x = NAN;
        for (i = 0, x = d_from; i < points; i++, x += step) {
            X[i] = x;
            script->setDouble(x, d_variable.toUtf8().constData());
            QVariant result = script->eval();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (result.metaType().id() != QMetaType::Double) {
#else
            if (result.type() != QVariant::Double) {
#endif
                delete[] X;
                delete[] Y;
                return false;
            }
            Y[i] = result.toDouble();
        }
        break;
    }
    case Parametric:
    case Polar: {
        Script *script_x = scriptEnv->newScript(d_formulas[0], nullptr, title().text());
        Script *script_y = scriptEnv->newScript(d_formulas[1], nullptr, title().text());
        int i = 0;
        double par = NAN;
        for (i = 0, par = d_from; i < points; i++, par += step) {
            script_x->setDouble(par, d_variable.toUtf8().constData());
            script_y->setDouble(par, d_variable.toUtf8().constData());
            QVariant result_x = script_x->eval();
            QVariant result_y = script_y->eval();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (result_x.metaType().id() != QMetaType::Double
                || result_y.metaType().id() != QMetaType::Double) {
#else
            if (result_x.type() != QVariant::Double || result_y.type() != QVariant::Double) {
#endif
                delete[] X;
                delete[] Y;
                return false;
            }
            if (d_function_type == Polar) {
                X[i] = result_x.toDouble() * cos(result_y.toDouble());
                Y[i] = result_x.toDouble() * sin(result_y.toDouble());
            } else {
                X[i] = result_x.toDouble();
                Y[i] = result_y.toDouble();
            }
        }
        break;
    }
    }
    setSamples(X, Y, points);
    delete[] X;
    delete[] Y;
    return true;
}

void FunctionCurve::scriptError(const QString &message, const QString &scriptName, int lineNumber)
{
    QMessageBox::critical(nullptr, tr("Input function error"),
                          QString("%1:%2\n\n%3").arg(scriptName).arg(lineNumber).arg(message));
}
