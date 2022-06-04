/***************************************************************************
    File                 : DateTime2DoubleFilter.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QDateTime -> double (using Julian day).

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
#ifndef DATE_TIME2DOUBLE_FILTER_H
#define DATE_TIME2DOUBLE_FILTER_H

#include "aspects/datatypes/NumericDateTimeBaseFilter.h"

//! Conversion filter QDateTime -> double (using offset from selected datetime).
class MAKHBER_EXPORT DateTime2DoubleFilter : public NumericDateTimeBaseFilter
{
    Q_OBJECT

public:
    // The equivalence of one unit defaults to a day if nothing else is specified.
    // Default offset date is the noon of January 1st, 4713 BC as per Julian Day Number convention.
    // DateTime2DoubleFilter(const UnitInterval unit = UnitInterval::Day, const QDateTime&
    // date_time_0 = zeroOffsetDate) :
    DateTime2DoubleFilter(const UnitInterval unit, const QDateTime &date_time_0)
        : NumericDateTimeBaseFilter(unit, date_time_0) {};

    virtual double valueAt(int row) const override
    {
        if (!d_inputs.value(0))
            return 0.0;
        QDateTime input_value = d_inputs.value(0)->dateTimeAt(row);
        return offsetToDouble(input_value);
    }

    //! Return the data type of the column
    virtual Makhber::ColumnDataType dataType() const override { return Makhber::TypeDouble; }

    //! Explicit conversion from base class using conversion ctor
    explicit DateTime2DoubleFilter(const NumericDateTimeBaseFilter &numeric)
        : NumericDateTimeBaseFilter(numeric) {};

protected:
    //! Using typed ports: only DateTime inputs are accepted.
    virtual bool inputAcceptable(int, const AbstractColumn *source) override
    {
        return source->dataType() == Makhber::TypeQDateTime;
    }
};

#endif // ifndef DATE_TIME2DOUBLE_FILTER_H
