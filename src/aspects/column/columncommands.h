/***************************************************************************
    File                 : columncommands.h
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

#ifndef COLUMNCOMMANDS_H
#define COLUMNCOMMANDS_H

#include "aspects/column/Column.h"
#include "aspects/AbstractSimpleFilter.h"
#include "lib/IntervalAttribute.h"

#include <QUndoCommand>
#include <QStringList>

///////////////////////////////////////////////////////////////////////////
// class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////
//! Set the column mode
class MAKHBER_EXPORT ColumnSetModeCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetModeCmd(Column::Private *col, Makhber::ColumnMode mode,
                     AbstractFilter *conversion_filter, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetModeCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The previous mode
    Makhber::ColumnMode d_old_mode;
    //! The new mode
    Makhber::ColumnMode d_mode;
    //! The old data type
    Makhber::ColumnDataType d_old_type;
    //! The new data type
    Makhber::ColumnDataType d_new_type;
    //! Pointer to old data
    void *d_old_data {};
    //! Pointer to new data
    void *d_new_data {};
    //! The new input filter
    AbstractSimpleFilter *d_new_in_filter {};
    //! The new output filter
    AbstractSimpleFilter *d_new_out_filter {};
    //! The old input filter
    AbstractSimpleFilter *d_old_in_filter {};
    //! The old output filter
    AbstractSimpleFilter *d_old_out_filter {};
    //! The old validity information
    IntervalAttribute<bool> d_old_validity;
    //! The new validity information
    IntervalAttribute<bool> d_new_validity;
    //! A status flag
    bool d_undone;
    //! A status flag
    bool d_executed;
    //! Filter to use for converting existing data
    AbstractFilter *d_conversion_filter;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////
//! Copy a complete column
class MAKHBER_EXPORT ColumnFullCopyCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnFullCopyCmd(Column::Private *col, const AbstractColumn *src, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnFullCopyCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The column to copy
    const AbstractColumn *d_src;
    //! A backup column
    Column::Private *d_backup;
    //! A dummy owner for the backup column
    /**
     * This is needed because a Column::Private must have an owner. We want access
     * to the Column::Private object to access its data pointer for fast data
     * replacement without too much copying.
     */
    Column *d_backup_owner;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////
//! Copy parts of a column
class MAKHBER_EXPORT ColumnPartialCopyCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnPartialCopyCmd(Column::Private *col, const AbstractColumn *src, int src_start,
                         int dest_start, int num_rows, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnPartialCopyCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The column to copy
    const AbstractColumn *d_src;
    //! A backup of the orig. column
    Column::Private *d_col_backup;
    //! A backup of the source column
    Column::Private *d_src_backup;
    //! A dummy owner for the backup column
    /**
     * This is needed because a Column::Private must have an owner and
     * we must have a Column::Private object as backup.
     * Using a Column object as backup would lead to an inifinite loop.
     */
    Column *d_col_backup_owner;
    //! A dummy owner for the source backup column
    /**
     * This is needed because a Column::Private must have an owner and
     * we must have a Column::Private object as backup.
     * Using a Column object as backup would lead to an inifinite loop.
     */
    Column *d_src_backup_owner;
    //! Start index in source column
    int d_src_start;
    //! Start index in destination column
    int d_dest_start;
    //! Number of rows to copy
    int d_num_rows;
    //! Previous number of rows in the destination column
    int d_old_row_count {};
    //! The old validity information
    IntervalAttribute<bool> d_old_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////
//! Insert empty rows
class MAKHBER_EXPORT ColumnInsertEmptyRowsCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnInsertEmptyRowsCmd(Column::Private *col, int before, int count, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnInsertEmptyRowsCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! Row to insert before
    int d_before;
    //! Number of rows
    int d_count;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////
//!
class MAKHBER_EXPORT ColumnRemoveRowsCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnRemoveRowsCmd(Column::Private *col, int first, int count, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnRemoveRowsCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The first row
    int d_first;
    //! The number of rows to be removed
    int d_count;
    //! Number of removed rows actually containing data
    int d_data_row_count {};
    //! The number of rows before the removal
    int d_old_size {};
    //! Column saving the removed rows
    Column::Private *d_backup;
    //! A dummy owner for the backup column
    /**
     * This is needed because a Column::Private must have an owner. We want access
     * to the Column::Private object to access its data pointer for fast data
     * replacement without too much copying.
     */
    Column *d_backup_owner {};
    //! Backup of the masking attribute
    IntervalAttribute<bool> d_masking;
    //! Backup of the formula attribute
    IntervalAttribute<QString> d_formulas;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////
//! Sets a column's plot designation
class MAKHBER_EXPORT ColumnSetPlotDesignationCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetPlotDesignationCmd(Column::Private *col, Makhber::PlotDesignation pd,
                                QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetPlotDesignationCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! New plot designation
    Makhber::PlotDesignation d_new_pd;
    //! Old plot designation
    Makhber::PlotDesignation d_old_pd;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////
//! Clear the column
class MAKHBER_EXPORT ColumnClearCmd : public QUndoCommand
{
public:
    //! Ctor
    explicit ColumnClearCmd(Column::Private *col, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnClearCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The column's data type
    Makhber::ColumnDataType d_type;
    //! Pointer to the old data pointer
    void *d_data {};
    //! Pointer to an empty data vector
    void *d_empty_data;
    //! The old validity
    IntervalAttribute<bool> d_validity;
    //! Status flag
    bool d_undone;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////
//! Clear validity information
class MAKHBER_EXPORT ColumnClearValidityCmd : public QUndoCommand
{
public:
    //! Ctor
    explicit ColumnClearValidityCmd(Column::Private *col, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnClearValidityCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The old validity
    IntervalAttribute<bool> d_validity;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////
//! Clear validity information
class MAKHBER_EXPORT ColumnClearMasksCmd : public QUndoCommand
{
public:
    //! Ctor
    explicit ColumnClearMasksCmd(Column::Private *col, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnClearMasksCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The old masks
    IntervalAttribute<bool> d_masking;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////
//! Mark an interval of rows as invalid
class MAKHBER_EXPORT ColumnSetInvalidCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetInvalidCmd(Column::Private *col, Interval<int> interval, bool invalid,
                        QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetInvalidCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The interval
    Interval<int> d_interval;
    //! Valid/invalid flag
    bool d_invalid;
    //! Interval attribute backup
    IntervalAttribute<bool> d_validity;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////
//! Mark an interval of rows as masked
class MAKHBER_EXPORT ColumnSetMaskedCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetMaskedCmd(Column::Private *col, Interval<int> interval, bool masked,
                       QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetMaskedCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The interval
    Interval<int> d_interval;
    //! Mask/unmask flag
    bool d_masked;
    //! Interval attribute backup
    IntervalAttribute<bool> d_masking;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////
//! Set the formula for a given interval
class MAKHBER_EXPORT ColumnSetFormulaCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetFormulaCmd(Column::Private *col, Interval<int> interval, QString formula,
                        QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetFormulaCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The interval
    Interval<int> d_interval;
    //! The new formula
    QString d_formula;
    //! Interval attribute backup
    IntervalAttribute<QString> d_formulas;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////
//! Clear all associated formulas
class MAKHBER_EXPORT ColumnClearFormulasCmd : public QUndoCommand
{
public:
    //! Ctor
    explicit ColumnClearFormulasCmd(Column::Private *col, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnClearFormulasCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The old formulas
    IntervalAttribute<QString> d_formulas;
    //! A status flag
    bool d_copied;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////
//! Set the text for a string cell
class MAKHBER_EXPORT ColumnSetTextCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetTextCmd(Column::Private *col, int row, QString new_value, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetTextCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The row to modify
    int d_row;
    //! The new value
    QString d_new_value;
    //! The old value
    QString d_old_value;
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////
//! Set the value for a double cell
class MAKHBER_EXPORT ColumnSetValueCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetValueCmd(Column::Private *col, int row, double new_value, QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetValueCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The row to modify
    int d_row;
    //! The new value
    double d_new_value;
    //! The old value
    double d_old_value {};
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////
//! Set the value of a date-time cell
class MAKHBER_EXPORT ColumnSetDateTimeCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnSetDateTimeCmd(Column::Private *col, int row, QDateTime new_value,
                         QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnSetDateTimeCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The row to modify
    int d_row;
    //! The new value
    QDateTime d_new_value;
    //! The old value
    QDateTime d_old_value;
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////
//! Replace a range of strings in a string column
class MAKHBER_EXPORT ColumnReplaceTextsCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnReplaceTextsCmd(Column::Private *col, int first, const QStringList &new_values,
                          QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnReplaceTextsCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The first row to replace
    int d_first;
    //! The new values
    QStringList d_new_values;
    //! The old values
    QStringList d_old_values;
    //! Status flag
    bool d_copied;
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////
//! Replace a range of doubles in a double column
class MAKHBER_EXPORT ColumnReplaceValuesCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnReplaceValuesCmd(Column::Private *col, int first, const QVector<qreal> &new_values,
                           QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnReplaceValuesCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The first row to replace
    int d_first;
    //! The new values
    QVector<qreal> d_new_values;
    //! The old values
    QVector<qreal> d_old_values;
    //! Status flag
    bool d_copied;
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////
//! Replace a range of date-times in a date-time column
class MAKHBER_EXPORT ColumnReplaceDateTimesCmd : public QUndoCommand
{
public:
    //! Ctor
    ColumnReplaceDateTimesCmd(Column::Private *col, int first, const QList<QDateTime> &new_values,
                              QUndoCommand *parent = 0);
    //! Dtor
    ~ColumnReplaceDateTimesCmd();

    //! Execute the command
    virtual void redo();
    //! Undo the command
    virtual void undo();

private:
    //! The private column data to modify
    Column::Private *d_col;
    //! The first row to replace
    int d_first;
    //! The new values
    QList<QDateTime> d_new_values;
    //! The old values
    QList<QDateTime> d_old_values;
    //! Status flag
    bool d_copied;
    //! The old number of rows
    int d_row_count {};
    //! The old validity
    IntervalAttribute<bool> d_validity;
};
///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////

#endif
