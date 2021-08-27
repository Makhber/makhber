/***************************************************************************
    File                 : AspectPrivate.cpp
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
#include "AspectPrivate.h"

#include "aspects/AbstractAspect.h"

#include <QRegularExpression>
#include <QStringList>

#include <stdexcept>

QHash<QString, QVariant> AbstractAspect::Private::g_defaults;

AbstractAspect::Private::Private(AbstractAspect *owner, const QString &name)
    : d_name(name.isEmpty() ? "1" : name),
      d_caption_spec("%n%C{ - }%c"),
      d_owner(owner),
      d_parent(nullptr)
{
    d_creation_time = QDateTime::currentDateTime();
}

AbstractAspect::Private::~Private()
{
    for (AbstractAspect *child : d_children)
        delete child;
}

void AbstractAspect::Private::addChild(AbstractAspect *child)
{
    insertChild(d_children.count(), child);
}

void AbstractAspect::Private::insertChild(int index, AbstractAspect *child)
{
    Q_EMIT d_owner->aspectAboutToBeAdded(d_owner, index);
    d_children.insert(index, child);
    // Always remove from any previous parent before adding to a new one!
    // Can't handle this case here since two undo commands have to be created.
    Q_ASSERT(child->d_aspect_private->d_parent == nullptr);
    child->d_aspect_private->d_parent = d_owner;
    connect(child, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)), d_owner,
            SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)));
    connect(child, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), d_owner,
            SIGNAL(aspectDescriptionChanged(const AbstractAspect *)));
    connect(child, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, int)), d_owner,
            SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, int)));
    connect(child, SIGNAL(aspectChildAboutToBeRemoved(const AbstractAspect *, int)), d_owner,
            SIGNAL(aspectChildAboutToBeRemoved(const AbstractAspect *, int)));
    connect(child, SIGNAL(aspectChildAdded(const AbstractAspect *, int)), d_owner,
            SIGNAL(aspectChildAdded(const AbstractAspect *, int)));
    connect(child, SIGNAL(aspectRemoved(const AbstractAspect *, int)), d_owner,
            SIGNAL(aspectRemoved(const AbstractAspect *, int)));
    connect(child, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)), d_owner,
            SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)));
    connect(child, SIGNAL(aspectAdded(const AbstractAspect *)), d_owner,
            SIGNAL(aspectAdded(const AbstractAspect *)));
    connect(child, SIGNAL(statusInfo(const QString &)), d_owner,
            SIGNAL(statusInfo(const QString &)));
    Q_EMIT d_owner->aspectChildAdded(d_owner, index);
    Q_EMIT child->aspectAdded(child);
}

int AbstractAspect::Private::indexOfChild(const AbstractAspect *child) const
{
    for (int i = 0; i < d_children.size(); i++)
        if (d_children.at(i) == child)
            return i;
    return -1;
}

int AbstractAspect::Private::removeChild(AbstractAspect *child)
{
    int index = indexOfChild(child);
    Q_ASSERT(index != -1);
    Q_EMIT d_owner->aspectChildAboutToBeRemoved(d_owner, index);
    Q_EMIT child->aspectAboutToBeRemoved(child);
    d_children.removeAll(child);
    QObject::disconnect(child, nullptr, d_owner, nullptr);
    child->d_aspect_private->d_parent = nullptr;
    Q_EMIT d_owner->aspectRemoved(d_owner, index);
    return index;
}

int AbstractAspect::Private::childCount() const
{
    return d_children.count();
}

AbstractAspect *AbstractAspect::Private::child(int index)
{
    Q_ASSERT(index >= 0 && index <= childCount());
    return d_children.at(index);
}

QString AbstractAspect::Private::name() const
{
    return d_name;
}

void AbstractAspect::Private::setName(const QString &value)
{
    Q_EMIT d_owner->aspectDescriptionAboutToChange(d_owner);
    d_name = value;
    Q_EMIT d_owner->aspectDescriptionChanged(d_owner);
}

QString AbstractAspect::Private::comment() const
{
    return d_comment;
}

void AbstractAspect::Private::setComment(const QString &value)
{
    Q_EMIT d_owner->aspectDescriptionAboutToChange(d_owner);
    d_comment = value;
    Q_EMIT d_owner->aspectDescriptionChanged(d_owner);
}

QString AbstractAspect::Private::captionSpec() const
{
    return d_caption_spec;
}

void AbstractAspect::Private::setCaptionSpec(const QString &value)
{
    Q_EMIT d_owner->aspectDescriptionAboutToChange(d_owner);
    d_caption_spec = value;
    Q_EMIT d_owner->aspectDescriptionChanged(d_owner);
}

void AbstractAspect::Private::setCreationTime(const QDateTime &time)
{
    d_creation_time = time;
}

int AbstractAspect::Private::indexOfMatchingBrace(const QString &str, int start)
{
    int result = str.indexOf('}', start);
    if (result < 0)
        result = start;
    return result;
}

QString AbstractAspect::Private::caption() const
{
    QString result = d_caption_spec;
    QRegularExpression magic("%(.)");
    QRegularExpressionMatch match = magic.match(result);
    for (int pos = match.capturedStart(); pos >= 0;) {
        QString replacement;
        int length = 0;
        switch (match.captured(1).at(0).toLatin1()) {
        case '%':
            replacement = "%";
            length = 2;
            break;
        case 'n':
            replacement = d_name;
            length = 2;
            break;
        case 'c':
            replacement = d_comment;
            length = 2;
            break;
        case 't':
            replacement = d_creation_time.toString();
            length = 2;
            break;
        case 'C':
            length = indexOfMatchingBrace(result, pos) - pos + 1;
            replacement = d_comment.isEmpty() ? "" : result.mid(pos + 3, length - 4);
            break;
        default:
            throw std::runtime_error("Invalid magic cap");
        }
        result.replace(pos, length, replacement);
        pos += replacement.size();
        match = magic.match(result, pos);
        pos = match.capturedStart();
    }
    return result;
}

QDateTime AbstractAspect::Private::creationTime() const
{
    return d_creation_time;
}

QString AbstractAspect::Private::uniqueNameFor(const QString &current_name) const
{
    QStringList child_names;
    for (AbstractAspect *child : d_children)
        child_names << child->name();

    if (!child_names.contains(current_name))
        return current_name;

    QString base = current_name;
    int last_non_digit = 0;
    for (last_non_digit = base.size() - 1;
         last_non_digit >= 0 && base[last_non_digit].category() == QChar::Number_DecimalDigit;
         --last_non_digit)
        base.chop(1);
    if (last_non_digit >= 0 && base[last_non_digit].category() != QChar::Separator_Space)
        base.append(" ");

    int new_nr = current_name.right(current_name.size() - base.size()).toInt();
    QString new_name;
    do
        new_name = base + QString::number(++new_nr);
    while (child_names.contains(new_name));

    return new_name;
}
