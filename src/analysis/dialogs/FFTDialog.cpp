/***************************************************************************
    File                 : FFTDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fast Fourier transform options dialog

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
#include "FFTDialog.h"

#include "plot2D/Graph.h"
#include "plot2D/MultiLayer.h"
#include "plot2D/Plot.h"
#include "scripting/MyParser.h"
#include "core/ApplicationWindow.h"
#include "table/Table.h"
#include "analysis/FFT.h"

#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>

#include <memory>
#include <cmath>

FFTDialog::FFTDialog(int type, QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setWindowTitle(tr("FFT Options"));

    d_table = nullptr;
    graph = nullptr;
    d_type = type;

    forwardBtn = new QRadioButton(tr("&Forward"));
    forwardBtn->setChecked(true);
    backwardBtn = new QRadioButton(tr("&Inverse"));

    auto *hbox1 = new QHBoxLayout();
    hbox1->addWidget(forwardBtn);
    hbox1->addWidget(backwardBtn);

    auto *gb1 = new QGroupBox();
    gb1->setLayout(hbox1);

    auto *gl1 = new QGridLayout();
    if (d_type == onGraph)
        gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
    else
        gl1->addWidget(new QLabel(tr("Sampling")), 0, 0);

    boxName = new QComboBox();
    gl1->addWidget(boxName, 0, 1);

    boxSampling = new QLineEdit();
    if (d_type == onTable) {
        gl1->addWidget(new QLabel(tr("Real")), 1, 0);
        boxReal = new QComboBox();
        gl1->addWidget(boxReal, 1, 1);

        gl1->addWidget(new QLabel(tr("Imaginary")), 2, 0);
        boxImaginary = new QComboBox();
        gl1->addWidget(boxImaginary, 2, 1);

        gl1->addWidget(new QLabel(tr("Sampling Interval")), 3, 0);
        gl1->addWidget(boxSampling, 3, 1);
    } else {
        gl1->addWidget(new QLabel(tr("Sampling Interval")), 1, 0);
        gl1->addWidget(boxSampling, 1, 1);
    }
    auto *gb2 = new QGroupBox();
    gb2->setLayout(gl1);

    boxNormalize = new QCheckBox(tr("&Normalize Amplitude"));
    boxNormalize->setChecked(true);

    boxOrder = new QCheckBox(tr("&Shift Results"));
    boxOrder->setChecked(true);

    auto *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb2);
    vbox1->addWidget(boxNormalize);
    vbox1->addWidget(boxOrder);
    vbox1->addStretch();

    buttonOK = new QPushButton(tr("&OK"));
    buttonOK->setDefault(true);
    buttonCancel = new QPushButton(tr("&Close"));

    auto *vbox2 = new QVBoxLayout();
    vbox2->addWidget(buttonOK);
    vbox2->addWidget(buttonCancel);
    vbox2->addStretch();

    auto *hbox2 = new QHBoxLayout(this);
    hbox2->addLayout(vbox1);
    hbox2->addLayout(vbox2);

    setFocusProxy(boxName);

    // signals and slots connections
    connect(boxName, SIGNAL(activated(const QString &)), this,
            SLOT(activateCurve(const QString &)));
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void FFTDialog::accept()
{
    double sampling = NAN;
    try {
        MyParser parser;
        parser.SetExpr(boxSampling->text());
        sampling = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(this, tr("Sampling value error"), QStringFromString(e.GetMsg()));
        boxSampling->setFocus();
        return;
    }

    auto *app = dynamic_cast<ApplicationWindow *>(parent());
    std::unique_ptr<FFT> fft;
    if (graph) {
        fft = std::make_unique<FFT>(app, graph, boxName->currentText());
    } else if (d_table) {
        if (boxReal->currentText().isEmpty()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Please choose a column for the real part of the data!"));
            boxReal->setFocus();
            return;
        }
        fft = std::make_unique<FFT>(app, d_table, boxReal->currentText(),
                                    boxImaginary->currentText());
    }
    if (fft) {
        fft->setInverseFFT(backwardBtn->isChecked());
        fft->setSampling(sampling);
        fft->normalizeAmplitudes(boxNormalize->isChecked());
        fft->shiftFrequencies(boxOrder->isChecked());
        fft->run();
    }
    close();
}

void FFTDialog::setGraph(Graph *g)
{
    graph = g;
    boxName->addItems(g->analysableCurvesList());
    activateCurve(boxName->currentText());
}

void FFTDialog::activateCurve(const QString &curveName)
{
    if (graph) {
        QwtPlotCurve *c = graph->curve(curveName);
        if (!c)
            return;

        boxSampling->setText(QString::number(c->sample(1).x() - c->sample(0).x()));
    } else if (d_table) {
        int col = d_table->colIndex(curveName);
        double x0 = d_table->text(0, col).toDouble();
        double x1 = d_table->text(1, col).toDouble();
        boxSampling->setText(QString::number(x1 - x0));
    }
}

void FFTDialog::setTable(Table *t)
{
    d_table = t;
    QStringList l = t->columnsList();
    boxName->addItems(l);
    boxReal->addItems(l);
    boxImaginary->addItems(l);

    int xcol = t->firstXCol();
    if (xcol >= 0) {
        boxName->setCurrentIndex(xcol);

        double x0 = t->text(0, xcol).toDouble();
        double x1 = t->text(1, xcol).toDouble();
        boxSampling->setText(QString::number(x1 - x0));
    }

    l = t->selectedColumns();
    int selected = (int)l.size();
    if (!selected) {
        boxReal->setItemText(boxReal->currentIndex(), QString());
        boxImaginary->setItemText(boxImaginary->currentIndex(), QString());
    } else if (selected == 1) {
        boxReal->setCurrentIndex(t->colIndex(l[0]));
        boxImaginary->setItemText(boxImaginary->currentIndex(), QString());
    } else {
        boxReal->setCurrentIndex(t->colIndex(l[0]));
        boxImaginary->setCurrentIndex(t->colIndex(l[1]));
    }
}
