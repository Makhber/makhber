/***************************************************************************
    File                 : TextDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Text label/axis label options dialog

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

#ifndef TEXTDLG_H
#define TEXTDLG_H

#include "core/ColorButton.h"
#include "core/TextFormatButtons.h"

#include <QDialog>

class QGroupBox;
class QTextEdit;
class QTextCursor;
class QComboBox;
class QLabel;
class QSpinBox;

//! Options dialog for text labels/axes labels
class MAKHBER_EXPORT TextDialog : public QDialog
{
    Q_OBJECT

public:
    //! Label types
    enum TextType {
        TextMarker, /*!< normal text label */
        AxisTitle /*!< axis label */
    };

    //! Constructor
    /**
     * \param type text type (TextMarker | AxisTitle)
     * \param parent parent widget
     * \param fl window flags
     */
    explicit TextDialog(TextType type, QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);
    //! Destructor
    ~TextDialog() {};

    //! Return axis label alignment
    /**
     * \sa setAlignment()
     */
    int alignment();
    //! Return rotation angle (not yet implemented)
    int angle();

public Q_SLOTS:
    //! Set label background type
    /**
     * \param bkg label background type
     * 0 -> plain, no border
     * 1 -> with border
     * 2 -> border + shadow
     */
    void setBackgroundType(int bkg);
    //! Set rotation angle (not yet implemented)
    void setAngle(int angle);
    //! Set the background color to 'c'
    void setBackgroundColor(QColor c);
    //! Set the text color to 'c'
    void setTextColor(QColor c);
    //! Set the current font to 'fnt'
    void setFont(const QFont &fnt);
    //! Set the contents of the text editor box
    void setText(const QString &t);
    //! Set axis label alignment
    /**
     * \param align alignment (can be -1 for invalid,
     *  Qt::AlignHCenter, Qt::AlignLeft, or Qt::AlignRight)
     */
    void setAlignment(int align);

private Q_SLOTS:
    //! Let the user select another font
    void customFont();
    //! Accept changes and close dialog
    void accept();
    //! Apply changes
    void apply();
    void setDefaultValues();

    void updateTransparency(int alpha);
    void pickBackgroundColor(const QColor &);

Q_SIGNALS:
    //! Emit all current values
    /**
     * \param text the label text
     * \param angle the rotation angle
     * \param bkg the background type
     * \param fnt the text font
     * \param textColor the text color
     * \param backgroundColor the backgroundcolor
     */
    void values(const QString &text, int angle, int bkg, const QFont &fnt, const QColor &textColor,
                const QColor &backgroundColor);

    //! Signal for axes labels: change text
    void changeText(const QString &);
    //! Signal for axes labels: change text color
    void changeColor(const QColor &);
    //! Signal for axes labels: change text alignment
    void changeAlignment(int);
    //! Signal for axes labels: change font
    void changeFont(const QFont &);

protected:
    //! current font
    QFont selectedFont;
    TextType textType;

    ColorButton *colorBtn, *backgroundBtn;
    QPushButton *buttonFont;
    QComboBox *backgroundBox;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QPushButton *buttonDefault;
    QComboBox *rotateBox {};
    QTextEdit *textEditBox;
    QGroupBox *groupBox1, *groupBox2 {};
    QComboBox *alignmentBox;
    TextFormatButtons *formatButtons;
    QSpinBox *boxBackgroundTransparency;
};

#endif // TEXTDLG_H
