/***************************************************************************
    File                 : globals.h
    Project              : Makhber
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

#ifndef MAKHBER_GLOBALS_H
#define MAKHBER_GLOBALS_H

#include "core/MakhberDefs.h"

#include <QObject>
#include <QString>

//! Definition of global constants and enums
/**
 * This class must not be instanced. All members are static.
 */
class MAKHBER_EXPORT Makhber : public QObject
{
    Q_OBJECT

private:
    Makhber() { } // don't allow instancing

public:
    virtual ~Makhber() { } // avoid the warning message
    //! Types of plot designations
    enum PlotDesignation {
        noDesignation = 0, //!< no plot designation
        X = 1, //!< x values
        Y = 2, //!< y values
        Z = 3, //!< z values
        xErr = 4, //!< x errors
        yErr = 5 //!< y errors
    };
    Q_ENUM(PlotDesignation)

    //! The column mode (defines output and input filter for table columns)
    enum class ColumnMode : int {
        Numeric = 0, //!< column contains doubles
        Text = 1, //!< column contains strings
        Month = 4, //!< column contains month names
        Day = 5, //!< column containts day of week names
        DateTime = 6, //!< column contains dates and/or times
        // 2 and 3 are skipped to avoid problems with old obsolete values
    };
    Q_ENUM(ColumnMode)

    //! Column data type
    enum ColumnDataType { TypeDouble = 1, TypeQString = 2, TypeQDateTime = 3 };
    Q_ENUM(ColumnDataType)

    static QString enumValueToString(int key, const QString &enum_name);
    static int enumStringToValue(const QString &string, const QString &enum_name);

    //! Return Version without suffix
    static QString version();
    //! Return Version with suffix
    static QString versionString();

    //! Return the copyright string
    static QString copyrightString();

    //! Return the release date as a string
    static QString releaseDateString();

    //! Show about dialog
    static void about();

private:
    //  Don't forget to change the Doxyfile when changing these!
    //! Makhber version number
    static const char *makhber_version;
    //! Extra version information string (like "Alpha", "Beta", "RC1", etc...)
    static const char *makhber_version_suffix;
    //! Copyright string containing the author names etc.
    static const QString copyright_string;
    //! Release date as a string
    static const char *release_date;
};

#endif
