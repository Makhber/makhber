/***************************************************************************
    File                 : ColorBox.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2007 by Ion Vasilief, Alex Kargovsky, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, kargovsky*yumr.phys.msu.su, thzs*gmx.net
    Description          : A combo box to select a standard color

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
#include "ColorBox.h"

#include <QPixmap>
#include <QPainter>
#include <algorithm>

std::array<const QColor, 24> ColorBox::colors = {
    QColor(Qt::black),        QColor(Qt::red),          QColor(Qt::green),
    QColor(Qt::blue),         QColor(Qt::cyan),         QColor(Qt::magenta),
    QColor(Qt::yellow),       QColor(Qt::darkYellow),   QColor(Qt::darkBlue),
    QColor(Qt::darkMagenta),  QColor(Qt::darkRed),      QColor(Qt::darkGreen),
    QColor(Qt::darkCyan),     QColor(0x00, 0x00, 0xA0), QColor(0xFF, 0x80, 0x00),
    QColor(0x80, 0x00, 0xFF), QColor(0xFF, 0x00, 0x80), QColor(Qt::white),
    QColor(Qt::lightGray),    QColor(Qt::gray),         QColor(0xFF, 0xFF, 0x80),
    QColor(0x80, 0xFF, 0xFF), QColor(0xFF, 0x80, 0xFF), QColor(Qt::darkGray),
};

const int ColorBox::colors_count = static_cast<int>(colors.size());

ColorBox::ColorBox(QWidget *parent) : QComboBox(parent)
{
    setEditable(false);
    init();
}

void ColorBox::init()
{
    QPixmap icon = QPixmap(28, 16);
    QRect r = QRect(0, 0, 27, 15);

    icon.fill(colors[0]);
    this->addItem(icon, tr("black"));

    QPainter p;
    p.begin(&icon);
    p.setBrush(QBrush(colors[1]));
    p.drawRect(r);
    this->addItem(icon, tr("red"));

    p.setBrush(QBrush(colors[2]));
    p.drawRect(r);
    this->addItem(icon, tr("green"));

    p.setBrush(QBrush(colors[3]));
    p.drawRect(r);
    this->addItem(icon, tr("blue"));

    p.setBrush(QBrush(colors[4]));
    p.drawRect(r);
    this->addItem(icon, tr("cyan"));

    p.setBrush(QBrush(colors[5]));
    p.drawRect(r);
    this->addItem(icon, tr("magenta"));

    p.setBrush(QBrush(colors[6]));
    p.drawRect(r);
    this->addItem(icon, tr("yellow"));

    p.setBrush(QBrush(colors[7]));
    p.drawRect(r);
    this->addItem(icon, tr("dark yellow"));

    p.setBrush(QBrush(colors[8]));
    p.drawRect(r);
    this->addItem(icon, tr("navy"));

    p.setBrush(QBrush(colors[9]));
    p.drawRect(r);
    this->addItem(icon, tr("purple"));

    p.setBrush(QBrush(colors[10]));
    p.drawRect(r);
    this->addItem(icon, tr("wine"));

    p.setBrush(QBrush(colors[11]));
    p.drawRect(r);
    this->addItem(icon, tr("olive"));

    p.setBrush(QBrush(colors[12]));
    p.drawRect(r);
    this->addItem(icon, tr("dark cyan"));

    p.setBrush(QBrush(colors[13]));
    p.drawRect(r);
    this->addItem(icon, tr("royal"));

    p.setBrush(QBrush(colors[14]));
    p.drawRect(r);
    this->addItem(icon, tr("orange"));

    p.setBrush(QBrush(colors[15]));
    p.drawRect(r);
    this->addItem(icon, tr("violet"));

    p.setBrush(QBrush(colors[16]));
    p.drawRect(r);
    this->addItem(icon, tr("pink"));

    p.setBrush(QBrush(colors[17]));
    p.drawRect(r);
    this->addItem(icon, tr("white"));

    p.setBrush(QBrush(colors[18]));
    p.drawRect(r);
    this->addItem(icon, tr("light gray"));

    p.setBrush(QBrush(colors[19]));
    p.drawRect(r);
    this->addItem(icon, tr("gray"));

    p.setBrush(QBrush(colors[20]));
    p.drawRect(r);
    this->addItem(icon, tr("light yellow"));

    p.setBrush(QBrush(colors[21]));
    p.drawRect(r);
    this->addItem(icon, tr("light cyan"));

    p.setBrush(QBrush(colors[22]));
    p.drawRect(r);
    this->addItem(icon, tr("light magenta"));

    p.setBrush(QBrush(colors[23]));
    p.drawRect(r);
    this->addItem(icon, tr("dark gray"));
    p.end();
}

void ColorBox::setColor(const QColor &c)
{
    auto ite = std::find(colors.begin(), colors.end(), c);
    if (ite != colors.end())
        this->setCurrentIndex(std::distance(colors.begin(), ite));
    else
        this->setCurrentIndex(0); // default color is black.
}

QColor ColorBox::color() const
{
    size_t i = this->currentIndex();
    if (i < std::size(colors))
        return colors.at(this->currentIndex());
    else
        return QColor(Qt::black); // default color is black.
}

unsigned int ColorBox::colorIndex(const QColor &c)
{
    auto ite = std::find(colors.begin(), colors.end(), c);
    if (ite != colors.end())
        return std::distance(colors.begin(), ite);
    else
        return c.rgba();
}

QColor ColorBox::color(unsigned int colorIndex)
{
    if (colorIndex < colors_count)
        return colors.at(colorIndex);
    else {
        QColor qc = QColor::fromRgba(colorIndex);
        if (qc.isValid())
            return qc;
        else
            return QColor(Qt::black); // default color is black.
    }
}

bool ColorBox::isValidColor(const QColor &color)
{
    for (const auto &i : colors) {
        if (color == i)
            return true;
    }
    return false;
}

int ColorBox::numPredefinedColors()
{
    return colors_count;
}
