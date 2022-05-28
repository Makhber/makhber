/***************************************************************************
    File                 : ErrDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Add error bars dialog

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
#ifndef ERRDIALOG_H
#define ERRDIALOG_H

#include "core/MyWidget.h"

#include <QDialog>
#include <QList>

class QLabel;
class QComboBox;
class QRadioButton;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QButtonGroup;
class QWidget;

//! Add error bars dialog
class MAKHBER_EXPORT ErrDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor
    /**
     * \param parent parent widget
     * \param fl window flags
     */
    ErrDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    //! Destructor
    ~ErrDialog();

private:
    QLabel *textLabel1;
    QComboBox *nameLabel, *tableNamesBox, *colNamesBox;
    QGroupBox *groupBox2 {};
    QGroupBox *groupBox1, *groupBox3;
    QButtonGroup *buttonGroup1, *buttonGroup2;
    QRadioButton *standardBox, *columnBox;
    QRadioButton *percentBox;
    QLineEdit *valueBox;
    QRadioButton *xErrBox;
    QRadioButton *yErrBox;
    QPushButton *buttonAdd;
    QPushButton *buttonCancel;
    QList<MyWidget *> *srcTables {};

protected Q_SLOTS:
    //! Set all string in the current language
    void languageChange();

public Q_SLOTS:
    //! Add a plot definition
    void add();
    //! Supply the dialog with a curves list
    void setCurveNames(const QStringList &names);
    //! Supply the dialog with a tables list
    void setSrcTables(QList<MyWidget *> *tables);
    //! Select a table
    void selectSrcTable(int tabnr);

Q_SIGNALS:
    //! This is usually connected to the main window's defineErrorBars() slot
    void options(const QString &curveName, int type, const QString &percent,
                 Qt::Orientation direction);
    //! This is usually connected to the main window's defineErrorBars() slot
    void options(const QString &curveName, const QString &errColumnName, Qt::Orientation direction);
};

#endif // ERRDIALOG_H
