/***************************************************************************
    File                 : Folder.h
    Project              : Makhber
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke*gmx.de)
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
#ifndef FUTURE_FOLDER_H
#define FUTURE_FOLDER_H

#include "aspects/AbstractAspect.h"

namespace future {
//! Folder in a project
class MAKHBER_EXPORT Folder : public AbstractAspect
{
    Q_OBJECT

public:
    Folder(const QString &name);

    virtual ~Folder();

    virtual QIcon icon() const;
    //! Return a new context menu.
    /**
     * The caller takes ownership of the menu.
     */
    virtual QMenu *createContextMenu() const;

    //! \name serialize/deserialize
    //@{
    //! Save
    virtual void save(QJsonObject *) const;
    //! Load
    virtual bool load(QJsonObject *);

protected:
    //! Read child aspect
    bool readChildAspectElement(QJsonObject *reader);
    //@}
};

} // namespace
#endif // ifndef FUTURE_FOLDER_H
