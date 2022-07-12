/***************************************************************************
    File                 : AsciiTableImportFilter.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2009 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Import an ASCII file as Table.

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

#include "AsciiTableImportFilter.h"

#include "table/future_Table.h"
#include "lib/IntervalAttribute.h"
#include "aspects/column/Column.h"
#include "aspects/datatypes/String2DoubleFilter.h"

#include <QTextStream>
#include <QStringList>
#include <QLocale>

#include <utility>
#include <vector>
#include <iostream>

QStringList AsciiTableImportFilter::fileExtensions() const
{
    return QStringList() << "txt"
                         << "csv"
                         << "dat";
}

namespace {
// redirect to QIODevice's readLine so that we can override it to handle '\r' line terminators
struct MakhberTextStream
{
    QIODevice &input;
    bool good;
    operator bool() const { return good; }
    enum { none, simplify, trim } whiteSpaceTreatment;
    QString separator;
    MakhberTextStream(QIODevice &inp, QString sep)
        : input(inp), good(true), whiteSpaceTreatment(none), separator(std::move(sep))
    {
    }

    QStringList readRow()
    {
        char c = 0;
        QString r;

        while ((good = input.getChar(&c)))
            if (c != '\r' && c != '\n') {
                r += c;
            } else {
                if (c == '\r' && input.getChar(&c) && c != '\n') // eat \n following \r
                    input.ungetChar(c);
                break;
            };
        switch (whiteSpaceTreatment) {
        case none:
            return r.split(separator);
        case simplify:
            return r.simplified().split(separator);
        case trim:
            return r.trimmed().split(separator);
        default:
            return QStringList();
        }
    }
};

template<class C>
C conv(const QString &x);
template<>
QString conv<QString>(const QString &x)
{
    return x;
}
template<>
qreal conv<qreal>(const QString &x)
{
    return (qreal)(QLocale::c().toDouble(x));
}

template<class T>
struct AP : public std::unique_ptr<T>
{
    AP() : std::unique_ptr<T>(new T) { }
    AP(const AP &x) : std::unique_ptr<T>(new T(*x)) { }
};

template<class C>
void readCols(QList<Column *> &cols, MakhberTextStream &stream, bool readColNames)
{
    QStringList row, column_names;
    int i = 0;

    // read first row
    row = stream.readRow();

    int dataSize = row.size();
    std::vector<AP<C>> data(dataSize);
    std::vector<IntervalAttribute<bool>> invalid_cells(row.size());

    if (readColNames)
        column_names = row;
    else
        for (i = 0; i < row.size(); ++i) {
            column_names << QString::number(i + 1);
            *data[i] << conv<typename C::value_type>(row[i]);
        }

    // read rest of data
    while (stream) {
        row = stream.readRow();
        if (stream || (row != QStringList(""))) {
            for (i = 0; i < row.size() && i < dataSize; ++i)
                *data[i] << conv<typename C::value_type>(row[i]);
            // some rows might have too few columns (re-use value of i from above loop)
            for (; i < dataSize; ++i) {
                invalid_cells[i].setValue(data[i]->size(), true);
                *data[i] << conv<typename C::value_type>("");
            }
        }
    }

    for (i = 0; i < dataSize; ++i) {
        cols << new Column(std::move(column_names[i]), std::unique_ptr<C>(std::move(data[i])),
                           std::move(invalid_cells[i]));
        if (i == 0)
            cols.back()->setPlotDesignation(Makhber::X);
        else
            cols.back()->setPlotDesignation(Makhber::Y);
    }
}

}

AbstractAspect *AsciiTableImportFilter::importAspect(QIODevice &input)
{
    MakhberTextStream stream(input, d_separator);
    if (d_simplify_whitespace)
        stream.whiteSpaceTreatment = MakhberTextStream::simplify;
    else if (d_trim_whitespace)
        stream.whiteSpaceTreatment = MakhberTextStream::trim;

    // skip ignored lines
    for (int i = 0; i < d_ignored_lines; i++)
        stream.readRow();

    // build a Table from the gathered data
    QList<Column *> cols;
    if (d_convert_to_numeric)
        readCols<QVector<qreal>>(cols, stream, d_first_row_names_columns);
    else
        readCols<QStringList>(cols, stream, d_first_row_names_columns);

    // renaming will be done by the kernel
    auto *result = new future::Table(0, 0, tr("Table"));
    result->appendColumns(cols);
    return result;
}
