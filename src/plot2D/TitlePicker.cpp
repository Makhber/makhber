/***************************************************************************
    File                 : TitlePicker.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Title picker

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
#include "TitlePicker.h"

#include <qwt_plot.h>
#include <qwt_text_label.h>

#include <QMouseEvent>
#include <QPen>

TitlePicker::TitlePicker(QwtPlot *plot) : QObject(plot)
{
    d_selected = false;
    title = dynamic_cast<QwtTextLabel *>(plot->titleLabel());
    if (title) {
        title->setFocusPolicy(Qt::StrongFocus);
        title->installEventFilter(this);
    }
}

bool TitlePicker::eventFilter(QObject *object, QEvent *e)
{
    if (object != dynamic_cast<QObject *>(title))
        return false;

    if (object->inherits("QwtTextLabel") && e->type() == QEvent::MouseButtonDblClick) {
        Q_EMIT doubleClicked();
        d_selected = true;
        return true;
    }

    if (object->inherits("QwtTextLabel") && e->type() == QEvent::MouseButtonPress) {
        const auto *me = dynamic_cast<const QMouseEvent *>(e);
        Q_EMIT clicked();

        if (me->button() == Qt::RightButton)
            Q_EMIT showTitleMenu();
        return !(me->modifiers() & Qt::ShiftModifier);
    }

    if (object->inherits("QwtTextLabel") && e->type() == QEvent::KeyPress) {
        switch ((dynamic_cast<const QKeyEvent *>(e))->key()) {
        case Qt::Key_Delete:
            Q_EMIT removeTitle();
            return true;
        }
    }

    return QObject::eventFilter(object, e);
}

void TitlePicker::setSelected(bool select)
{
    if (!title || d_selected == select)
        return;

    d_selected = select;

    QwtText text = title->text();
    if (select)
        text.setBackgroundBrush(QBrush(Qt::blue));
    else
        text.setBackgroundBrush(QBrush());

    (dynamic_cast<QwtPlot *>(parent()))->setTitle(text);
}
