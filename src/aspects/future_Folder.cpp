/***************************************************************************
    File                 : Folder.cpp
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
#include "future_Folder.h"

#include "aspects/Project.h"
#include "aspects/column/Column.h"
#include "lib/XmlStreamReader.h"

#include <QIcon>
#include <QApplication>
#include <QStyle>
#include <QPluginLoader>
#include <QtDebug>
#include <QJsonArray>

namespace future {
Folder::Folder(const QString &name) : AbstractAspect(name) { }

Folder::~Folder() = default;

QIcon Folder::icon() const
{
    QIcon result;
    result.addFile(":/folder_closed.xpm", QSize(), QIcon::Normal, QIcon::Off);
    result.addFile(":/folder_open.xpm", QSize(), QIcon::Normal, QIcon::On);
    return result;
}

QMenu *Folder::createContextMenu() const
{
    if (project())
        return project()->createFolderContextMenu(this);
    return nullptr;
}

void Folder::save(QJsonObject *jsObject) const
{
    writeBasicAttributes(jsObject);
    // writeCommentElement(writer);

    int child_count = childCount();
    QJsonArray jsChildren {};
    for (int i = 0; i < child_count; i++) {
        QJsonObject jsChild {};
        child(i)->save(&jsChild);
        jsChildren.append(jsChild);
    }
    jsObject->insert("children", jsChildren);

    jsObject->insert("type", "Folder");
}

bool Folder::load(XmlStreamReader *reader)
{
    if (reader->isStartElement() && reader->name().toString() == "folder") {
        setComment("");
        removeAllChildAspects();

        if (!readBasicAttributes(reader))
            return false;

        // read child elements
        while (!reader->atEnd()) {
            reader->readNext();

            if (reader->isEndElement())
                break;

            if (reader->isStartElement()) {
                if (reader->name().toString() == "comment") {
                    if (!readCommentElement(reader))
                        return false;
                } else if (reader->name().toString() == "child_aspect") {
                    if (!readChildAspectElement(reader))
                        return false;
                } else // unknown element
                {
                    reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
                    if (!reader->skipToEndElement())
                        return false;
                }
            }
        }
    } else // no folder element
        reader->raiseError(tr("no folder element found"));

    return !reader->hasError();
}

bool Folder::readChildAspectElement(XmlStreamReader *reader)
{
    bool loaded = false;
    Q_ASSERT(reader->isStartElement() && reader->name().toString() == "child_aspect");

    if (!reader->skipToNextTag())
        return false;
    if (reader->isEndElement() && reader->name().toString() == "child_aspect")
        return true; // empty element tag
    QString element_name = reader->name().toString();
    if (element_name == "folder") {
        auto *folder = new Folder(tr("Folder %1").arg(1));
        if (!folder->load(reader)) {
            delete folder;
            return false;
        }
        addChild(folder);
        loaded = true;
    } else if (element_name == "column") {
        auto *column = new Column(tr("Column %1").arg(1), Makhber::ColumnMode::Text);
        if (!column->load(reader)) {
            delete column;
            return false;
        }
        addChild(column);
        loaded = true;
    } else {
        for (QObject *plugin : QPluginLoader::staticInstances()) {
            XmlElementAspectMaker *maker = qobject_cast<XmlElementAspectMaker *>(plugin);
            if (maker && maker->canCreate(element_name)) {
                AbstractAspect *aspect = maker->createAspectFromXml(reader);
                if (aspect) {
                    addChild(aspect);
                    loaded = true;
                    break;
                } else {
                    reader->raiseError(
                            tr("creation of aspect from element '%1' failed").arg(element_name));
                    return false;
                }
            }
        }
    }
    if (!loaded) {
        reader->raiseWarning(tr("no plugin to load element '%1' found").arg(element_name));
        if (!reader->skipToEndElement())
            return false;
    }
    if (!reader->skipToNextTag())
        return false;
    Q_ASSERT(reader->isEndElement() && reader->name().toString() == "child_aspect");
    return !reader->hasError();
}

} // namespace
