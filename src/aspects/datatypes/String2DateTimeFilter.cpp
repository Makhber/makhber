/***************************************************************************
    File                 : String2DateTimeFilter.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QString -> QDateTime.

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
#include "String2DateTimeFilter.h"

#include <QStringList>
#include <QJsonObject>

std::array<const char *, 11> String2DateTimeFilter::date_formats = {
    "yyyy-M-d", // ISO 8601 w/ and w/o leading zeros
    "yyyy/M/d",
    "d/M/yyyy", // European style day/month order (this order seems to be used in more countries
                // than the US style M/d/yyyy)
    "d/M/yy", "d-M-yyyy", "d-M-yy",
    "d.M.yyyy", // German style
    "d.M.yy", "M/yyyy",
    "d.M.", // German form w/o year
    "yyyyMMdd"
};

std::array<const char *, 9> String2DateTimeFilter::time_formats = {
    "h", "h ap", "h:mm", "h:mm ap", "h:mm:ss", "h:mm:ss.zzz", "h:mm:ss:zzz", "mm:ss.zzz", "hmmss"
};

QDateTime String2DateTimeFilter::dateTimeAt(int row) const
{
    if (!d_inputs.value(0))
        return QDateTime();
    QString input_value = d_inputs.value(0)->textAt(row);
    if (input_value.isEmpty())
        return QDateTime();

    // first try the selected format string d_format
    QDateTime result = QDateTime::fromString(input_value, d_format);
    if (result.isValid())
        return result;

        // fallback:
        // try other format strings built from date_formats and time_formats
        // comma and space are valid separators between date and time
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList strings = input_value.simplified().split(",", Qt::SkipEmptyParts);
    if (strings.size() == 1)
        strings = strings.at(0).split(" ", Qt::SkipEmptyParts);
#else
    QStringList strings = input_value.simplified().split(",", QString::SkipEmptyParts);
    if (strings.size() == 1)
        strings = strings.at(0).split(" ", QString::SkipEmptyParts);
#endif

    if (strings.size() < 1)
        return result; // invalid date/time from first attempt

    QDate date_result;
    QTime time_result;

    QString date_string = strings.at(0).trimmed();
    QString time_string;
    if (strings.size() > 1)
        time_string = strings.at(1).trimmed();
    else
        time_string = date_string;

    // try to find a valid date
    for (auto date_format : date_formats) {
        date_result = QDate::fromString(date_string, date_format);
        if (date_result.isValid())
            break;
    }
    // try to find a valid time
    for (auto time_format : time_formats) {
        time_result = QTime::fromString(time_string, time_format);
        if (time_result.isValid())
            break;
    }

    if (!date_result.isValid() && time_result.isValid())
        date_result.setDate(1900, 1, 1); // this is what QDateTime does e.g. for
                                         // QDateTime::fromString("00:00","hh:mm");
    return QDateTime(date_result, time_result);
}

void String2DateTimeFilter::writeExtraAttributes(QJsonObject *jsObject) const
{
    jsObject->insert("format", format());
}

bool String2DateTimeFilter::load(QJsonObject *reader)
{
    QString str = reader->value("format").toString();

    if (AbstractSimpleFilter::load(reader))
        setFormat(str);
    else
        return false;

    return true;
}

void String2DateTimeFilter::setFormat(const QString &format)
{
    exec(new String2DateTimeFilterSetFormatCmd(this, format));
}

String2DateTimeFilterSetFormatCmd::String2DateTimeFilterSetFormatCmd(String2DateTimeFilter *target,
                                                                     const QString &new_format)
    : d_target(target), d_other_format(new_format)
{
    if (d_target->parentAspect())
        setText(QObject::tr("%1: set date-time format to %2")
                        .arg(d_target->parentAspect()->name(), new_format));
    else
        setText(QObject::tr("set date-time format to %1").arg(new_format));
}

void String2DateTimeFilterSetFormatCmd::redo()
{
    QString tmp = d_target->d_format;
    d_target->d_format = d_other_format;
    d_other_format = tmp;
    Q_EMIT d_target->formatChanged();
}

void String2DateTimeFilterSetFormatCmd::undo()
{
    redo();
}
