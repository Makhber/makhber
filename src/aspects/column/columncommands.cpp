/***************************************************************************
    File                 : columncommands.cpp
    Project              : Makhber
    Description          : Commands to be called by Column to modify Column::Private
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

#include "columncommands.h"

#include "aspects/column/ColumnPrivate.h"

#include <utility>

///////////////////////////////////////////////////////////////////////////
// class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetModeCmd::ColumnSetModeCmd(Column::Private *col, Makhber::ColumnMode mode,
                                   AbstractFilter *conversion_filter, QUndoCommand *parent)
    : QUndoCommand(parent),
      d_col(col),
      d_old_mode(mode),
      d_mode(mode),
      d_old_type(Makhber::ColumnDataType::TypeDouble),
      d_new_type(Makhber::ColumnDataType::TypeDouble),
      d_conversion_filter(conversion_filter)
{
    setText(QObject::tr("%1: change column type").arg(col->name()));
    d_undone = false;
    d_executed = false;
}

ColumnSetModeCmd::~ColumnSetModeCmd()
{
    if (d_undone) {
        if (d_new_data != d_old_data) {
            if (d_new_type == Makhber::TypeDouble)
                delete static_cast<QVector<double> *>(d_new_data);
            else if (d_new_type == Makhber::TypeQString)
                delete static_cast<QStringList *>(d_new_data);
            else if (d_new_type == Makhber::TypeQDateTime)
                delete static_cast<QList<QDateTime> *>(d_new_data);
        }
    } else {
        if (d_new_data != d_old_data) {
            if (d_old_type == Makhber::TypeDouble)
                delete static_cast<QVector<double> *>(d_old_data);
            else if (d_old_type == Makhber::TypeQString)
                delete static_cast<QStringList *>(d_old_data);
            else if (d_old_type == Makhber::TypeQDateTime)
                delete static_cast<QList<QDateTime> *>(d_old_data);
        }
    }
    if (d_conversion_filter)
        delete d_conversion_filter;
}

void ColumnSetModeCmd::redo()
{
    if (!d_executed) {
        // save old values
        d_old_mode = d_col->columnMode();
        d_old_type = d_col->dataType();
        d_old_data = d_col->dataPointer();
        d_old_in_filter = d_col->inputFilter();
        d_old_out_filter = d_col->outputFilter();
        d_old_validity = d_col->validityAttribute();

        // do the conversion
        d_col->setColumnMode(d_mode, d_conversion_filter);

        // save new values
        d_new_type = d_col->dataType();
        d_new_data = d_col->dataPointer();
        d_new_in_filter = d_col->inputFilter();
        d_new_out_filter = d_col->outputFilter();
        d_new_validity = d_col->validityAttribute();
        d_executed = true;
    } else {
        // set to saved new values
        d_col->replaceModeData(d_mode, d_new_type, d_new_data, d_new_in_filter, d_new_out_filter,
                               d_new_validity);
    }
    d_undone = false;
}

void ColumnSetModeCmd::undo()
{
    // reset to old values
    d_col->replaceModeData(d_old_mode, d_old_type, d_old_data, d_old_in_filter, d_old_out_filter,
                           d_old_validity);

    d_undone = true;
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////
ColumnFullCopyCmd::ColumnFullCopyCmd(Column::Private *col, const AbstractColumn *src,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_src(src), d_backup(nullptr), d_backup_owner(nullptr)
{
    setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

ColumnFullCopyCmd::~ColumnFullCopyCmd()
{
    delete d_backup;
    delete d_backup_owner;
}

void ColumnFullCopyCmd::redo()
{
    if (d_backup == nullptr) {
        d_backup_owner = new Column("temp", d_src->columnMode());
        d_backup = new Column::Private(d_backup_owner, d_src->columnMode());
        d_backup->copy(d_col);
        d_col->copy(d_src);
    } else {
        // swap data + validity of orig. column and backup
        IntervalAttribute<bool> val_temp = IntervalAttribute<bool>(d_col->invalidIntervals());
        void *data_temp = d_col->dataPointer();
        d_col->replaceData(d_backup->dataPointer(), d_backup->validityAttribute());
        d_backup->replaceData(data_temp, val_temp);
    }
}

void ColumnFullCopyCmd::undo()
{
    // swap data + validity of orig. column and backup
    IntervalAttribute<bool> val_temp = d_col->validityAttribute();
    void *data_temp = d_col->dataPointer();
    d_col->replaceData(d_backup->dataPointer(), d_backup->validityAttribute());
    d_backup->replaceData(data_temp, val_temp);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////
ColumnPartialCopyCmd::ColumnPartialCopyCmd(Column::Private *col, const AbstractColumn *src,
                                           int src_start, int dest_start, int num_rows,
                                           QUndoCommand *parent)
    : QUndoCommand(parent),
      d_col(col),
      d_src(src),
      d_col_backup(nullptr),
      d_src_backup(nullptr),
      d_col_backup_owner(nullptr),
      d_src_backup_owner(nullptr),
      d_src_start(src_start),
      d_dest_start(dest_start),
      d_num_rows(num_rows)
{
    setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

ColumnPartialCopyCmd::~ColumnPartialCopyCmd()
{
    delete d_src_backup;
    delete d_col_backup;
    delete d_src_backup_owner;
    delete d_col_backup_owner;
}

void ColumnPartialCopyCmd::redo()
{
    if (d_src_backup == nullptr) {
        // copy the relevant rows of source and destination column into backup columns
        d_src_backup_owner = new Column("temp", d_col->columnMode());
        d_src_backup = new Column::Private(d_src_backup_owner, d_col->columnMode());
        d_src_backup->copy(d_src, d_src_start, 0, d_num_rows);
        d_col_backup_owner = new Column("temp", d_col->columnMode());
        d_col_backup = new Column::Private(d_col_backup_owner, d_col->columnMode());
        d_col_backup->copy(d_col, d_dest_start, 0, d_num_rows);
        d_old_row_count = d_col->rowCount();
        d_old_validity = d_col->validityAttribute();
    }
    d_col->copy(d_src_backup, 0, d_dest_start, d_num_rows);
}

void ColumnPartialCopyCmd::undo()
{
    d_col->copy(d_col_backup, 0, d_dest_start, d_num_rows);
    d_col->resizeTo(d_old_row_count);
    d_col->replaceData(d_col->dataPointer(), d_old_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////
ColumnInsertEmptyRowsCmd::ColumnInsertEmptyRowsCmd(Column::Private *col, int before, int count,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_before(before), d_count(count)
{
    setText(QObject::tr("%1: insert %2 row(s)").arg(col->name()).arg(count));
}

ColumnInsertEmptyRowsCmd::~ColumnInsertEmptyRowsCmd() = default;

void ColumnInsertEmptyRowsCmd::redo()
{
    d_col->insertRows(d_before, d_count);
}

void ColumnInsertEmptyRowsCmd::undo()
{
    d_col->removeRows(d_before, d_count);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////
ColumnRemoveRowsCmd::ColumnRemoveRowsCmd(Column::Private *col, int first, int count,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_first(first), d_count(count), d_backup(nullptr)
{
    setText(QObject::tr("%1: remove %2 row(s)").arg(col->name()).arg(count));
}

ColumnRemoveRowsCmd::~ColumnRemoveRowsCmd()
{
    delete d_backup;
    delete d_backup_owner;
}

void ColumnRemoveRowsCmd::redo()
{
    if (d_backup == nullptr) {
        if (d_first >= d_col->rowCount())
            d_data_row_count = 0;
        else if (d_first + d_count > d_col->rowCount())
            d_data_row_count = d_col->rowCount() - d_first;
        else
            d_data_row_count = d_count;

        d_old_size = d_col->rowCount();
        d_backup_owner = new Column("temp", d_col->columnMode());
        d_backup = new Column::Private(d_backup_owner, d_col->columnMode());
        d_backup->copy(d_col, d_first, 0, d_data_row_count);
        d_masking = d_col->maskingAttribute();
        d_formulas = d_col->formulaAttribute();
    }
    d_col->removeRows(d_first, d_count);
}

void ColumnRemoveRowsCmd::undo()
{
    d_col->insertRows(d_first, d_count);
    d_col->copy(d_backup, 0, d_first, d_data_row_count);
    d_col->resizeTo(d_old_size);
    d_col->replaceMasking(d_masking);
    d_col->replaceFormulas(d_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetPlotDesignationCmd::ColumnSetPlotDesignationCmd(Column::Private *col,
                                                         Makhber::PlotDesignation pd,
                                                         QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_new_pd(pd)
{
    setText(QObject::tr("%1: set plot designation").arg(col->name()));
}

ColumnSetPlotDesignationCmd::~ColumnSetPlotDesignationCmd() = default;

void ColumnSetPlotDesignationCmd::redo()
{
    d_old_pd = d_col->plotDesignation();
    d_col->setPlotDesignation(d_new_pd);
}

void ColumnSetPlotDesignationCmd::undo()
{
    d_col->setPlotDesignation(d_old_pd);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearCmd::ColumnClearCmd(Column::Private *col, QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col)
{
    setText(QObject::tr("%1: clear column").arg(col->name()));
    d_empty_data = nullptr;
    d_undone = false;
}

ColumnClearCmd::~ColumnClearCmd()
{
    if (d_undone) {
        if (d_type == Makhber::TypeDouble)
            delete static_cast<QVector<double> *>(d_empty_data);
        else if (d_type == Makhber::TypeQString)
            delete static_cast<QStringList *>(d_empty_data);
        else if (d_type == Makhber::TypeQDateTime)
            delete static_cast<QList<QDateTime> *>(d_empty_data);
    } else {
        if (d_type == Makhber::TypeDouble)
            delete static_cast<QVector<double> *>(d_data);
        else if (d_type == Makhber::TypeQString)
            delete static_cast<QStringList *>(d_data);
        else if (d_type == Makhber::TypeQDateTime)
            delete static_cast<QList<QDateTime> *>(d_data);
    }
}

void ColumnClearCmd::redo()
{
    if (!d_empty_data) {
        d_type = d_col->dataType();
        switch (d_type) {
        case Makhber::TypeDouble:
            d_empty_data = new QVector<double>();
            break;
        case Makhber::TypeQDateTime:
            d_empty_data = new QList<QDateTime>();
            break;
        case Makhber::TypeQString:
            d_empty_data = new QStringList();
            break;
        }
        d_data = d_col->dataPointer();
        d_validity = d_col->validityAttribute();
    }
    d_col->replaceData(d_empty_data, IntervalAttribute<bool>());
    d_undone = false;
}

void ColumnClearCmd::undo()
{
    d_col->replaceData(d_data, d_validity);
    d_undone = true;
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearValidityCmd::ColumnClearValidityCmd(Column::Private *col, QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col)
{
    setText(QObject::tr("%1: mark all cells valid").arg(col->name()));
    d_copied = false;
}

ColumnClearValidityCmd::~ColumnClearValidityCmd() = default;

void ColumnClearValidityCmd::redo()
{
    if (!d_copied) {
        d_validity = d_col->validityAttribute();
        d_copied = true;
    }
    d_col->clearValidity();
}

void ColumnClearValidityCmd::undo()
{
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearMasksCmd::ColumnClearMasksCmd(Column::Private *col, QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col)
{
    setText(QObject::tr("%1: clear masks").arg(col->name()));
    d_copied = false;
}

ColumnClearMasksCmd::~ColumnClearMasksCmd() = default;

void ColumnClearMasksCmd::redo()
{
    if (!d_copied) {
        d_masking = d_col->maskingAttribute();
        d_copied = true;
    }
    d_col->clearMasks();
}

void ColumnClearMasksCmd::undo()
{
    d_col->replaceMasking(d_masking);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetInvalidCmd::ColumnSetInvalidCmd(Column::Private *col, Interval<int> interval, bool invalid,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_interval(std::move(interval)), d_invalid(invalid)
{
    if (invalid)
        setText(QObject::tr("%1: mark cells invalid").arg(col->name()));
    else
        setText(QObject::tr("%1: mark cells valid").arg(col->name()));
    d_copied = false;
}

ColumnSetInvalidCmd::~ColumnSetInvalidCmd() = default;

void ColumnSetInvalidCmd::redo()
{
    if (!d_copied) {
        d_validity = d_col->validityAttribute();
        d_copied = true;
    }
    d_col->setInvalid(d_interval, d_invalid);
}

void ColumnSetInvalidCmd::undo()
{
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetMaskedCmd::ColumnSetMaskedCmd(Column::Private *col, Interval<int> interval, bool masked,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_interval(std::move(interval)), d_masked(masked)
{
    if (masked)
        setText(QObject::tr("%1: mask cells").arg(col->name()));
    else
        setText(QObject::tr("%1: unmask cells").arg(col->name()));
    d_copied = false;
}

ColumnSetMaskedCmd::~ColumnSetMaskedCmd() = default;

void ColumnSetMaskedCmd::redo()
{
    if (!d_copied) {
        d_masking = d_col->maskingAttribute();
        d_copied = true;
    }
    d_col->setMasked(d_interval, d_masked);
}

void ColumnSetMaskedCmd::undo()
{
    d_col->replaceMasking(d_masking);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetFormulaCmd::ColumnSetFormulaCmd(Column::Private *col, Interval<int> interval,
                                         QString formula, QUndoCommand *parent)
    : QUndoCommand(parent),
      d_col(col),
      d_interval(std::move(interval)),
      d_formula(std::move(formula))
{
    setText(QObject::tr("%1: set cell formula").arg(col->name()));
    d_copied = false;
}

ColumnSetFormulaCmd::~ColumnSetFormulaCmd() = default;

void ColumnSetFormulaCmd::redo()
{
    if (!d_copied) {
        d_formulas = d_col->formulaAttribute();
        d_copied = true;
    }
    d_col->setFormula(d_interval, d_formula);
}

void ColumnSetFormulaCmd::undo()
{
    d_col->replaceFormulas(d_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearFormulasCmd::ColumnClearFormulasCmd(Column::Private *col, QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col)
{
    setText(QObject::tr("%1: clear all formulas").arg(col->name()));
    d_copied = false;
}

ColumnClearFormulasCmd::~ColumnClearFormulasCmd() = default;

void ColumnClearFormulasCmd::redo()
{
    if (!d_copied) {
        d_formulas = d_col->formulaAttribute();
        d_copied = true;
    }
    d_col->clearFormulas();
}

void ColumnClearFormulasCmd::undo()
{
    d_col->replaceFormulas(d_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetTextCmd::ColumnSetTextCmd(Column::Private *col, int row, QString new_value,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_row(row), d_new_value(std::move(new_value))
{
    setText(QObject::tr("%1: set text for row %2").arg(col->name()).arg(row));
}

ColumnSetTextCmd::~ColumnSetTextCmd() = default;

void ColumnSetTextCmd::redo()
{
    d_old_value = d_col->textAt(d_row);
    d_row_count = d_col->rowCount();
    d_validity = d_col->validityAttribute();
    d_col->setTextAt(d_row, d_new_value);
}

void ColumnSetTextCmd::undo()
{
    d_col->setTextAt(d_row, d_old_value);
    d_col->resizeTo(d_row_count);
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetValueCmd::ColumnSetValueCmd(Column::Private *col, int row, double new_value,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_row(row), d_new_value(new_value)
{
    setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

ColumnSetValueCmd::~ColumnSetValueCmd() = default;

void ColumnSetValueCmd::redo()
{
    d_old_value = d_col->valueAt(d_row);
    d_row_count = d_col->rowCount();
    d_validity = d_col->validityAttribute();
    d_col->setValueAt(d_row, d_new_value);
}

void ColumnSetValueCmd::undo()
{
    d_col->setValueAt(d_row, d_old_value);
    d_col->resizeTo(d_row_count);
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetDateTimeCmd::ColumnSetDateTimeCmd(Column::Private *col, int row, QDateTime new_value,
                                           QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_row(row), d_new_value(std::move(new_value))
{
    setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

ColumnSetDateTimeCmd::~ColumnSetDateTimeCmd() = default;

void ColumnSetDateTimeCmd::redo()
{
    d_old_value = d_col->dateTimeAt(d_row);
    d_row_count = d_col->rowCount();
    d_validity = d_col->validityAttribute();
    d_col->setDateTimeAt(d_row, d_new_value);
}

void ColumnSetDateTimeCmd::undo()
{
    d_col->setDateTimeAt(d_row, d_old_value);
    d_col->resizeTo(d_row_count);
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceTextsCmd::ColumnReplaceTextsCmd(Column::Private *col, int first,
                                             const QStringList &new_values, QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_first(first), d_new_values(new_values)
{
    setText(QObject::tr("%1: replace the texts for rows %2 to %3")
                    .arg(col->name())
                    .arg(first)
                    .arg(first + new_values.count() - 1));
    d_copied = false;
}

ColumnReplaceTextsCmd::~ColumnReplaceTextsCmd() = default;

void ColumnReplaceTextsCmd::redo()
{
    if (!d_copied) {
        d_old_values = static_cast<QStringList *>(d_col->dataPointer())
                               ->mid(d_first, d_new_values.count());
        d_row_count = d_col->rowCount();
        d_validity = d_col->validityAttribute();
        d_copied = true;
    }
    d_col->replaceTexts(d_first, d_new_values);
}

void ColumnReplaceTextsCmd::undo()
{
    d_col->replaceTexts(d_first, d_old_values);
    d_col->resizeTo(d_row_count);
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceValuesCmd::ColumnReplaceValuesCmd(Column::Private *col, int first,
                                               const QVector<qreal> &new_values,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_first(first), d_new_values(new_values)
{
    setText(QObject::tr("%1: replace the values for rows %2 to %3")
                    .arg(col->name())
                    .arg(first)
                    .arg(first + new_values.count() - 1));
    d_copied = false;
}

ColumnReplaceValuesCmd::~ColumnReplaceValuesCmd() = default;

void ColumnReplaceValuesCmd::redo()
{
    if (!d_copied) {
        d_old_values = static_cast<QVector<qreal> *>(d_col->dataPointer())
                               ->mid(d_first, d_new_values.count());
        d_row_count = d_col->rowCount();
        d_validity = d_col->validityAttribute();
        d_copied = true;
    }
    d_col->replaceValues(d_first, d_new_values);
}

void ColumnReplaceValuesCmd::undo()
{
    d_col->replaceValues(d_first, d_old_values);
    d_col->resizeTo(d_row_count);
    d_col->replaceData(d_col->dataPointer(), d_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceDateTimesCmd::ColumnReplaceDateTimesCmd(Column::Private *col, int first,
                                                     const QList<QDateTime> &new_values,
                                                     QUndoCommand *parent)
    : QUndoCommand(parent), d_col(col), d_first(first), d_new_values(new_values)
{
    setText(QObject::tr("%1: replace the values for rows %2 to %3")
                    .arg(col->name())
                    .arg(first)
                    .arg(first + new_values.count() - 1));
    d_copied = false;
}

ColumnReplaceDateTimesCmd::~ColumnReplaceDateTimesCmd() = default;

void ColumnReplaceDateTimesCmd::redo()
{
    if (!d_copied) {
        d_old_values = static_cast<QList<QDateTime> *>(d_col->dataPointer())
                               ->mid(d_first, d_new_values.count());
        d_row_count = d_col->rowCount();
        d_validity = d_col->validityAttribute();
        d_copied = true;
    }
    d_col->replaceDateTimes(d_first, d_new_values);
}

void ColumnReplaceDateTimesCmd::undo()
{
    d_col->replaceDateTimes(d_first, d_old_values);
    d_col->replaceData(d_col->dataPointer(), d_validity);
    d_col->resizeTo(d_row_count);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////
