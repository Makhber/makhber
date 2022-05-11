/***************************************************************************
    File                 : PluginFit.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Plugin Fit class

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
#ifndef PLUGINFIT_H
#define PLUGINFIT_H

#include "analysis/Fit.h"

class MAKHBER_EXPORT PluginFit : public Fit
{
    Q_OBJECT

public:
    PluginFit(ApplicationWindow *parent, Graph *g);
    PluginFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle);
    PluginFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
              double end);

    bool load(const QString &pluginName);

private:
    void init();
    typedef double (*fitFunctionEval)(double, double *);
    void calculateFitCurveData(const std::vector<double> &, double *, double *) override;
    fitFunctionEval f_eval {};
};
#endif
