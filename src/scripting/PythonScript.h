/***************************************************************************
        File                 : PythonScript.h
        Project              : Makhber
--------------------------------------------------------------------
        Copyright            : (C) 2006 by Knut Franke
        Email (use @ for *)  : knut.franke*gmx.de
        Description          : Execute Python code from within Makhber

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
#ifndef PYTHON_SCRIPT_H
#define PYTHON_SCRIPT_H

#include "core/MakhberDefs.h"
#include "scripting/Script.h"

class QString;
class QObject;

typedef struct _object PyObject;
class PythonScripting;
class ScriptingEnv;

class MAKHBER_EXPORT PythonScript : public Script
{
    Q_OBJECT

public:
    PythonScript(PythonScripting *env, const QString &code, QObject *context = 0,
                 const QString &name = "<input>");
    ~PythonScript();

    void write(const QString &text) { Q_EMIT print(text); }

public Q_SLOTS:
    bool compile(bool for_eval = true) override;
    QVariant eval() override;
    bool exec() override;
    bool setQObject(QObject *val, const char *name) override;
    bool setInt(int val, const char *name) override;
    bool setDouble(double val, const char *name) override;
    void setContext(QObject *context) override;

private:
    PythonScripting *env() { return (PythonScripting *)Env; }
    void beginStdoutRedirect();
    void endStdoutRedirect();

    PyObject *PyCode, *modLocalDict, *modGlobalDict, *stdoutSave, *stderrSave;
    bool isFunction, hasOldGlobals;
};

#endif
