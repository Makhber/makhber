/***************************************************************************
    File                 : SmoothCurveDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Smoothing options dialog

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
#include "SmoothCurveDialog.h"

#include "plot2D/Graph.h"
#include "scripting/MyParser.h"
#include "core/ColorButton.h"
#include "analysis/SmoothFilter.h"

#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>

SmoothCurveDialog::SmoothCurveDialog(int method, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    smooth_method = method;

    setWindowTitle(tr("Smoothing Options"));

    auto *gb1 = new QGroupBox();
    auto *gl1 = new QGridLayout(gb1);
    gl1->addWidget(new QLabel(tr("Curve")), 0, 0);

    boxName = new QComboBox();
    gl1->addWidget(boxName, 0, 1);

    btnColor = new ColorButton();
    btnColor->setColor(QColor(Qt::red));

    if (method == SmoothFilter::SavitzkyGolay) {
        gl1->addWidget(new QLabel(tr("Polynomial Order")), 1, 0);
        boxOrder = new QSpinBox();
        boxOrder->setRange(0, 9);
        boxOrder->setValue(2);
        gl1->addWidget(boxOrder, 1, 1);

        gl1->addWidget(new QLabel(tr("Points to the Left")), 2, 0);
        boxPointsLeft = new QSpinBox();
        boxPointsLeft->setRange(1, 25);
        boxPointsLeft->setValue(2);
        gl1->addWidget(boxPointsLeft, 2, 1);

        gl1->addWidget(new QLabel(tr("Points to the Right")), 3, 0);
        boxPointsRight = new QSpinBox();
        boxPointsRight->setRange(1, 25);
        boxPointsRight->setValue(2);
        gl1->addWidget(boxPointsRight, 3, 1);

        gl1->addWidget(new QLabel(tr("Color")), 4, 0);
        gl1->addWidget(btnColor, 4, 1);
        gl1->setRowStretch(5, 1);
    } else {
        gl1->addWidget(new QLabel(tr("Points")), 1, 0);
        boxPointsLeft = new QSpinBox();
        boxPointsLeft->setRange(1, 1000000);
        boxPointsLeft->setSingleStep(10);
        boxPointsLeft->setValue(5);
        gl1->addWidget(boxPointsLeft, 1, 1);

        gl1->addWidget(new QLabel(tr("Color")), 2, 0);
        gl1->addWidget(btnColor, 2, 1);
        gl1->setRowStretch(3, 1);
    }
    gl1->setColumnStretch(2, 1);

    btnSmooth = new QPushButton(tr("&Smooth"));
    btnSmooth->setDefault(true);
    buttonCancel = new QPushButton(tr("&Close"));

    auto *vl = new QVBoxLayout();
    vl->addWidget(btnSmooth);
    vl->addWidget(buttonCancel);
    vl->addStretch();

    auto *hb = new QHBoxLayout(this);
    hb->addWidget(gb1);
    hb->addLayout(vl);

    connect(btnSmooth, SIGNAL(clicked()), this, SLOT(smooth()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(boxName, SIGNAL(activated(const QString &)), this,
            SLOT(activateCurve(const QString &)));
}

void SmoothCurveDialog::smooth()
{
    auto *sf = new SmoothFilter(dynamic_cast<ApplicationWindow *>(this->parent()), graph,
                                boxName->currentText(), smooth_method);
    if (smooth_method == SmoothFilter::SavitzkyGolay) {
        sf->setSmoothPoints(boxPointsLeft->value(), boxPointsRight->value());
        sf->setPolynomOrder(boxOrder->value());
    } else
        sf->setSmoothPoints(boxPointsLeft->value());

    sf->setColor(btnColor->color());
    sf->run();
    delete sf;
}

void SmoothCurveDialog::setGraph(Graph *g)
{
    graph = g;
    boxName->addItems(g->analysableCurvesList());
    activateCurve(boxName->currentText());
}

void SmoothCurveDialog::activateCurve(const QString &curveName)
{
    if (smooth_method == SmoothFilter::Average) {
        QwtPlotCurve *c = graph->curve(curveName);
        if (!c || c->rtti() != QwtPlotItem::Rtti_PlotCurve)
            return;

        boxPointsLeft->setMaximum(static_cast<int>(c->dataSize()) / 2);
    }
}
