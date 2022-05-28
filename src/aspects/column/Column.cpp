/***************************************************************************
    File                 : Column.cpp
    Project              : Makhber
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)

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

#include "Column.h"

#include "aspects/column/ColumnPrivate.h"
#include "aspects/column/columncommands.h"

#include <QIcon>
#include <QtDebug>
#include <QJsonObject>
#include <QJsonArray>

Column::Column(const QString &name, Makhber::ColumnMode mode) : AbstractColumn(name)
{
    d_column_private = new Private(this, mode);
    d_string_io = new ColumnStringIO(this);
    d_column_private->inputFilter()->input(0, d_string_io);
    outputFilter()->input(0, this);
    addChild(d_column_private->inputFilter());
    addChild(outputFilter());
}

template<>
void Column::initPrivate(std::unique_ptr<QVector<qreal>> d, IntervalAttribute<bool> v)
{
    d_column_private =
            new Private(this, Makhber::TypeDouble, Makhber::ColumnMode::Numeric, d.release(), v);
}

template<>
void Column::initPrivate(std::unique_ptr<QStringList> d, IntervalAttribute<bool> v)
{
    d_column_private =
            new Private(this, Makhber::TypeQString, Makhber::ColumnMode::Text, d.release(), v);
}

template<>
void Column::initPrivate(std::unique_ptr<QList<QDateTime>> d, IntervalAttribute<bool> v)
{
    d_column_private = new Private(this, Makhber::TypeQDateTime, Makhber::ColumnMode::DateTime,
                                   d.release(), v);
}

void Column::init()
{
    d_string_io = new ColumnStringIO(this);
    d_column_private->inputFilter()->input(0, d_string_io);
    outputFilter()->input(0, this);
    addChild(d_column_private->inputFilter());
    addChild(outputFilter());
}

Column::~Column()
{
    delete d_string_io;
    delete d_column_private;
}

void Column::setColumnMode(Makhber::ColumnMode mode, AbstractFilter *conversion_filter)
{
    if (mode != columnMode()) // mode changed
    {
        beginMacro(QObject::tr("%1: change column type").arg(name()));
        AbstractSimpleFilter *old_input_filter = inputFilter();
        exec(new ColumnSetModeCmd(d_column_private, mode, conversion_filter));
        if (d_column_private->inputFilter() != old_input_filter) {
            removeChild(old_input_filter);
            addChild(d_column_private->inputFilter());
            d_column_private->inputFilter()->input(0, d_string_io);
        }
        /* AbstractSimpleFilter *old_output_filter = outputFilter();
        if (outputFilter() != old_output_filter) { // Always false
            removeChild(old_output_filter);
            addChild(outputFilter());
            outputFilter()->input(0, this);
        }*/
        endMacro();
    }
    // mode is not changed, but DateTime numeric converter changed
    else if ((mode == Makhber::ColumnMode::DateTime) && (nullptr != conversion_filter)) {
        auto numeric_datetime_converter =
                reinterpret_cast<NumericDateTimeBaseFilter *>(conversion_filter);
        // if (nullptr != numeric_datetime_converter) Always true
        //  the ownership of converter is taken
        d_column_private->setNumericDateTimeFilter(numeric_datetime_converter);
    }
}

bool Column::copy(const AbstractColumn *other)
{
    Q_CHECK_PTR(other);
    if (other->dataType() != dataType())
        return false;
    exec(new ColumnFullCopyCmd(d_column_private, other));
    return true;
}

bool Column::copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows)
{
    Q_CHECK_PTR(source);
    if (source->dataType() != dataType())
        return false;
    exec(new ColumnPartialCopyCmd(d_column_private, source, source_start, dest_start, num_rows));
    return true;
}

void Column::insertRows(int before, int count)
{
    if (count > 0)
        exec(new ColumnInsertEmptyRowsCmd(d_column_private, before, count));
}

void Column::removeRows(int first, int count)
{
    if (count > 0)
        exec(new ColumnRemoveRowsCmd(d_column_private, first, count));
}

void Column::setPlotDesignation(Makhber::PlotDesignation pd)
{
    if (pd != plotDesignation())
        exec(new ColumnSetPlotDesignationCmd(d_column_private, pd));
}

void Column::clear()
{
    exec(new ColumnClearCmd(d_column_private));
}

void Column::notifyReplacement(const AbstractColumn *replacement)
{
    Q_EMIT aboutToBeReplaced(this, replacement);
}

void Column::clearValidity()
{
    exec(new ColumnClearValidityCmd(d_column_private));
}

void Column::clearMasks()
{
    exec(new ColumnClearMasksCmd(d_column_private));
}

void Column::setInvalid(Interval<int> i, bool invalid)
{
    exec(new ColumnSetInvalidCmd(d_column_private, i, invalid));
}

void Column::setInvalid(int row, bool invalid)
{
    setInvalid(Interval<int>(row, row), invalid);
}

void Column::setMasked(Interval<int> i, bool mask)
{
    exec(new ColumnSetMaskedCmd(d_column_private, i, mask));
}

void Column::setMasked(int row, bool mask)
{
    setMasked(Interval<int>(row, row), mask);
}

void Column::setFormula(Interval<int> i, const QString &formula)
{
    exec(new ColumnSetFormulaCmd(d_column_private, i, formula));
}

void Column::setFormula(int row, const QString &formula)
{
    setFormula(Interval<int>(row, row), formula);
}

void Column::clearFormulas()
{
    exec(new ColumnClearFormulasCmd(d_column_private));
}

void Column::setTextAt(int row, const QString &new_value)
{
    exec(new ColumnSetTextCmd(d_column_private, row, new_value));
}

void Column::replaceTexts(int first, const QStringList &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceTextsCmd(d_column_private, first, new_values));
}

void Column::setDateAt(int row, const QDate &new_value)
{
    setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

void Column::setTimeAt(int row, const QTime &new_value)
{
    setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

void Column::setDateTimeAt(int row, const QDateTime &new_value)
{
    exec(new ColumnSetDateTimeCmd(d_column_private, row, new_value));
}

void Column::replaceDateTimes(int first, const QList<QDateTime> &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceDateTimesCmd(d_column_private, first, new_values));
}

void Column::setValueAt(int row, double new_value)
{
    exec(new ColumnSetValueCmd(d_column_private, row, new_value));
}

void Column::replaceValues(int first, const QVector<qreal> &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceValuesCmd(d_column_private, first, new_values));
}

QString Column::textAt(int row) const
{
    return d_column_private->textAt(row);
}

QDate Column::dateAt(int row) const
{
    return d_column_private->dateAt(row);
}

QTime Column::timeAt(int row) const
{
    return d_column_private->timeAt(row);
}

QDateTime Column::dateTimeAt(int row) const
{
    return d_column_private->dateTimeAt(row);
}

double Column::valueAt(int row) const
{
    return d_column_private->valueAt(row);
}

QIcon Column::icon() const
{
    switch (dataType()) {
    case Makhber::TypeDouble:
        return QIcon(QPixmap(":/numerictype.png"));
    case Makhber::TypeQString:
        return QIcon(QPixmap(":/texttype.png"));
    case Makhber::TypeQDateTime:
        return QIcon(QPixmap(":/datetype.png"));
    }
    return QIcon();
}

void Column::save(QJsonObject *jsObject) const
{
    writeBasicAttributes(jsObject);
    jsObject->insert("type", Makhber::enumValueToString(dataType(), "ColumnDataType"));
    jsObject->insert("mode",
                     Makhber::enumValueToString(static_cast<int>(columnMode()), "ColumnMode"));
    jsObject->insert("plotDesignation",
                     Makhber::enumValueToString(plotDesignation(), "PlotDesignation"));

    writeCommentElement(jsObject);

    QJsonObject jsInput {};
    d_column_private->inputFilter()->save(&jsInput);
    jsObject->insert("inputFilter", jsInput);

    QJsonObject jsOutput {};
    outputFilter()->save(&jsOutput);
    jsObject->insert("outputFilter", jsOutput);

    QList<Interval<int>> masks = maskedIntervals();
    QJsonArray jsMasks {};
    for (Interval<int> interval : masks) {
        QJsonObject jsMask {};
        jsMask.insert("startRow", interval.minValue());
        jsMask.insert("endRow", interval.maxValue());
        jsMasks.append(jsMask);
    }
    jsObject->insert("masks", jsMasks);

    QList<Interval<int>> formulas = formulaIntervals();
    QJsonArray jsFormulas {};
    for (Interval<int> interval : formulas) {
        QJsonObject jsFormula {};
        jsFormula.insert("startRow", interval.minValue());
        jsFormula.insert("endRow", interval.maxValue());
        jsFormula.insert("formula", formula(interval.minValue()));
        jsFormulas.append(jsFormula);
    }
    jsObject->insert("formulas", jsFormulas);

    jsObject->insert("columnDataType", Makhber::enumValueToString(dataType(), "ColumnDataType"));
    QJsonArray jsData {};
    switch (dataType()) {
    case Makhber::TypeDouble:
        for (int i = 0; i < rowCount(); i++) {
            jsData.append(valueAt(i));
            // writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
        }
        break;
    case Makhber::TypeQString:
        for (int i = 0; i < rowCount(); i++) {
            jsData.append(textAt(i));
            // writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
        }
        break;
    case Makhber::TypeQDateTime: {
        { // this conversion is needed to store base class;
            NumericDateTimeBaseFilter numericFilter(
                    *(d_column_private->getNumericDateTimeFilter()));
            QJsonObject jsFilter {};
            numericFilter.save(&jsFilter);
            jsObject->insert("numericDateTimeFilter", jsFilter);
        }
        for (int i = 0; i < rowCount(); i++) {
            jsData.append(QLocale::c().toString(dateTimeAt(i), "dd-MM-yyyy hh:mm:ss:zzz"));
            // writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
        }
        break;
    }
    }
    jsObject->insert("data", jsData);
}

bool Column::load(QJsonObject *reader)
{
    QString str {};

    readBasicAttributes(reader);
    // read type
    str = reader->value("type").toString();
    if (str.isEmpty()) {
        //            reader->raiseError(tr("column type missing"));
        return false;
    }
    int type_code = Makhber::enumStringToValue(str, "ColumnDataType");
    if (type_code == -1) {
        //            reader->raiseError(tr("column type invalid"));
        return false;
    }
    // read mode
    str = reader->value("mode").toString();
    if (str.isEmpty()) {
        //            reader->raiseError(tr("column mode missing"));
        return false;
    }
    int mode_code = Makhber::enumStringToValue(str, "ColumnMode");
    if (mode_code == -1) {
        //            reader->raiseError(tr("column mode invalid"));
        return false;
    }
    setColumnMode((Makhber::ColumnMode)mode_code);
    if (type_code != int(dataType())) {
        //            reader->raiseError(tr("column type or mode invalid"));
        return false;
    }
    // read plot designation
    str = reader->value("plotDesignation").toString();
    int pd_code = Makhber::enumStringToValue(str, "PlotDesignation");
    if (str.isEmpty())
        setPlotDesignation(Makhber::noDesignation);
    else if (pd_code == -1) {
        //            reader->raiseError(tr("column plot designation invalid"));
        return false;
    } else
        setPlotDesignation((Makhber::PlotDesignation)pd_code);

    clearValidity();

    setComment("");
    readCommentElement(reader);

    if (type_code == Makhber::TypeQDateTime)
        ReadNumericDateTimeFilter(reader);
    ReadInputFilter(reader);
    ReadOutputFilter(reader);

    clearMasks();
    QJsonArray jsMasks = reader->value("masks").toArray();
    for (int i = 0; i < jsMasks.size(); i++) {
        ReadMask(jsMasks.at(i).toObject());
    }

    clearFormulas();
    QJsonArray jsFormulas = reader->value("formulas").toArray();
    for (int i = 0; i < jsFormulas.size(); i++) {
        ReadFormula(jsFormulas.at(i).toObject());
    }

    if (rowCount() > 0)
        removeRows(0, rowCount());
    QJsonArray jsData = reader->value("data").toArray();
    for (int i = 0; i < jsData.size(); i++) {
        switch (dataType()) {
        case Makhber::TypeDouble: {
            setValueAt(i, jsData.at(i).toDouble());
            break;
        }
        case Makhber::TypeQString:
            setTextAt(i, jsData.at(i).toString());
            break;

        case Makhber::TypeQDateTime:
            QDateTime date_time =
                    QDateTime::fromString(jsData.at(i).toString(), "dd-MM-yyyy hh:mm:ss:zzz");
            setDateTimeAt(i, date_time);
            break;
        }
    }
    return true;
}

void Column::ReadNumericDateTimeFilter(QJsonObject *reader)
{
    QJsonObject jsObject = reader->value("numericDateTimeFilter").toObject();
    d_column_private->getNumericDateTimeFilter()->load(&jsObject);
}

void Column::ReadInputFilter(QJsonObject *reader)
{
    QJsonObject jsObject = reader->value("inputFilter").toObject();
    d_column_private->inputFilter()->load(&jsObject);
}

void Column::ReadOutputFilter(QJsonObject *reader)
{
    QJsonObject jsObject = reader->value("outputFilter").toObject();
    outputFilter()->load(&jsObject);
}

void Column::ReadMask(QJsonObject reader)
{
    int start = 0, end = 0;
    start = reader["start_row"].toInt();
    end = reader["end_row"].toInt();
    setMasked(Interval<int>(start, end));
}

void Column::ReadFormula(QJsonObject reader)
{
    int start = 0, end = 0;
    start = reader["start_row"].toInt();
    end = reader["end_row"].toInt();
    setFormula(Interval<int>(start, end), reader["formula"].toString());
}

Makhber::ColumnDataType Column::dataType() const
{
    return d_column_private->dataType();
}

Makhber::ColumnMode Column::columnMode() const
{
    return d_column_private->columnMode();
}
int Column::rowCount() const
{
    return d_column_private->rowCount();
}

Makhber::PlotDesignation Column::plotDesignation() const
{
    return d_column_private->plotDesignation();
}

AbstractSimpleFilter *Column::outputFilter() const
{
    return d_column_private->outputFilter();
}

AbstractSimpleFilter *Column::inputFilter() const
{
    return d_column_private->inputFilter();
}

NumericDateTimeBaseFilter *Column::numericDateTimeBaseFilter() const
{
    return d_column_private->getNumericDateTimeFilter();
}

bool Column::isInvalid(int row) const
{
    return d_column_private->isInvalid(row);
}

bool Column::isInvalid(Interval<int> i) const
{
    return d_column_private->isInvalid(i);
}

QList<Interval<int>> Column::invalidIntervals() const
{
    return d_column_private->invalidIntervals();
}

bool Column::isMasked(int row) const
{
    return d_column_private->isMasked(row);
}

bool Column::isMasked(Interval<int> i) const
{
    return d_column_private->isMasked(i);
}

QList<Interval<int>> Column::maskedIntervals() const
{
    return d_column_private->maskedIntervals();
}

QString Column::formula(int row) const
{
    return d_column_private->formula(row);
}

QList<Interval<int>> Column::formulaIntervals() const
{
    return d_column_private->formulaIntervals();
}

void Column::notifyDisplayChange()
{
    Q_EMIT dataChanged(this); // all cells must be repainted
    Q_EMIT aspectDescriptionChanged(this); // the icon for the type changed
}

QString ColumnStringIO::textAt(int row) const
{
    if (d_setting)
        return d_to_set;
    else
        return d_owner->d_column_private->outputFilter()->output(0)->textAt(row);
}

void ColumnStringIO::setTextAt(int row, const QString &value)
{
    d_setting = true;
    d_to_set = value;
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0), 0, row, 1);
    d_setting = false;
    d_to_set.clear();
}

bool ColumnStringIO::copy(const AbstractColumn *other)
{
    if (other->columnMode() != Makhber::ColumnMode::Text)
        return false;
    d_owner->d_column_private->inputFilter()->input(0, other);
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0));
    d_owner->d_column_private->inputFilter()->input(0, this);
    return true;
}

bool ColumnStringIO::copy(const AbstractColumn *source, int source_start, int dest_start,
                          int num_rows)
{
    if (source->columnMode() != Makhber::ColumnMode::Text)
        return false;
    d_owner->d_column_private->inputFilter()->input(0, source);
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0), source_start, dest_start,
                  num_rows);
    d_owner->d_column_private->inputFilter()->input(0, this);
    return true;
}

void ColumnStringIO::replaceTexts(int start_row, const QStringList &texts)
{
    Column tmp("tmp", texts);
    copy(&tmp, 0, start_row, texts.size());
}
