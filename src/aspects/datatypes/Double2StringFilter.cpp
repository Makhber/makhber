/***************************************************************************
    File                 : Double2StringFilter.cpp
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

#include "Double2StringFilter.h"

#include "lib/XmlStreamReader.h"

#include <QJsonObject>

void Double2StringFilter::writeExtraAttributes(QJsonObject *jsObject) const
{
    jsObject->insert("format", QString(QChar(numericFormat())));
    jsObject->insert("digits", numDigits());
}

bool Double2StringFilter::load(XmlStreamReader *reader)
{
    QXmlStreamAttributes attribs = reader->attributes();
    QString format_str = attribs.value(reader->namespaceUri().toString(), "format").toString();
    QString digits_str = attribs.value(reader->namespaceUri().toString(), "digits").toString();

    if (AbstractSimpleFilter::load(reader)) {
        bool ok = false;
        int digits = digits_str.toInt(&ok);
        if ((format_str.size() != 1) || !ok)
            reader->raiseError(tr("missing or invalid format attribute(s)"));
        else {
            setNumericFormat(format_str.at(0).toLatin1());
            setNumDigits(digits);
        }
    } else
        return false;

    return !reader->hasError();
}

void Double2StringFilter::setNumericFormat(char format)
{
    exec(new Double2StringFilterSetFormatCmd(this, format));
}

void Double2StringFilter::setNumDigits(int digits)
{
    exec(new Double2StringFilterSetDigitsCmd(this, digits));
}

Double2StringFilterSetFormatCmd::Double2StringFilterSetFormatCmd(Double2StringFilter *target,
                                                                 char new_format)
    : d_target(target), d_other_format(new_format)
{
    if (d_target->parentAspect())
        setText(QObject::tr("%1: set numeric format to '%2'")
                        .arg(d_target->parentAspect()->name())
                        .arg(new_format));
    else
        setText(QObject::tr("set numeric format to '%1'").arg(new_format));
}

void Double2StringFilterSetFormatCmd::redo()
{
    char tmp = d_target->d_format;
    d_target->d_format = d_other_format;
    d_other_format = tmp;
    Q_EMIT d_target->formatChanged();
}

void Double2StringFilterSetFormatCmd::undo()
{
    redo();
}

Double2StringFilterSetDigitsCmd::Double2StringFilterSetDigitsCmd(Double2StringFilter *target,
                                                                 int new_digits)
    : d_target(target), d_other_digits(new_digits)
{
    if (d_target->parentAspect())
        setText(QObject::tr("%1: set decimal digits to %2")
                        .arg(d_target->parentAspect()->name())
                        .arg(new_digits));
    else
        setText(QObject::tr("set decimal digits to %1").arg(new_digits));
}

void Double2StringFilterSetDigitsCmd::redo()
{
    int tmp = d_target->d_digits;
    d_target->d_digits = d_other_digits;
    d_other_digits = tmp;
    Q_EMIT d_target->formatChanged();
}

void Double2StringFilterSetDigitsCmd::undo()
{
    redo();
}
