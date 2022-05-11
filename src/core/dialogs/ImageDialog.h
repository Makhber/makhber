/***************************************************************************
    File                 : ImageDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Image geometry dialog

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
#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include "core/MakhberDefs.h"

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

//! Image geometry dialog
class MAKHBER_EXPORT ImageDialog : public QDialog
{
    Q_OBJECT

public:
    ImageDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    ~ImageDialog() {};

    void setOrigin(const QPoint &o);
    void setSize(const QSize &size);

protected Q_SLOTS:
    void accept();
    void update();
    void adjustHeight(int width);
    void adjustWidth(int height);

Q_SIGNALS:
    void setImageGeometry(int, int, int, int);

protected:
    double aspect_ratio {};

private:
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QSpinBox *boxX, *boxY, *boxWidth, *boxHeight;
    QCheckBox *keepRatioBox;
};

#endif // IMAGEDIALOG_H
