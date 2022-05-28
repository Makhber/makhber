/***************************************************************************
    File                 : LineProfileTool.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Plot tool for calculating intensity profiles of
                           image markers.

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
#include "LineProfileTool.h"

#include "plot2D/ImageMarker.h"
#include "plot2D/ArrowMarker.h"
#include "plot2D/Graph.h"

#include <QPoint>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <qwt_plot_canvas.h>

LineProfileTool::LineProfileTool(Graph *graph, int average_pixels)
    : QWidget(graph->plotWidget()->canvas()),
      PlotToolInterface(graph),
      d_op_start(QPoint(0, 0)),
      d_op_dp(QPoint(0, 0))
{
    // make sure we average over an odd number of pixels
    d_average_pixels = (average_pixels % 2) ? average_pixels : average_pixels + 1;
    d_target = dynamic_cast<ImageMarker *>(d_graph->selectedMarkerPtr());
    if (!d_target)
        QMessageBox::critical(d_graph->window(), tr("Pixel selection warning"),
                              "Please select an image marker first.");
    d_graph->deselectMarker();
    setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    show();
    setFocus();
}

void LineProfileTool::calculateLineProfile(const QPoint &start, const QPoint &end)
{
    QRect rect = d_target->rect();
    if (!rect.contains(start) || !rect.contains(end)) {
        QMessageBox::warning(d_graph, tr("Pixel selection warning"),
                             "Please select the end line point inside the image rectangle!");
        return;
    }

    QPoint o = d_target->origin();
    QPixmap pic = d_target->pixmap();
    QImage image = pic.toImage();

    int x1 = start.x() - o.x();
    int x2 = end.x() - o.x();
    int y1 = start.y() - o.y();
    int y2 = end.y() - o.y();

    QSize realSize = pic.size();
    QSize actualSize = d_target->size();

    if (realSize != actualSize) {
        double ratioX = (double)realSize.width() / (double)actualSize.width();
        double ratioY = (double)realSize.height() / (double)actualSize.height();
        x1 = int(x1 * ratioX);
        x2 = int(x2 * ratioX);
        y1 = int(y1 * ratioY);
        y2 = int(y2 * ratioY);
    }

    auto *pixelCol = new Column(tr("pixel"), Makhber::ColumnMode::Numeric);
    auto *xCol = new Column(tr("x"), Makhber::ColumnMode::Numeric);
    auto *yCol = new Column(tr("y"), Makhber::ColumnMode::Numeric);
    auto *intCol = new Column(tr("intensity"), Makhber::ColumnMode::Numeric);
    pixelCol->setPlotDesignation(Makhber::X);
    xCol->setPlotDesignation(Makhber::Y);
    yCol->setPlotDesignation(Makhber::Y);
    intCol->setPlotDesignation(Makhber::Y);

    // uses the fast Bresenham's line-drawing algorithm
#define sgn(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0))
    int i = 0, dx = 0, dy = 0, sdx = 0, sdy = 0, dxabs = 0, dyabs = 0, x = 0, y = 0, px = 0, py = 0;

    dx = x2 - x1; // the horizontal distance of the line
    dy = y2 - y1; // the vertical distance of the line
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    if (dxabs >= dyabs) // the line is more horizontal than vertical
    {
        for (i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;

            pixelCol->setValueAt(i, i);
            xCol->setValueAt(i, px);
            yCol->setValueAt(i, py);
            intCol->setValueAt(i, averageImagePixel(image, px, py, true));
        }
    } else // the line is more vertical than horizontal
    {
        for (i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;

            pixelCol->setValueAt(i, i);
            xCol->setValueAt(i, px);
            yCol->setValueAt(i, py);
            intCol->setValueAt(i, averageImagePixel(image, px, py, false));
        }
    }
    QString caption = tr("Line profile %1").arg(1);
    Q_EMIT createTablePlot(caption, QString(),
                           QList<Column *>() << pixelCol << xCol << yCol << intCol);
}

int LineProfileTool::averageImagePixel(const QImage &image, int px, int py, bool moreHorizontal)
{
    QRgb pixel = 0;
    int sum = 0, start = 0, i = 0;
    int middle = int(0.5 * (d_average_pixels - 1));
    if (moreHorizontal) {
        start = py - middle;
        for (i = 0; i < d_average_pixels; i++) {
            pixel = image.pixel(px, start + i);
            sum += qGray(pixel);
        }
    } else {
        start = px - middle;
        for (i = 0; i < d_average_pixels; i++) {
            pixel = image.pixel(start + i, py);
            sum += qGray(pixel);
        }
    }
    return sum / d_average_pixels;
}

void LineProfileTool::addLineMarker(const QPoint &start, const QPoint &end)
{
    auto *mrk = new ArrowMarker();
    mrk->attach(d_graph->plotWidget());

    mrk->setStartPoint(start);
    mrk->setEndPoint(end);
    mrk->setColor(Qt::red);
    mrk->setWidth(1);
    mrk->setStyle(Qt::SolidLine);
    mrk->drawEndArrow(false);
    mrk->drawStartArrow(false);

    d_graph->addArrow(mrk);
    mrk->detach();
    d_graph->replot();
}

void LineProfileTool::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    p.drawLine(d_op_start, d_op_start + d_op_dp);
}

void LineProfileTool::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    d_op_start = e->pos();
    e->accept();
}

void LineProfileTool::mouseMoveEvent(QMouseEvent *e)
{
    d_op_dp = e->pos() - d_op_start;
    repaint();
    e->accept();
}

void LineProfileTool::mouseReleaseEvent(QMouseEvent *e)
{
    calculateLineProfile(d_op_start, e->pos());
    addLineMarker(d_op_start, e->pos());
    d_graph->setActiveTool(nullptr);
    // attention: I'm now deleted
}
