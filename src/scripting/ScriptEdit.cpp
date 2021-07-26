/***************************************************************************
    File                 : ScriptEdit.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Editor widget for scripting code

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
#include "ScriptEdit.h"

#include "scripting/Note.h"

#include <QAction>
#include <QMenu>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <QTextBlock>
#include <QKeyEvent>

ScriptEdit::ScriptEdit(ScriptingEnv *env, QWidget *parent, QString name)
    : QTextEdit(parent), scripted(env), d_error(false), d_changing_fmt(false)
{
    setObjectName(name);
    myScript = scriptEnv->newScript("", this, name);
    connect(myScript, SIGNAL(error(const QString &, const QString &, int)), this,
            SLOT(insertErrorMsg(const QString &)));
    connect(myScript, SIGNAL(print(const QString &)), this, SLOT(scriptPrint(const QString &)));

    setLineWrapMode(NoWrap);
    setAcceptRichText(false);
    setFontFamily("Monospace");

    d_fmt_default.setBackground(palette().brush(QPalette::Base));
    d_fmt_success.setBackground(QBrush(QColor(128, 255, 128)));
    d_fmt_failure.setBackground(QBrush(QColor(255, 128, 128)));

    printCursor = textCursor();

    actionExecute = new QAction(tr("E&xecute"), this);
    actionExecute->setShortcut(tr("Ctrl+J"));
    connect(actionExecute, SIGNAL(triggered()), this, SLOT(execute()));

    actionExecuteAll = new QAction(tr("Execute &All"), this);
    actionExecuteAll->setShortcut(tr("Ctrl+Shift+J"));
    connect(actionExecuteAll, SIGNAL(triggered()), this, SLOT(executeAll()));

    actionEval = new QAction(tr("&Evaluate Expression"), this);
    actionEval->setShortcut(tr("Ctrl+Return"));
    connect(actionEval, SIGNAL(triggered()), this, SLOT(evaluate()));

    actionPrint = new QAction(tr("&Print"), this);
    connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));

    actionImport = new QAction(tr("&Import"), this);
    connect(actionImport, SIGNAL(triggered()), this, SLOT(importASCII()));

    actionExport = new QAction(tr("&Export"), this);
    connect(actionExport, SIGNAL(triggered()), this, SLOT(exportASCII()));

    functionsMenu = new QMenu(this);
    Q_CHECK_PTR(functionsMenu);
    connect(functionsMenu, SIGNAL(triggered(QAction *)), this, SLOT(insertFunction(QAction *)));

    connect(document(), SIGNAL(contentsChange(int, int, int)), this,
            SLOT(handleContentsChange(int, int, int)));
}

void ScriptEdit::customEvent(QEvent *e)
{
    if (e->type() == SCRIPTING_CHANGE_EVENT) {
        scriptingChangeEvent(dynamic_cast<ScriptingChangeEvent *>(e));
        myScript->deleteLater();
        myScript = scriptEnv->newScript("", this, objectName());
        connect(myScript, SIGNAL(error(const QString &, const QString &, int)), this,
                SLOT(insertErrorMsg(const QString &)));
        connect(myScript, SIGNAL(print(const QString &)), this, SLOT(scriptPrint(const QString &)));
    }
}

void ScriptEdit::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Return)
        updateIndentation();
}

void ScriptEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    Q_CHECK_PTR(menu);

    menu->addAction(actionPrint);
    menu->addAction(actionImport);
    menu->addAction(actionExport);
    menu->addSeparator();

    menu->addAction(actionExecute);
    menu->addAction(actionExecuteAll);
    menu->addAction(actionEval);

    if (parent()->inherits("Note")) {
        Note *sp = dynamic_cast<Note *>(parent());
        auto *actionAutoexec = new QAction(tr("Auto&exec"), menu);
        actionAutoexec->setCheckable(true);
        actionAutoexec->setChecked(sp->autoexec());
        connect(actionAutoexec, SIGNAL(toggled(bool)), sp, SLOT(setAutoexec(bool)));
        menu->addAction(actionAutoexec);
    }

    functionsMenu->clear();
    functionsMenu->setTearOffEnabled(true);
    QStringList flist = scriptEnv->mathFunctions();
    QMenu *submenu = nullptr;
    for (int i = 0; i < flist.size(); i++) {
        QAction *newAction = nullptr;
        QString menupart;
        // group by text before "_" (would make sense if we renamed several functions...)
        /*if (flist[i].contains("_") || (i<flist.size()-1 && flist[i+1].split("_")[0]==flist[i]))
                menupart = flist[i].split("_")[0];
        else
                menupart = "";*/
        // group by first letter, avoiding submenus with only one entry
        if ((i == 0 || flist[i - 1][0] != flist[i][0])
            && (i == flist.size() - 1 || flist[i + 1][0] != flist[i][0]))
            menupart = "";
        else
            menupart = flist[i].left(1);
        if (!menupart.isEmpty()) {
            if (!submenu || menupart != submenu->title())
                submenu = functionsMenu->addMenu(menupart);
            newAction = submenu->addAction(flist[i]);
        } else
            newAction = functionsMenu->addAction(flist[i]);
        newAction->setData(i);
        newAction->setWhatsThis(scriptEnv->mathFunctionDoc(flist[i]));
    }
    functionsMenu->setTitle(tr("&Functions"));
    menu->addMenu(functionsMenu);

    menu->exec(e->globalPos());
    delete menu;
}

void ScriptEdit::insertErrorMsg(const QString &message)
{
    QString err = message;
    err.prepend("\n").replace("\n", "\n#> ");
    int start = printCursor.position();
    printCursor.insertText(err);
    printCursor.setPosition(start, QTextCursor::KeepAnchor);
    setTextCursor(printCursor);
    d_error = true;
}

void ScriptEdit::scriptPrint(const QString &text)
{
    printCursor.insertText(text);
}

void ScriptEdit::insertFunction(const QString &fname)
{
    QTextCursor cursor = textCursor();
    QString markedText = cursor.selectedText();
    cursor.insertText(fname + "(" + markedText + ")");
    if (markedText.isEmpty()) {
        // if no text is selected, place cursor inside the ()
        // instead of after it
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 1);
        // the next line makes the selection visible to the user
        // (the line above only changes the selection in the
        // underlying QTextDocument)
        setTextCursor(cursor);
    }
}

void ScriptEdit::insertFunction(QAction *action)
{
    insertFunction(scriptEnv->mathFunctions()[action->data().toInt()]);
}

int ScriptEdit::lineNumber(int pos) const
{
    int n = 1;
    for (QTextBlock i = document()->begin(); !i.contains(pos) && i != document()->end();
         i = i.next())
        n++;
    return n;
}

void ScriptEdit::handleContentsChange(int position, int, int)
{
    if (d_changing_fmt)
        return; // otherwise we overwrite our own changes
    QTextCursor cursor = textCursor();
    cursor.setPosition(position);
    cursor.mergeBlockFormat(d_fmt_default);
}

void ScriptEdit::execute()
{
    QString fname = "<%1:%2>";
    fname = fname.arg(objectName());
    QTextCursor codeCursor = textCursor();
    if (codeCursor.selectedText().isEmpty()) {
        codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    fname = fname.arg(lineNumber(codeCursor.selectionStart()));

    myScript->setName(fname);
    myScript->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator, "\n"));
    printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
    printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    printCursor.insertText("\n");
    myScript->exec();
    d_changing_fmt = true;
    if (d_error)
        codeCursor.mergeBlockFormat(d_fmt_failure);
    else
        codeCursor.mergeBlockFormat(d_fmt_success);
    d_changing_fmt = false;
    d_error = false;
}

bool ScriptEdit::executeAll()
{
    QString fname = "<%1>";
    fname = fname.arg(objectName());
    myScript->setName(fname);
    myScript->setCode(toPlainText());
    printCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    printCursor.insertText("\n");
    return myScript->exec();
}

void ScriptEdit::evaluate()
{
    QString fname = "<%1:%2>";
    fname = fname.arg(objectName());
    QTextCursor codeCursor = textCursor();
    if (codeCursor.selectedText().isEmpty()) {
        codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    fname = fname.arg(lineNumber(codeCursor.selectionStart()));

    myScript->setName(fname);
    myScript->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator, "\n"));
    printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
    printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    printCursor.insertText("\n");
    QVariant res = myScript->eval();

    d_changing_fmt = true;
    if (d_error)
        codeCursor.mergeBlockFormat(d_fmt_failure);
    else
        codeCursor.mergeBlockFormat(d_fmt_success);

    if (res.isValid())
        if (!res.isNull() && res.canConvert<QString>()) {
            QString strVal = res.toString();
            strVal.replace("\n", "\n#> ");
            printCursor.insertText("\n");
            printCursor.mergeBlockFormat(d_fmt_default);
            if (!strVal.isEmpty())
                printCursor.insertText("#> " + strVal + "\n");
        }

    d_changing_fmt = false;
    d_error = false;
    setTextCursor(printCursor);
}

void ScriptEdit::exportPDF(const QString &fileName)
{
    QTextDocument *doc = document();
    QPrinter printer;
    printer.setColorMode(QPrinter::GrayScale);
    printer.setCreator("Makhber");
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    doc->print(&printer);
}

void ScriptEdit::print()
{
    QTextDocument *doc = document();
    QPrinter printer;
    printer.setColorMode(QPrinter::GrayScale);
    QPrintDialog printDialog(&printer);
    // TODO: Write a dialog to use more features of Qt4's QPrinter class
    if (printDialog.exec() == QDialog::Accepted)
        doc->print(&printer);
}

QString ScriptEdit::importASCII(const QString &filename)
{
    QString filter = tr("Text") + " (*.txt *.TXT);;";
    filter += scriptEnv->fileFilter();
    filter += tr("All Files") + " (*)";

    QString f;
    if (filename.isEmpty())
        f = QFileDialog::getOpenFileName(this, tr("Import Text From File"), QString(), filter,
                                         nullptr);
    else
        f = filename;
    if (f.isEmpty())
        return QString();
    QFile file(f);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error Opening File"),
                              tr("Could not open file \"%1\" for reading.").arg(f));
        return QString();
    }
    QTextStream s(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    s.setEncoding(QStringConverter::Utf8);
#else
    s.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
    while (!s.atEnd())
        insertPlainText(s.readLine() + "\n");
    file.close();
    return f;
}

QString ScriptEdit::exportASCII(const QString &filename)
{
    QString filter = tr("Text") + " (*.txt *.TXT);;";
    filter += scriptEnv->fileFilter();
    filter += tr("All Files") + " (*)";

    QString selectedFilter;
    QString fn;
    if (filename.isEmpty())
        fn = QFileDialog::getSaveFileName(this, tr("Save Text to File"), QString(), filter,
                                          &selectedFilter, QFileDialog::DontResolveSymlinks);
    else
        fn = filename;

    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        QString baseName = fi.fileName();
        if (!baseName.contains(".")) {
            if (selectedFilter.contains(".txt"))
                fn.append(".txt");
            else if (selectedFilter.contains(".py"))
                fn.append(".py");
        }

        QFile f(fn);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(nullptr, tr("File Save Error"),
                                  tr("Could not write to file: <br><h4> %1 </h4><p>Please verify "
                                     "that you have the right to write to this location!")
                                          .arg(fn));
            return QString();
        }

        QTextStream t(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        t.setEncoding(QStringConverter::Utf8);
#else
        t.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
        t << toPlainText();
        f.close();
    }
    return fn;
}

void ScriptEdit::updateIndentation()
{
    QTextCursor cursor = textCursor();
    QTextBlock para = cursor.block();
    QString prev = para.previous().text();
    int i = 0;
    for (i = 0; prev[i].isSpace(); i++)
        ;
    QString indent = prev.mid(0, i);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.insertText(indent);
}
