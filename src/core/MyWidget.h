/***************************************************************************
    File                 : MyWidget.h
    Project              : Makhber
    Description          : MDI window widget
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2007 by Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email address)

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
#ifndef WIDGET_H
#define WIDGET_H

#include "core/MakhberDefs.h"

#include <QMdiSubWindow>

class QEvent;
class QCloseEvent;
class QString;
class Folder;

/**
 * \brief Base class of all MDI client windows.
 *
 * These are the main objects of every Makhber project.
 * All content (apart from the directory structure) is managed by subclasses of MyWidget.
 *
 * \section future_plans Future Plans
 * Rename to Aspect.
 *
 * \sa Folder, ApplicationWindow
 */
class MAKHBER_EXPORT MyWidget : public QMdiSubWindow
{
    Q_OBJECT

public:
    //! Constructor
    /**
     * \param label window label
     * \param parent parent widget
     * \param name window name
     * \param f window flags
     * \sa setCaptionPolicy(), captionPolicy()
     */
    MyWidget(const QString &label = QString(), QWidget *parent = 0, const QString &name = 0,
             Qt::WindowFlags f = Qt::Widget);

    //! Possible window captions.
    enum CaptionPolicy {
        Name = 0, //!< caption determined by the window name
        Label = 1, //!< caption detemined by the window label
        Both = 2 //!< caption = "name - label"
    };
    enum Status { Hidden = -1, Normal = 0, Minimized = 1, Maximized = 2 };

    //! Return the window label
    virtual QString windowLabel() { return QString(w_label); };
    //! Set the window label
    virtual void setWindowLabel(const QString &s)
    {
        auto label = s;
        w_label = label.replace("\n", " ").replace("\t", " ");
        updateCaption();
    };

    //! Return the window name
    virtual QString name() { return objectName(); };
    //! Set the window name
    virtual void setName(const QString &s)
    {
        setObjectName(s);
        updateCaption();
    };

    //! Return the caption policy
    virtual CaptionPolicy captionPolicy() { return caption_policy; };
    //! Set the caption policy
    virtual void setCaptionPolicy(CaptionPolicy policy)
    {
        caption_policy = policy;
        updateCaption();
    }

    //! Return the creation date
    virtual QString birthDate() { return birthdate; };
    //! Set the creation date
    virtual void setBirthDate(const QString &s) { birthdate = s; };

    //! Return the window status as a string
    QString aspect();
    //! Return the window status flag (hidden, normal, minimized or maximized)
    Status status() { return w_status; };
    //! Set the window status flag (hidden, normal, minimized or maximized)
    void setStatus(Status s);

    virtual void saveAsTemplate(QJsonObject *, const QJsonObject &) {};
    // TODO:
    //! Not implemented yet
    virtual void restore(QJsonObject *) {};

    virtual void print() {};
    virtual void exportPDF(const QString &) {};

    virtual void saveToJson(QJsonObject *, const QJsonObject &) {};

    //!Notifies that a window was hidden by a direct user action
    virtual void setHidden();

    // event handlers
    //! Close event handler
    /**
     * Ask the user "delete, hide, or cancel?" if the
     * "ask on close" flag is set.
     */
    void closeEvent(QCloseEvent *);
    //! Toggle the "ask on close" flag
    void askOnCloseEvent(bool ask) { askOnClose = ask; };
    //! Customizes title bar's context menu)
    void contextMenuEvent(QContextMenuEvent *e);

    //! Returns the pointer to the parent folder of the window
    Folder *folder() { return parentFolder; };

    //! Initializes the pointer to the parent folder of the window
    void setFolder(Folder *f) { parentFolder = f; };

    //! Notifies the main application that the window has been modified
    void notifyChanges() { Q_EMIT modifiedWindow(this); };

    void setNormal();
    void setMinimized();
    void setMaximized();

Q_SIGNALS:
    //! Emitted when the window was closed
    void closedWindow(MyWidget *);
    //! Emitted when the window was hidden
    void hiddenWindow(MyWidget *);
    void modifiedWindow(MyWidget *);
    void resizedWindow(MyWidget *);
    //! Emitted when the window status changed
    void statusChanged(MyWidget *);
    //! Emitted when the title bar recieves a QContextMenuEvent
    void showTitleBarMenu();

protected Q_SLOTS:
    //! Set caption according to current CaptionPolicy, name and label
    void updateCaption();

protected:
    virtual void changeEvent(QEvent *event);
    //!Pointer to the parent folder of the window
    Folder *parentFolder {};
    //! The window label
    /**
     * \sa setWindowLabel(), windowLabel(), setCaptionPolicy()
     */
    QString w_label;
    //! The creation date
    QString birthdate;
    //! The window status
    Status w_status;
    //! The caption policy
    /**
     * \sa setCaptionPolicy(), captionPolicy()
     */
    CaptionPolicy caption_policy;
    //! Toggle on/off: Ask the user "delete, hide, or cancel?" on a close event
    bool askOnClose;
};

#endif
