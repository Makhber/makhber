/***************************************************************************
    File                 : SymbolDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Tool window to select special text characters

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
#include "SymbolDialog.h"

#include <QPushButton>
#include <QSizePolicy>
#include <QGroupBox>
#include <QShortcut>
#include <QHBoxLayout>
#include <QButtonGroup>

SymbolDialog::SymbolDialog(CharSet charSet, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizeGripEnabled(false);

    buttons = new QButtonGroup(this);
    mainLayout = new QVBoxLayout(this);
    gridLayout = new QGridLayout();

    if (charSet == SymbolDialog::lowerGreek)
        initLowerGreekChars();
    else if (charSet == SymbolDialog::upperGreek)
        initUpperGreekChars();
    else if (charSet == SymbolDialog::mathSymbols)
        initMathSymbols();
    else if (charSet == SymbolDialog::arrowSymbols)
        initArrowSymbols();
    else
        initNumberSymbols();

    closeButton = new QPushButton(tr("&Close"), this);

    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(closeButton);

    languageChange();

    connect(buttons, SIGNAL(buttonClicked(int)), this, SLOT(getChar(int)));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    auto *shortcut = new QShortcut(Qt::Key_Return, this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(addCurrentChar()));
}

void SymbolDialog::initLowerGreekChars()
{
    int i = 0, counter = 0;
    for (i = 0; i <= (0x3C9 - 0x3B1); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x3B1)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x3D1)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x3D5)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    for (i = 0; i <= (0x3F1 - 0x3F0); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x3F0)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    numButtons = counter;
}

void SymbolDialog::initUpperGreekChars()
{
    int i = 0, counter = 0;
    for (i = 0; i <= (0x3A1 - 0x391); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x391)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    for (i = 0; i <= (0x3A9 - 0x3A3); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x3A3)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 5, counter % 5);
    }
    numButtons = counter;
}

void SymbolDialog::initNumberSymbols()
{
    int i = 0, counter = 0;
    for (i = 0; i <= (0x216B - 0x2153); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2153)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x217B - 0x2170); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2170)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    numButtons = counter;
}

void SymbolDialog::initMathSymbols()
{
    int i = 0, counter = 0;
    for (i = 0; i <= (0x220D - 0x2200); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2200)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x2212 - 0x220F); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x220F)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x00B1)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x2213)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x221E - 0x2217); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2217)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x2222)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x2230 - 0x2227); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2227)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x223F)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(i + 0x2245)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x2248)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x2255 - 0x2254); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2254)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x2259)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x2267 - 0x225F); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x225F)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x226B - 0x226A); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x226A)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    for (i = 0; i <= (0x2289 - 0x2282); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2282)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    // h bar
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x210F)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    // angstrom
    {
        counter++;
        auto *btn = new QPushButton(QString(QChar(0x212B)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    // per mille and per ten thousand
    for (i = 0; i <= (0x2031 - 0x2030); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2030)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 8, counter % 8);
    }
    numButtons = counter;
}

void SymbolDialog::initArrowSymbols()
{
    int i = 0, counter = 0;
    for (i = 0; i <= (0x219B - 0x2190); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x2190)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 6, counter % 6);
    }
    for (i = 0; i <= (0x21A7 - 0x21A4); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x21A4)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 6, counter % 6);
    }
    for (i = 0; i <= (0x21D5 - 0x21CD); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x21CD)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 6, counter % 6);
    }
    for (i = 0; i <= (0x21E9 - 0x21E6); i++, counter++) {
        auto *btn = new QPushButton(QString(QChar(i + 0x21E6)));
        btn->setMaximumWidth(40);
        btn->setFlat(true);
        btn->setAutoDefault(false);
        buttons->addButton(btn, counter + 1);
        gridLayout->addWidget(btn, counter / 6, counter % 6);
    }
    numButtons = counter;
}

void SymbolDialog::addCurrentChar()
{
    for (int i = 1; i < numButtons; i++) {
        auto *btn = dynamic_cast<QPushButton *>(buttons->button(i));
        if (btn && btn->hasFocus())
            Q_EMIT addLetter(btn->text());
    }
}

void SymbolDialog::getChar(int btnIndex)
{
    auto *btn = dynamic_cast<QPushButton *>(buttons->button(btnIndex));
    if (btn)
        Q_EMIT addLetter(btn->text());
}

void SymbolDialog::languageChange()
{
    setWindowTitle(tr("Choose Symbol"));
}

SymbolDialog::~SymbolDialog() = default;

void SymbolDialog::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    // select the first button as default (in case [return] is pressed)
    (dynamic_cast<QPushButton *>(buttons->button(1)))->setFocus(Qt::TabFocusReason);
}
