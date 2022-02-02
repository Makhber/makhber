/***************************************************************************
    File                 : DateTime2StringFilter.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QDateTime -> QString.

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
#ifndef DATE_TIME2STRING_FILTER_H
#define DATE_TIME2STRING_FILTER_H

#include "aspects/AbstractSimpleFilter.h"

#include <QDateTime>
#include <QRegularExpression>

class DateTime2StringFilterSetFormatCmd;

//! Conversion filter QDateTime -> QString.
class MAKHBER_EXPORT DateTime2StringFilter : public AbstractSimpleFilter
{
    Q_OBJECT

public:
    //! Standard constructor.
    explicit DateTime2StringFilter(QString format = "yyyy-MM-dd hh:mm:ss.zzz")
        : d_format(format) { }
    //! Set the format string to be used for conversion.
    void setFormat(const QString &format);

    //! Return the format string
    /**
     * The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
     * \sa QDate::toString()
     */
    QString format() const { return d_format; }

    //! Return the data type of the column
    virtual Makhber::ColumnDataType dataType() const { return Makhber::TypeQString; }

Q_SIGNALS:
    void formatChanged();

private:
    friend class DateTime2StringFilterSetFormatCmd;
    //! The format string.
    QString d_format;

public:
    virtual QString textAt(int row) const
    {
        if (!d_inputs.value(0))
            return QString();
        QDateTime input_value = d_inputs.value(0)->dateTimeAt(row);
        if (!input_value.date().isValid() && input_value.time().isValid())
            input_value.setDate(QDate(1900, 1, 1));
        return input_value.toString(d_format);
    }

    //! \name Json related functions
    //@{
    virtual void writeExtraAttributes(QJsonObject *) const;
    virtual bool load(QJsonObject *reader);
    //@}

protected:
    //! Using typed ports: only DateTime inputs are accepted.
    virtual bool inputAcceptable(int, const AbstractColumn *source)
    {
        return source->dataType() == Makhber::TypeQDateTime;
    }
};

class MAKHBER_EXPORT DateTime2StringFilterSetFormatCmd : public QUndoCommand
{
public:
    DateTime2StringFilterSetFormatCmd(DateTime2StringFilter *target, const QString &new_format);

    virtual void redo();
    virtual void undo();

private:
    DateTime2StringFilter *d_target;
    QString d_other_format;
};

#endif // ifndef DATE_TIME2STRING_FILTER_H
