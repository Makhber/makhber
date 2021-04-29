/***************************************************************************
    File                 : ColorButton.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : A button used for color selection

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
#include "ColorButton.h"

#include <QColorDialog>
#include <QPalette>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

const QColor ColorButton::colors[] = {
    QColor(Qt::black),
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::blue),
    QColor(Qt::cyan),
    QColor(Qt::magenta),
    QColor(Qt::yellow),
    QColor(Qt::darkYellow),
    QColor(Qt::darkBlue),
    QColor(Qt::darkMagenta),
    QColor(Qt::darkRed),
    QColor(Qt::darkGreen),
    QColor(Qt::darkCyan),
    QColor(0x00, 0x00, 0xA0),
    QColor(0xFF, 0x80, 0x00),
    QColor(0x80, 0x00, 0xFF),
    QColor(0xFF, 0x00, 0x80),
    QColor(Qt::white),
    QColor(Qt::lightGray),
    QColor(Qt::gray),
    QColor(0xFF, 0xFF, 0x80),
    QColor(0x80, 0xFF, 0xFF),
    QColor(0xFF, 0x80, 0xFF),
    QColor(Qt::darkGray),
    // additional colors from figure 6 in doi:10.1016/j.csda.2008.11.033
    QColor(0x02, 0x3f, 0xa5),
    QColor(0x4a, 0x6f, 0xe3),
    QColor(0x11, 0xc6, 0x38),
    QColor(0x0f, 0xcf, 0xc0),
    QColor(0x8e, 0x06, 0x3b),
    QColor(0xd3, 0x3f, 0x6a),
    QColor(0xef, 0x97, 0x08),
    QColor(0xf7, 0x9c, 0xd4),
    QColor(0x7d, 0x87, 0xb9),
    QColor(0x85, 0x95, 0xe1),
    QColor(0x8d, 0xd5, 0x93),
    QColor(0x9c, 0xde, 0xd6),
    QColor(0xbb, 0x77, 0x84),
    QColor(0xe0, 0x7b, 0x91),
    QColor(0xf0, 0xb9, 0x8d),
    QColor(0xf6, 0xc4, 0xe1),
    QColor(0xbe, 0xc1, 0xd4),
    QColor(0xb5, 0xbb, 0xe3),
    QColor(0xc6, 0xde, 0xc7),
    QColor(0xd5, 0xea, 0xe7),
    QColor(0xd6, 0xbc, 0xc0),
    QColor(0xe6, 0xaf, 0xb9),
    QColor(0xea, 0xd3, 0xc6),
    QColor(0xf3, 0xe1, 0xeb),
};

const unsigned int ColorButton::colors_count = sizeof(colors) / sizeof(colors[0]);

ColorButton::ColorButton(QWidget *parent) : QWidget(parent)
{
    init();
}

void ColorButton::init()
{
    // transpose colors in the 6x8 basic colour grid.
    constexpr int rows = 8, cols = 6;
    static_assert(rows * cols <= sizeof(colors) / sizeof(colors[0]));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            QColorDialog::setStandardColor(j + cols * i, colors[i + rows * j].rgb());
        }
    }
    const int btn_size = 28;
    selectButton = new QPushButton(QPixmap(":/palette.xpm"), QString(), this);
    selectButton->setMinimumWidth(btn_size);
    selectButton->setMinimumHeight(btn_size);

    display = new QFrame(this);
    display->setLineWidth(2);
    display->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    display->setMinimumHeight(btn_size);
    display->setMinimumWidth(2 * btn_size);
    display->setAutoFillBackground(true);
    setColor(QColor(Qt::white));

    auto *l = new QHBoxLayout(this);
    l->setMargin(0);
    l->addWidget(display);
    l->addWidget(selectButton);

    setMaximumWidth(3 * btn_size);
    setMaximumHeight(btn_size);

    connect(selectButton, SIGNAL(clicked()), this, SLOT(pickColor()));
}

void ColorButton::setColor(const QColor &c)
{
    QPalette pal;
    pal.setColor(QPalette::Window, c);
    display->setPalette(pal);
    emit changed(c);
}

QColor ColorButton::color() const
{
    return display->palette().color(QPalette::Window);
}

unsigned int ColorButton::colorIndex(const QColor &c)
{
    const QColor *ite = std::find(std::begin(colors), std::end(colors), c);
    if (ite != std::end(colors) && ite->isValid())
        return (ite - colors);
    else
        return c.rgba();
}

QColor ColorButton::color(unsigned int colorIndex)
{
    if (colorIndex < colors_count)
        return colors[colorIndex];
    else {
        QColor qc = QColor::fromRgba(colorIndex);
        if (qc.isValid())
            return qc;
        else
            return QColor(Qt::black); // default color is black.
    }
}

bool ColorButton::isValidColor(const QColor &c)
{
    const QColor *ite = std::find(std::begin(colors), std::end(colors), c);
    return (ite != std::end(colors) && ite->isValid());
}

QSize ColorButton::sizeHint() const
{
    return QSize(4 * btn_size, btn_size);
}

void ColorButton::pickColor()
{
    QColor c = QColorDialog::getColor(color(), this, "Select color",
                                      QColorDialog::DontUseNativeDialog
                                              | QColorDialog::ShowAlphaChannel);
    if (!c.isValid() || c == color())
        return;
    setColor(c);
}
