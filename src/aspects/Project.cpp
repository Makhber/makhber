/***************************************************************************
    File                 : Project.cpp
    Project              : Makhber
    Description          : Represents a Makhber project.
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
#include "Project.h"

#include "aspects/interfaces.h"
#include "aspects/ProjectConfigPage.h"
#include "core/globals.h"

#include <QUndoStack>
#include <QString>
#include <QKeySequence>
#include <QMenu>
#include <QPluginLoader>
#include <QComboBox>
#include <QFile>
#include <QtDebug>

#define NOT_IMPL (QMessageBox::information(0, "info", "not yet implemented"))

class MAKHBER_EXPORT Project::Private
{
public:
    Private()
        : mdi_window_visibility(static_cast<MdiWindowVisibility>(
                Project::global("default_mdi_window_visibility").toInt()))
    {
    }
    ~Private() = default;
    QUndoStack undo_stack;
    MdiWindowVisibility mdi_window_visibility;
    void *primary_view { nullptr };
    AbstractScriptingEngine *scripting_engine { nullptr };
    QString file_name;
};

Project::Project() : future::Folder(tr("Unnamed")), d(new Private()) { }

Project::~Project()
{
    delete d;
}

QUndoStack *Project::undoStack() const
{
    return &d->undo_stack;
}

void *Project::view()
{
    return nullptr;
}

QMenu *Project::createContextMenu() const
{
    return nullptr;
}

QMenu *Project::createFolderContextMenu(const future::Folder *folder) const
{
    Q_UNUSED(folder)
    return nullptr;
}

void Project::setMdiWindowVisibility(MdiWindowVisibility visibility)
{
    d->mdi_window_visibility = visibility;
}

Project::MdiWindowVisibility Project::mdiWindowVisibility() const
{
    return d->mdi_window_visibility;
}

AbstractScriptingEngine *Project::scriptingEngine() const
{
    return d->scripting_engine;
}

/* ================== static methods ======================= */
ConfigPageWidget *Project::makeConfigPage()
{
    return new ProjectConfigPage();
}

QString Project::configPageLabel()
{
    return QObject::tr("General");
}

void Project::setFileName(const QString &file_name)
{
    d->file_name = file_name;
}

QString Project::fileName() const
{
    return d->file_name;
}

void Project::save(QJsonObject *jsObject) const
{
    QJsonObject jsProject {};
    jsProject.insert("version", Makhber::version());
    // TODO: write project attributes

    QJsonObject jsRoot {};
    future::Folder::save(&jsRoot);
    jsProject.insert("project_root", jsRoot);

    jsObject->insert("makhber_project", jsProject);
}

bool Project::load(QJsonObject *reader)
{
    Q_ASSERT(reader->value("name").toString() == "makhber_project");

    QJsonObject jsRoot = reader->value("project_root").toObject();
    future::Folder::load(&jsRoot);

    return true;
}
