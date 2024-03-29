/***************************************************************************
        File                 : PythonScripting.cpp
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

#include "PythonScripting.h"

#include "scripting/PythonScript.h"
#include "core/ApplicationWindow.h"

// get rid of a compiler warning
#ifdef _POSIX_C_SOURCE
#    undef _POSIX_C_SOURCE
#endif
#include <Python.h>
#if PY_VERSION_HEX < 0x03090000
#    include <frameobject.h>
#endif

#include "sip.h"

#include <QObject>
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

#include <iostream>

#define str(x) xstr(x)
#define xstr(x) #x

const char *PythonScripting::langName = "Python";

QString PythonScripting::toString(PyObject *object, bool decref)
{
    QString ret;
    if (!object)
        return "";
    PyObject *repr = PyObject_Str(object);
    if (decref)
        Py_DECREF(object);
    if (!repr)
        return "";
    ret = PyUnicode_AsUTF8(repr);
    Py_DECREF(repr);
    return ret;
}

PyObject *PythonScripting::eval(const QString &code, PyObject *argDict, const char *name)
{
    PyObject *args;
    if (argDict)
        args = argDict;
    else
        args = globals;
    PyObject *ret = NULL;
    PyObject *co = Py_CompileString(code.toUtf8().constData(), name, Py_eval_input);
    if (co) {
        ret = PyEval_EvalCode(co, globals, args);
        Py_DECREF(co);
    }
    return ret;
}

bool PythonScripting::exec(const QString &code, PyObject *argDict, const char *name)
{
    PyObject *args;
    if (argDict)
        args = argDict;
    else
        args = globals;
    PyObject *tmp = NULL;
    PyObject *co = Py_CompileString(code.toUtf8().constData(), name, Py_file_input);
    if (co) {
        tmp = PyEval_EvalCode(co, globals, args);
        Py_DECREF(co);
    }
    if (!tmp)
        return false;
    Py_DECREF(tmp);
    return true;
}

QString PythonScripting::errorMsg()
{
    PyObject *exception = 0, *value = 0, *traceback = 0;
    PyTracebackObject *excit = 0;
    PyFrameObject *frame;
    const char *fname;
    QString msg;
    if (!PyErr_Occurred())
        return "";

    PyErr_Fetch(&exception, &value, &traceback);
    PyErr_NormalizeException(&exception, &value, &traceback);
    if (PyErr_GivenExceptionMatches(exception, PyExc_SyntaxError)) {
        QString text = toString(PyObject_GetAttrString(value, "text"), true);
        msg.append(text + "\n");
        PyObject *offset = PyObject_GetAttrString(value, "offset");
        for (int i = 0; i < (PyLong_AsLong(offset) - 1); i++)
            if (text[i] == '\t')
                msg.append("\t");
            else
                msg.append(" ");
        msg.append("^\n");
        Py_DECREF(offset);
        msg.append("SyntaxError: ");
        msg.append(toString(PyObject_GetAttrString(value, "msg"), true) + "\n");
        msg.append("at ").append(toString(PyObject_GetAttrString(value, "filename"), true));
        msg.append(":").append(toString(PyObject_GetAttrString(value, "lineno"), true));
        msg.append("\n");
        Py_DECREF(exception);
        Py_DECREF(value);
    } else {
        msg.append(toString(exception, true)).remove("exceptions.").append(": ");
        msg.append(toString(value, true));
        msg.append("\n");
    }

    if (traceback) {
        excit = (PyTracebackObject *)traceback;
        while (excit && (PyObject *)excit != Py_None) {
            frame = excit->tb_frame;
#if PY_VERSION_HEX >= 0x03090000
            PyCodeObject *f_code = PyFrame_GetCode(frame);
#else
            PyCodeObject *f_code = frame->f_code;
            Py_INCREF(f_code);
#endif
            msg.append("at ").append(PyUnicode_AsUTF8(f_code->co_filename));
            msg.append(":").append(QString::number(excit->tb_lineno));
            if (f_code->co_name && *(fname = PyUnicode_AsUTF8(f_code->co_name)) != '?')
                msg.append("\n");
            excit = excit->tb_next;
            Py_DECREF(f_code);
        }
        Py_DECREF(traceback);
    }

    return msg;
}

PythonScripting::PythonScripting(ApplicationWindow *parent, bool batch)
    : ScriptingEnv(parent, langName)
{
    Q_UNUSED(batch)
    PyObject *mainmod = NULL, *makhbermod = NULL, *sysmod = NULL, *sipmod = NULL;
    math = NULL;
    sys = NULL;
    d_initialized = false;
    PyGILState_STATE gstate;
    if (Py_IsInitialized()) {
        gstate = PyGILState_Ensure();
        mainmod = PyImport_ImportModule("__main__");
        if (!mainmod) {
            QMessageBox::warning(d_parent, tr("Loading Module Failure"),
                                 tr("Failed to load __main__ module."));
            PyGILState_Release(gstate);
            PyErr_Print();
            return;
        }
        globals = PyModule_GetDict(mainmod);
        Py_DECREF(mainmod);
    } else {
        // if we need to bundle Python libraries with the executable,
        // specify the library location here
#ifdef PYTHONHOME
        Py_SetPythonHome(Py_DecodeLocale(str(PYTHONHOME), NULL));
#endif
        Py_Initialize();
        if (!Py_IsInitialized())
            return;

        gstate = PyGILState_Ensure();
        mainmod = PyImport_AddModule("__main__");
        if (!mainmod) {
            QMessageBox::warning(d_parent, tr("Loading Module Failure"),
                                 tr("Failed to load __main__ module."));
            PyGILState_Release(gstate);
            PyErr_Print();
            return;
        }
        globals = PyModule_GetDict(mainmod);
    }

    if (!globals) {
        PyErr_Print();
        PyGILState_Release(gstate);
        return;
    }
    Py_INCREF(globals);

    math = PyDict_New();
    if (!math)
        PyErr_Print();

#if QT_VERSION_MAJOR >= 6
    sipmod = PyImport_ImportModule("PyQt6.sip");
#else
#    ifdef PYQT_SIP
    sipmod = PyImport_ImportModule("PyQt5.sip");
#    else
    sipmod = PyImport_ImportModule("sip");
#    endif
#endif
    if (sipmod) {
        sip = PyModule_GetDict(sipmod);
        Py_INCREF(sip);
    } else {
        QMessageBox::warning(d_parent, tr("Loading Module Failure"),
                             tr("Failed to load sip module."));
        PyErr_Print();
    }

    makhbermod = PyImport_ImportModule("makhber");
    if (makhbermod) {
        PyDict_SetItemString(globals, "makhber", makhbermod);
        PyObject *makhberDict = PyModule_GetDict(makhbermod);
        if (!setQObject(d_parent, "app", makhberDict))
            QMessageBox::warning(
                    d_parent, tr("Failed to export Makhber API"),
                    tr("Accessing Makhber functions or objects from Python code won't work."
                       "Probably your version of SIP differs from the one Makhber was compiled "
                       "against; try updating SIP or recompiling Makhber."));
        PyDict_SetItemString(makhberDict, "mathFunctions", math);
        Py_DECREF(makhbermod);
    } else {
        QMessageBox::warning(d_parent, tr("Loading Module Failure"),
                             tr("Failed to load makhber module."
                                "Try exporting PYTHONPATH to the directory of makhber module."));
        PyErr_Print();
    }

    sysmod = PyImport_ImportModule("sys");
    if (sysmod) {
        sys = PyModule_GetDict(sysmod);
        Py_INCREF(sys);
    } else {
        QMessageBox::warning(d_parent, tr("Loading Module Failure"),
                             tr("Failed to load sys module."));
        PyErr_Print();
    }

    PyGILState_Release(gstate);
    d_initialized = true;
}

void PythonScripting::redirectStdIO()
{
    // Redirect output to the print(const QString&) signal.
    // Also see method write(const QString&) and Python documentation on
    // sys.stdout and sys.stderr.
    setQObject(this, "stdout", sys);
    setQObject(this, "stderr", sys);
}

bool PythonScripting::initialize()
{
    if (!d_initialized)
        return false;
    //	PyEval_AcquireLock();

    if (!d_parent->batchMode()) {
        // Redirect output to the print(const QString&) signal.
        // Also see method write(const QString&) and Python documentation on
        // sys.stdout and sys.stderr.
        setQObject(this, "stdout", sys);
        setQObject(this, "stderr", sys);
    }
    bool initialized = false;
    QStringList initPaths = { QDir::homePath() + "/", qApp->applicationDirPath() + "/", "" };
#ifdef PYTHONPATH
    initPaths.insert(1, qApp->applicationDirPath() + PYTHONPATH + "/");
#endif
    bool initFileFound = false;
    for (int i = 0; i < initPaths.size(); i++) {
        QString initFile = initPaths[i] + "makhberrc.py";
        if (QFileInfo::exists(initFile)) {
            initFileFound = true;
            initialized = loadInitFile(initFile);
            if (initialized)
                break;
        }
    }
    if (!initFileFound)
        QMessageBox::critical(d_parent, tr("File opening error"),
                              tr("Could not find 'makhberrc.py' file to initialize Python"
                                 " environment\n"
                                 "Try to copy the 'makhberrc.py' to the current PYTHONPATH,"
                                 " HOME or working directory"));

    //	PyEval_ReleaseLock();
    return initialized;
}

PythonScripting::~PythonScripting()
{
    Py_XDECREF(globals);
    Py_XDECREF(math);
    Py_XDECREF(sys);
}

bool PythonScripting::loadInitFile(const QString &path)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString("import sys\nsys.path.append('" PYTHONPATH "')");
    QFileInfo pyFile(path);
    bool success = false;
    if (pyFile.isReadable()) {
        FILE *f = fopen(pyFile.filePath().toLocal8Bit(), "rb");
        success = PyRun_SimpleFile(f, pyFile.filePath().toLocal8Bit()) == 0;
        fclose(f);
    }
    PyGILState_Release(gstate);
    return success;
}

bool PythonScripting::isRunning() const
{
    return Py_IsInitialized();
}

bool PythonScripting::setQObject(QObject *val, const char *name, PyObject *dict)
{
    if (!val)
        return false;
    PyObject *pyobj = NULL;

    PyGILState_STATE state = PyGILState_Ensure();

    // sipWrapperType * klass = sipFindClass(val->className());
#if QT_VERSION_MAJOR >= 6
    auto *PyQt_sip_CAPI = (const sipAPIDef *)(PyCapsule_Import("PyQt6.sip._C_API", 0));
#else
#    ifdef PYQT_SIP
    auto *PyQt_sip_CAPI = (const sipAPIDef *)(PyCapsule_Import("PyQt5.sip._C_API", 0));
#    else
    auto *PyQt_sip_CAPI = (const sipAPIDef *)(PyCapsule_Import("sip._C_API", 0));
#    endif
#endif
    if (!PyQt_sip_CAPI)
        return false;
    const sipTypeDef *klass = PyQt_sip_CAPI->api_find_type(val->metaObject()->className());
    if (!klass)
        return false;
    // pyobj = sipConvertFromInstance(val, klass, NULL);
    pyobj = PyQt_sip_CAPI->api_convert_from_type(val, klass, NULL);
    if (!pyobj)
        return false;

    if (dict)
        PyDict_SetItemString(dict, name, pyobj);
    else
        PyDict_SetItemString(globals, name, pyobj);
    Py_DECREF(pyobj);

    PyGILState_Release(state);
    return true;
}

bool PythonScripting::setInt(int val, const char *name, PyObject *dict)
{
    PyObject *pyobj = Py_BuildValue("i", val);
    if (!pyobj)
        return false;
    if (dict)
        PyDict_SetItemString(dict, name, pyobj);
    else
        PyDict_SetItemString(globals, name, pyobj);
    Py_DECREF(pyobj);
    return true;
}

bool PythonScripting::setDouble(double val, const char *name, PyObject *dict)
{
    PyObject *pyobj = Py_BuildValue("d", val);
    if (!pyobj)
        return false;
    if (dict)
        PyDict_SetItemString(dict, name, pyobj);
    else
        PyDict_SetItemString(globals, name, pyobj);
    Py_DECREF(pyobj);
    return true;
}

const QStringList PythonScripting::mathFunctions() const
{
    QStringList flist;
    PyObject *key, *value;
    Py_ssize_t i = 0;
    while (PyDict_Next(math, &i, &key, &value))
        if (PyCallable_Check(value))
            flist << PyUnicode_AsUTF8(key);
    flist.sort();
    return flist;
}

const QString PythonScripting::mathFunctionDoc(const QString &name) const
{
    PyGILState_STATE state = PyGILState_Ensure();
    PyObject *mathf = PyDict_GetItemString(math, name.toLocal8Bit()); // borrowed
    if (!mathf)
        return "";
    PyObject *pydocstr = PyObject_GetAttrString(mathf, "__doc__"); // new
    QString qdocstr = PyUnicode_AsUTF8(pydocstr);
    Py_XDECREF(pydocstr);
    PyGILState_Release(state);
    return qdocstr;
}

const QStringList PythonScripting::fileExtensions() const
{
    QStringList extensions;
    extensions << "py"
               << "PY";
    return extensions;
}
