/***************************************************************************
    File                 : AssociationsDialog.cpp
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
#include "AssociationsDialog.h"

#include "table/Table.h"
#include "plot2D/FunctionCurve.h"
#include "plot2D/PlotCurve.h"
#include "plot2D/ErrorPlotCurve.h"
#include "plot2D/VectorCurve.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QEvent>
#include <QLayout>
#include <QApplication>
#include <QMessageBox>

AssociationsDialog::AssociationsDialog(QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl), graph(nullptr)
{
    setWindowTitle(tr("Plot Associations"));
    setSizeGripEnabled(true);
    setFocus();

    auto *vl = new QVBoxLayout();

    auto *hbox1 = new QHBoxLayout();
    hbox1->addWidget(new QLabel(tr("Spreadsheet: ")));

    tableCaptionLabel = new QLabel();
    hbox1->addWidget(tableCaptionLabel);
    vl->addLayout(hbox1);

    table = new QTableWidget(3, 5);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionsClickable(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    table->setMaximumHeight(8 * table->rowHeight(0));
    table->setHorizontalHeaderLabels(QStringList() << tr("Column") << tr("X") << tr("Y")
                                                   << tr("xErr") << tr("yErr"));
    vl->addWidget(table);

    associations = new QListWidget();
    associations->setSelectionMode(QListWidget::SingleSelection);
    vl->addWidget(associations);

    btnApply = new QPushButton(tr("&Update curves"));
    btnOK = new QPushButton(tr("&OK"));
    btnOK->setDefault(true);
    btnCancel = new QPushButton(tr("&Cancel"));

    auto *hbox2 = new QHBoxLayout();
    hbox2->addStretch();
    hbox2->addWidget(btnApply);
    hbox2->addWidget(btnOK);
    hbox2->addWidget(btnCancel);
    vl->addStretch();
    vl->addLayout(hbox2);
    setLayout(vl);

    active_table = nullptr;

    connect(associations, SIGNAL(currentRowChanged(int)), this, SLOT(updateTable(int)));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(updateCurves()));
}

void AssociationsDialog::accept()
{
    updateCurves();
    close();
}

void AssociationsDialog::updateCurves()
{
    if (!graph)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (int i = 0; i < associations->count(); i++)
        changePlotAssociation(i, plotAssociation(associations->item(i)->text()));
    graph->updatePlot();

    QApplication::restoreOverrideCursor();
}

void AssociationsDialog::changePlotAssociation(int curve, const QString &text)
{
    auto *c = dynamic_cast<DataCurve *>(graph->curve(dataCurvesList[curve])); // c_keys[curve]);
    if (!c || c->type() == Graph::Function)
        return;

    if (c->plotAssociation() == text)
        return;

    QStringList lst = text.split(",", Qt::SkipEmptyParts);
    if (lst.count() == 2) {
        c->setXColumnName(lst[0].remove("(X)"));
        c->setTitle(lst[1].remove("(Y)"));
        c->loadData();
    } else if (lst.count() == 3) { // curve with error bars
        auto *er = dynamic_cast<ErrorPlotCurve *>(c);
        QString xColName = lst[0].remove("(X)");
        QString yColName = lst[1].remove("(Y)");
        QString erColName = lst[2].remove("(xErr)").remove("(yErr)");
        DataCurve *master_curve = graph->masterCurve(xColName, yColName);
        if (!master_curve)
            return;

        Qt::Orientation type = Qt::Vertical;
        if (text.contains("(xErr)"))
            type = Qt::Horizontal;
        er->setDirection(type);
        er->setTitle(erColName);
        if (master_curve != er->masterCurve())
            er->setMasterCurve(master_curve);
        else
            er->loadData();
    } else if (lst.count() == 4) {
        auto *v = dynamic_cast<VectorCurve *>(c);
        v->setXColumnName(lst[0].remove("(X)"));
        v->setTitle(lst[1].remove("(Y)"));

        QString xEndCol = lst[2].remove("(X)").remove("(A)");
        QString yEndCol = lst[3].remove("(Y)").remove("(M)");
        if (v->vectorEndXAColName() != xEndCol || v->vectorEndYMColName() != yEndCol)
            v->setVectorEnd(xEndCol, yEndCol);
        else
            v->loadData();
    }
    graph->notifyChanges();
}

QString AssociationsDialog::plotAssociation(const QString &text)
{
    QString s = text;
    QStringList lst = s.split(": ", Qt::SkipEmptyParts);
    QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

    QString tableName = lst[0];
    s = tableName + "_" + cols[0];
    for (int i = 1; i < (int)cols.count(); i++)
        s += "," + tableName + "_" + cols[i];
    return s;
}

void AssociationsDialog::initTablesList(QList<MyWidget *> *lst, int curve)
{
    tables = lst;
    active_table = nullptr;

    if (curve < 0 || curve >= (int)associations->count())
        curve = 0;

    associations->setCurrentRow(curve);
}

Table *AssociationsDialog::findTable(int index)
{
    QString text = associations->item(index)->text();
    QStringList lst = text.split(":", Qt::SkipEmptyParts);
    for (int i = 0; i < (int)tables->count(); i++) {
        if (tables->at(i)->objectName() == lst[0])
            return dynamic_cast<Table *>(tables->at(i));
    }
    return nullptr;
}

void AssociationsDialog::updateTable(int index)
{
    Table *t = findTable(index);
    if (!t)
        return;

    if (active_table != t) {
        active_table = t;
        tableCaptionLabel->setText(t->name());
        table->clearContents();
        table->setRowCount(t->numCols());

        QStringList colNames = t->colNames();
        for (int i = 0; i < table->rowCount(); i++) {
            auto *cell = new QTableWidgetItem(colNames[i]);
            cell->setBackground(QBrush(Qt::lightGray));
            cell->setFlags(Qt::ItemIsEnabled);
            table->setItem(i, 0, cell);
        }

        for (int j = 1; j < table->columnCount(); j++) {
            for (int i = 0; i < table->rowCount(); i++) {
                auto *cell = new QTableWidgetItem();
                cell->setBackground(QBrush(Qt::lightGray));
                table->setItem(i, j, cell);

                auto *cb = new QCheckBox(table);
                cb->installEventFilter(this);
                table->setCellWidget(i, j, cb);
            }
        }
    }
    updateColumnTypes();
}

void AssociationsDialog::updateColumnTypes()
{
    QString text = associations->currentItem()->text();
    QStringList lst = text.split(": ", Qt::SkipEmptyParts);
    QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

    QString xColName = cols[0].remove("(X)");
    QString yColName {};

    int n = (int)cols.count();
    if (n == 1)
        table->hideColumn(2);
    else
        yColName = cols[1].remove("(Y)");

    if (n < 3) {
        table->hideColumn(3);
        table->hideColumn(4);
    }

    QCheckBox *it = nullptr;
    for (int i = 0; i < table->rowCount(); i++) {
        it = dynamic_cast<QCheckBox *>(table->cellWidget(i, 1));
        if (table->item(i, 0)->text() == xColName)
            it->setChecked(true);
        else
            it->setChecked(false);

        it = dynamic_cast<QCheckBox *>(table->cellWidget(i, 2));
        if (table->item(i, 0)->text() == yColName)
            it->setChecked(true);
        else
            it->setChecked(false);
    }

    bool xerr = false, yerr = false, vectors = false;
    QString errColName, xEndColName, yEndColName;
    if (n > 2) {
        table->showColumn(3);
        table->showColumn(4);

        if (cols[2].contains("(xErr)") || cols[2].contains("(yErr)")) { // if error bars
            table->horizontalHeaderItem(3)->setText(tr("xErr"));
            table->horizontalHeaderItem(4)->setText(tr("yErr"));
        }

        if (cols[2].contains("(xErr)")) {
            xerr = true;
            errColName = cols[2].remove("(xErr)");
        } else if (cols[2].contains("(yErr)")) {
            yerr = true;
            errColName = cols[2].remove("(yErr)");
        } else if (cols.count() > 3 && cols[2].contains("(X)") && cols[3].contains("(Y)")) {
            vectors = true;
            xEndColName = cols[2].remove("(X)");
            yEndColName = cols[3].remove("(Y)");
            table->horizontalHeaderItem(3)->setText(tr("xEnd"));
            table->horizontalHeaderItem(4)->setText(tr("yEnd"));
        } else if (cols.count() > 3 && cols[2].contains("(A)") && cols[3].contains("(M)")) {
            vectors = true;
            xEndColName = cols[2].remove("(A)");
            yEndColName = cols[3].remove("(M)");
            table->horizontalHeaderItem(3)->setText(tr("Angle"));
            table->horizontalHeaderItem(4)->setText(tr("Magn.", "Magnitude, vector length"));
        }
    }

    for (int i = 0; i < table->rowCount(); i++) {
        it = dynamic_cast<QCheckBox *>(table->cellWidget(i, 3));
        if (xerr || vectors) {
            if (table->item(i, 0)->text() == errColName || table->item(i, 0)->text() == xEndColName)
                it->setChecked(true);
            else
                it->setChecked(false);
        } else
            it->setChecked(false);

        it = dynamic_cast<QCheckBox *>(table->cellWidget(i, 4));
        if (yerr || vectors) {
            if (table->item(i, 0)->text() == errColName || table->item(i, 0)->text() == yEndColName)
                it->setChecked(true);
            else
                it->setChecked(false);
        } else
            it->setChecked(false);
    }
}

void AssociationsDialog::uncheckCol(int col)
{
    for (int i = 0; i < table->rowCount(); i++) {
        auto *it = dynamic_cast<QCheckBox *>(table->cellWidget(i, col));
        if (it)
            it->setChecked(false);
    }
}

void AssociationsDialog::setGraph(Graph *g)
{
    graph = g;

    for (int i = 0; i < graph->curves(); i++) {
        const QwtPlotItem *it = dynamic_cast<QwtPlotItem *>(graph->plotItem(i));
        if (!it)
            continue;
        if (it->rtti() != QwtPlotItem::Rtti_PlotCurve)
            continue;

        if (dynamic_cast<const DataCurve *>(it)->type() != Graph::Function) {
            QString s = dynamic_cast<const DataCurve *>(it)->plotAssociation();
            QString table_name = dynamic_cast<const DataCurve *>(it)->table()->name();
            plotAssociationsList << table_name + ": " + s.remove(table_name + "_");
            dataCurvesList << i;
        }
    }
    associations->addItems(plotAssociationsList);
    associations->setMaximumHeight((plotAssociationsList.count() + 1)
                                   * associations->visualItemRect(associations->item(0)).height());
}

void AssociationsDialog::updatePlotAssociation(int row, int col)
{
    int index = associations->currentRow();
    QString text = associations->currentItem()->text();
    QStringList lst = text.split(": ", Qt::SkipEmptyParts);
    QStringList cols = lst[1].split(",", Qt::SkipEmptyParts);

    if (col == 1) {
        cols[0] = table->item(row, 0)->text() + "(X)";
        text = lst[0] + ": " + cols.join(",");
    } else if (col == 2) {
        cols[1] = table->item(row, 0)->text() + "(Y)";
        text = lst[0] + ": " + cols.join(",");
    } else if (col == 3) {
        if (text.contains("(A)")) { // vect XYAM curve
            cols[2] = table->item(row, 0)->text() + "(A)";
            text = lst[0] + ": " + cols.join(",");
        } else if (!text.contains("(A)") && text.count("(X)") == 1) {
            cols[2] = table->item(row, 0)->text() + "(xErr)";
            text = lst[0] + ": " + cols.join(",");
            uncheckCol(4);
        } else if (text.count("(X)") == 2) { // vect XYXY curve
            cols[2] = table->item(row, 0)->text() + "(X)";
            text = lst[0] + ": " + cols.join(",");
        }
    } else if (col == 4) {
        if (text.contains("(M)")) { // vect XYAM curve
            cols[3] = table->item(row, 0)->text() + "(M)";
            text = lst[0] + ": " + cols.join(",");
        } else if (!text.contains("(M)") && text.count("(X)") == 1) {
            cols[2] = table->item(row, 0)->text() + "(yErr)";
            text = lst[0] + ": " + cols.join(",");
            uncheckCol(3);
        } else if (text.count("(Y)") == 2) { // vect XYXY curve
            cols[3] = table->item(row, 0)->text() + "(Y)";
            text = lst[0] + ": " + cols.join(",");
        }
    }

    // change associations for error bars depending on the curve "index"
    QString old_as = plotAssociationsList[index];
    for (int i = 0; i < (int)plotAssociationsList.count(); i++) {
        QString as = plotAssociationsList[i];
        if (as.contains(old_as) && (as.contains("(xErr)") || as.contains("(yErr)"))) {
            QStringList ls = as.split(",", Qt::SkipEmptyParts);
            as = text + "," + ls[2];
            plotAssociationsList[i] = as;
        }
    }

    plotAssociationsList[index] = text;
    associations->item(index)->setText(text);
}

bool AssociationsDialog::eventFilter(QObject *object, QEvent *e)
{
    auto *it = dynamic_cast<QTableWidgetItem *>(object);
    if (!it)
        return false;

    if (e->type() == QEvent::MouseButtonPress) {
        if ((dynamic_cast<QCheckBox *>(it))->isChecked())
            return true;

        int col = 0, row = 0;
        for (int j = 1; j < table->columnCount(); j++) {
            for (int i = 0; i < table->rowCount(); i++) {
                auto *cb = dynamic_cast<QCheckBox *>(table->cellWidget(i, j));
                if (cb == dynamic_cast<QCheckBox *>(object)) {
                    row = i;
                    col = j;
                    break;
                }
            }
        }

        uncheckCol(col);
        (dynamic_cast<QCheckBox *>(it))->setChecked(true);

        updatePlotAssociation(row, col);
        return true;
    } else if (e->type() == QEvent::MouseButtonDblClick)
        return true;
    else
        return false;
}

AssociationsDialog::~AssociationsDialog()
{
    delete tables;
}
