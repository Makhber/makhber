/***************************************************************************
    File                 : Table.cpp
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
#include "Table.h"

#include "aspects/column/Column.h"
#include "aspects/datatypes/Double2StringFilter.h"
#include "aspects/datatypes/String2DoubleFilter.h"
#include "aspects/datatypes/DateTime2StringFilter.h"
#include "lib/Interval.h"
#include "table/TableModel.h"
#include "table/AsciiTableImportFilter.h"
#include "scripting/ScriptEdit.h"

#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QClipboard>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QLayout>
#include <QPrintDialog>
#include <QLocale>
#include <QShortcut>
#include <QProgressDialog>
#include <QFile>
#include <QTemporaryFile>

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

Table::Table(ScriptingEnv *env, const QString &fname, const QString &sep, int ignoredLines,
             bool renameCols, bool stripSpaces, bool simplifySpaces, bool convertToNumeric,
             QLocale numericLocale, const QString &label, QWidget *parent, const char *name,
             Qt::WindowFlags f)
    : TableView(label, parent, name, f), scripted(env)
{

    AsciiTableImportFilter filter;
    filter.set_ignored_lines(ignoredLines);
    filter.set_separator(sep);
    filter.set_first_row_names_columns(renameCols);
    filter.set_trim_whitespace(stripSpaces);
    filter.set_simplify_whitespace(simplifySpaces);
    filter.set_convert_to_numeric(convertToNumeric);
    filter.set_numeric_locale(numericLocale);

    QFile file(fname);
    if (file.open(QIODevice::ReadOnly)) {
        d_future_table = dynamic_cast<future::Table *>(filter.importAspect(file));
        if (!d_future_table)
            d_future_table = new future::Table(0, 0, label);
        else
            d_future_table->setName(label);
    }
    setWindowLabel(fname);
    init();
}

Table::Table(ScriptingEnv *env, int r, int c, const QString &label, QWidget *parent,
             const char *name, Qt::WindowFlags f)
    : TableView(label, parent, name, f), scripted(env)
{
    d_future_table = new future::Table(r, c, label);
    init();
}

void Table::init()
{
    if (d_future_table) {
        TableView::setTable(d_future_table);
        birthdate = QLocale().toString(d_future_table->creationTime());
    } else // the rest is meaningless
        return;

    ui.gridLayout1->removeWidget(ui.formula_box);
    delete ui.formula_box;
    ui.formula_box = new ScriptEdit(scriptEnv, ui.formula_tab);
    ui.formula_box->setObjectName(QString::fromUtf8("formula_box"));
    ui.formula_box->setMinimumSize(QSize(60, 10));
    ui.formula_box->setAcceptRichText(false);
    ui.formula_box->setLineWrapMode(QTextEdit::WidgetWidth);
    ui.gridLayout1->addWidget(ui.formula_box, 1, 0, 1, 3);

    for (int i = 0; i < columnCount(); i++)
        ui.add_reference_combobox->addItem("col(\"" + column(i)->name() + "\")");

    ui.add_function_combobox->addItems(scriptEnv->mathFunctions());
    updateFunctionDoc();

    connect(ui.add_function_combobox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateFunctionDoc()));
    connect(ui.set_formula_button, SIGNAL(pressed()), this, SLOT(applyFormula()));
    connect(ui.add_function_button, SIGNAL(pressed()), this, SLOT(addFunction()));
    connect(ui.add_reference_button, SIGNAL(pressed()), this, SLOT(addReference()));

    connect(d_future_table, SIGNAL(columnsAboutToBeRemoved(int, int)), this,
            SLOT(handleColumnsAboutToBeRemoved(int, int)));
    connect(d_future_table, SIGNAL(columnsRemoved(int, int)), this,
            SLOT(handleColumnsRemoved(int, int)));
    connect(d_future_table, SIGNAL(rowsInserted(int, int)), this, SLOT(handleRowChange()));
    connect(d_future_table, SIGNAL(rowsRemoved(int, int)), this, SLOT(handleRowChange()));
    connect(d_future_table, SIGNAL(dataChanged(int, int, int, int)), this,
            SLOT(handleColumnChange(int, int, int, int)));
    connect(d_future_table, SIGNAL(columnsReplaced(int, int)), this,
            SLOT(handleColumnChange(int, int)));

    connect(d_future_table, SIGNAL(columnsInserted(int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(columnsReplaced(int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(columnsRemoved(int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(rowsInserted(int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(rowsRemoved(int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(dataChanged(int, int, int, int)), this, SLOT(handleChange()));
    connect(d_future_table, SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this,
            SLOT(handleChange()));
    connect(d_future_table, SIGNAL(recalculate()), this, SLOT(recalculate()));

    connect(d_future_table, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), this,
            SLOT(handleAspectDescriptionChange(const AbstractAspect *)));
    connect(d_future_table, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)), this,
            SLOT(handleAspectDescriptionAboutToChange(const AbstractAspect *)));
}

void Table::handleChange()
{
    Q_EMIT modifiedWindow(this);
}

void Table::handleColumnChange(int first, int count)
{
    for (int i = first; i < first + count; i++)
        Q_EMIT modifiedData(this, colName(i));
}

void Table::handleColumnChange(int top, int left, int bottom, int right)
{
    Q_UNUSED(top);
    Q_UNUSED(bottom);
    handleColumnChange(left, right - left + 1);
}

void Table::handleColumnsAboutToBeRemoved(int first, int count)
{
    for (int i = first; i < first + count; i++)
        Q_EMIT aboutToRemoveCol(colName(i));
}

void Table::handleColumnsRemoved(int first, int count)
{
    for (int i = first; i < first + count; i++)
        Q_EMIT removedCol(colName(i));
}

void Table::handleRowChange()
{
    for (int i = 0; i < numCols(); i++)
        Q_EMIT modifiedData(this, colName(i));
}

void Table::setBackgroundColor(const QColor &col)
{
    QPalette palette;
    palette.setColor(QPalette::Window, col);
    d_view_widget->setPalette(palette);
}

void Table::setTextColor(const QColor &col)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, col);
    d_view_widget->setPalette(palette);
}

void Table::setTextFont(const QFont &fnt)
{
    d_view_widget->setFont(fnt);
}

void Table::setHeaderColor(const QColor &col)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, col);
    d_view_widget->horizontalHeader()->setPalette(palette);
}

void Table::setHeaderFont(const QFont &fnt)
{
    d_view_widget->horizontalHeader()->setFont(fnt);
}

void Table::exportPDF(const QString &fileName)
{
    print(fileName);
}

void Table::print()
{
    print(QString());
}

void Table::print(const QString &fileName)
{
    QPrinter printer;
    printer.setColorMode(QPrinter::GrayScale);

    if (!fileName.isEmpty()) {
        printer.setCreator("Makhber");
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
    } else {
        QPrintDialog printDialog(&printer);
        if (printDialog.exec() != QDialog::Accepted)
            return;
    }

    printer.setFullPage(true);
    QPainter p;
    if (!p.begin(&printer))
        return; // paint on printer
    int dpiy = printer.logicalDpiY();
    const int margin = (int)((1 / 2.54) * dpiy); // 1 cm margins

    QHeaderView *hHeader = d_view_widget->horizontalHeader();
    QHeaderView *vHeader = d_view_widget->verticalHeader();

    int rows = numRows();
    int cols = numCols();
    int height = margin;
    int i = 0, vertHeaderWidth = vHeader->width();
    int right = margin + vertHeaderWidth;

    // print header
    p.setFont(hHeader->font());
    QString header_label = d_view_widget->model()->headerData(0, Qt::Horizontal).toString();
    QRect br {};
    br = p.boundingRect(br, Qt::AlignCenter, header_label);
    p.drawLine(right, height, right, height + br.height());
    QRect tr(br);

    for (i = 0; i < cols; i++) {
        int w = columnWidth(i);
        tr.setTopLeft(QPoint(right, height));
        tr.setWidth(w);
        tr.setHeight(br.height());
        header_label = d_view_widget->model()->headerData(i, Qt::Horizontal).toString();
        p.drawText(tr, Qt::AlignCenter, header_label);
        right += w;
        p.drawLine(right, height, right, height + tr.height());

        if (right >= printer.width() - 2 * margin)
            break;
    }

    p.drawLine(margin + vertHeaderWidth, height, right - 1, height); // first horizontal line
    height += tr.height();
    p.drawLine(margin, height, right - 1, height);

    // print table values
    for (i = 0; i < rows; i++) {
        right = margin;
        QString cell_text = d_view_widget->model()->headerData(i, Qt::Vertical).toString() + "\t";
        tr = p.boundingRect(tr, Qt::AlignCenter, cell_text);
        p.drawLine(right, height, right, height + tr.height());

        br.setTopLeft(QPoint(right, height));
        br.setWidth(vertHeaderWidth);
        br.setHeight(tr.height());
        p.drawText(br, Qt::AlignCenter, cell_text);
        right += vertHeaderWidth;
        p.drawLine(right, height, right, height + tr.height());

        for (int j = 0; j < cols; j++) {
            int w = columnWidth(j);
            cell_text = text(i, j) + "\t";
            tr = p.boundingRect(tr, Qt::AlignCenter, cell_text);
            br.setTopLeft(QPoint(right, height));
            br.setWidth(w);
            br.setHeight(tr.height());
            p.drawText(br, Qt::AlignCenter, cell_text);
            right += w;
            p.drawLine(right, height, right, height + tr.height());

            if (right >= printer.width() - 2 * margin)
                break;
        }
        height += br.height();
        p.drawLine(margin, height, right - 1, height);

        if (height >= printer.height() - margin) {
            printer.newPage();
            height = margin;
            p.drawLine(margin, height, right, height);
        }
    }
}

int Table::colX(int col)
{
    return d_future_table ? d_future_table->colX(col) : 0;
}

int Table::colY(int col)
{
    return d_future_table ? d_future_table->colY(col) : 0;
}

void Table::setPlotDesignation(Makhber::PlotDesignation pd)
{
    if (d_future_table)
        d_future_table->setSelectionAs(pd);
}

int Table::columnWidth(int col)
{
    return d_future_table ? d_view_widget->columnWidth(col) : 0;
}

void Table::setColWidths(const QStringList &widths)
{
    for (int i = 0; i < widths.count(); i++)
        d_view_widget->setColumnWidth(i, widths[i].toInt());
}

void Table::setColumnTypes(const QStringList &ctl)
{
    // TODO: obsolete, remove in 0.3.0
    int n = qMin((int)ctl.count(), numCols());
    for (int i = 0; i < n; i++) {
        QStringList l = ctl[i].split(";");
        switch (l[0].toInt()) {
        //	old enum: enum ColType{Numeric = 0, Text = 1, Date = 2, Time = 3, Month = 4, Day =
        // 5};
        case 0:
            column(i)->setColumnMode(Makhber::ColumnMode::Numeric);
            break;
        case 1:
            column(i)->setColumnMode(Makhber::ColumnMode::Text);
            break;
        case 2:
        case 3:
        case 6:
            column(i)->setColumnMode(Makhber::ColumnMode::DateTime);
            break;
        case 4:
            column(i)->setColumnMode(Makhber::ColumnMode::Month);
            break;
        case 5:
            column(i)->setColumnMode(Makhber::ColumnMode::Day);
            break;
        }
    }
}

QString Table::saveColumnWidths()
{
    // TODO: obsolete, remove in 0.3.0
    QString s = "ColWidth\t";
    for (int i = 0; i < numCols(); i++)
        s += QString::number(columnWidth(i)) + "\t";

    return s + "\n";
}

QString Table::saveColumnTypes()
{
    // TODO: obsolete, remove in 0.3.0
    QString s = "ColType";
    for (int i = 0; i < numCols(); i++)
        s += "\t" + QString::number(static_cast<int>(column(i)->columnMode())) + ";0/6";
    return s + "\n";
}

void Table::setCommands(const QStringList &com)
{
    for (int i = 0; i < (int)com.size() && i < numCols(); i++)
        column(i)->setFormula(Interval<int>(0, numRows() - 1), com.at(i).trimmed());
}

void Table::setCommand(int col, const QString &com)
{
    column(col)->setFormula(Interval<int>(0, numRows() - 1), com.trimmed());
}

void Table::setCommands(const QString &com)
{
    QStringList lst = com.split("\t");
    lst.pop_front();
    setCommands(lst);
}

bool Table::recalculate()
{
    for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++)
        if (!recalculate(col, true))
            return false;
    return true;
}

bool Table::recalculate(int col, bool only_selected_rows)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    Column *col_ptr = column(col);
    if (!col_ptr)
        return false;

    QList<Interval<int>> formula_intervals = col_ptr->formulaIntervals();
    if (only_selected_rows) {
        // remove non-selected rows from list of intervals
        QList<Interval<int>> deselected =
                Interval<int>(0, col_ptr->rowCount() - 1) - selectedRows().intervals();
        for (Interval<int> i : deselected)
            Interval<int>::subtractIntervalFromList(&formula_intervals, i);
    }
    for (Interval<int> interval : formula_intervals) {
        QString formula = col_ptr->formula(interval.start());
        if (formula.isEmpty())
            continue;

        Script *colscript = scriptEnv->newScript(formula, this, QString("<%1>").arg(colName(col)));
        connect(colscript, SIGNAL(error(const QString &, const QString &, int)), scriptEnv,
                SIGNAL(error(const QString &, const QString &, int)));
        connect(colscript, SIGNAL(print(const QString &)), scriptEnv,
                SIGNAL(print(const QString &)));

        if (!colscript->compile()) {
            delete colscript;
            QApplication::restoreOverrideCursor();
            return false;
        }

        colscript->setInt(col + 1, "j");
        QVariant ret;
        int start_row = interval.start();
        int end_row = interval.end();
        switch (col_ptr->columnMode()) {
        case Makhber::ColumnMode::Numeric: {
            QVector<qreal> results(end_row - start_row + 1);
            for (int i = start_row; i <= end_row; i++) {
                colscript->setInt(i + 1, "i");
                ret = colscript->eval();
                if (!ret.isValid()) {
                    delete colscript;
                    QApplication::restoreOverrideCursor();
                    return false;
                }
                if (ret.canConvert(QVariant::Double))
                    results[i - start_row] = ret.toDouble();
                else
                    results[i - start_row] = NAN;
            }
            col_ptr->replaceValues(start_row, results);
            break;
        }
        default: {
            QStringList results;
            for (int i = start_row; i <= end_row; i++) {
                colscript->setInt(i + 1, "i");
                ret = colscript->eval();
                if (!ret.isValid()) {
                    delete colscript;
                    QApplication::restoreOverrideCursor();
                    return false;
                }
                if (ret.type() == QVariant::Double)
                    results << QLocale().toString(ret.toDouble(), 'g', 14);
                else if (ret.canConvert(QVariant::String))
                    results << ret.toString();
                else
                    results << QString();
            }
            col_ptr->asStringColumn()->replaceTexts(start_row, results);
            break;
        }
        }
        delete colscript;
    }
    QApplication::restoreOverrideCursor();
    return true;
}

QString Table::saveCommands()
{
    // TODO: obsolete, remove for 0.3.0, only needed for template saving
    QString s = "<com>\n";
    for (int col = 0; col < numCols(); col++)
        if (!column(col)->formula(0).isEmpty()) {
            s += "<col nr=\"" + QString::number(col) + "\">\n";
            s += column(col)->formula(0);
            s += "\n</col>\n";
        }
    s += "</com>\n";
    return s;
}

QString Table::saveComments()
{
    // TODO: obsolete, remove for 0.3.0, only needed for template saving
    QString s = "Comments\t";
    for (int i = 0; i < numCols(); i++) {
        s += column(i)->comment() + "\t";
    }
    return s + "\n";
}

QString Table::saveToString(const QString &geometry)
{
    QString s = "<table>\n";
    QString xml;
    if (d_future_table) {
        QXmlStreamWriter writer(&xml);
        d_future_table->save(&writer);
    }
    s += QString::number(xml.length()) + "\n"; // this is need in case there are newlines in the XML
    s += xml + "\n";
    s += geometry + "\n";
    s += "</table>\n";
    return s;
}

void Table::saveToDevice(QIODevice *device, const QString &geometry)
{
    QTextStream stream(device);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));

    // write start tag
    stream << "<table>\n";
    stream.flush();

    // On Windows, writing to a QString has been observed to crash for large tables
    // (apparently due to excessive memory usage).
    // => use temporary file if possible
    QTemporaryFile tmp_file;
    QString tmp_string;
    if (d_future_table) {
        QXmlStreamWriter xml(&tmp_string);
        if (tmp_file.open())
            xml.setDevice(&tmp_file);
        d_future_table->save(&xml);
    }

    // write number of characters of QXmlStreamWriter's output
    // this is needed in case there are newlines in the XML
    int xml_chars = 0;
    if (tmp_file.isOpen()) {
        tmp_file.seek(0);
        QTextStream count(&tmp_file);
        count.setCodec(QTextCodec::codecForName("UTF-8"));
        while (!count.atEnd())
            xml_chars += count.read(1024).length();
    } else
        xml_chars = tmp_string.length();
    stream << xml_chars << "\n";
    stream.flush();

    // Copy QXmlStreamWriter's output to device
    if (tmp_file.isOpen()) {
        tmp_file.seek(0);
        qint64 bytes_read = 0;
        std::array<char, 1024> buffer;
        while ((bytes_read = tmp_file.read(buffer.data(), 1024)) > 0)
            device->write(buffer.data(), bytes_read);
    } else
        stream << tmp_string;
    stream << "\n";

    // write geometry and end tag
    stream << geometry << "\n";
    stream << "</table>\n";
}

QString Table::saveHeader()
{
    // TODO: obsolete, remove for 0.3.0, only needed for template saving
    QString s = "header";
    for (int j = 0; j < numCols(); j++) {
        switch (column(j)->plotDesignation()) {
        case Makhber::X:
            s += "\t" + colLabel(j) + "[X]";
            break;
        case Makhber::Y:
            s += "\t" + colLabel(j) + "[Y]";
            break;
        case Makhber::Z:
            s += "\t" + colLabel(j) + "[Z]";
            break;
        case Makhber::xErr:
            s += "\t" + colLabel(j) + "[xEr]";
            break;
        case Makhber::yErr:
            s += "\t" + colLabel(j) + "[yEr]";
            break;
        default:
            s += "\t" + colLabel(j);
        }
    }
    return s += "\n";
}

int Table::firstXCol()
{
    for (int j = 0; j < numCols(); j++) {
        if (column(j)->plotDesignation() == Makhber::X)
            return j;
    }
    return -1;
}

void Table::setColComment(int col, const QString &s)
{
    column(col)->setComment(s);
}

void Table::setColName(int col, const QString &text)
{
    if (col < 0 || col >= numCols())
        return;

    column(col)->setName(text);
}

QStringList Table::selectedColumns()
{
    // TODO for 0.3.0: extended selection support, Column * lists
    QStringList names;
    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i))
            names << name() + "_" + column(i)->name();
    }
    return names;
}

QStringList Table::YColumns()
{
    // TODO for 0.3.0: Column * list
    QStringList names;
    for (int i = 0; i < numCols(); i++) {
        if (column(i)->plotDesignation() == Makhber::Y)
            names << name() + "_" + column(i)->name();
    }
    return names;
}

QStringList Table::selectedYColumns()
{
    // TODO for 0.3.0: Column * list
    QStringList names;
    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i) && column(i)->plotDesignation() == Makhber::Y)
            names << name() + "_" + column(i)->name();
    }
    return names;
}

QStringList Table::selectedErrColumns()
{
    // TODO for 0.3.0: Column * list
    QStringList names;
    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i)
            && (column(i)->plotDesignation() == Makhber::xErr
                || column(i)->plotDesignation() == Makhber::yErr))
            names << name() + "_" + column(i)->name();
    }
    return names;
}

QStringList Table::drawableColumnSelection()
{
    // TODO for 0.3.0: Column * list
    QStringList names;
    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i) && column(i)->plotDesignation() == Makhber::Y)
            names << name() + "_" + column(i)->name();
    }

    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i)
            && (column(i)->plotDesignation() == Makhber::xErr
                || column(i)->plotDesignation() == Makhber::yErr))
            names << name() + "_" + column(i)->name();
    }
    return names;
}

QMap<int, QString> Table::selectedYLabels()
{
    // TODO for 0.3.0: Column * list
    QMap<int, QString> names;
    for (int i = 0; i < numCols(); i++) {
        if (isColumnSelected(i) && column(i)->plotDesignation() == Makhber::Y)
            names.insert(i, column(i)->name());
    }
    return names;
}

QStringList Table::columnsList()
{
    // TODO for 0.3.0: Column * list
    QStringList names;
    for (int i = 0; i < numCols(); i++)
        names << name() + "_" + column(i)->name();

    return names;
}

int Table::numSelectedRows()
{
    return selectedRowCount();
}

int Table::selectedColsNumber()
{
    return selectedColumnCount();
}

QString Table::colName(int col)
{ // returns the table name + horizontal header text
    if (col < 0 || col >= numCols())
        return QString();

    return QString(name() + "_" + column(col)->name());
}

void Table::insertCols(int start, int count)
{
    if (d_future_table) {
        if (start < 0)
            start = 0;

        QList<Column *> cols;
        for (int i = 0; i < count; i++)
            cols << new Column(QString::number(i + 1), Makhber::ColumnMode::Numeric);
        d_future_table->insertColumns(start, cols);
    }
}

void Table::insertCol()
{
    if (d_future_table)
        d_future_table->insertEmptyColumns();
}

void Table::insertRow()
{
    if (d_future_table)
        d_future_table->insertEmptyRows();
}

void Table::addCol(Makhber::PlotDesignation pd)
{
    if (d_future_table) {
        d_future_table->addColumn();
        column(d_future_table->columnCount() - 1)
                ->setColumnMode(Makhber::ColumnMode::Numeric); // in case we ever change the default
        column(d_future_table->columnCount() - 1)->setPlotDesignation(pd);
    }
}

void Table::addColumns(int c)
{
    if (d_future_table) {
        QList<Column *> cols;
        for (int i = 0; i < c; i++)
            cols << new Column(QString::number(i + 1), Makhber::ColumnMode::Numeric);
        d_future_table->appendColumns(cols);
    }
}

void Table::clearCol()
{
    if (d_future_table)
        d_future_table->clearSelectedColumns();
}

void Table::clearCell(int row, int col)
{
    column(col)->setTextAt(row, QString());
    column(col)->setValueAt(row, 0.0);
    column(col)->setDateTimeAt(row, QDateTime());
    column(col)->setInvalid(row, true);
}

void Table::deleteSelectedRows()
{
    if (d_future_table)
        d_future_table->removeSelectedRows();
}

void Table::cutSelection()
{
    if (d_future_table)
        d_future_table->cutSelection();
}

void Table::selectAllTable()
{
    selectAll();
}

void Table::deselect()
{
    d_view_widget->clearSelection();
}

void Table::clearSelection()
{
    if (d_future_table)
        d_future_table->clearSelectedCells();
}

void Table::copySelection()
{
    if (d_future_table)
        d_future_table->copySelection();
}

void Table::pasteSelection()
{
    if (d_future_table)
        d_future_table->pasteIntoSelection();
}

void Table::removeCol()
{
    if (d_future_table)
        d_future_table->removeSelectedColumns();
}

void Table::removeCol(const QStringList &list)
{
    if (d_future_table) {
        for (QString name : list)
            d_future_table->removeColumns(colIndex(name), 1);
    }
}

int Table::numRows()
{
    return d_future_table ? d_future_table->rowCount() : 0;
}

int Table::numCols()
{
    return d_future_table ? d_future_table->columnCount() : 0;
}

int Table::rowCount()
{
    return d_future_table ? d_future_table->rowCount() : 0;
}

int Table::columnCount()
{
    return d_future_table ? d_future_table->columnCount() : 0;
}

double Table::cell(int row, int col)
{
    Column *colPtr = column(col);
    if (!colPtr)
        return 0.0;
    if (!colPtr->isInvalid(row)) {
        if (colPtr->columnMode() == Makhber::ColumnMode::Text) {
            QString yval = colPtr->textAt(row);
            bool valid_data = true;
            double dbval = QLocale().toDouble(yval, &valid_data);
            if (!valid_data)
                return 0.0;
            return dbval;
        }
        return colPtr->valueAt(row);
    } else
        return 0.0;
}

void Table::setCell(int row, int col, double val)
{
    column(col)->setValueAt(row, val);
}

QString Table::text(int row, int col)
{
    Column *colPtr = column(col);
    if (!colPtr)
        return QString();
    return colPtr->asStringColumn()->textAt(row);
}

void Table::setText(int row, int col, const QString &text)
{
    column(col)->asStringColumn()->setTextAt(row, text);
}

void Table::importV0x0001XXHeader(QStringList header)
{
    if (!d_future_table)
        return;
    QStringList col_label = QStringList();
    QList<Makhber::PlotDesignation> col_plot_type = QList<Makhber::PlotDesignation>();
    for (int i = 0; i < header.count(); i++) {
        if (header[i].isEmpty())
            continue;

        QString s = header[i].replace("_", "-");
        if (s.contains("[X]")) {
            col_label << s.remove("[X]");
            col_plot_type << Makhber::X;
        } else if (s.contains("[Y]")) {
            col_label << s.remove("[Y]");
            col_plot_type << Makhber::Y;
        } else if (s.contains("[Z]")) {
            col_label << s.remove("[Z]");
            col_plot_type << Makhber::Z;
        } else if (s.contains("[xEr]")) {
            col_label << s.remove("[xEr]");
            col_plot_type << Makhber::xErr;
        } else if (s.contains("[yEr]")) {
            col_label << s.remove("[yEr]");
            col_plot_type << Makhber::yErr;
        } else {
            col_label << s;
            col_plot_type << Makhber::noDesignation;
        }
    }
    QList<Column *> quarantine;
    for (int i = 0; i < col_label.count() && i < d_future_table->columnCount(); i++)
        quarantine << column(i);
    int i = 0;
    for (Column *col : quarantine) {
        d_future_table->removeChild(col, true);
        // setting column name while col is still part of table triggers renaming
        // to prevent name clashes
        col->setName(col_label.at(i));
        col->setPlotDesignation(col_plot_type.at(i));
        i++;
    }
    d_future_table->appendColumns(quarantine);
}

void Table::setHeader(QStringList header)
{
    if (!d_future_table)
        return;
    QList<Column *> quarantine;
    for (int i = 0; i < header.count() && i < d_future_table->columnCount(); i++)
        quarantine << column(i);
    int i = 0;
    for (Column *col : quarantine) {
        d_future_table->removeChild(col, true);
        // setting column name while col is still part of table triggers renaming
        // to prevent name clashes
        col->setName(header.at(i));
        i++;
    }
    d_future_table->appendColumns(quarantine);
}

int Table::colIndex(const QString &name)
{
    return d_future_table ? d_future_table->columnIndex(column(name)) : 0;
}

bool Table::noXColumn()
{
    return d_future_table ? d_future_table->columnCount(Makhber::X) == 0 : true;
}

bool Table::noYColumn()
{
    return d_future_table ? d_future_table->columnCount(Makhber::Y) == 0 : true;
}

bool Table::exportASCII(const QString &fname, const QString &separator, bool withLabels,
                        bool exportSelection)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(nullptr, tr("ASCII Export Error"),
                              tr("Could not write to file: <br><h4>") + fname
                                      + tr("</h4><p>Please verify that you have the right to write "
                                           "to this location!"));
        return false;
    }

    QTextStream out(&file);
    int i = 0, j = 0;
    int rows = numRows();
    int cols = numCols();
    int selectedCols = 0;
    int topRow = 0, bottomRow = 0;
    vector<int> sCols;
    if (exportSelection) {
        for (i = 0; i < cols; i++) {
            if (isColumnSelected(i))
                selectedCols++;
        }

        sCols.resize(selectedCols + 1);
        int temp = 1;
        for (i = 0; i < cols; i++) {
            if (isColumnSelected(i)) {
                sCols[temp] = i;
                temp++;
            }
        }

        topRow = firstSelectedRow();
        bottomRow = lastSelectedRow();
    }

    if (withLabels) {
        QStringList header = colNames();
        QStringList ls = header.filter(QRegularExpression("\\D"));
        if (exportSelection) {
            for (i = 1; i < selectedCols; i++) {
                if (ls.count() > 0)
                    out << header[sCols[i]] + separator;
                else
                    out << "C" + header[sCols[i]] + separator;
            }

            if (ls.count() > 0)
                out << header[sCols[selectedCols]] + "\n";
            else
                out << "C" + header[sCols[selectedCols]] + "\n";
        } else {
            if (ls.count() > 0) {
                for (j = 0; j < cols - 1; j++)
                    out << header[j] + separator;
                out << header[cols - 1] + "\n";
            } else {
                for (j = 0; j < cols - 1; j++)
                    out << "C" + header[j] + separator;
                out << "C" + header[cols - 1] + "\n";
            }
        }
    } // finished writting labels

    QList<Column *> col_ptrs;
    if (exportSelection) {
        for (j = 1; j <= selectedCols; j++)
            col_ptrs << column(sCols[j]);
    } else {
        for (j = 0; j < cols; j++)
            col_ptrs << column(j);
        topRow = 0;
        bottomRow = rows - 1;
    }

    for (i = topRow; i <= bottomRow; i++) {
        bool first = true;
        for (Column *col : col_ptrs) {
            if (first)
                first = false;
            else
                out << separator;
            out << col->asStringColumn()->textAt(i);
        }
        out << "\n";
    }

    file.close();
    return true;
}

void Table::customEvent(QEvent *e)
{
    if (e->type() == SCRIPTING_CHANGE_EVENT)
        scriptingChangeEvent(dynamic_cast<ScriptingChangeEvent *>(e));
}

void Table::closeEvent(QCloseEvent *e)
{
    if (askOnClose) {
        switch (QMessageBox::information(this, tr("Makhber"),
                                         tr("Do you want to hide or delete") + "<p><b>'"
                                                 + objectName() + "'</b> ?",
                                         tr("Delete"), tr("Hide"), tr("Cancel"), 0, 2)) {
        case 0:
            e->accept();
            if (d_future_table)
                d_future_table->remove();
            return;

        case 1:
            e->ignore();
            Q_EMIT hiddenWindow(this);
            break;

        case 2:
            e->ignore();
            break;
        }
    } else {
        e->accept();
        if (d_future_table)
            d_future_table->remove();
        return;
    }
}

void Table::setNumRows(int rows)
{
    if (d_future_table)
        d_future_table->setRowCount(rows);
}

void Table::setNumCols(int cols)
{
    if (d_future_table)
        d_future_table->setColumnCount(cols);
}

void Table::copy(Table *m)
{
    if (!m || !d_future_table)
        return;

    d_future_table->copy(m->d_future_table);
}

QString Table::saveAsTemplate(const QString &geometryInfo)
{
    QString s = "<table>\t" + QString::number(numRows()) + "\t";
    s += QString::number(numCols()) + "\n";
    s += geometryInfo;
    s += saveHeader();
    s += saveColumnWidths();
    s += saveCommands();
    s += saveColumnTypes();
    s += saveComments();
    s += "</table>\n";
    return s;
}

void Table::restore(const QStringList &list_in)
{
    // TODO: obsolete, remove in 0.3.0, only needed for template loading
    QStringList temp_list;
    QStringList::const_iterator iterator = list_in.begin();

    temp_list = (*iterator++).split("\t");
    temp_list.removeFirst();
    importV0x0001XXHeader(temp_list);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    setColWidths((*iterator).right((*iterator).length() - 9).split("\t", Qt::SkipEmptyParts));
#else
    setColWidths((*iterator).right((*iterator).length() - 9).split("\t", QString::SkipEmptyParts));
#endif
    iterator++;

    temp_list = (*iterator++).split("\t");
    if (temp_list[0] == "com") {
        temp_list.removeFirst();
        setCommands(temp_list);
    } else if (temp_list[0] == "<com>") {
        QStringList commands;
        for (int col = 0; col < numCols(); col++)
            commands << "";
        for (; iterator != list_in.end() && *iterator != "</com>"; iterator++) {
            int col = (*iterator).midRef(9, (*iterator).length() - 11).toInt();
            QString formula;
            for (iterator++; iterator != list_in.end() && *iterator != "</col>"; iterator++)
                formula += *iterator + "\n";
            formula.truncate(formula.length() - 1);
            commands[col] = formula;
        }
        iterator++;
        setCommands(commands);
    }

    temp_list = (*iterator++).split("\t");
    temp_list.removeFirst();
    setColumnTypes(temp_list);

    temp_list = (*iterator++).split("\t");
    temp_list.removeFirst();
    setColComments(temp_list);
}

void Table::clear()
{
    if (d_future_table)
        d_future_table->clear();
}

QStringList Table::colNames()
{
    QStringList list;
    if (d_future_table)
        for (int i = 0; i < d_future_table->columnCount(); i++)
            list << column(i)->name();
    return list;
}

QString Table::colLabel(int col)
{
    return column(col)->name();
}

Makhber::PlotDesignation Table::colPlotDesignation(int col)
{
    return column(col)->plotDesignation();
}

void Table::setColPlotDesignation(int col, Makhber::PlotDesignation d)
{
    column(col)->setPlotDesignation(d);
}

QList<int> Table::plotDesignations()
{
    QList<int> list;
    if (d_future_table)
        for (int i = 0; i < d_future_table->columnCount(); i++)
            list << column(i)->plotDesignation();
    return list;
}

QList<Makhber::ColumnMode> Table::columnTypes()
{
    QList<Makhber::ColumnMode> list;
    if (d_future_table)
        for (int i = 0; i < d_future_table->columnCount(); i++)
            list << column(i)->columnMode();
    return list;
}

Makhber::ColumnMode Table::columnType(int col)
{
    return column(col)->columnMode();
}

void Table::setColumnTypes(QList<Makhber::ColumnMode> ctl)
{
    if (!d_future_table)
        return;
    Q_ASSERT(ctl.size() == d_future_table->columnCount());
    for (int i = 0; i < d_future_table->columnCount(); i++)
        column(i)->setColumnMode(ctl.at(i));
}

void Table::setColumnType(int col, Makhber::ColumnMode mode)
{
    column(col)->setColumnMode(mode);
}

QString Table::columnFormat(int col)
{
    // TODO: obsolete, remove in 0.3.0
    Column *col_ptr = column(col);
    if (col_ptr->columnMode() != Makhber::ColumnMode::DateTime
        && col_ptr->columnMode() != Makhber::ColumnMode::Month
        && col_ptr->columnMode() != Makhber::ColumnMode::Day)
        return QString();

    auto *filter = dynamic_cast<DateTime2StringFilter *>(col_ptr->outputFilter());
    return filter->format();
}

int Table::verticalHeaderWidth()
{
    return d_view_widget->verticalHeader()->width();
}

QString Table::colComment(int col)
{
    return column(col)->comment();
}

QStringList Table::colComments()
{
    QStringList list;
    if (d_future_table)
        for (int i = 0; i < d_future_table->columnCount(); i++)
            list << column(i)->comment();
    return list;
}

void Table::setColComments(const QStringList &list)
{
    if (d_future_table)
        for (int i = 0; i < d_future_table->columnCount(); i++)
            column(i)->setComment(list.at(i));
}

bool Table::commentsEnabled()
{
    return areCommentsShown();
}

void Table::applyFormula()
{
    if (!d_future_table)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    d_future_table->beginMacro(tr("%1: apply formula to column").arg(name()));

    QString formula = ui.formula_box->toPlainText();
    for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++) {
        Column *col_ptr = column(col);
        col_ptr->insertRows(col_ptr->rowCount(), rowCount() - col_ptr->rowCount());
        col_ptr->setFormula(Interval<int>(0, rowCount() - 1), formula);
        if (!recalculate(col, false))
            break;
    }

    d_future_table->endMacro();
    QApplication::restoreOverrideCursor();
}

void Table::addFunction()
{
    dynamic_cast<ScriptEdit *>(ui.formula_box)
            ->insertFunction(ui.add_function_combobox->currentText());
}

void Table::addReference()
{
    ui.formula_box->insertPlainText(ui.add_reference_combobox->currentText());
}

void Table::updateFunctionDoc()
{
    ui.add_function_combobox->setToolTip(
            scriptEnv->mathFunctionDoc(ui.add_function_combobox->currentText()));
}

void Table::handleAspectDescriptionAboutToChange(const AbstractAspect *aspect)
{
    const auto *col = qobject_cast<const Column *>(aspect);
    if (col && d_future_table && d_future_table->columnIndex(col) != -1) {
        d_stored_column_labels[col] = aspect->name();
    }
}

void Table::handleAspectDescriptionChange(const AbstractAspect *aspect)
{
    if (aspect == d_future_table) {
        setObjectName(d_future_table->name());
        updateCaption();
        return;
    }
    const auto *col = qobject_cast<const Column *>(aspect);
    if (col && d_future_table && d_future_table->columnIndex(col) != -1
        && d_stored_column_labels.contains(col)) {
        QString old_name = d_stored_column_labels.value(col);
        QString new_name = col->name();
        Q_EMIT changedColHeader(name() + "_" + old_name, name() + "_" + new_name);

        for (int i = 0; i < d_future_table->columnCount(); i++) {
            QList<Interval<int>> formula_intervals = column(i)->formulaIntervals();
            for (Interval<int> interval : formula_intervals) {
                QString formula = column(i)->formula(interval.start());
                if (formula.contains("\"" + old_name + "\"")) {
                    formula.replace("\"" + old_name + "\"", "\"" + new_name + "\"");
                    column(i)->setFormula(interval, formula);
                }
            }
        }
    }
}

// this function is for backwards compatibility (used by Python),
void Table::importASCII(const QString &fname, const QString &sep, int ignoredLines, bool renameCols,
                        bool stripSpaces, bool simplifySpaces, bool newTable)
{
    Q_UNUSED(newTable)

    AsciiTableImportFilter filter;
    filter.set_ignored_lines(ignoredLines);
    filter.set_separator(sep);
    filter.set_first_row_names_columns(renameCols);
    filter.set_trim_whitespace(stripSpaces);
    filter.set_simplify_whitespace(simplifySpaces);

    QFile file(fname);
    if (file.open(QIODevice::ReadOnly)) {
        auto *temp = dynamic_cast<future::Table *>(filter.importAspect(file));
        if (!temp)
            return;
        int preexisting_cols = columnCount();
        int overwritten_cols = qMin(temp->columnCount(), preexisting_cols);
        for (int i = 0; i < overwritten_cols; i++) {
            column(i)->asStringColumn()->copy(temp->column(i));
            if (renameCols)
                column(i)->setName(temp->column(i)->name());
        }
        for (int i = overwritten_cols; i < preexisting_cols; i++)
            column(overwritten_cols)->remove();
        auto *filter = new String2DoubleFilter;
        for (int i = overwritten_cols; i < temp->columnCount(); i++) {
            filter->input(0, temp->column(i));
            auto *new_col = new Column(temp->column(i)->name(), Makhber::ColumnMode::Numeric);
            new_col->setPlotDesignation(Makhber::Y);
            new_col->copy(filter->output(0));
            d_future_table->addChild(new_col);
        }
        delete filter;
        delete temp;
        setWindowLabel(fname);
    }
}
