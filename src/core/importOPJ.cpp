/***************************************************************************
    File                 : importOPJ.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2010 Miquel Garriga (gbmiquel*gmail.com)
    Copyright            : (C) 2010 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 by Ion Vasilief (ion_vasilief*yahoo.fr)
    Copyright            : (C) 2006-2007 by Alex Kargovsky (kargovsky*yumr.phys.msu.su)
    Copyright            : (C) 2006-2007 by Tilman Benkert (thzs*gmx.net)
    Description          : Origin project import class

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
#include "importOPJ.h"

#include "matrix/Matrix.h"
#include "scripting/Note.h"
#include "core/ColorButton.h"
#include "core/Folder.h"
#include "plot2D/MultiLayer.h"
#include "plot2D/HistogramCurve.h"
#include "plot2D/Grid.h"
#include "aspects/datatypes/Double2StringFilter.h"
#include "aspects/datatypes/DateTime2StringFilter.h"

#include <QRegularExpression>
#include <QApplication>
#include <QMessageBox>
#include <QDockWidget>
#include <QLocale>
#include <QDate>

#include <cmath>

#define OBJECTXOFFSET 200

bool ImportOPJ::setCodec(const QString &codecName)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto codec = QStringConverter::encodingForName(codecName.toUtf8());
    if (codec.has_value()) {
        d_codec = codec.value();
#else
    auto codec = QTextCodec::codecForName(codecName.toUtf8());
    if (nullptr != codec) {
        d_codec = codec;
#endif
        return true;
    }
    return false;
}

QString ImportOPJ::decodeMbcs(char const *const input) const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto decoder = QStringDecoder(d_codec);
    return decoder(input);
#else
    if (nullptr != d_codec)
        return d_codec->toUnicode(input);
    return QString(input);
#endif
}

QString strreverse(const QString &str) // QString reversing
{
    QString out = str;
    std::reverse(out.begin(), out.end());
    return out;
}

QString posixTimeToString(time_t pt)
{
    QDateTime qdt;
    qdt.setSecsSinceEpoch(pt);
    return qdt.toString("dd.MM.yyyy hh.mm.ss");
}

ImportOPJ::ImportOPJ(ApplicationWindow *app, const QString &filename, const QString &codec)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    : mw(app)
#else
    : mw(app), d_codec(nullptr)
#endif
{
    setCodec(codec);
    xoffset = 0;
    try {
        mw->setStatusBarText(QString("Import start ..."));
        OriginFile opj((const char *)filename.toLocal8Bit());
        parse_error = opj.parse();
        mw->setStatusBarText(QString("... file parsed. Starting conversion to Makhber ..."));
        importTables(opj);
        importGraphs(opj);
        importNotes(opj);
        mw->setStatusBarText(QString());
        if (filename.endsWith(".opj", Qt::CaseInsensitive))
            createProjectTree(opj);
        mw->showResults(opj.resultsLogString().c_str(), mw->logWindow.isVisible());
    } catch (const std::logic_error &er) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(mw, "Origin Project Import Error", QString(er.what()));
    }
}

inline size_t qHash(const tree<Origin::ProjectNode>::iterator &key)
{
    return qHash(key->name.c_str());
}

bool ImportOPJ::createProjectTree(const OriginFile &opj)
{
    const tree<Origin::ProjectNode> *projectTree = opj.project();
    tree<Origin::ProjectNode>::iterator root = projectTree->begin(projectTree->begin());
    if (!root.node)
        return false;
    auto *item = dynamic_cast<FolderListItem *>(mw->folders.topLevelItem(0));
    item->setText(0, decodeMbcs(root->name.c_str()));
    item->folder()->setName(decodeMbcs(root->name.c_str()));
    Folder *projectFolder = mw->projectFolder();
    QHash<tree<Origin::ProjectNode>::iterator, Folder *> parent;
    parent[root] = projectFolder;
    for (tree<Origin::ProjectNode>::iterator sib = projectTree->begin(root);
         sib != projectTree->end(root); ++sib) {
        if (sib->type == Origin::ProjectNode::Folder) {

            auto &f = parent.value(projectTree->parent(sib))
                              ->addChild<Folder>(decodeMbcs(sib->name.c_str()));
            parent[sib] = &f;
            f.setBirthDate(posixTimeToString(sib->creationDate));
            f.setModificationDate(posixTimeToString(sib->modificationDate));
            mw->addFolderListViewItem(&f);
            f.setFolderListItem(new FolderListItem(mw->current_folder->folderListItem(), &f));
        } else {
            QString name = decodeMbcs(sib->name.c_str());
            if (sib->type == Origin::ProjectNode::Note) {
                QRegularExpression rx(R"(^@\((\S+)\)$)");
                auto match = rx.match(name);
                if (match.capturedStart() == 0)
                    name = match.captured(1);
            }
            const char *nodetype = nullptr;
            switch (sib->type) {
            case Origin::ProjectNode::SpreadSheet:
                nodetype = "Table";
                break;
            case Origin::ProjectNode::Matrix:
                nodetype = "Matrix";
                break;
            case Origin::ProjectNode::Graph:
                nodetype = "MultiLayer";
                break;
            case Origin::ProjectNode::Graph3D:
                // there is no Graph3D type yet
                nodetype = "MultiLayer"; // "Graph3D";
                break;
            case Origin::ProjectNode::Note:
                nodetype = "Note";
                break;
            case Origin::ProjectNode::Excel:
                // there is no Excel type yet
                nodetype = "Table";
                break;
            default:
                nodetype = "Unknown";
                break;
            }
            MyWidget *w = projectFolder->window(name, nodetype);
            while (w) { // Origin window names are unique, but we need to loop on all sheets of
                        // Excel windows
                Folder *f = parent.value(projectTree->parent(sib));
                if (f) {
                    if (f == parent[root])
                        break; // skip windows that go to root folder
                    // removeWindow  uses QList.removeAll, so remove w before adding it to its
                    // folder
                    projectFolder->removeWindow(w);
                    f->addWindow(w);
                    f->setActiveWindow(w);
                }
                w = projectFolder->window(name, nodetype);
            }
        }
    }
    mw->changeFolder(projectFolder, true);
    return true;
}

int ImportOPJ::translateOrigin2MakhberLineStyle(int linestyle)
{
    int makhberstyle = 0;
    switch (linestyle) {
    case Origin::GraphCurve::Solid:
        makhberstyle = 0;
        break;
    case Origin::GraphCurve::Dash:
    case Origin::GraphCurve::ShortDash:
        makhberstyle = 1;
        break;
    case Origin::GraphCurve::Dot:
    case Origin::GraphCurve::ShortDot:
        makhberstyle = 2;
        break;
    case Origin::GraphCurve::DashDot:
    case Origin::GraphCurve::ShortDashDot:
        makhberstyle = 3;
        break;
    case Origin::GraphCurve::DashDotDot:
        makhberstyle = 4;
        break;
    }
    return makhberstyle;
}

// spreadsheets can be either in its own window or as a sheet in excels windows
bool ImportOPJ::importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread)
{
    static int visible_count = 0;
    QLocale locale = mw->locale();
    int Makhber_scaling_factor = 10; // in Origin width is measured in characters while in Makhber
                                     // - pixels --- need to be accurate
    int columnCount = static_cast<int>(spread.columns.size());
    int maxrows = spread.maxRows;
    if (!columnCount) // remove tables without cols
        return false;

    Table *table = mw->newTable(decodeMbcs(spread.name.c_str()), maxrows, columnCount);
    if (!table)
        return false;
    if (spread.hidden || spread.loose)
        mw->hideWindow(table);

    table->setWindowLabel(decodeMbcs(spread.label.c_str()));
    for (int j = 0; j < columnCount; ++j) {
        Origin::SpreadColumn column = spread.columns[j];
        Column *makhber_column = table->column(j);

        QString name(decodeMbcs(column.name.c_str()));
        makhber_column->setName(name.replace(QRegularExpression(".*_"), ""));
        if (column.command.size() > 0)
            makhber_column->setFormula(Interval<int>(0, maxrows),
                                       QString(decodeMbcs(column.command.c_str())));
        makhber_column->setComment(QString(decodeMbcs(column.comment.c_str())));
        table->setColumnWidth(j, (int)column.width * Makhber_scaling_factor);

        switch (column.type) {
        case Origin::SpreadColumn::X:
            table->setColPlotDesignation(j, Makhber::X);
            break;
        case Origin::SpreadColumn::Y:
            table->setColPlotDesignation(j, Makhber::Y);
            break;
        case Origin::SpreadColumn::Z:
            table->setColPlotDesignation(j, Makhber::Z);
            break;
        case Origin::SpreadColumn::XErr:
            table->setColPlotDesignation(j, Makhber::xErr);
            break;
        case Origin::SpreadColumn::YErr:
            table->setColPlotDesignation(j, Makhber::yErr);
            break;
        case Origin::SpreadColumn::Label:
        default:
            table->setColPlotDesignation(j, Makhber::noDesignation);
        }

        QString format;
        switch (column.valueType) {
        case Origin::Numeric:
        case Origin::TextNumeric:
            /*
              A TextNumeric column in Origin is a column whose filled cells contain either a double
              or a string. In Makhber there is no equivalent column type. Set the Makhber column
              type as 'Numeric' or 'Text' depending on the type of first element in column.
              TODO: Add a "per column" flag, settable at import dialog, to choose between both
              types.
            */
            {
                double datavalue = NAN;
                bool setAsText = false;
                table->column(j)->setColumnMode(Makhber::ColumnMode::Numeric);
                for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i) {
                    Origin::variant value(column.data[i]);
                    if (value.type() == Origin::variant::V_DOUBLE) {
                        datavalue = value.as_double();
                        if (datavalue == _ONAN)
                            continue; // mark for empty cell
                        if (!setAsText) {
                            makhber_column->setValueAt(i, datavalue);
                        } else { // convert double to string for Text columns
                            makhber_column->setTextAt(i, locale.toString(datavalue, 'g', 16));
                        }
                    } else { // string
                        if (!setAsText && i == 0) {
                            table->column(j)->setColumnMode(Makhber::ColumnMode::Text);
                            setAsText = true;
                        }
                        makhber_column->setTextAt(i, column.data[i].as_string());
                    }
                }
                int f = 0;
                if (column.numericDisplayType != 0) {
                    switch (column.valueTypeSpecification) {
                    case 0: // Decimal 1000
                        f = 1;
                        break;
                    case 1: // Scientific
                        f = 2;
                        break;
                    case 2: // Engeneering
                    case 3: // Decimal 1,000
                        f = 0;
                        break;
                    }
                    auto *filter =
                            dynamic_cast<Double2StringFilter *>(table->column(j)->outputFilter());
                    filter->setNumericFormat(f);
                    filter->setNumDigits(column.decimalPlaces);
                }
                break;
            }
        case Origin::Text:
            table->column(j)->setColumnMode(Makhber::ColumnMode::Text);
            for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i) {
                makhber_column->setTextAt(i, column.data[i].as_string());
            }
            break;
        case Origin::Date: {
            switch (column.valueTypeSpecification) {
            case -128:
                format = "dd/MM/yyyy";
                break;
            case -119:
                format = "dd/MM/yyyy HH:mm";
                break;
            case -118:
                format = "dd/MM/yyyy HH:mm:ss";
                break;
            case 0:
            case 9:
            case 10:
                format = "dd.MM.yyyy";
                break;
            case 2:
                format = "MMM d";
                break;
            case 3:
                format = "M/d";
                break;
            case 4:
                format = "d";
                break;
            case 5:
            case 6:
                format = "ddd";
                break;
            case 7:
                format = "yyyy";
                break;
            case 8:
                format = "yy";
                break;
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                format = "yyMMdd";
                break;
            case 16:
            case 17:
                format = "MMM";
                break;
            case 19:
                format = "M-d-yyyy";
                break;
            default:
                format = "dd.MM.yyyy";
            }
            for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i)
                makhber_column->setValueAt(i, column.data[i].as_double());
            table->column(j)->setColumnMode(Makhber::ColumnMode::DateTime);
            auto *filter = dynamic_cast<DateTime2StringFilter *>(makhber_column->outputFilter());
            filter->setFormat(format);
            break;
        }
        case Origin::Time: {
            switch (column.valueTypeSpecification + 128) {
            case 0:
                format = "hh:mm";
                break;
            case 1:
                format = "hh";
                break;
            case 2:
                format = "hh:mm:ss";
                break;
            case 3:
                format = "hh:mm:ss.zzz";
                break;
            case 4:
                format = "hh ap";
                break;
            case 5:
                format = "hh:mm ap";
                break;
            case 6:
                format = "mm:ss";
                break;
            case 7:
                format = "mm:ss.zzz";
                break;
            case 8:
                format = "hhmm";
                break;
            case 9:
                format = "hhmmss";
                break;
            case 10:
                format = "hh:mm:ss.zzz";
                break;
            }
            for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i)
                makhber_column->setValueAt(i, column.data[i].as_double());
            table->column(j)->setColumnMode(Makhber::ColumnMode::DateTime);
            auto *filter = dynamic_cast<DateTime2StringFilter *>(table->column(j)->outputFilter());
            filter->setFormat(format);
            break;
        }
        case Origin::Month: {
            switch (column.valueTypeSpecification) {
            case 0:
                format = "MMM";
                break;
            case 1:
                format = "MMMM";
                break;
            case 2:
                format = "M";
                break;
            }
            for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i)
                makhber_column->setValueAt(i, column.data[i].as_double());
            table->column(j)->setColumnMode(Makhber::ColumnMode::Month);
            auto *filter = dynamic_cast<DateTime2StringFilter *>(table->column(j)->outputFilter());
            filter->setFormat(format);
            break;
        }
        case Origin::Day: {
            switch (column.valueTypeSpecification) {
            case 0:
                format = "ddd";
                break;
            case 1:
                format = "dddd";
                break;
            case 2:
                format = "d";
                break;
            }
            for (int i = 0; i < std::min((int)column.data.size(), maxrows); ++i)
                makhber_column->setValueAt(i, column.data[i].as_double());
            table->column(j)->setColumnMode(Makhber::ColumnMode::Day);
            auto *filter = dynamic_cast<DateTime2StringFilter *>(table->column(j)->outputFilter());
            filter->setFormat(format);
            break;
        }
        default:
            break;
        }
    }

    if (!(spread.hidden || spread.loose) || opj.version() != 7.5) {
        table->showNormal();

        // cascade the tables
        if (opj.version() >= 6.0) {
            Origin::Rect windowRect;
            windowRect = spread.frameRect;
            table->move(QPoint(windowRect.left, windowRect.top));
        } else {
            int dx = table->verticalHeaderWidth();
            int dy = table->frameGeometry().height() - table->height();
            table->move(QPoint(visible_count * dx + xoffset * OBJECTXOFFSET, visible_count * dy));
            visible_count++;
        }
    }
    return true;
}
bool ImportOPJ::importTables(const OriginFile &opj)
{
    static int visible_count = 0;
    int Makhber_scaling_factor = 10; // in Origin width is measured in characters while in Makhber
                                     // - pixels --- need to be accurate
    for (unsigned int s = 0; s < opj.spreadCount(); ++s) {
        mw->setStatusBarText(QString("Spreadsheet %1 / %2").arg(s + 1).arg(opj.spreadCount()));
        Origin::SpreadSheet spread = opj.spread(s);
        auto columnCount = spread.columns.size();
        if (!columnCount) // remove tables without cols
            continue;
        importSpreadsheet(opj, spread);
    }
    // Import excels
    for (unsigned int s = 0; s < opj.excelCount(); ++s) {
        Origin::Excel excelwb = opj.excel(s);
        for (unsigned int j = 0; j < excelwb.sheets.size(); ++j) {
            mw->setStatusBarText(QString("Excel %1 / %2, sheet %3 / %4")
                                         .arg(s + 1)
                                         .arg(opj.excelCount())
                                         .arg(j + 1)
                                         .arg(excelwb.sheets.size()));
            Origin::SpreadSheet spread = excelwb.sheets[j];
            auto columnCount = spread.columns.size();
            if (!columnCount) // remove tables without cols
                continue;
            spread.name = excelwb.name;
            // makhber does not have windows with multiple sheets
            if (j > 0) {
                spread.name.append("@").append(std::to_string(j + 1));
            }
            spread.maxRows = excelwb.maxRows;
            importSpreadsheet(opj, spread);
        }
    }

    // Import matrices
    for (unsigned int s = 0; s < opj.matrixCount(); ++s) {
        Origin::Matrix matrix = opj.matrix(s);
        auto layers = matrix.sheets.size();
        for (size_t l = 0; l < layers; ++l) {
            Origin::MatrixSheet &layer = matrix.sheets[l];
            int columnCount = layer.columnCount;
            int rowCount = layer.rowCount;
            mw->setStatusBarText(QString("Matrix %1 / %2, sheet %3 / %4")
                                         .arg(s + 1)
                                         .arg(opj.matrixCount())
                                         .arg(l + 1)
                                         .arg(layers));
            Matrix *Matrix = mw->newMatrix(decodeMbcs(matrix.name.c_str()), rowCount, columnCount);
            if (!Matrix)
                return false;
            Matrix->setWindowLabel(decodeMbcs(matrix.label.c_str()));
            Matrix->setFormula(decodeMbcs(layer.command.c_str()));
            Matrix->setColumnsWidth(layer.width * Makhber_scaling_factor);
// TODO
#if 0
			Matrix->table()->blockSignals(true);
#endif
            QVector<qreal> values;
            values.resize(rowCount * columnCount);
            for (int i = 0; i < std::min((int)values.size(), (int)layer.data.size()); i++) {
                values[i] = layer.data[i];
            }
            Matrix->setCells(values);

            Matrix->saveCellsToMemory();

            QChar format;
            int prec = 6;
            switch (layer.valueTypeSpecification) {
            case 0: // Decimal 1000
                format = 'f';
                prec = layer.decimalPlaces;
                break;
            case 1: // Scientific
                format = 'e';
                prec = layer.decimalPlaces;
                break;
            case 2: // Engineering
            case 3: // Decimal 1,000
                format = 'g';
                prec = layer.significantDigits;
                break;
            }
            Matrix->setNumericFormat(format, prec);
            Matrix->forgetSavedCells();
// TODO
#if 0
        Matrix->table()->blockSignals(false);
#endif
            Matrix->showNormal();

            // cascade the matrices
#if 0
			int dx=Matrix->verticalHeaderWidth();
			int dy=Matrix->frameGeometry().height() - matrix->height();
#endif
            // TODO
            int dx = 100;
            int dy = 100;
            Matrix->move(QPoint(visible_count * dx + xoffset * OBJECTXOFFSET, visible_count * dy));
            visible_count++;
        }

        if (visible_count > 0)
            xoffset++;
    }
    return true;
}

bool ImportOPJ::importNotes(const OriginFile &opj)
{
    int visible_count = 0;
    for (unsigned int n = 0; n < opj.noteCount(); ++n) {
        Origin::Note _note = opj.note(n);
        QString name = decodeMbcs(_note.name.c_str());
        QRegularExpression rx("^@(\\S+)$");
        if (rx.match(name).capturedStart() == 0) {
            name = name.mid(2, name.length() - 3);
        }
        Note *note = mw->newNote(name);
        if (!note)
            return false;
        note->setWindowLabel(decodeMbcs(_note.label.c_str()));
        note->setText(QString(decodeMbcs(_note.text.c_str())));

        // cascade the notes
        int dx = 20;
        int dy = note->frameGeometry().height() - note->height();
        note->move(QPoint(visible_count * dx + xoffset * OBJECTXOFFSET, visible_count * dy));
        visible_count++;
    }
    if (visible_count > 0)
        xoffset++;
    return true;
}

bool ImportOPJ::importGraphs(const OriginFile &opj)
{
    const double pi = 3.141592653589793;
    int visible_count = 0;
    std::array<int, 4> tickTypeMap = { 0, 3, 1, 2 };
    for (unsigned int g = 0; g < opj.graphCount(); ++g) {
        Origin::Graph _graph = opj.graph(g);
        MultiLayer *ml = mw->multilayerPlot(decodeMbcs(_graph.name.c_str())); //, 0);
        if (!ml)
            return false;

        ml->hide(); //!hack used in order to avoid resize and repaint events
        ml->setWindowLabel(decodeMbcs(_graph.label.c_str()));
        auto layers = _graph.layers.size();
        for (size_t l = 0; l < layers; ++l) {
            mw->setStatusBarText(QString("Graph %1 / %2, layer %3 / %4")
                                         .arg(g + 1)
                                         .arg(opj.graphCount())
                                         .arg(l + 1)
                                         .arg(layers));
            Origin::GraphLayer &layer = _graph.layers[l];
            Graph *graph = ml->addLayer();
            if (!graph)
                return false;
            if (!layer.legend.text.empty()) {
                graph->newLegend(parseOriginText(decodeMbcs(layer.legend.text.c_str())));
            }
            // add texts
            for (auto &text_ : layer.texts) {
                graph->newLegend(parseOriginText(decodeMbcs(text_.text.c_str())));
            }
            int auto_color = 0;
            int style = 0;
            for (int c = 0; c < static_cast<int>(layer.curves.size()); ++c) {
                Origin::GraphCurve &_curve = layer.curves[c];
                QString data(decodeMbcs(_curve.dataName.c_str()));
                int color = 0;
                switch (_curve.type) {
                case Origin::GraphCurve::Line:
                    style = Graph::Line;
                    break;
                case Origin::GraphCurve::Scatter:
                    style = Graph::Scatter;
                    break;
                case Origin::GraphCurve::LineSymbol:
                    style = Graph::LineSymbols;
                    break;
                case Origin::GraphCurve::ErrorBar:
                case Origin::GraphCurve::XErrorBar:
                    style = Graph::ErrorBars;
                    break;
                case Origin::GraphCurve::Column:
                    style = Graph::VerticalBars;
                    break;
                case Origin::GraphCurve::Bar:
                    style = Graph::HorizontalBars;
                    break;
                case Origin::GraphCurve::Histogram:
                    style = Graph::Histogram;
                    break;
                default:
                    continue;
                }
                QString tableName;
                switch (data[0].toLatin1()) {
                case 'T':
                case 'E': {
                    tableName = data.right(data.length() - 2);
                    Table *table = mw->table(tableName);
                    if (!table)
                        break;
                    if (style == Graph::ErrorBars) {
                        int flags = _curve.symbolShape;
                        graph->addErrorBars(
                                QString("%1_%2").arg(tableName,
                                                     decodeMbcs(_curve.xColumnName.c_str())),
                                table,
                                QString("%1_%2").arg(tableName,
                                                     decodeMbcs(_curve.yColumnName.c_str())),
                                ((flags & 0x10) == 0x10 ? Qt::Horizontal : Qt::Vertical),
                                ceil(_curve.lineWidth), ceil(_curve.symbolSize), QColor(Qt::black),
                                (flags & 0x40) == 0x40, (flags & 2) == 2, (flags & 1) == 1);
                    } else if (style == Graph::Histogram) {

                        graph->insertCurve(
                                table,
                                QString("%1_%2").arg(tableName,
                                                     decodeMbcs(_curve.yColumnName.c_str())),
                                style);
                    } else {
                        graph->insertCurve(
                                table,
                                QString("%1_%2").arg(tableName,
                                                     decodeMbcs(_curve.xColumnName.c_str())),
                                QString("%1_%2").arg(tableName,
                                                     decodeMbcs(_curve.yColumnName.c_str())),
                                style);
                    }
                    break;
                }
                case 'F': {
                    Origin::Function function;
                    QStringList formulas;
                    QList<double> ranges;
                    int s = opj.functionIndex(data.right(data.length() - 2).toStdString().c_str());
                    function = opj.function(s);

                    int type = 0;
                    if (function.type == Origin::Function::Polar) {
                        type = 2;
                        formulas << decodeMbcs(function.formula.c_str()) << "x";
                        ranges << pi / 180 * function.begin << pi / 180 * function.end;
                    } else {
                        type = 0;
                        formulas << decodeMbcs(function.formula.c_str());
                        ranges << function.begin << function.end;
                    }
                    graph->addFunctionCurve(mw, type, formulas, "x", ranges, function.totalPoints,
                                            decodeMbcs(function.name.c_str()));

                    mw->updateFunctionLists(type, formulas);
                    break;
                }
                default:
                    continue;
                }

                CurveLayout cl =
                        graph->initCurveLayout(style, static_cast<int>(layer.curves.size()));
                cl.sSize = ceil(_curve.symbolSize * 0.5);
                cl.penWidth = _curve.symbolThickness;
                color = _curve.symbolColor.regular;
                if ((style == Graph::Scatter || style == Graph::LineSymbols)
                    && color == 0xF7) // 0xF7 -Automatic color
                    color = auto_color++;
                cl.symCol = color;
                switch (_curve.symbolShape) {
                case 0: // NoSymbol
                    cl.sType = 0;
                    break;
                case 1: // Rect
                    cl.sType = 2;
                    break;
                case 2: // Ellipse
                case 20: // Sphere
                    cl.sType = 1;
                    break;
                case 3: // UTriangle
                    cl.sType = 6;
                    break;
                case 4: // DTriangle
                    cl.sType = 5;
                    break;
                case 5: // Diamond
                    cl.sType = 3;
                    break;
                case 6: // Cross +
                    cl.sType = 9;
                    break;
                case 7: // Cross x
                    cl.sType = 10;
                    break;
                case 8: // Snow
                    cl.sType = 13;
                    break;
                case 9: // Horizontal -
                    cl.sType = 11;
                    break;
                case 10: // Vertical |
                    cl.sType = 12;
                    break;
                case 15: // LTriangle
                    cl.sType = 7;
                    break;
                case 16: // RTriangle
                    cl.sType = 8;
                    break;
                case 17: // Hexagon
                case 19: // Pentagon
                    cl.sType = 15;
                    break;
                case 18: // Star
                    cl.sType = 14;
                    break;
                default:
                    cl.sType = 0;
                }

                switch (_curve.symbolInterior) {
                case 0:
                    cl.fillCol = color;
                    break;
                case 1:
                case 2:
                case 8:
                case 9:
                case 10:
                case 11:
                    color = _curve.symbolFillColor.regular;
                    if ((style == Graph::Scatter || style == Graph::LineSymbols)
                        && color == 0xF7) // 0xF7 -Automatic color
                        color = 17; // depend on Origin settings - not stored in file
                    cl.fillCol = color;
                    break;
                default:
                    cl.fillCol = 0;
                }

                cl.lWidth = _curve.lineWidth;
                color = _curve.lineColor.regular % ColorButton::colors_count;
                cl.lCol = (_curve.lineColor.type == Origin::Color::Automatic
                                   ? 0
                                   : color); // 0xF7 -Automatic color
                int linestyle = _curve.lineStyle;
                cl.filledArea = (_curve.fillArea || style == Graph::VerticalBars
                                 || style == Graph::HorizontalBars || style == Graph::Histogram
                                 || style == Graph::Pie || style == Graph::Box)
                        ? 1
                        : 0;
                if (cl.filledArea) {
                    switch (_curve.fillAreaPattern) {
                    case 0:
                        cl.aStyle = 0;
                        break;
                    case 1:
                    case 2:
                    case 3:
                        cl.aStyle = 4;
                        break;
                    case 4:
                    case 5:
                    case 6:
                        cl.aStyle = 5;
                        break;
                    case 7:
                    case 8:
                    case 9:
                        cl.aStyle = 6;
                        break;
                    case 10:
                    case 11:
                    case 12:
                        cl.aStyle = 1;
                        break;
                    case 13:
                    case 14:
                    case 15:
                        cl.aStyle = 2;
                        break;
                    case 16:
                    case 17:
                    case 18:
                        cl.aStyle = 3;
                        break;
                    }
                    Origin::Color color {};
                    color = (cl.aStyle == 0 ? _curve.fillAreaColor : _curve.fillAreaPatternColor);
                    cl.aCol = (color.type == Origin::Color::Automatic
                                       ? 0
                                       : color.regular); // 0xF7 -Automatic color
                    if (style == Graph::VerticalBars || style == Graph::HorizontalBars
                        || style == Graph::Histogram) {
                        color = _curve.fillAreaPatternBorderColor;
                        cl.lCol = (color.type == Origin::Color::Automatic
                                           ? 0
                                           : color.regular); // 0xF7 -Automatic color
                        color = (cl.aStyle == 0 ? _curve.fillAreaColor
                                                : _curve.fillAreaPatternColor);
                        cl.aCol = (color.type == Origin::Color::Automatic
                                           ? cl.lCol
                                           : color.regular); // 0xF7 -Automatic color
                        cl.lWidth = _curve.fillAreaPatternBorderWidth;
                        linestyle = _curve.fillAreaPatternBorderStyle;
                    }
                }
                switch (linestyle) {
                case Origin::GraphCurve::Solid:
                    cl.lStyle = 0;
                    break;
                case Origin::GraphCurve::Dash:
                case Origin::GraphCurve::ShortDash:
                    cl.lStyle = 1;
                    break;
                case Origin::GraphCurve::Dot:
                case Origin::GraphCurve::ShortDot:
                    cl.lStyle = 2;
                    break;
                case Origin::GraphCurve::DashDot:
                case Origin::GraphCurve::ShortDashDot:
                    cl.lStyle = 3;
                    break;
                case Origin::GraphCurve::DashDotDot:
                    cl.lStyle = 4;
                    break;
                }

                graph->updateCurveLayout(c, &cl);
                if (style == Graph::VerticalBars || style == Graph::HorizontalBars) {
                    auto *b = dynamic_cast<BarCurve *>(graph->curve(c));
                    if (b)
                        b->setGap(qRound(100 - _curve.symbolSize * 10));
                } else if (style == Graph::Histogram) {
                    auto *h = dynamic_cast<HistogramCurve *>(graph->curve(c));
                    if (h) {
                        h->setBinning(false, layer.histogramBin, layer.histogramBegin,
                                      layer.histogramEnd);
                        h->loadData();
                    }
                }
                switch (_curve.lineConnect) {
                case Origin::GraphCurve::NoLine:
                    graph->setCurveStyle(c, QwtPlotCurve::NoCurve);
                    break;
                case Origin::GraphCurve::Straight:
                    graph->setCurveStyle(c, QwtPlotCurve::Lines);
                    break;
                case Origin::GraphCurve::BSpline:
                case Origin::GraphCurve::Bezier:
                case Origin::GraphCurve::Spline:
                    graph->setCurveStyle(c, 4);
                    break;
                case Origin::GraphCurve::StepHorizontal:
                case Origin::GraphCurve::StepHCenter:
                    graph->setCurveStyle(c, QwtPlotCurve::Steps);
                    break;
                case Origin::GraphCurve::StepVertical:
                case Origin::GraphCurve::StepVCenter:
                    graph->setCurveStyle(c, 5);
                    break;
                }
            }
            if (style == Graph::HorizontalBars) {
                graph->setScale(0, layer.xAxis.min, layer.xAxis.max, layer.xAxis.step,
                                layer.xAxis.majorTicks, layer.xAxis.minorTicks, layer.xAxis.scale);
                graph->setScale(2, layer.yAxis.min, layer.yAxis.max, layer.yAxis.step,
                                layer.yAxis.majorTicks, layer.yAxis.minorTicks, layer.yAxis.scale);
            } else {
                graph->setScale(2, layer.xAxis.min, layer.xAxis.max, layer.xAxis.step,
                                layer.xAxis.majorTicks, layer.xAxis.minorTicks, layer.xAxis.scale);
                graph->setScale(0, layer.yAxis.min, layer.yAxis.max, layer.yAxis.step,
                                layer.yAxis.majorTicks, layer.yAxis.minorTicks, layer.yAxis.scale);
            }

            // grid
            Grid *grid = graph->grid();

            grid->enableX(!layer.xAxis.majorGrid.hidden);
            grid->enableXMin(!layer.xAxis.minorGrid.hidden);
            grid->enableY(!layer.yAxis.majorGrid.hidden);
            grid->enableYMin(!layer.yAxis.minorGrid.hidden);

            grid->setMajPenX(
                    QPen(ColorButton::color(layer.xAxis.majorGrid.color),
                         ceil(layer.xAxis.majorGrid.width),
                         Graph::getPenStyle(translateOrigin2MakhberLineStyle(
                                 (Origin::GraphCurve::LineStyle)layer.xAxis.majorGrid.style))));
            grid->setMinPenX(
                    QPen(ColorButton::color(layer.xAxis.minorGrid.color),
                         ceil(layer.xAxis.minorGrid.width),
                         Graph::getPenStyle(translateOrigin2MakhberLineStyle(
                                 (Origin::GraphCurve::LineStyle)layer.xAxis.minorGrid.style))));
            grid->setMajPenY(
                    QPen(ColorButton::color(layer.yAxis.majorGrid.color),
                         ceil(layer.yAxis.majorGrid.width),
                         Graph::getPenStyle(translateOrigin2MakhberLineStyle(
                                 (Origin::GraphCurve::LineStyle)layer.yAxis.majorGrid.style))));
            grid->setMinPenY(
                    QPen(ColorButton::color(layer.yAxis.minorGrid.color),
                         ceil(layer.yAxis.minorGrid.width),
                         Graph::getPenStyle(translateOrigin2MakhberLineStyle(
                                 (Origin::GraphCurve::LineStyle)layer.yAxis.minorGrid.style))));

            grid->setAxes(2, 0);
            grid->enableZeroLineX(false);
            grid->enableZeroLineY(false);

            std::vector<Origin::GraphAxisFormat> formats;
            formats.push_back(layer.yAxis.formatAxis[0]); // left
            formats.push_back(layer.yAxis.formatAxis[1]); // right
            formats.push_back(layer.xAxis.formatAxis[0]); // bottom
            formats.push_back(layer.xAxis.formatAxis[1]); // top
            graph->setXAxisTitle(parseOriginText(decodeMbcs(formats[2].label.text.c_str())));
            graph->setYAxisTitle(parseOriginText(decodeMbcs(formats[0].label.text.c_str())));

            std::vector<Origin::GraphAxisTick> ticks;
            ticks.push_back(layer.yAxis.tickAxis[0]); // left
            ticks.push_back(layer.yAxis.tickAxis[1]); // right
            ticks.push_back(layer.xAxis.tickAxis[0]); // bottom
            ticks.push_back(layer.xAxis.tickAxis[1]); // top
            for (int i = 0; i < 4; ++i) {
                QString data(decodeMbcs(ticks[i].dataName.c_str()));
                QString tableName = data.right(data.length() - 2) + "_"
                        + decodeMbcs(ticks[i].columnName.c_str());

                int format = 0;
                Graph::AxisType type;
                int prec = ticks[i].decimalPlaces;
                switch (ticks[i].valueType) {
                case Origin::Numeric:
                    type = Graph::AxisType::Numeric;
                    switch (ticks[i].valueTypeSpecification) {
                    case 0: // Decimal 1000
                        format = 1;
                        break;
                    case 1: // Scientific
                        format = 2;
                        break;
                    case 2: // Engeneering
                    case 3: // Decimal 1,000
                        format = 0;
                        break;
                    }
                    if (prec == -1)
                        prec = 2;
                    break;
                case Origin::Text: // Text
                    type = Graph::AxisType::Txt;
                    break;
                case 2: // Date
                    type = Graph::AxisType::Date;
                    break;
                case 3: // Time
                    type = Graph::AxisType::Time;
                    break;
                case Origin::Month: // Month
                    type = Graph::AxisType::Month;
                    format = ticks[i].valueTypeSpecification;
                    break;
                case Origin::Day: // Day
                    type = Graph::AxisType::Day;
                    format = ticks[i].valueTypeSpecification;
                    break;
                case Origin::ColumnHeading:
                    type = Graph::AxisType::ColHeader;
                    switch (ticks[i].valueTypeSpecification) {
                    case 0: // Decimal 1000
                        format = 1;
                        break;
                    case 1: // Scientific
                        format = 2;
                        break;
                    case 2: // Engeneering
                    case 3: // Decimal 1,000
                        format = 0;
                        break;
                    }
                    prec = 2;
                    break;
                default:
                    type = Graph::AxisType::Numeric;
                    format = 0;
                    prec = 2;
                }

                graph->showAxis(i, type, tableName, mw->table(tableName), !(formats[i].hidden),
                                tickTypeMap[formats[i].majorTicksType],
                                tickTypeMap[formats[i].minorTicksType], !(ticks[i].showMajorLabels),
                                ColorButton::color(formats[i].color), format, prec,
                                ticks[i].rotation, 0, "",
                                (ticks[i].color == 0xF7 ? ColorButton::color(formats[i].color)
                                                        : ColorButton::color(ticks[i].color)));
            }

            graph->setAutoscaleFonts(mw->autoScaleFonts); // restore user defined fonts behaviour
            graph->setIgnoreResizeEvents(!mw->autoResizeLayers);
        }
        // cascade the graphs
        if (!_graph.hidden) {
            int dx = 20;
            int dy = ml->frameGeometry().height() - ml->height();
            ml->move(QPoint(visible_count * dx + xoffset * OBJECTXOFFSET, visible_count * dy));
            visible_count++;
            ml->show();
            ml->arrangeLayers(true, true);
        } else {
            ml->show();
            ml->arrangeLayers(true, true);
            mw->hideWindow(ml);
        }
    }
    if (visible_count > 0)
        xoffset++;
    return true;
}

QString ImportOPJ::parseOriginText(const QString &str)
{
    QStringList lines = str.split("\n");
    QString text = "";
    for (int i = 0; i < lines.size(); ++i) {
        if (i > 0)
            text.append("\n");
        text.append(parseOriginTags(lines[i]));
    }
    return text;
}

QString ImportOPJ::parseOriginTags(const QString &str)
{
    QString line = str;
    // replace \l(...) and %(...) tags
    QRegularExpression rxline(R"(\\\s*l\s*\(\s*\d+\s*\))");
    auto matchrxline = rxline.match(line);
    int pos = matchrxline.capturedStart();
    while (pos > -1) {
        QString value = matchrxline.captured(0);
        int len = value.length();
        value.replace(QRegularExpression(" "), "");
        value = "\\c{" + value.mid(3, value.length() - 4) + "}";
        line.replace(pos, len, value);
        matchrxline = rxline.match(line);
        pos = matchrxline.capturedStart();
    }
    // Lookbehind conditions are not supported - so need to reverse string
    QRegularExpression rx(R"(\)[^\)\(]*\((?!\s*[buig\+\-]\s*\\))");
    QRegularExpression rxfont(R"(\)[^\)\(]*\((?![^\:]*\:f\s*\\))");
    QString linerev = strreverse(line);
    QString lBracket = strreverse("&lbracket;");
    QString rBracket = strreverse("&rbracket;");
    QString ltagBracket = strreverse("&ltagbracket;");
    QString rtagBracket = strreverse("&rtagbracket;");
    auto matchrx = rx.match(linerev);
    int pos1 = matchrx.capturedStart();
    auto matchrxfont = rxfont.match(linerev);
    int pos2 = matchrxfont.capturedStart();

    while (pos1 > -1 || pos2 > -1) {
        if (pos1 == pos2) {
            QString value = matchrx.captured(0);
            int len = value.length();
            value = rBracket + value.mid(1, len - 2) + lBracket;
            linerev.replace(pos1, len, value);
        } else if ((pos1 > pos2 && pos2 != -1) || pos1 == -1) {
            QString value = matchrxfont.captured(0);
            int len = value.length();
            value = rtagBracket + value.mid(1, len - 2) + ltagBracket;
            linerev.replace(pos2, len, value);
        } else if ((pos2 > pos1 && pos1 != -1) || pos2 == -1) {
            QString value = matchrx.captured(0);
            int len = value.length();
            value = rtagBracket + value.mid(1, len - 2) + ltagBracket;
            linerev.replace(pos1, len, value);
        }

        matchrx = rx.match(linerev);
        pos1 = matchrx.capturedStart();
        matchrxfont = rxfont.match(linerev);
        pos2 = matchrxfont.capturedStart();
    }
    linerev.replace(ltagBracket, "(");
    linerev.replace(rtagBracket, ")");

    line = strreverse(linerev);

    // replace \b(...), \i(...), \u(...), \g(...), \+(...), \-(...), \f:font(...) tags
    std::array<QString, 7> rxstr = { R"(\\\s*b\s*\()",     R"(\\\s*i\s*\()",  R"(\\\s*u\s*\()",
                                     R"(\\\s*g\s*\()",     R"(\\\s*\+\s*\()", R"(\\\s*\-\s*\()",
                                     R"(\\\s*f\:[^\(]*\()" };
    std::array<int, 7> postag = { 0, 0, 0, 0, 0, 0, 0 };
    std::array<QString, 7> ltag = { "<b>",   "<i>",   "<u>",           "<font face=Symbol>",
                                    "<sup>", "<sub>", "<font face=%1>" };
    std::array<QString, 7> rtag = {
        "</b>", "</i>", "</u>", "</font>", "</sup>", "</sub>", "</font>"
    };
    std::array<QRegularExpression, 7> rxtags;
    for (int i = 0; i < 7; ++i)
        rxtags[i].setPattern(rxstr[i] + R"([^\(\)]*\))");

    bool flag = true;
    while (flag) {
        for (int i = 0; i < 7; ++i) {
            auto matchrxtags = rxtags[i].match(line);
            postag[i] = matchrxtags.capturedStart();
            while (postag[i] > -1) {
                QString value = matchrxtags.captured(0);
                int len = value.length();
                int pos2 = value.indexOf("(");
                if (i < 6)
                    value = ltag[i] + value.mid(pos2 + 1, len - pos2 - 2) + rtag[i];
                else {
                    int posfont = value.indexOf("f:");
                    value = ltag[i].arg(value.mid(posfont + 2, pos2 - posfont - 2))
                            + value.mid(pos2 + 1, len - pos2 - 2) + rtag[i];
                }
                line.replace(postag[i], len, value);
                matchrxtags = rxtags[i].match(line);
                postag[i] = matchrxtags.capturedStart();
            }
        }
        flag = false;
        for (const auto &rxtag : rxtags) {
            if (rxtag.match(line).capturedStart() > -1) {
                flag = true;
                break;
            }
        }
    }

    // replace unclosed tags
    for (int i = 0; i < 6; ++i)
        line.replace(QRegularExpression(rxstr[i]), ltag[i]);
    rxfont.setPattern(rxstr[6]);
    matchrxfont = rxfont.match(line);
    pos = matchrxfont.capturedStart();
    while (pos > -1) {
        QString value = matchrxfont.captured(0);
        int len = value.length();
        int posfont = value.indexOf("f:");
        value = ltag[6].arg(value.mid(posfont + 2, len - posfont - 3));
        line.replace(pos, len, value);
        matchrxfont = rxfont.match(line);
        pos = matchrxfont.capturedStart();
    }

    line.replace("&lbracket;", "(");
    line.replace("&rbracket;", ")");

    return line;
}

// TODO: bug in grid dialog
//		scale/minor ticks checkbox
//		histogram: autobin export
//		if prec not setted - automac+4digits
