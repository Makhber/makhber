/***************************************************************************
    File                 : tablecommands.h
    Project              : Makhber
    Description          : Commands used in Table (part of the undo/redo framework)
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
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

#ifndef TABLE_COMMANDS_H
#define TABLE_COMMANDS_H

#include "aspects/column/Column.h"
#include "aspects/column/ColumnPrivate.h"
#include "aspects/AbstractFilter.h"
#include "table/future_Table.h"
#include "lib/IntervalAttribute.h"

#include <QUndoCommand>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QItemSelection>

///////////////////////////////////////////////////////////////////////////
// class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////
//! Insert columns
/**
 * The number of inserted columns is cols.size().
 */
class MAKHBER_EXPORT TableInsertColumnsCmd : public QUndoCommand
{
public:
    TableInsertColumnsCmd(future::Table::Private &private_obj, int before, QList<Column *> cols,
                          QUndoCommand *parent = 0);

    void redo() override;
    void undo() override;

private:
    //! The private object to modify
    future::Table::Private &d_private_obj;
    //! Column to insert before
    int d_before;
    //! The new columns
    QList<Column *> d_cols;
    //! Row count before the command
    int d_rows_before {};
};

///////////////////////////////////////////////////////////////////////////
// end of class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////
//! Set the number of rows in the table
class MAKHBER_EXPORT TableSetNumberOfRowsCmd : public QUndoCommand
{
public:
    TableSetNumberOfRowsCmd(future::Table::Private &private_obj, int rows,
                            QUndoCommand *parent = 0);

    void redo() override;
    void undo() override;

private:
    //! The private object to modify
    future::Table::Private &d_private_obj;
    //! Number of rows
    int d_rows;
    //! Number of rows before
    int d_old_rows {};
};

///////////////////////////////////////////////////////////////////////////
// end of class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////
//! Remove columns
class MAKHBER_EXPORT TableRemoveColumnsCmd : public QUndoCommand
{
public:
    TableRemoveColumnsCmd(future::Table::Private &private_obj, int first, int count,
                          QList<Column *> cols, QUndoCommand *parent = 0);

    void redo() override;
    void undo() override;

private:
    //! The private object to modify
    future::Table::Private &d_private_obj;
    //! The first column
    int d_first;
    //! The number of columns to be removed
    int d_count;
    //! The removed columns
    QList<Column *> d_old_cols;
};

///////////////////////////////////////////////////////////////////////////
// end of class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////
//! Move a column
class MAKHBER_EXPORT TableMoveColumnCmd : public QUndoCommand
{
public:
    TableMoveColumnCmd(future::Table::Private &private_obj, int from, int to,
                       QUndoCommand *parent = 0);

    void redo() override;
    void undo() override;

private:
    //! The private object to modify
    future::Table::Private &d_private_obj;
    //! The old column index
    int d_from;
    //! The new column index
    int d_to;
};

///////////////////////////////////////////////////////////////////////////
// end of class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////

#endif // ifndef TABLE_COMMANDS_H
