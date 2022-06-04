/***************************************************************************
    File                 : Double2StringFilter.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Locale-aware conversion filter double -> QString.

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
#ifndef DOUBLE2STRING_FILTER_H
#define DOUBLE2STRING_FILTER_H

#include "aspects/AbstractSimpleFilter.h"

#include <QLocale>
#include <QChar>
#include <QtDebug>

//! Locale-aware conversion filter double -> QString.
class MAKHBER_EXPORT Double2StringFilter : public AbstractSimpleFilter
{
    Q_OBJECT

public:
    //! Standard constructor.
    explicit Double2StringFilter(char format = 'e', int digits = 6)
        : d_format(format), d_digits(digits)
    {
    }
    //! Set format character as in QString::number
    void setNumericFormat(char format);
    //! Set number of displayed digits
    void setNumDigits(int digits);
    //! Get format character as in QString::number
    char numericFormat() const { return d_format; }
    //! Get number of displayed digits
    int numDigits() const { return d_digits; }

    //! Return the data type of the column
    virtual Makhber::ColumnDataType dataType() const override { return Makhber::TypeQString; }

Q_SIGNALS:
    void formatChanged();

private:
    friend class Double2StringFilterSetFormatCmd;
    friend class Double2StringFilterSetDigitsCmd;
    //! Format character as in QString::number
    char d_format;
    //! Display digits or precision as in QString::number
    int d_digits;

    //! \name Json related functions
    //@{
    virtual void writeExtraAttributes(QJsonObject *) const override;
    virtual bool load(QJsonObject *reader) override;
    //@}

public:
    virtual QString textAt(int row) const override
    {
        if (!d_inputs.value(0))
            return QString();
        if (d_inputs.value(0)->rowCount() <= row)
            return QString();
        if (d_inputs.value(0)->isInvalid(row))
            return QString();
        return QLocale().toString(d_inputs.value(0)->valueAt(row), d_format, d_digits);
    }

protected:
    //! Using typed ports: only double inputs are accepted.
    virtual bool inputAcceptable(int, const AbstractColumn *source) override
    {
        return source->dataType() == Makhber::TypeDouble;
    }
};

class MAKHBER_EXPORT Double2StringFilterSetFormatCmd : public QUndoCommand
{
public:
    Double2StringFilterSetFormatCmd(Double2StringFilter *target, char new_format);

    virtual void redo();
    virtual void undo();

private:
    Double2StringFilter *d_target;
    char d_other_format;
};

class MAKHBER_EXPORT Double2StringFilterSetDigitsCmd : public QUndoCommand
{
public:
    Double2StringFilterSetDigitsCmd(Double2StringFilter *target, int new_digits);

    virtual void redo();
    virtual void undo();

private:
    Double2StringFilter *d_target;
    int d_other_digits;
};

#endif // ifndef DOUBLE2STRING_FILTER_H
