/***************************************************************************
    File                 : ErrDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Add error bars dialog

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
#include "ErrDialog.h"

#include "table/Table.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QList>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QList>
#include <QWidget>

ErrDialog::ErrDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizeGripEnabled(true);

    auto *vbox1 = new QVBoxLayout();
    vbox1->setSpacing(5);

    auto *hbox1 = new QHBoxLayout();
    vbox1->addLayout(hbox1);

    textLabel1 = new QLabel();
    hbox1->addWidget(textLabel1);

    nameLabel = new QComboBox();
    hbox1->addWidget(nameLabel);

    groupBox1 = new QGroupBox(QString(tr("Source of errors")));
    auto *gridLayout = new QGridLayout(groupBox1);
    vbox1->addWidget(groupBox1);

    buttonGroup1 = new QButtonGroup();
    buttonGroup1->setExclusive(true);

    columnBox = new QRadioButton();
    columnBox->setChecked(true);
    buttonGroup1->addButton(columnBox);
    gridLayout->addWidget(columnBox, 0, 0);

    colNamesBox = new QComboBox();
    tableNamesBox = new QComboBox();

    auto *comboBoxes = new QHBoxLayout();
    comboBoxes->addWidget(tableNamesBox);
    comboBoxes->addWidget(colNamesBox);

    gridLayout->addLayout(comboBoxes, 0, 1);

    percentBox = new QRadioButton();
    buttonGroup1->addButton(percentBox);
    gridLayout->addWidget(percentBox, 1, 0);

    valueBox = new QLineEdit();
    valueBox->setAlignment(Qt::AlignHCenter);
    valueBox->setEnabled(false);
    gridLayout->addWidget(valueBox, 1, 1);

    standardBox = new QRadioButton();
    buttonGroup1->addButton(standardBox);
    gridLayout->addWidget(standardBox, 2, 0);

    groupBox3 = new QGroupBox(QString());
    vbox1->addWidget(groupBox3);
    auto *hbox2 = new QHBoxLayout(groupBox3);

    buttonGroup2 = new QButtonGroup();
    buttonGroup2->setExclusive(true);

    xErrBox = new QRadioButton();
    buttonGroup2->addButton(xErrBox);
    hbox2->addWidget(xErrBox);

    yErrBox = new QRadioButton();
    buttonGroup2->addButton(yErrBox);
    hbox2->addWidget(yErrBox);
    yErrBox->setChecked(true);

    auto *vbox2 = new QVBoxLayout();
    buttonAdd = new QPushButton();
    buttonAdd->setDefault(true);
    vbox2->addWidget(buttonAdd);

    buttonCancel = new QPushButton();
    vbox2->addWidget(buttonCancel);

    vbox2->addStretch(1);

    auto *hlayout1 = new QHBoxLayout(this);
    hlayout1->addLayout(vbox1);
    hlayout1->addLayout(vbox2);

    languageChange();

    // signals and slots connections
    connect(buttonAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(percentBox, SIGNAL(toggled(bool)), valueBox, SLOT(setEnabled(bool)));
    connect(columnBox, SIGNAL(toggled(bool)), tableNamesBox, SLOT(setEnabled(bool)));
    connect(columnBox, SIGNAL(toggled(bool)), colNamesBox, SLOT(setEnabled(bool)));
    connect(tableNamesBox, SIGNAL(activated(int)), this, SLOT(selectSrcTable(int)));
}

void ErrDialog::setCurveNames(const QStringList &names)
{
    nameLabel->addItems(names);
}

void ErrDialog::setSrcTables(QList<MyWidget *> *tables)
{
    tableNamesBox->clear();

    srcTables = new QList<MyWidget *>;
    for (MyWidget *table : *tables) {
        Table *srcTable = dynamic_cast<Table *>(table);
        if (srcTable->d_future_table->columnCount(Makhber::xErr) > 0
            || srcTable->d_future_table->columnCount(Makhber::yErr) > 0) {
            srcTables->append(table);
            tableNamesBox->addItem(srcTable->objectName());
        }
    }

    if (srcTables->isEmpty()) {
        percentBox->setChecked(true);
        return;
    }

    if (!nameLabel->currentText().contains("="))
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        tableNamesBox->setCurrentIndex(
                std::max(0,
                         tableNamesBox->findText(
                                 nameLabel->currentText().split("_", Qt::SkipEmptyParts)[0])));
#else
        tableNamesBox->setCurrentIndex(
                std::max(0,
                         tableNamesBox->findText(
                                 nameLabel->currentText().split("_", QString::SkipEmptyParts)[0])));
#endif
    selectSrcTable(tableNamesBox->currentIndex());
}

void ErrDialog::selectSrcTable(int tabnr)
{
    colNamesBox->clear();
    if (tabnr > -1) {
        Table *srcTable = dynamic_cast<Table *>(srcTables->at(tabnr));
        for (int i = 0; i < srcTable->columnCount(); i++) {
            if (srcTable->column(i)->plotDesignation() == Makhber::xErr
                || srcTable->column(i)->plotDesignation() == Makhber::yErr)
                colNamesBox->addItem(srcTable->colName(i).remove(srcTable->name() + "_"));
        }
        if (srcTable->d_future_table->columnCount(Makhber::yErr) == 0)
            xErrBox->setChecked(true);
    }
}

void ErrDialog::add()
{
    Qt::Orientation direction {};
    if (xErrBox->isChecked())
        direction = Qt::Horizontal;
    else
        direction = Qt::Vertical;

    if (columnBox->isChecked())
        Q_EMIT options(nameLabel->currentText(),
                       tableNamesBox->currentText() + "_" + colNamesBox->currentText(), direction);
    else {
        int type = 0;
        if (percentBox->isChecked())
            type = 0;
        else
            type = 1;

        Q_EMIT options(nameLabel->currentText(), type, valueBox->text(), direction);
    }
}

ErrDialog::~ErrDialog() = default;

void ErrDialog::languageChange()
{
    setWindowTitle(tr("Error Bars"));
    xErrBox->setText(tr("&X Error Bars"));
    buttonAdd->setText(tr("&Add"));
    textLabel1->setText(tr("Add Error Bars to"));
    groupBox1->setTitle(tr("Source of errors"));
    percentBox->setText(tr("Percent of data (%)"));
    valueBox->setText(tr("5"));
    standardBox->setText(tr("Standard Deviation of Data"));
    yErrBox->setText(tr("&Y Error Bars"));
    buttonCancel->setText(tr("&Close"));
    columnBox->setText("Existing column");
}
