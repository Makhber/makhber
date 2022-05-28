/***************************************************************************
    File                 : AbstractSimpleFilter.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Simplified filter interface for filters with
                           only one output port.

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

#include "AbstractSimpleFilter.h"

#include <QtDebug>
#include <QJsonObject>

// TODO: should simple filters have a name argument?
AbstractSimpleFilter::AbstractSimpleFilter()
    : AbstractFilter("SimpleFilter"), d_output_column(new SimpleFilterColumn(this))
{
    addChild(d_output_column);
}

void AbstractSimpleFilter::clearMasks()
{
    Q_EMIT d_output_column->maskingAboutToChange(d_output_column);
    d_masking.clear();
    Q_EMIT d_output_column->maskingChanged(d_output_column);
}

void AbstractSimpleFilter::setMasked(Interval<int> i, bool mask)
{
    Q_EMIT d_output_column->maskingAboutToChange(d_output_column);
    d_masking.setValue(i, mask);
    Q_EMIT d_output_column->maskingChanged(d_output_column);
}

void AbstractSimpleFilter::inputPlotDesignationAboutToChange(const AbstractColumn *)
{
    Q_EMIT d_output_column->plotDesignationAboutToChange(d_output_column);
}

void AbstractSimpleFilter::inputPlotDesignationChanged(const AbstractColumn *)
{
    Q_EMIT d_output_column->plotDesignationChanged(d_output_column);
}

void AbstractSimpleFilter::inputModeAboutToChange(const AbstractColumn *)
{
    Q_EMIT d_output_column->dataAboutToChange(d_output_column);
}

void AbstractSimpleFilter::inputModeChanged(const AbstractColumn *)
{
    Q_EMIT d_output_column->dataChanged(d_output_column);
}

void AbstractSimpleFilter::inputDataAboutToChange(const AbstractColumn *)
{
    Q_EMIT d_output_column->dataAboutToChange(d_output_column);
}

void AbstractSimpleFilter::inputDataChanged(const AbstractColumn *)
{
    Q_EMIT d_output_column->dataChanged(d_output_column);
}

void AbstractSimpleFilter::inputRowsAboutToBeInserted(const AbstractColumn *source, int before,
                                                      int count)
{
    Q_UNUSED(source);
    Q_UNUSED(count);
    for (Interval<int> output_range : dependentRows(Interval<int>(before, before)))
        Q_EMIT d_output_column->rowsAboutToBeInserted(d_output_column, output_range.minValue(),
                                                      output_range.size());
}

void AbstractSimpleFilter::inputRowsInserted(const AbstractColumn *source, int before, int count)
{
    Q_UNUSED(source);
    Q_UNUSED(count);
    for (Interval<int> output_range : dependentRows(Interval<int>(before, before)))
        Q_EMIT d_output_column->rowsInserted(d_output_column, output_range.minValue(),
                                             output_range.size());
}

void AbstractSimpleFilter::inputRowsAboutToBeRemoved(const AbstractColumn *source, int first,
                                                     int count)
{
    Q_UNUSED(source);
    for (Interval<int> output_range : dependentRows(Interval<int>(first, first + count - 1)))
        Q_EMIT d_output_column->rowsAboutToBeRemoved(d_output_column, output_range.minValue(),
                                                     output_range.size());
}

void AbstractSimpleFilter::inputRowsRemoved(const AbstractColumn *source, int first, int count)
{
    Q_UNUSED(source);
    for (Interval<int> output_range : dependentRows(Interval<int>(first, first + count - 1)))
        Q_EMIT d_output_column->rowsRemoved(d_output_column, output_range.minValue(),
                                            output_range.size());
}

AbstractColumn *AbstractSimpleFilter::output(int port)
{
    return port == 0 ? static_cast<AbstractColumn *>(d_output_column) : nullptr;
}

const AbstractColumn *AbstractSimpleFilter::output(int port) const
{
    return port == 0 ? static_cast<const AbstractColumn *>(d_output_column) : nullptr;
}

void AbstractSimpleFilter::save(QJsonObject *jsObject) const
{
    writeBasicAttributes(jsObject);
    writeExtraAttributes(jsObject);
    jsObject->insert("filterName", metaObject()->className());
}

bool AbstractSimpleFilter::load(QJsonObject *reader)
{
    if (reader->value("name").toString() != "simple_filter")
        return false;
    readBasicAttributes(reader);

    QString str = reader->value("filter_name").toString();
    if (str != metaObject()->className())
        return false;

    readCommentElement(reader);
    return true;
}
