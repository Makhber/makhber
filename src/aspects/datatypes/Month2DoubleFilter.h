/***************************************************************************
    File                 : MonthDoubleFilter.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Conversion filter QDateTime -> double, translating
                           dates into months (January -> 1).

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
#ifndef MONTH2DOUBLE_FILTER_H
#define MONTH2DOUBLE_FILTER_H

#include "aspects/AbstractSimpleFilter.h"

#include <QDateTime>

/**
 * \brief Conversion filter QDateTime -> double, translating dates into months (January -> 1).
 *
 * \sa QDate::month()
 */
class MAKHBER_EXPORT Month2DoubleFilter : public AbstractSimpleFilter
{
    Q_OBJECT

public:
    virtual double valueAt(int row) const override
    {
        if (!d_inputs.value(0))
            return 0;
        return double(d_inputs.value(0)->dateAt(row).month());
    }

    //! Return the data type of the column
    virtual Makhber::ColumnDataType dataType() const override { return Makhber::TypeDouble; }

protected:
    //! Using typed ports: only date-time inputs are accepted.
    virtual bool inputAcceptable(int, const AbstractColumn *source) override
    {
        return source->dataType() == Makhber::TypeQDateTime;
    }
};

#endif // ifndef MONTH2DOUBLE_FILTER_H
