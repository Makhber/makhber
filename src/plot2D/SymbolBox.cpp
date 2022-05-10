/***************************************************************************
    File                 : SymbolBox.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Plot symbol combo box

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
#include "SymbolBox.h"

#include <QPixmap>
#include <QPainter>

#include <algorithm>

std::array<const QwtSymbol::Style, 16> SymbolBox::symbols = {
    QwtSymbol::NoSymbol,  QwtSymbol::Ellipse,   QwtSymbol::Rect,      QwtSymbol::Diamond,
    QwtSymbol::Triangle,  QwtSymbol::DTriangle, QwtSymbol::UTriangle, QwtSymbol::LTriangle,
    QwtSymbol::RTriangle, QwtSymbol::Cross,     QwtSymbol::XCross,    QwtSymbol::HLine,
    QwtSymbol::VLine,     QwtSymbol::Star1,     QwtSymbol::Star2,     QwtSymbol::Hexagon
};

auto symbolsSize = SymbolBox::symbols.size();

SymbolBox::SymbolBox(bool rw, QWidget *parent) : QComboBox(parent)
{
    setEditable(rw);
    init();
}

SymbolBox::SymbolBox(QWidget *parent) : QComboBox(parent)
{
    init();
}

void SymbolBox::init()
{
    QPixmap icon = QPixmap(14, 14);
    icon.fill(QColor(Qt::gray));
    const QRect r = QRect(0, 0, 14, 14);
    QPainter p(&icon);
    p.setBackground(QColor(Qt::gray));
    QwtSymbol symb {};
    symb.setSize(8);
    p.setBrush(QBrush(QColor(Qt::white)));

    this->addItem(tr("No Symbol"));

    symb.setStyle(QwtSymbol::Ellipse);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Ellipse"));

    symb.setStyle(QwtSymbol::Rect);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Rectangle"));

    symb.setStyle(QwtSymbol::Diamond);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Diamond"));

    symb.setStyle(QwtSymbol::Triangle);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Triangle"));

    symb.setStyle(QwtSymbol::DTriangle);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Down Triangle"));

    symb.setStyle(QwtSymbol::UTriangle);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Up Triangle"));

    symb.setStyle(QwtSymbol::LTriangle);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Left Triangle"));

    symb.setStyle(QwtSymbol::RTriangle);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Right Triangle"));

    symb.setStyle(QwtSymbol::Cross);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Cross"));

    symb.setStyle(QwtSymbol::XCross);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Diagonal Cross"));

    symb.setStyle(QwtSymbol::HLine);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Horizontal Line"));

    symb.setStyle(QwtSymbol::VLine);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Vertical Line"));

    symb.setStyle(QwtSymbol::Star1);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Star 1"));

    symb.setStyle(QwtSymbol::Star2);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Star 2"));

    symb.setStyle(QwtSymbol::Hexagon);
    p.eraseRect(r);
    symb.drawSymbol(&p, r);
    this->addItem(icon, tr("Hexagon"));

    p.end();
}

void SymbolBox::setStyle(const QwtSymbol::Style &style)
{
    auto ite = std::find(symbols.begin(), symbols.end(), style);
    if (ite == symbols.end())
        this->setCurrentIndex(0);
    else
        this->setCurrentIndex(std::distance(symbols.begin(), ite));
}

QwtSymbol::Style SymbolBox::selectedSymbol() const
{
    size_t i = this->currentIndex();
    if (i < sizeof(symbols))
        return symbols[this->currentIndex()];
    else
        return QwtSymbol::NoSymbol;
}

int SymbolBox::symbolIndex(const QwtSymbol::Style &style)
{
    auto ite = std::find(symbols.begin(), symbols.end(), style);
    if (ite == symbols.end())
        return 0;
    else
        return std::distance(symbols.begin(), ite);
}

QwtSymbol::Style SymbolBox::style(int index)
{
    if (index < (int)sizeof(symbols))
        return symbols[index];
    else
        return QwtSymbol::NoSymbol;
}
