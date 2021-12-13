/***************************************************************************
    File                 : ExportDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Export ASCII dialog

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
#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>

class QPushButton;
class QCheckBox;
class QComboBox;

//! Export ASCII dialog
class MAKHBER_EXPORT ExportDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor
    /**
     * \param parent parent widget
     * \param fl window flags
     */
    ExportDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    //! Destructor
    ~ExportDialog();

private:
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QPushButton *buttonHelp;
    QCheckBox *boxNames;
    QCheckBox *boxSelection;
    QCheckBox *boxAllTables;
    QComboBox *boxSeparator;
    QComboBox *boxTable;

public Q_SLOTS:
    //! Set the column delimiter
    void setColumnSeparator(const QString &sep);
    //! Set the list of tables
    void setTableNames(const QStringList &names);
    //! Select a table
    void setActiveTableName(const QString &name);

private Q_SLOTS:
    //! Enable/disable the tables combox box
    /**
     * The tables combo box is disabled when
     * the checkbox "all" is selected.
     */
    void enableTableName(bool ok);

protected Q_SLOTS:
    //! Accept changes
    void accept();
    //! Display help
    void help();

Q_SIGNALS:
    //! Export one table
    /**
     * \param tableName name of the table to export
     * \param separator separator to be put between the columns
     * \param exportColumnNames flag: column names in the first line or not
     * \param exportSelection flag: export only selection or all cells
     */
    void exportTable(const QString &tableName, const QString &separator, bool exportColumnNames,
                     bool exportSelection);
    //! Export all tables
    /**
     * \param separator separator to be put between the columns
     * \param exportColumnNames flag: column names in the first line or not
     * \param exportSelection flag: export only selection or all cells
     */
    void exportAllTables(const QString &separator, bool exportColumnNames, bool exportSelection);
};

#endif // ExportDialog_H
