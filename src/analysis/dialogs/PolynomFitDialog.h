/***************************************************************************
    File                 : PolynomFitDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fit polynomial dialog

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
#ifndef POLINOMFITDIALOG_H
#define POLINOMFITDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class Graph;
class ColorButton;

//! Fit polynomial dialog
class MAKHBER_EXPORT PolynomFitDialog : public QDialog
{
    Q_OBJECT

public:
    PolynomFitDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);

public Q_SLOTS:
    void fit();
    void setGraph(Graph *g);
    void activateCurve(const QString &curveName);
    void changeDataRange();

private:
    Graph *graph {};

    QPushButton *buttonFit;
    QPushButton *buttonCancel;
    QCheckBox *boxShowFormula;
    QComboBox *boxName;
    QSpinBox *boxOrder;
    QLineEdit *boxStart;
    QLineEdit *boxEnd;
    ColorButton *btnColor;
};

#endif
