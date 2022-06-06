/***************************************************************************
    File                 : ScriptingEnv.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Implementations of generic scripting classes

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
#include "ScriptingEnv.h"

#include "scripting/Script.h"

#ifdef SCRIPTING_MUPARSER
#    include "scripting/MuParserScript.h"
#    include "scripting/MuParserScripting.h"
#endif
#ifdef SCRIPTING_PYTHON
#    include "scripting/PythonScript.h"
#    include "scripting/PythonScripting.h"
#endif

#include <cstring>

ScriptingEnv::ScriptingEnv(ApplicationWindow *parent, const char *langName)
    : QObject(nullptr), d_parent(parent)
{
    setObjectName(langName);
    d_initialized = false;
    d_refcount = 0;
}

const QString ScriptingEnv::fileFilter() const
{
    QStringList extensions = fileExtensions();
    if (extensions.isEmpty())
        return "";
    else
        return tr("%1 Source (*.%2);;").arg(objectName(), extensions.join(" *."));
}

void ScriptingEnv::incref()
{
    d_refcount++;
}

void ScriptingEnv::decref()
{
    d_refcount--;
    if (d_refcount == 0)
        delete this;
}
