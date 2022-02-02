/***************************************************************************
    File                 : CurvesDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Add/remove curves dialog

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
#include "CurvesDialog.h"

#include "plot2D/Graph.h"
#include "plot2D/FunctionCurve.h"
#include "plot2D/PlotCurve.h"
#include "table/Table.h"
#include "matrix/Matrix.h"
#include "core/ApplicationWindow.h"
#include "core/Folder.h"

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QPixmap>
#include <QShortcut>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>

CurvesDialog::CurvesDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setWindowTitle(tr("Add/Remove curves"));
    setSizeGripEnabled(true);
    setFocus();

    auto *hl = new QHBoxLayout();

    hl->addWidget(new QLabel(tr("New curves style")));
    boxStyle = new QComboBox();
    boxStyle->addItem(QPixmap(":/lPlot.xpm"), tr(" Line"));
    boxStyle->addItem(QPixmap(":/pPlot.xpm"), tr(" Scatter"));
    boxStyle->addItem(QPixmap(":/lpPlot.xpm"), tr(" Line + Symbol"));
    boxStyle->addItem(QPixmap(":/dropLines.xpm"), tr(" Vertical drop lines"));
    boxStyle->addItem(QPixmap(":/spline.xpm"), tr(" Spline"));
    boxStyle->addItem(QPixmap(":/vert_steps.xpm"), tr(" Vertical steps"));
    boxStyle->addItem(QPixmap(":/hor_steps.xpm"), tr(" Horizontal steps"));
    boxStyle->addItem(QPixmap(":/area.xpm"), tr(" Area"));
    boxStyle->addItem(QPixmap(":/vertBars.xpm"), tr(" Vertical Bars"));
    boxStyle->addItem(QPixmap(":/hBars.xpm"), tr(" Horizontal Bars"));
    hl->addWidget(boxStyle);

    boxMatrixStyle = new QComboBox();
    boxMatrixStyle->addItem(QPixmap(":/color_map.xpm"), tr("Contour - Color Fill"));
    boxMatrixStyle->addItem(QPixmap(":/contour_map.xpm"), tr("Contour Lines"));
    boxMatrixStyle->addItem(QPixmap(":/gray_map.xpm"), tr("Gray Scale Map"));
    boxMatrixStyle->hide();
    hl->addWidget(boxMatrixStyle);
    hl->addStretch();

    auto *gl = new QGridLayout();
    gl->addWidget(new QLabel(tr("Available data")), 0, 0);
    gl->addWidget(new QLabel(tr("Graph contents")), 0, 2);

    available = new QListWidget();
    available->setSelectionMode(QAbstractItemView::ExtendedSelection);
    gl->addWidget(available, 1, 0);

    auto *vl1 = new QVBoxLayout();
    btnAdd = new QPushButton();
    btnAdd->setIcon(QIcon(QPixmap(":/next.xpm")));
    btnAdd->setFixedWidth(35);
    btnAdd->setFixedHeight(30);
    vl1->addWidget(btnAdd);

    btnRemove = new QPushButton();
    btnRemove->setIcon(QIcon(QPixmap(":/prev.xpm")));
    btnRemove->setFixedWidth(35);
    btnRemove->setFixedHeight(30);
    vl1->addWidget(btnRemove);
    vl1->addStretch();

    gl->addLayout(vl1, 1, 1);
    contents = new QListWidget();
    contents->setSelectionMode(QAbstractItemView::ExtendedSelection);
    gl->addWidget(contents, 1, 2);

    auto *vl2 = new QVBoxLayout();
    btnAssociations = new QPushButton(tr("&Plot Associations..."));
    btnAssociations->setEnabled(false);
    vl2->addWidget(btnAssociations);

    btnRange = new QPushButton(tr("Edit &Range..."));
    btnRange->setEnabled(false);
    vl2->addWidget(btnRange);

    btnEditFunction = new QPushButton(tr("&Edit Function..."));
    btnEditFunction->setEnabled(false);
    vl2->addWidget(btnEditFunction);

    btnOK = new QPushButton(tr("OK"));
    btnOK->setDefault(true);
    vl2->addWidget(btnOK);

    btnCancel = new QPushButton(tr("Close"));
    vl2->addWidget(btnCancel);

    boxShowRange = new QCheckBox(tr("&Show Range"));
    vl2->addWidget(boxShowRange);

    vl2->addStretch();
    gl->addLayout(vl2, 1, 3);

    auto *vl3 = new QVBoxLayout(this);
    vl3->addLayout(hl);
    vl3->addLayout(gl);

    boxShowCurrentFolder = new QCheckBox(tr("Show current &folder only"));
    vl3->addWidget(boxShowCurrentFolder);

    init();

    connect(boxShowCurrentFolder, SIGNAL(toggled(bool)), this, SLOT(showCurrentFolder(bool)));
    connect(boxShowRange, SIGNAL(toggled(bool)), this, SLOT(showCurveRange(bool)));
    connect(btnRange, SIGNAL(clicked()), this, SLOT(showCurveRangeDialog()));
    connect(btnAssociations, SIGNAL(clicked()), this, SLOT(showPlotAssociations()));
    connect(btnEditFunction, SIGNAL(clicked()), this, SLOT(showFunctionDialog()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addCurves()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeCurves()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(contents, SIGNAL(currentRowChanged(int)), this, SLOT(showCurveBtn(int)));
    connect(contents, SIGNAL(itemSelectionChanged()), this, SLOT(enableRemoveBtn()));
    connect(available, SIGNAL(itemSelectionChanged()), this, SLOT(enableAddBtn()));

    auto *shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(removeCurves()));
    shortcut = new QShortcut(QKeySequence("-"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(removeCurves()));
    shortcut = new QShortcut(QKeySequence("+"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(addCurves()));
}

void CurvesDialog::showCurveBtn(int)
{
    QwtPlotItem *it = d_graph->plotItem(contents->currentRow());
    if (!it)
        return;

    bool function = false;
    bool associations = false;
    bool range = false;

    if (it->rtti() != QwtPlotItem::Rtti_PlotSpectrogram) {
        auto *c = dynamic_cast<PlotCurve *>(it);
        if (c->type() == Graph::Function)
            function = true;
        else {
            associations = true;
            if (c->type() != Graph::ErrorBars)
                range = true;
        }
    }

    btnEditFunction->setEnabled(function);
    btnAssociations->setEnabled(associations);
    btnRange->setEnabled(range);
}

void CurvesDialog::showCurveRangeDialog()
{
    int curve = contents->currentRow();
    if (curve < 0)
        curve = 0;

    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (app) {
        app->showCurveRangeDialog(d_graph, curve);
        updateCurveRange();
    }
}

void CurvesDialog::showPlotAssociations()
{
    int curve = contents->currentRow();
    if (curve < 0)
        curve = 0;

    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    close();
    if (app)
        app->showPlotAssociations(curve);
}

void CurvesDialog::showFunctionDialog()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    int currentRow = contents->currentRow();
    close();

    if (app)
        app->showFunctionDialog(d_graph, currentRow);
}

QSize CurvesDialog::sizeHint() const
{
    return QSize(700, 400);
}

void CurvesDialog::contextMenuEvent(QContextMenuEvent *e)
{
    QPoint pos = available->viewport()->mapFromGlobal(QCursor::pos());
    QRect rect = available->visualItemRect(available->currentItem());
    if (rect.contains(pos)) {
        QMenu contextMenu(this);
        QList<QListWidgetItem *> lst = available->selectedItems();
        if (lst.size() > 1)
            contextMenu.addAction(tr("&Plot Selection"), this, SLOT(addCurves()));
        else if (lst.size() == 1)
            contextMenu.addAction(tr("&Plot"), this, SLOT(addCurves()));
        contextMenu.exec(QCursor::pos());
    }

    pos = contents->viewport()->mapFromGlobal(QCursor::pos());
    rect = contents->visualItemRect(contents->currentItem());
    if (rect.contains(pos)) {
        QMenu contextMenu(this);
        QList<QListWidgetItem *> lst = contents->selectedItems();
        if (lst.size() > 1)
            contextMenu.addAction(tr("&Delete Selection"), this, SLOT(removeCurves()));
        else if (lst.size() == 1)
            contextMenu.addAction(tr("&Delete Curve"), this, SLOT(removeCurves()));
        contextMenu.exec(QCursor::pos());
    }

    e->accept();
}

void CurvesDialog::init()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (app) {
        bool currentFolderOnly = app->d_show_current_folder;
        boxShowCurrentFolder->setChecked(currentFolderOnly);
        showCurrentFolder(currentFolderOnly);

        QStringList matrices = app->matrixNames();
        if (!matrices.isEmpty()) {
            boxMatrixStyle->show();
            available->addItems(matrices);
        }

        int style = app->defaultCurveStyle;
        if (style == Graph::Line)
            boxStyle->setCurrentIndex(0);
        else if (style == Graph::Scatter)
            boxStyle->setCurrentIndex(1);
        else if (style == Graph::LineSymbols)
            boxStyle->setCurrentIndex(2);
        else if (style == Graph::VerticalDropLines)
            boxStyle->setCurrentIndex(3);
        else if (style == Graph::Spline)
            boxStyle->setCurrentIndex(4);
        else if (style == Graph::VerticalSteps)
            boxStyle->setCurrentIndex(5);
        else if (style == Graph::HorizontalSteps)
            boxStyle->setCurrentIndex(6);
        else if (style == Graph::Area)
            boxStyle->setCurrentIndex(7);
        else if (style == Graph::VerticalBars)
            boxStyle->setCurrentIndex(8);
        else if (style == Graph::HorizontalBars)
            boxStyle->setCurrentIndex(9);
    }

    if (!available->count())
        btnAdd->setDisabled(true);
}

void CurvesDialog::setGraph(Graph *graph)
{
    d_graph = graph;
    contents->addItems(d_graph->plotItemsList());
    enableRemoveBtn();
    enableAddBtn();
}

void CurvesDialog::addCurves()
{
    QStringList emptyColumns;
    QList<QListWidgetItem *> lst = available->selectedItems();
    for (auto i : lst) {
        QString text = i->text();
        if (contents->findItems(text, Qt::MatchExactly).isEmpty()) {
            if (!addCurve(text))
                emptyColumns << text;
        }
    }
    d_graph->updatePlot();
    Graph::showPlotErrorMessage(this, emptyColumns);

    showCurveRange(boxShowRange->isChecked());
}

bool CurvesDialog::addCurve(const QString &name)
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (!app)
        return false;

    QStringList matrices = app->matrixNames();
    if (matrices.contains(name)) {
        Matrix *m = app->matrix(name);
        if (!m)
            return false;

        switch (boxMatrixStyle->currentIndex()) {
        case 0:
            d_graph->plotSpectrogram(m, Graph::ColorMap);
            break;
        case 1:
            d_graph->plotSpectrogram(m, Graph::ContourMap);
            break;
        case 2:
            d_graph->plotSpectrogram(m, Graph::GrayMap);
            break;
        }

        contents->addItem(name);
        return true;
    }

    int style = curveStyle();
    Table *t = app->table(name);
    if (t && d_graph->insertCurve(t, name, style)) {
        CurveLayout cl = Graph::initCurveLayout();
        int color = 0, symbol = 0;
        d_graph->guessUniqueCurveLayout(color, symbol);

        cl.lCol = color;
        cl.symCol = color;
        cl.fillCol = color;
        cl.lWidth = app->defaultCurveLineWidth;
        cl.sSize = app->defaultSymbolSize;
        cl.sType = symbol;

        if (style == Graph::Line)
            cl.sType = 0;
        else if (style == Graph::VerticalBars || style == Graph::HorizontalBars) {
            cl.filledArea = 1;
            cl.lCol = 0;
            cl.aCol = color;
            cl.sType = 0;
        } else if (style == Graph::Area) {
            cl.filledArea = 1;
            cl.aCol = color;
            cl.sType = 0;
        } else if (style == Graph::VerticalDropLines)
            cl.connectType = QwtPlotCurve::Sticks;
        else if (style == Graph::VerticalSteps || style == Graph::HorizontalSteps) {
            cl.connectType = QwtPlotCurve::Steps;
            cl.sType = 0;
        } else if (style == Graph::Spline)
            cl.connectType = 4;

        d_graph->updateCurveLayout(d_graph->curves() - 1, &cl);

        contents->addItem(name);
        return true;
    }
    return false;
}

void CurvesDialog::removeCurves()
{
    QList<QListWidgetItem *> lst = contents->selectedItems();
    for (auto it : lst) {
        QString s = it->text();
        if (boxShowRange->isChecked()) {
            QStringList lst = s.split("[");
            s = lst[0];
        }
        d_graph->removeCurve(s);
    }

    showCurveRange(boxShowRange->isChecked());
    d_graph->updatePlot();
}

void CurvesDialog::enableAddBtn()
{
    btnAdd->setEnabled(available->count() > 0 && !available->selectedItems().isEmpty());
}

void CurvesDialog::enableRemoveBtn()
{
    btnRemove->setEnabled(contents->count() > 0 && !contents->selectedItems().isEmpty());
}

int CurvesDialog::curveStyle()
{
    int style = 0;
    switch (boxStyle->currentIndex()) {
    case 0:
        style = Graph::Line;
        break;
    case 1:
        style = Graph::Scatter;
        break;
    case 2:
        style = Graph::LineSymbols;
        break;
    case 3:
        style = Graph::VerticalDropLines;
        break;
    case 4:
        style = Graph::Spline;
        break;
    case 5:
        style = Graph::VerticalSteps;
        break;
    case 6:
        style = Graph::HorizontalSteps;
        break;
    case 7:
        style = Graph::Area;
        break;
    case 8:
        style = Graph::VerticalBars;
        break;
    case 9:
        style = Graph::HorizontalBars;
        break;
    }
    return style;
}

void CurvesDialog::showCurveRange(bool on)
{
    int row = contents->currentRow();
    contents->clear();
    if (on) {
        QStringList lst = QStringList();
        for (int i = 0; i < d_graph->curves(); i++) {
            QwtPlotItem *it = d_graph->plotItem(i);
            if (!it)
                continue;

            if (it->rtti() == QwtPlotItem::Rtti_PlotCurve
                && (dynamic_cast<PlotCurve *>(it))->type() != Graph::Function) {
                auto *c = dynamic_cast<DataCurve *>(it);
                lst << c->title().text() + "[" + QString::number(c->startRow() + 1) + ":"
                                + QString::number(c->endRow() + 1) + "]";
            } else
                lst << it->title().text();
        }
        contents->addItems(lst);
    } else
        contents->addItems(d_graph->plotItemsList());

    contents->setCurrentRow(row);
    enableRemoveBtn();
}

void CurvesDialog::updateCurveRange()
{
    showCurveRange(boxShowRange->isChecked());
}

void CurvesDialog::showCurrentFolder(bool currentFolder)
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (!app)
        return;

    app->d_show_current_folder = currentFolder;
    available->clear();

    if (currentFolder) {
        Folder *f = app->currentFolder();
        if (f) {
            QStringList columns;
            for (QWidget *w : f->windowsList()) {
                if (!w->inherits("Table"))
                    continue;

                auto *t = dynamic_cast<Table *>(w);
                for (int i = 0; i < t->numCols(); i++) {
                    if (t->colPlotDesignation(i) == Makhber::Y)
                        columns << QString(t->name()) + "_" + t->colLabel(i);
                }
            }
            available->addItems(columns);
        }
    } else
        available->addItems(app->columnsList(Makhber::Y));
}

void CurvesDialog::closeEvent(QCloseEvent *e)
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (app)
        app->d_add_curves_dialog_size = this->size();

    e->accept();
}
