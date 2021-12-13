/***************************************************************************
    File                 : SigmoidalFit.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Sigmoidal (Boltzmann) Fit class

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
#ifndef SIGMOIDALFIT_H
#define SIGMOIDALFIT_H

#include "analysis/Fit.h"

class MAKHBER_EXPORT SigmoidalFit : public Fit
{
    Q_OBJECT

public:
    SigmoidalFit(ApplicationWindow *parent, Graph *g);
    SigmoidalFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle);
    SigmoidalFit(ApplicationWindow *parent, Graph *g, const QString &curveTitle, double start,
                 double end);
    void guessInitialValues() override;

private:
    void init();
    void calculateFitCurveData(const std::vector<double> &, double *, double *) override;
};

#endif
