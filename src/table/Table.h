/***************************************************************************
    File                 : Table.h
    Project              : Makhber
    Description          : Table worksheet class
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
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
#ifndef TABLE_H
#define TABLE_H

#include "table/future_Table.h"
#include "table/TableView.h"
#include "core/MyWidget.h"
#include "core/globals.h"
#include "plot2D/Graph.h"
#include "scripting/ScriptingEnv.h"
#include "scripting/Script.h"

#include <QVarLengthArray>
#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QPointer>

/*!\brief MDI window providing a spreadsheet table with column logic.
 */
class MAKHBER_EXPORT Table : public TableView, public scripted
{
    Q_OBJECT

public:
    QPointer<future::Table> d_future_table;

    /*  enum ColType {
          Numeric = 0,
          Text = 1,
          Date = 2,
          Time = 3,
          Month = 4,
          Day = 5,
          DateTime = 6
      }; // TODO: remove this in favor of Makhber::columnMode*/

    Table(ScriptingEnv *env, const QString &fname, const QString &sep, int ignoredLines,
          bool renameCols, bool stripSpaces, bool simplifySpaces, bool convertToNumeric,
          QLocale numericFormat, const QString &label, QWidget *parent = 0, const char *name = 0,
          Qt::WindowFlags f = Qt::Widget);
    Table(ScriptingEnv *env, int r, int c, const QString &label, QWidget *parent = 0,
          const char *name = 0, Qt::WindowFlags f = Qt::Widget);

    //! Sets the number of significant digits
    void setNumericPrecision(int prec);

    //! Return the window name
    virtual QString name() { return d_future_table ? d_future_table->name() : QString(); }
    //! Set the window name
    virtual void setName(const QString &s)
    {
        if (d_future_table)
            d_future_table->setName(s);
    }
    //! Return the window label
    virtual QString windowLabel() { return d_future_table ? d_future_table->comment() : QString(); }
    //! Set the window label
    virtual void setWindowLabel(const QString &s)
    {
        if (d_future_table)
            d_future_table->setComment(s);
        updateCaption();
    }
    //! Set the caption policy
    void setCaptionPolicy(CaptionPolicy policy)
    {
        if (d_future_table) {
            caption_policy = policy;
            updateCaption();
            switch (policy) {
            case Name:
                d_future_table->setCaptionSpec("%n");
                break;
            case Label:
                d_future_table->setCaptionSpec("%c");
                break;
            case Both:
                d_future_table->setCaptionSpec("%n%C{ - }%c");
                break;
            }
        }
    }
    //! Set the creation date
    virtual void setBirthDate(const QString &s)
    {
        birthdate = s;
        if (d_future_table)
            d_future_table->importV0x0001XXCreationTime(s);
    }

    void closeEvent(QCloseEvent *);
public Q_SLOTS:
    void copy(Table *m);
    int numRows();
    int numCols();
    int rowCount();
    int columnCount();
    void setNumRows(int rows);
    void setNumCols(int cols);
    void handleChange();
    void handleRowChange();
    void handleColumnChange(int, int);
    void handleColumnChange(int, int, int, int);
    void handleColumnsAboutToBeRemoved(int, int);
    void handleColumnsRemoved(int, int);

    //! Return column number 'index'
    Column *column(int index) const
    {
        return d_future_table ? d_future_table->column(index) : nullptr;
    }
    //! Return the column determined by the given name
    /**
     * This method should not be used unless absolutely necessary.
     * Columns should be addressed by their index.
     * This method is mainly meant to be used in scripts.
     */
    Column *column(const QString &name) const
    {
        return d_future_table ? d_future_table->column(name) : nullptr;
    }

    //! Return the value of the cell as a double
    /**
     * If one of the indices or the cell content is invalid, return 0.0. For the next non-bugfix
     * Makhber release, indication of failure should be done by returning NaN (wherever
     * applicable).
     * TODO: Make sure we don't rely on zero-on-failure, and document the resulting change in the
     * Python API.
     */
    double cell(int row, int col);
    void setCell(int row, int col, double val);

    QString text(int row, int col);
    QStringList columnsList();
    QStringList colNames();
    QString colName(int col);
    QString colLabel(int col);
    int colIndex(const QString &name);

    Makhber::PlotDesignation colPlotDesignation(int col);
    void setColPlotDesignation(int col, Makhber::PlotDesignation d);
    void setPlotDesignation(Makhber::PlotDesignation pd);
    QList<int> plotDesignations();

    void setColName(int col, const QString &text);
    void setHeader(QStringList header);
    void importV0x0001XXHeader(QStringList header);
    void setText(int row, int col, const QString &text);

    void clearCell(int row, int col);

    void print();
    void print(const QString &fileName);
    void exportPDF(const QString &fileName);
    void customEvent(QEvent *e);

    //! \name Column Operations
    //@{
    void removeCol();
    void removeCol(const QStringList &list);
    void clearCol();
    void insertCol();
    void insertCols(int start, int count);
    void addCol(Makhber::PlotDesignation pd = Makhber::Y);
    void addColumns(int c);
    //@}

    int firstXCol();
    bool noXColumn();
    bool noYColumn();
    int colX(int col);
    int colY(int col);

    //! Set all column formulae.
    void setCommands(const QStringList &com);
    //! Set all column formulae.
    void setCommands(const QString &com);
    //! Set formula for column col.
    void setCommand(int col, const QString &com);
    //! Compute cells from the cell formulas
    bool recalculate(int col, bool only_selected_rows = true);
    //! Recalculate selected cells
    bool recalculate();

    //! \name Row Operations
    //@{
    void deleteSelectedRows();
    void insertRow();
    //@}

    //! Selection Operations
    //@{
    void cutSelection();
    void copySelection();
    void clearSelection();
    void pasteSelection();
    void selectAllTable();
    void deselect();
    void clear();
    //@}

    void init();
    QStringList selectedColumns();
    QStringList selectedYColumns();
    QStringList selectedErrColumns();
    QMap<int, QString> selectedYLabels();
    QStringList drawableColumnSelection();
    QStringList YColumns();
    int selectedColsNumber();

    int columnWidth(int col);
    void setColWidths(const QStringList &widths);

    int numSelectedRows();

    Makhber::ColumnMode columnType(int col);

    QList<Makhber::ColumnMode> columnTypes();
    void setColumnTypes(QList<Makhber::ColumnMode> ctl);
    void setColumnTypes(const QStringList &ctl);
    void setColumnType(int col, Makhber::ColumnMode mode);

    QString columnFormat(int col);

    void importASCII(const QString &fname, const QString &sep, int ignoredLines, bool renameCols,
                     bool stripSpaces, bool simplifySpaces, bool newTable);
    bool exportASCII(const QString &fname, const QString &separator, bool withLabels = false,
                     bool exportSelection = false);

    //! \name Saving and Restoring
    //@{
    void saveToJson(QJsonObject *jsObject, const QJsonObject &geometry);
    void saveHeader(QJsonObject *);
    void saveComments(QJsonObject *);
    void saveCommands(QJsonObject *);
    void saveColumnWidths(QJsonObject *);
    void saveColumnTypes(QJsonObject *);
    //@}

    void setBackgroundColor(const QColor &col);
    void setTextColor(const QColor &col);
    void setHeaderColor(const QColor &col);
    void setTextFont(const QFont &fnt);
    void setHeaderFont(const QFont &fnt);

    int verticalHeaderWidth();

    QString colComment(int col);
    void setColComment(int col, const QString &s);
    QStringList colComments();
    void setColComments(const QStringList &lst);
    bool commentsEnabled();

    void saveAsTemplate(QJsonObject *, const QJsonObject &);
    void restore(const QStringList &lst);

Q_SIGNALS:
    void changedColHeader(const QString &, const QString &);
    void aboutToRemoveCol(const QString &);
    void removedCol(const QString &);
    void modifiedData(Table *, const QString &);
    void resizedTable(QWidget *);
    void showContextMenu(bool selection);

protected Q_SLOTS:
    void applyFormula();
    void addFunction();
    void addReference();
    void updateFunctionDoc();
    void handleAspectDescriptionChange(const AbstractAspect *aspect);
    void handleAspectDescriptionAboutToChange(const AbstractAspect *aspect);

private:
    QHash<const AbstractAspect *, QString> d_stored_column_labels;
};

#endif
