/***************************************************************************
    File                 : IntDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Vasileios Gkanis, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Integration options dialog

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
#ifndef INTDIALOG_H
#define INTDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>

class QPushButton;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QSpinBox;
class Graph;

//! Integration options dialog
class MAKHBER_EXPORT IntDialog : public QDialog
{
    Q_OBJECT

public:
    IntDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    ~IntDialog() {};

    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QPushButton *buttonHelp;
    QCheckBox *boxShowFormula {};
    QComboBox *boxName;
    QComboBox *boxMethod;
    QLineEdit *boxStart;
    QLineEdit *boxEnd;

public Q_SLOTS:
    void accept();
    void setGraph(Graph *g);
    void activateCurve(const QString &curveName);
    void help();
    void changeDataRange();

Q_SIGNALS:
    void integrate(int, int, int, double, double, double);

private:
    Graph *graph {};
};

#endif
