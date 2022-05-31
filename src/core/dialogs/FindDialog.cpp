/***************************************************************************
    File                 : FindDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Find dialog

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
#include "FindDialog.h"

#include "core/ApplicationWindow.h"
#include "core/Folder.h"

#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include <QGroupBox>

FindDialog::FindDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setWindowTitle(tr("Makhber") + " - " + tr("Find"));
    setSizeGripEnabled(true);

    auto *topLayout = new QGridLayout();
    auto *bottomLayout = new QGridLayout();

    topLayout->addWidget(new QLabel(tr("Start From")), 0, 0);
    labelStart = new QLabel();
    labelStart->setFrameStyle(static_cast<int>(QFrame::Panel) | static_cast<int>(QFrame::Sunken));
    labelStart->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    topLayout->addWidget(labelStart, 0, 1, 1, 4);

    topLayout->addWidget(new QLabel(tr("Find")), 1, 0);
    boxFind = new QComboBox();
    boxFind->setEditable(true);
    boxFind->setDuplicatesEnabled(false);
    boxFind->setInsertPolicy(QComboBox::InsertAtTop);
    boxFind->setMaxCount(10);
    boxFind->setMaxVisibleItems(10);
    boxFind->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    topLayout->addWidget(boxFind, 1, 1, 1, 4);

    auto *groupBox = new QGroupBox(tr("Search in"));
    auto *groupBoxLayout = new QVBoxLayout(groupBox);

    boxWindowNames = new QCheckBox(tr("&Window Names"));
    boxWindowNames->setChecked(true);
    groupBoxLayout->addWidget(boxWindowNames);

    boxWindowLabels = new QCheckBox(tr("Window &Labels"));
    boxWindowLabels->setChecked(false);
    groupBoxLayout->addWidget(boxWindowLabels);

    boxFolderNames = new QCheckBox(tr("Folder &Names"));
    boxFolderNames->setChecked(false);
    groupBoxLayout->addWidget(boxFolderNames);

    bottomLayout->addWidget(groupBox, 0, 0, 3, 1);

    boxCaseSensitive = new QCheckBox(tr("Case &Sensitive"));
    boxCaseSensitive->setChecked(false);
    bottomLayout->addWidget(boxCaseSensitive, 0, 1);

    boxPartialMatch = new QCheckBox(tr("&Partial Match Allowed"));
    boxPartialMatch->setChecked(true);
    bottomLayout->addWidget(boxPartialMatch, 1, 1);

    boxSubfolders = new QCheckBox(tr("&Include Subfolders"));
    boxSubfolders->setChecked(true);
    bottomLayout->addWidget(boxSubfolders, 2, 1);

    buttonFind = new QPushButton(tr("&Find"));
    buttonFind->setDefault(true);
    bottomLayout->addWidget(buttonFind, 0, 2);

    buttonReset = new QPushButton(tr("&Update Start Path"));
    bottomLayout->addWidget(buttonReset, 1, 2);
    buttonCancel = new QPushButton(tr("&Close"));
    bottomLayout->addWidget(buttonCancel, 2, 2);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setStartPath();

    // signals and slots connections
    connect(buttonFind, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonReset, SIGNAL(clicked()), this, SLOT(setStartPath()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void FindDialog::setStartPath()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    labelStart->setText(app->current_folder->path());
}

void FindDialog::accept()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    app->find(boxFind->currentText(), boxWindowNames->isChecked(), boxWindowLabels->isChecked(),
              boxFolderNames->isChecked(), boxCaseSensitive->isChecked(),
              boxPartialMatch->isChecked(), boxSubfolders->isChecked());
    // add the combo box's current text to the list when the find button is pressed
    QString text = boxFind->currentText();
    if (!text.isEmpty()) {
        if (boxFind->findText(text) == -1) // no duplicates
        {
            boxFind->insertItem(0, text);
            boxFind->setCurrentIndex(0);
        }
    }
}

FindDialog::~FindDialog() = default;
