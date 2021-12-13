/***************************************************************************
    File                 : Note.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Benkert,
                                          Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Notes window class

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
#include "Note.h"

#include "scripting/ScriptEdit.h"

#include <QDateTime>
#include <QLayout>
#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QPaintDevice>
#include <QVBoxLayout>
#include <QPrintDialog>
#include <QJsonObject>

Note::Note(ScriptingEnv *env, const QString &label, QWidget *parent, const char *name,
           Qt::WindowFlags f)
    : MyWidget(label, parent, name, f)
{
    init(env);
}

void Note::init(ScriptingEnv *env)
{
    autoExec = false;
    QDateTime dt = QDateTime::currentDateTime();
    setBirthDate(QLocale::c().toString(dt, "dd-MM-yyyy hh:mm:ss:zzz"));

    te = new ScriptEdit(env, this, name());
    te->setContext(this);
    this->setWidget(te);

    setGeometry(0, 0, 500, 200);
    connect(te, SIGNAL(textChanged()), this, SLOT(modifiedNote()));
}

void Note::modifiedNote()
{
    Q_EMIT modifiedWindow(this);
}

void Note::saveToJson(QJsonObject *jsObject, const QJsonObject &jsGeometry)
{
    jsObject->insert("name", name());
    jsObject->insert("creationDate", birthDate());
    jsObject->insert("windowLabel", windowLabel());
    jsObject->insert("captionPolicy", captionPolicy());
    jsObject->insert("autoExec", autoExec);
    jsObject->insert("content", te->toPlainText().trimmed());
    jsObject->insert("geometry", jsGeometry);
    jsObject->insert("type", "Note");
}

void Note::restore(QJsonObject *jsNote)
{
    setAutoexec(jsNote->value("autoExec").toBool());
    te->insertPlainText(jsNote->value("content").toString());
}

void Note::setAutoexec(bool exec)
{
    autoExec = exec;
    QPalette palette;
    if (autoExec)
        palette.setColor(te->backgroundRole(), QColor(255, 239, 185));
    te->setPalette(palette);
}
