/***************************************************************************
    File                 : AbstractPart.h
    Project              : Makhber
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2009 Knut Franke (knut.franke*gmx.de)
                           (replace * with @ in the email address)

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
#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "core/MakhberDefs.h"

#include "aspects/AbstractAspect.h"

class PartMdiView;
class QMenu;
class QToolBar;

//! Base class of Aspects with MDI windows as views.
/**
 * Makhber's Parts are somewhat similar to KDE's KParts in that they are independent application
 * components running on top of a kernel (a bit like KOffice's shell).
 */
class MAKHBER_EXPORT AbstractPart : public AbstractAspect
{
    Q_OBJECT

public:
    //! Constructor.
    AbstractPart(const QString &name) : AbstractAspect(name), d_mdi_window(0) { }
    //! Construct a primary view on me.
    /**
     * The caller recieves ownership of the view.
     *
     * This method may be called multiple times during the life time of a Part, or it might not get
     * called at all. Parts must not depend on the existence of a view for their operation.
     */
    virtual QWidget *view() = 0;
    //! Wrap the view() into a PartMdiView.
    /**
     * A new view is only created the first time this method is called;
     * after that, a pointer to the pre-existing view is returned.
     */
    PartMdiView *mdiSubWindow();
    //! Return AbstractAspect::createContextMenu() plus operations on the primary view.
    virtual QMenu *createContextMenu() const override;
    //! Fill the part specific menu for the main window including setting the title
    /**
     * \return true on success, otherwise false (e.g. part has no actions).
     */
    virtual bool fillProjectMenu(QMenu *menu)
    {
        Q_UNUSED(menu);
        return false;
    }
    //! Fill the part specific tool bar for the main window including setting the title
    /**
     * \return true on success, otherwise false (e.g. part has no actions to be shown in a toolbar).
     */
    virtual bool fillProjectToolBar(QToolBar *bar)
    {
        Q_UNUSED(bar);
        return false;
    }

public Q_SLOTS:
    //! Copy current selection.
    virtual void copy() {};
    //! Cut current selection.
    virtual void cut() {};
    //! Paste at the current location or into the current selection.
    virtual void paste() {};

private:
    //! The MDI sub-window that is wrapped around my primary view.
    PartMdiView *d_mdi_window;
};

#endif // ifndef ABSTRACT_PART_H
