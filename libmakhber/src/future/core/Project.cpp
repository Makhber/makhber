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
#include "core/Project.h"
#include "core/interfaces.h"
#include "globals.h"
#include "lib/XmlStreamReader.h"
#include "core/ProjectConfigPage.h"
#include <QUndoStack>
#include <QString>
#include <QKeySequence>
#include <QMenu>
#include <QPluginLoader>
#include <QComboBox>
#include <QFile>
#include <QXmlStreamWriter>
#include <QtDebug>

#define NOT_IMPL (QMessageBox::information(0, "info", "not yet implemented"))

class Project::Private
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

Project::Project() : future::Folder(tr("Unnamed")), d(new Private())
{
}

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

void Project::save(QXmlStreamWriter *writer) const
{
    writer->writeStartDocument();
    writer->writeStartElement("makhber_project");
    writer->writeAttribute("version", QString::number(Makhber::version()));
    // TODO: write project attributes
    writer->writeStartElement("project_root");
    future::Folder::save(writer);
    writer->writeEndElement(); // "project_root"
    writer->writeEndElement(); // "makhber_project"
    writer->writeEndDocument();
}

bool Project::load(XmlStreamReader *reader)
{
    while (!(reader->isStartDocument() || reader->atEnd()))
        reader->readNext();
    if (!(reader->atEnd())) {
        if (!reader->skipToNextTag())
            return false;

        if (reader->name() == "makhber_project") {
            bool ok = false;
            reader->readAttributeInt("version", &ok);
            if (!ok) {
                reader->raiseError(tr("invalid or missing project version"));
                return false;
            }

            // version dependent staff goes here

            while (!reader->atEnd()) {
                reader->readNext();

                if (reader->isEndElement())
                    break;

                if (reader->isStartElement()) {
                    if (reader->name() == "project_root") {
                        if (!reader->skipToNextTag())
                            return false;
                        if (!future::Folder::load(reader))
                            return false;
                        if (!reader->skipToNextTag())
                            return false;
                        Q_ASSERT(reader->isEndElement() && reader->name() == "project_root");
                    } else // unknown element
                    {
                        reader->raiseWarning(
                                tr("unknown element '%1'").arg(reader->name().toString()));
                        if (!reader->skipToEndElement())
                            return false;
                    }
                }
            }
        } else // no project element
            reader->raiseError(tr("no makhber_project element found"));
    } else // no start document
        reader->raiseError(tr("no valid XML document found"));

    return !reader->hasError();
}
