/***************************************************************************
    File                 : TranslateCurveTool.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Plot tool for translating curves.

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
#include "TranslateCurveTool.h"

#include "plot2D/Graph.h"
#include "plot2D/PlotCurve.h"
#include "plot2D/FunctionCurve.h"
#include "plot2D/DataPickerTool.h"
#include "plot2D/ScreenPickerTool.h"
#include "core/ApplicationWindow.h"
#include "aspects/column/Column.h"

#include <qwt_plot_curve.h>

#include <QMessageBox>
#include <QLocale>

#include <stdexcept>
#include <cmath>

using namespace std;

TranslateCurveTool::TranslateCurveTool(Graph *graph, ApplicationWindow *app, Direction dir,
                                       const QObject *status_target, const char *status_slot)
    : PlotToolInterface(graph), d_dir(dir), d_app(app)
{
    if (status_target)
        connect(this, SIGNAL(statusText(const QString &)), status_target, status_slot);
    // d_graph->plotWidget()->canvas()->setCursor(QCursor(QPixmap(vizor_xpm), -1, -1));
    // emit statusText(tr("Double-click on plot to select a data point!"));

    // Phase 1: select curve point
    d_sub_tool = new DataPickerTool(d_graph, app, DataPickerTool::Display, this,
                                    SIGNAL(statusText(const QString &)));
    connect(dynamic_cast<DataPickerTool *>(d_sub_tool), SIGNAL(selected(QwtPlotCurve *, int)), this,
            SLOT(selectCurvePoint(QwtPlotCurve *, int)));
}

void TranslateCurveTool::selectCurvePoint(QwtPlotCurve *curve, int point_index)
{
    d_selected_curve = curve;
    d_curve_point = QPointF(curve->sample(point_index).x(), curve->sample(point_index).y());
    delete d_sub_tool;

    // Phase 2: select destination
    d_sub_tool = new ScreenPickerTool(d_graph, this, SIGNAL(statusText(const QString &)));
    connect(dynamic_cast<ScreenPickerTool *>(d_sub_tool), SIGNAL(selected(const QPointF &)), this,
            SLOT(selectDestination(const QPointF &)));
    Q_EMIT statusText(tr("Curve selected! Move cursor and click to choose a point and "
                         "double-click/press 'Enter' to finish!"));
}

void TranslateCurveTool::selectDestination(const QPointF &point)
{
    delete d_sub_tool;
    if (!d_selected_curve)
        return;

    // Phase 3: execute the translation

    if ((dynamic_cast<PlotCurve *>(d_selected_curve))->type() == Graph::Function) {
        if (d_dir == Horizontal) {
            QMessageBox::warning(d_graph, tr("Warning"),
                                 tr("This operation cannot be performed on function curves."));
        } else {
            auto *func = dynamic_cast<FunctionCurve *>(d_selected_curve);
            if (func->functionType() == FunctionCurve::Normal) {
                QString formula = func->formulas().first();
                double d = point.y() - d_curve_point.y();
                if (d > 0)
                    func->setFormula(formula + "+" + QString::number(d, 'g', 15));
                else
                    func->setFormula(formula + QString::number(d, 'g', 15));
                func->loadData();
            }
        }
        d_graph->setActiveTool(nullptr);
        return;
    } else {
        auto *c = dynamic_cast<DataCurve *>(d_selected_curve);
        double d = NAN;
        QString col_name;
        switch (d_dir) {
        case Vertical: {
            col_name = c->title().text();
            d = point.y() - d_curve_point.y();
            break;
        }
        case Horizontal: {
            col_name = c->xColumnName();
            d = point.x() - d_curve_point.x();
            break;
        }
        default:
            throw runtime_error("Invalid direction");
        }
        Table *tab = d_app->table(col_name);
        if (!tab)
            return;
        int col = tab->colIndex(col_name);
        if (tab->columnType(col) != Makhber::ColumnMode::Numeric) {
            QMessageBox::warning(d_graph, tr("Warning"),
                                 tr("This operation cannot be performed on curves plotted from "
                                    "columns having a non-numerical format."));
            return;
        }

        int row_start = c->tableRow(0);
        int row_end = row_start + static_cast<int>(c->dataSize());
        for (int i = row_start; i < row_end; i++) {
            if (!tab->column(col)->isInvalid(i))
                tab->column(col)->setValueAt(i,
                                             (d_dir == Horizontal ? d_selected_curve->sample(i).x()
                                                                  : d_selected_curve->sample(i).y())
                                                     + d);
        }
        d_app->updateCurves(tab, col_name);
        d_app->modifiedProject();
        d_graph->setActiveTool(nullptr);
        // attention: I'm now deleted. Maybe there is a cleaner solution...*/
    }
}
