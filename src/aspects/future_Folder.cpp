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
    writeCommentElement(jsObject);

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

bool Folder::load(QJsonObject *reader)
{
    Q_ASSERT(reader->value("name").toString() == "folder");
    setComment("");
    removeAllChildAspects();

    readBasicAttributes(reader);
    readCommentElement(reader);
    QJsonArray jsChildren = reader->value("children").toArray();
    for (int i = 0; i < jsChildren.size(); i++) {
        QJsonObject jsChild = jsChildren.at(i).toObject();
        readChildAspectElement(&jsChild);
    }

    return true;
}

bool Folder::readChildAspectElement(QJsonObject *reader)
{
    bool loaded = false;
    QString element_name = reader->value("name").toString();
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
            JsonElementAspectMaker *maker = qobject_cast<JsonElementAspectMaker *>(plugin);
            if (maker && maker->canCreate(element_name)) {
                AbstractAspect *aspect = maker->createAspectFromJson(reader);
                if (aspect) {
                    addChild(aspect);
                    loaded = true;
                    break;
                } else {
                    return false;
                }
            }
        }
    }
    if (!loaded)
        return false;
    else
        return true;
}

} // namespace
