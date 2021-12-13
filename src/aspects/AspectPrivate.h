/***************************************************************************
    File                 : AspectPrivate.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Private data managed by AbstractAspect.

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
#ifndef ASPECT_PRIVATE_H
#define ASPECT_PRIVATE_H

#include "aspects/AbstractAspect.h"
#include "core/ApplicationWindow.h"

#include <QString>
#include <QDateTime>
#include <QList>
#include <QHash>

//! Private data managed by AbstractAspect.
class MAKHBER_EXPORT AbstractAspect::Private
{
public:
    Private(AbstractAspect *owner, const QString &name);
    ~Private();

    void addChild(AbstractAspect *child);
    void insertChild(int index, AbstractAspect *child);
    int indexOfChild(const AbstractAspect *child) const;
    int removeChild(AbstractAspect *child);
    int childCount() const;
    AbstractAspect *child(int index);

    QString name() const;
    void setName(const QString &value);
    QString comment() const;
    void setComment(const QString &value);
    QString captionSpec() const;
    void setCaptionSpec(const QString &value);
    QDateTime creationTime() const;
    void setCreationTime(const QDateTime &time);

    QString caption() const;
    AbstractAspect *owner() { return d_owner; }
    AbstractAspect *parent() { return d_parent; }

    QString uniqueNameFor(const QString &current_name) const;

    static QHash<QString, QVariant> g_defaults;

private:
    static int indexOfMatchingBrace(const QString &str, int start);
    QList<AbstractAspect *> d_children;
    QString d_name, d_comment, d_caption_spec;
    QDateTime d_creation_time;
    AbstractAspect *d_owner;
    AbstractAspect *d_parent;
};

#endif // ifndef ASPECT_PRIVATE_H
