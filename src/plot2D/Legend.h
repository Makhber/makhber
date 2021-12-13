/***************************************************************************
    File                 : Legend.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Legend marker (extension to QwtPlotMarker)

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
#ifndef LEGENDMARKER_H
#define LEGENDMARKER_H

#include "plot2D/Graph.h"
#include "plot2D/Plot.h"
#include "plot2D/PlotEnrichement.h"

#include <qwt_plot.h>
#include <qwt_text.h>

#include <QFont>
#include <QPen>

/**
 * \brief A piece of text to be drawn on a Plot.
 *
 * Contrary to its name, Legend is not just used for the plot legend,
 * but for any kind of text; particularly also for the "Add Text" tool.
 * Accordingly, it is also referred to as "TextMarker" by other classes.
 *
 * \section future_plans Future Plans
 * Rename to TextMarker (or maybe TextEnrichment; see documentation of ImageMarker for details).
 *
 * \sa ImageMarker, ArrowMarker
 */
class MAKHBER_EXPORT Legend : public PlotEnrichement
{
public:
    Legend(Plot *);
    ~Legend();

    //! The kinds of frame a Legend can draw around the Text.
    enum FrameStyle { None = 0, Line = 1, Shadow = 2 };

    QString text() { return d_text->text(); };
    void setText(const QString &s);

    //! Bounding rectangle in paint coordinates.
    QRect rect() const;
    //! Bounding rectangle in plot coordinates.
    virtual QRectF boundingRect() const;

    void setOrigin(const QPoint &p);

    //! Sets the position of the top left corner in axis coordinates
    void setOriginCoord(double x, double y);

    //! Keep the markers on screen each time the scales are modified by adding/removing curves
    void updateOrigin();

    QColor textColor() { return d_text->color(); };
    void setTextColor(const QColor &c);

    QColor backgroundColor() { return d_text->backgroundBrush().color(); };
    void setBackgroundColor(const QColor &c);

    int frameStyle() { return d_frame; };
    void setFrameStyle(int style);

    QFont font() { return d_text->font(); };
    void setFont(const QFont &font);

    int angle() { return d_angle; };
    void setAngle(int ang) { d_angle = ang; };

private:
    void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &r) const;

    void drawFrame(QPainter *p, int type, const QRect &rect) const;
    void drawSymbols(QPainter *p, const QRect &rect, QVector<long> height,
                     int symbolLineLength) const;
    void drawLegends(QPainter *p, const QRect &rect, QVector<long> height,
                     int symbolLineLength) const;
    void drawVector(QPainter *p, int x, int y, int l, int curveIndex) const;

    QVector<long> itemsHeight(int y, int symbolLineLength, int &width, int &height) const;
    int symbolsMaxLineLength() const;
    QString parse(const QString &str) const;

protected:
    //! Parent plot
    Plot *d_plot;

    //! Frame type
    int d_frame;

    //! Rotation angle: not implemented yet
    int d_angle;

    //! Pointer to the QwtText object
    QwtText *d_text;

    //! TopLeft position in pixels
    QPoint d_pos;

    //!Distance between symbols and legend text
    int hspace;

    //!Distance between frame and content
    int left_margin, top_margin;

    int d_shadow_size_x, d_shadow_size_y;
};

#endif
