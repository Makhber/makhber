/***************************************************************************
    File                 : CurveRangeDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Filter options dialog

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
#ifndef CURVERANGEDIALOG_H
#define CURVERANGEDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>

class QPushButton;
class QLabel;
class QSpinBox;
class Graph;
class DataCurve;

//! Curve range dialog
class MAKHBER_EXPORT CurveRangeDialog : public QDialog
{
    Q_OBJECT

public:
    CurveRangeDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    ~CurveRangeDialog() {};

    QPushButton *buttonOK;
    QPushButton *buttonCancel;
    QLabel *boxName;
    QSpinBox *boxStart;
    QSpinBox *boxEnd;

public Q_SLOTS:
    void setCurveToModify(Graph *g, int curve);
    void accept();

private:
    DataCurve *d_curve {};
    Graph *d_graph {};
};

#endif
