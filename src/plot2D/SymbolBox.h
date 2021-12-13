/***************************************************************************
    File                 : SymbolBox.h
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
#ifndef SYMBOLBOX_H
#define SYMBOLBOX_H

#include "core/MakhberDefs.h"

#include <qwt_symbol.h>

#include <QComboBox>

#include <array>

//! Plot symbol combo box
class MAKHBER_EXPORT SymbolBox : public QComboBox
{
    Q_OBJECT
public:
    SymbolBox(bool rw, QWidget *parent = 0);
    SymbolBox(QWidget *parent = 0);

    void setStyle(const QwtSymbol::Style &c);
    QwtSymbol::Style selectedSymbol() const;

    static QwtSymbol::Style style(int index);
    static int symbolIndex(const QwtSymbol::Style &style);

    static std::array<const QwtSymbol::Style, 16> symbols;

protected:
    void init();
};

#endif
