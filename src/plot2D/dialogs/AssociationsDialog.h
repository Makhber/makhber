/***************************************************************************
    File                 : AssociationsDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Plot associations dialog

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
#ifndef ASSOCIATIONSDIALOG_H
#define ASSOCIATIONSDIALOG_H

#include "core/MyWidget.h"

#include <QDialog>

class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;
class QStringList;
class Table;
class Graph;

//! Plot associations dialog
class MAKHBER_EXPORT AssociationsDialog : public QDialog
{
    Q_OBJECT

public:
    AssociationsDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    ~AssociationsDialog();

    void initTablesList(QList<MyWidget *> *lst, int curve);
    void setGraph(Graph *g);

private Q_SLOTS:
    void updateTable(int index);
    void updateCurves();
    void accept();

private:
    void changePlotAssociation(int curve, const QString &text);
    void updateColumnTypes();
    void uncheckCol(int col);
    void updatePlotAssociation(int row, int col);
    QString plotAssociation(const QString &text);
    Table *findTable(int index);
    bool eventFilter(QObject *object, QEvent *e);

    QList<MyWidget *> *tables {};
    Table *active_table;
    Graph *graph;
    QStringList plotAssociationsList;
    QList<int> dataCurvesList;

    QLabel *tableCaptionLabel;
    QTableWidget *table;
    QPushButton *btnOK, *btnCancel, *btnApply;
    QListWidget *associations;
};

#endif // ASSOCIATIONSDIALOG_H
