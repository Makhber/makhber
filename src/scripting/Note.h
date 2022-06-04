/***************************************************************************
    File                 : Note.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
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
#ifndef NOTE_H
#define NOTE_H

#include "core/MyWidget.h"
#include "scripting/ScriptEdit.h"

#include <QTextEdit>

class ScriptingEnv;

/*!\brief Notes window class.
 *
 * \section future_plans Future Plans
 * - Search and replace
 */
class MAKHBER_EXPORT Note : public MyWidget
{
    Q_OBJECT

public:
    Note(ScriptingEnv *env, const QString &label, QWidget *parent = 0, const char *name = 0,
         Qt::WindowFlags f = Qt::Widget);
    ~Note() {};

    void init(ScriptingEnv *env);

public Q_SLOTS:
    void saveToJson(QJsonObject *jsObject, const QJsonObject &jsGeometry) override;
    void restore(QJsonObject *jsNote) override;

    QTextEdit *textWidget() { return (QTextEdit *)te; };
    bool autoexec() const { return autoExec; }
    void setAutoexec(bool);
    void modifiedNote();

    // ScriptEdit methods
    QString text() { return te->toPlainText(); };
    void setText(const QString &s) { te->setText(s); };
    void print() override { te->print(); };
    void exportPDF(const QString &fileName) override { te->exportPDF(fileName); };
    QString exportASCII(const QString &file = {}) { return te->exportASCII(file); };
    QString importASCII(const QString &file = {}) { return te->importASCII(file); };
    void execute() { te->execute(); };
    bool executeAll() { return te->executeAll(); };
    void evaluate() { te->evaluate(); };
    void insert(const QString &s) { te->insertPlainText(s); }

private:
    ScriptEdit *te {};
    bool autoExec {};
};

#endif
