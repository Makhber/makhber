/***************************************************************************
    File                 : DataSetDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Multi purpose dialog for choosing a data set

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
#ifndef DATASETDIALOG_H
#define DATASETDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>

class QGroupBox;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QComboBox;

class Graph;

//! Multi purpose dialog for choosing a data set
class MAKHBER_EXPORT DataSetDialog : public QDialog
{
    Q_OBJECT

public:
    DataSetDialog(const QString &text, QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    ~DataSetDialog() {};

    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QGroupBox *groupBox1;
    QCheckBox *boxShowFormula {};
    QComboBox *boxName;

public Q_SLOTS:
    void accept();
    void setCurveNames(const QStringList &names);
    void setOperationType(const QString &s) { operation = s; };
    void setCurentDataSet(const QString &s);
    void setGraph(Graph *g);

Q_SIGNALS:
    void options(const QString &);

private:
    QString operation;
    QString windowTitle;
    Graph *d_graph;
};

#endif
