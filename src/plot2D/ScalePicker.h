/***************************************************************************
    File                 : ScalePicker.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Scale picker

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
#ifndef SCALE_PICKER_H
#define SCALE_PICKER_H

#include "core/MakhberDefs.h"

#include <QObject>

class QRect;
class QPoint;
class QwtPlot;
class QwtScaleWidget;
class Graph;

/*!\brief Handles user interaction with a QwtScaleWidget.
 *
 * This class is used by Graph to catch events for the scales on its Plot.
 * ScalePicker doesn't take any actions beyond emitting signals, which are then processed by Graph.
 */
class MAKHBER_EXPORT ScalePicker : public QObject
{
    Q_OBJECT
public:
    explicit ScalePicker(QwtPlot *plot);

    //! Returns the bounding rectangle of a scale without the title.
    QRect scaleRect(const QwtScaleWidget *) const;

    //! Returns the bounding rectangle of a scale excluding the title and the tick labels.
    QRect scaleTicksRect(const QwtScaleWidget *scale) const;

    //! Returns the bounding rectangle of a scale's title.
    QRect titleRect(const QwtScaleWidget *scale) const;

    /*! Install myself as event filter for all axes of my parent.
     * For every axis of plot(), add myself to the corresponding QwtScaleWidget.
     * \sa QwtPlot::axisWidget()
     */
    void refresh();

    //! Return my parent casted to QwtPlot.
    QwtPlot *plot() { return (QwtPlot *)parent(); }
    Graph *graph() { return (Graph *)(parent()->parent()); }

    void deselect();

    bool titleSelected() { return d_title_selected; };
    void selectTitle(QwtScaleWidget *scale, bool select = true);

    bool labelsSelected() { return d_labels_selected; };
    void selectLabels(QwtScaleWidget *scale, bool select = true);

    /*! Returns a pointer to the selected axis in the plot layer.
     * The selected axis has selected title or selected tick labels (blue frame around texts).
     */
    QwtScaleWidget *selectedAxis() { return d_selected_axis; };
    //! Returns a pointer to the active axis in the plot layer.
    QwtScaleWidget *currentAxis() { return d_current_axis; };

Q_SIGNALS:
    //! Emitted when the user clicks on one of the monitored axes.
    void clicked();

    /*! Emitted when the user right-clicks on an axis (but not its title).
     * The argument specifies the axis' QwtScaleDraw::Alignment.
     */
    void axisRightClicked(int);
    /*! Emitted when the user right-clicks on the title of an axis.
     * The argument specifies the axis' QwtScaleDraw::Alignment.
     */
    void axisTitleRightClicked(int);

    /*! Emitted when the user double-clicks on an axis (but not its title).
     * The argument specifies the axis' QwtScaleDraw::Alignment.
     */
    void axisDblClicked(int);

    /*! Emitted when the user double-clicks on an the bottom-axis title.
     * \sa QwtScaleDraw::Alignment
     */
    void xAxisTitleDblClicked();
    /*! Emitted when the user double-clicks on an the left-axis title.
     * \sa QwtScaleDraw::Alignment
     */
    void yAxisTitleDblClicked();
    /*! Emitted when the user double-clicks on an the right-axis title.
     * \sa QwtScaleDraw::Alignment
     */
    void rightAxisTitleDblClicked();
    /*! Emitted when the user double-clicks on an the top-axis title.
     * \sa QwtScaleDraw::Alignment
     */
    void topAxisTitleDblClicked();

private:
    bool eventFilter(QObject *, QEvent *);

    void mouseDblClicked(const QwtScaleWidget *, const QPoint &);
    void mouseClicked(const QwtScaleWidget *scale, const QPoint &pos);
    void mouseRightClicked(const QwtScaleWidget *scale, const QPoint &pos);

    bool d_title_selected;
    bool d_labels_selected;
    QwtScaleWidget *d_selected_axis, *d_current_axis;
};

#endif // ifndef SCALE_PICKER_H
