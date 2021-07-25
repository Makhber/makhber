/***************************************************************************
    File                 : globals.cpp
    Description          : Definition of global constants and enums
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
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

#include "globals.h"
#include "version.h"

#include "ui_MakhberAbout.h"

#include <qwt_global.h>
#include <muParserDef.h>
#include <gsl/gsl_version.h>
#ifdef SCRIPTING_PYTHON
#include <Python.h>
#endif

#include <QMessageBox>
#include <QIcon>
#include <QObject>
#include <QMetaObject>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QtDebug>

const QString Makhber::copyright_string = "";

int Makhber::version()
{
    return makhber_versionNo;
}

QString Makhber::schemaVersion()
{
    return "Makhber " + QString::number((version() & 0xFF0000) >> 16) + "."
            + QString::number((version() & 0x00FF00) >> 8) + "."
            + QString::number(version() & 0x0000FF);
}

QString Makhber::versionString()
{
    return QString("Makhber ") + makhber_version;
}

QString Makhber::extraVersion()
{
    return QString(extra_version);
}

void Makhber::about()
{
    QString text = Makhber::copyright_string;
    text.replace(QRegularExpression("\\[1\\]"), "<sup>1</sup>");
    text.replace("é", "&eacute;");
    text.replace("á", "&aacute;");
    text.replace("ö", "&ouml;");
    text.replace("\n", "<br>");
    text.replace("=== ", "<h1>");
    text.replace(" ===", "</h1>");
    text.replace("--- ", "<h2>");
    text.replace(" ---", "</h2>");
    text.replace(" ---", "</h2>");
    text.replace("</h1><br><br>", "</h1>");
    text.replace("</h2><br><br>", "</h2>");
    text.replace("<br><h1>", "<h1>");
    text.replace("<br><h2>", "<h2>");

    Qt::WindowFlags flags = Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
#if QT_VERSION >= 0x040500
    flags |= Qt::WindowCloseButtonHint;
#endif
    auto *dialog = new QDialog(nullptr, flags);
    Ui::MakhberAbout ui;
    ui.setupUi(dialog);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(QObject::tr("About Makhber"));
    ui.version_label->setText(versionString() + extraVersion());
    ui.release_date_label->setText(QObject::tr("Released") + ": " + QString(Makhber::release_date));
    ui.used_libraries->setText(QString("Makhber was built with the fellowing libraries: ")
                               + "\nQt: " + QT_VERSION_STR + "\nQwt: " + QWT_VERSION_STR
#ifdef SCRIPTING_PYTHON
                               + "\nPython: " + PY_VERSION
#endif
                               + "\nGSL: " + GSL_VERSION
#ifdef MUP_VERSION
                               + "\nmuParser: " + MUP_VERSION);
#else
                               + "\nmuParser: " + QString::fromStdString(mu::ParserVersion));
#endif
    dialog->exec();
}

QString Makhber::copyrightString()
{
    return copyright_string;
}

QString Makhber::releaseDateString()
{
    return release_date;
}

QString Makhber::enumValueToString(int key, const QString &enum_name)
{
    int index = staticMetaObject.indexOfEnumerator(enum_name.toUtf8());
    if (index == -1)
        return QString("invalid");
    QMetaEnum meta_enum = staticMetaObject.enumerator(index);
    return QString(meta_enum.valueToKey(key));
}

int Makhber::enumStringToValue(const QString &string, const QString &enum_name)
{
    int index = staticMetaObject.indexOfEnumerator(enum_name.toUtf8());
    if (index == -1)
        return -1;
    QMetaEnum meta_enum = staticMetaObject.enumerator(index);
    return meta_enum.keyToValue(string.toUtf8());
}
