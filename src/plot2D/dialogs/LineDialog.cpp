/***************************************************************************
    File                 : LineDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Line options dialog

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
#include "LineDialog.h"

#include "plot2D/ArrowMarker.h"
#include "plot2D/Graph.h"
#include "plot2D/Plot.h"
#include "core/ColorButton.h"
#include "core/PenWidget.h"
#include "core/ApplicationWindow.h"

#include <qwt_plot.h>

#include <QMessageBox>
#include <QLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTabWidget>

LineDialog::LineDialog(ArrowMarker *line, QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setWindowTitle(tr("Line options"));

    lm = line;

    auto *gb1 = new QGroupBox();
    auto *gl1 = new QGridLayout();

    penWidget = new PenWidget(this, lm->linePen());
    penWidget->setPen(lm->linePen());
    gl1->addWidget(penWidget, 0, 0, 1, 2);

    startBox = new QCheckBox();
    startBox->setText(tr("Arrow at &start"));
    startBox->setChecked(lm->hasStartArrow());
    gl1->addWidget(startBox, 1, 0, Qt::AlignTop);

    endBox = new QCheckBox();
    endBox->setText(tr("Arrow at &end"));
    endBox->setChecked(lm->hasEndArrow());
    gl1->addWidget(endBox, 1, 1, Qt::AlignTop);

    gb1->setLayout(gl1);

    auto *hl1 = new QHBoxLayout();
    hl1->addWidget(gb1);

    options = new QWidget();
    options->setLayout(hl1);

    tw = new QTabWidget();
    tw->addTab(options, tr("Opti&ons"));

    auto *gb2 = new QGroupBox();
    auto *gl2 = new QGridLayout();

    gl2->addWidget(new QLabel(tr("Length")), 0, 0);
    boxHeadLength = new QSpinBox();
    boxHeadLength->setValue(lm->headLength());
    gl2->addWidget(boxHeadLength, 0, 1);

    gl2->addWidget(new QLabel(tr("Angle")), 1, 0);
    boxHeadAngle = new QSpinBox();
    boxHeadAngle->setRange(0, 85);
    boxHeadAngle->setSingleStep(5);
    boxHeadAngle->setValue(lm->headAngle());
    gl2->addWidget(boxHeadAngle, 1, 1);

    filledBox = new QCheckBox();
    filledBox->setText(tr("&Filled"));
    filledBox->setChecked(lm->filledArrowHead());
    gl2->addWidget(filledBox, 2, 1);

    gb2->setLayout(gl2);

    auto *hl2 = new QHBoxLayout();
    hl2->addWidget(gb2);

    head = new QWidget();
    head->setLayout(hl2);
    tw->addTab(head, tr("Arrow &Head"));

    initGeometryTab();

    buttonDefault = new QPushButton(tr("Set &Default"));
    btnApply = new QPushButton(tr("&Apply"));
    btnOk = new QPushButton(tr("&Ok"));
    btnOk->setDefault(true);

    auto *bl1 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl1->addStretch();
    bl1->addWidget(buttonDefault);
    bl1->addWidget(btnApply);
    bl1->addWidget(btnOk);

    auto *vl = new QVBoxLayout();
    vl->addWidget(tw);
    vl->addLayout(bl1);
    setLayout(vl);

    enableHeadTab();

    connect(btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(tw, SIGNAL(currentChanged(int)), this, SLOT(enableButtonDefault(int)));
    connect(buttonDefault, SIGNAL(clicked()), this, SLOT(setDefaultValues()));
}

void LineDialog::initGeometryTab()
{
    if (unitBox != nullptr)
        delete unitBox;
    unitBox = new QComboBox();
    unitBox->addItem(tr("Scale Coordinates"));
    unitBox->addItem(tr("Pixels"));

    auto *bl1 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl1->addWidget(new QLabel(tr("Unit")));
    bl1->addWidget(unitBox);

    auto *gb1 = new QGroupBox(tr("Start Point"));
    xStartBox = new QLineEdit();
    yStartBox = new QLineEdit();

    auto *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("X")), 0, 0);
    gl1->addWidget(xStartBox, 0, 1);
    gl1->addWidget(new QLabel(tr("Y")), 1, 0);
    gl1->addWidget(yStartBox, 1, 1);
    gb1->setLayout(gl1);

    auto *gb2 = new QGroupBox(tr("End Point"));
    xEndBox = new QLineEdit();
    yEndBox = new QLineEdit();

    auto *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("X")), 0, 0);
    gl2->addWidget(xEndBox, 0, 1);
    gl2->addWidget(new QLabel(tr("Y")), 1, 0);
    gl2->addWidget(yEndBox, 1, 1);
    gb2->setLayout(gl2);

    auto *bl2 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl2->addWidget(gb1);
    bl2->addWidget(gb2);

    auto *vl = new QVBoxLayout();
    vl->addLayout(bl1);
    vl->addLayout(bl2);

    geometry = new QWidget();
    geometry->setLayout(vl);
    tw->addTab(geometry, tr("&Geometry"));

    connect(unitBox, SIGNAL(activated(int)), this, SLOT(displayCoordinates(int)));
    displayCoordinates(0);
}

void LineDialog::displayCoordinates(int unit)
{
    if (unit == ScaleCoordinates) {
        QPointF sp = lm->startPointCoord();
        xStartBox->setText(QString::number(sp.x()));
        yStartBox->setText(QString::number(sp.y()));

        QPointF ep = lm->endPointCoord();
        xEndBox->setText(QString::number(ep.x()));
        yEndBox->setText(QString::number(ep.y()));
    } else {
        QPoint startPoint = lm->startPoint();
        QPoint endPoint = lm->endPoint();

        xStartBox->setText(QString::number(startPoint.x()));
        yStartBox->setText(QString::number(startPoint.y()));

        xEndBox->setText(QString::number(endPoint.x()));
        yEndBox->setText(QString::number(endPoint.y()));
    }
}

void LineDialog::setCoordinates(int unit)
{
    if (unit == ScaleCoordinates) {
        lm->setStartPoint(xStartBox->text().replace(",", ".").toDouble(),
                          yStartBox->text().replace(",", ".").toDouble());
        lm->setEndPoint(xEndBox->text().replace(",", ".").toDouble(),
                        yEndBox->text().replace(",", ".").toDouble());
    } else {
        lm->setStartPoint(QPoint(xStartBox->text().toInt(), yStartBox->text().toInt()));
        lm->setEndPoint(QPoint(xEndBox->text().toInt(), yEndBox->text().toInt()));
    }
}

void LineDialog::apply()
{
    if (tw->currentWidget() == dynamic_cast<QWidget *>(options)) {
        lm->drawEndArrow(endBox->isChecked());
        lm->drawStartArrow(startBox->isChecked());
        lm->setLinePen(penWidget->pen());
    } else if (tw->currentWidget() == dynamic_cast<QWidget *>(head)) {
        if (lm->headLength() != boxHeadLength->value())
            lm->setHeadLength(boxHeadLength->value());

        if (lm->headAngle() != boxHeadAngle->value())
            lm->setHeadAngle(boxHeadAngle->value());

        if (lm->filledArrowHead() != filledBox->isChecked())
            lm->fillArrowHead(filledBox->isChecked());
    } else if (tw->currentWidget() == dynamic_cast<QWidget *>(geometry))
        setCoordinates(unitBox->currentIndex());

    QwtPlot *plot = lm->plot();
    auto *g = dynamic_cast<Graph *>(plot->parent());
    plot->replot();
    g->notifyChanges();

    enableHeadTab();
}

void LineDialog::accept()
{
    apply();
    close();
}

void LineDialog::enableHeadTab()
{
    if (startBox->isChecked() || endBox->isChecked())
        tw->setTabEnabled(tw->indexOf(head), true);
    else
        tw->setTabEnabled(tw->indexOf(head), false);
}

void LineDialog::setDefaultValues()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (!app)
        return;

    app->setArrowDefaultSettings(penWidget->pen(), boxHeadLength->value(), boxHeadAngle->value(),
                                 filledBox->isChecked());
}

void LineDialog::enableButtonDefault(int page)
{
    QWidget *pageWidget = tw->widget(page);
    if (pageWidget == geometry)
        buttonDefault->setEnabled(false);
    else
        buttonDefault->setEnabled(true);
}
