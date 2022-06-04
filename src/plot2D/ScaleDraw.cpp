/***************************************************************************
    File                 : ScaleDraw.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Extension to QwtScaleDraw

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
#include "ScaleDraw.h"

#include "scripting/MyParser.h"

#include <qwt_painter.h>
#include <qwt_text.h>

#include <QDateTime>
#include <QMessageBox>

#include <utility>

ScaleDraw::ScaleDraw(QString s)
    : formula_string(std::move(s)), d_fmt('g'), d_prec(4), d_minTicks(Out), d_majTicks(Out)
{
}

ScaleDraw::ScaleDraw(const ScaleDraw &other, const QString &s)
    : formula_string(std::move(s)),
      d_minTicks(other.majorTicksStyle()),
      d_majTicks(other.minorTicksStyle())
{
    other.labelFormat(d_fmt, d_prec);
    invalidateCache();
}

double ScaleDraw::transformValue(double value) const
{
    if (!formula_string.isEmpty()) {
        double lbl = 0.0;
        try {
            MyParser parser;
            if (formula_string.contains("x"))
                parser.DefineVar(_T("x"), &value);
            else if (formula_string.contains("y"))
                parser.DefineVar(_T("y"), &value);

            parser.SetExpr(formula_string.toUtf8().constData());
            lbl = parser.Eval();
        } catch (mu::ParserError &) {
            return 0;
        }

        return lbl;
    } else
        return value;
}

/*!
  \brief Set the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f', 'g'
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa labelFormat()
*/
void ScaleDraw::setLabelFormat(char f, int prec)
{
    d_fmt = f;
    d_prec = prec;
}

/*!
  \brief Return the number format for the major scale labels

  Format character and precision have the same meaning as for
  sprintf().
  \param f format character 'e', 'f' or 'g'
  \param prec
    - for 'e', 'f': the number of digits after the radix character (point)
    - for 'g': the maximum number of significant digits

  \sa setLabelFormat()
*/
void ScaleDraw::labelFormat(char &f, int &prec) const
{
    f = d_fmt;
    prec = d_prec;
}

void ScaleDraw::drawTick(QPainter *p, double value, double len) const
{
    QwtScaleDiv scDiv = scaleDiv();
    QList<double> majTicks = scDiv.ticks(QwtScaleDiv::MajorTick);
    if (majTicks.contains(value) && (d_majTicks == In || d_majTicks == None))
        return;

    QList<double> medTicks = scDiv.ticks(QwtScaleDiv::MediumTick);
    if (medTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
        return;

    QList<double> minTicks = scDiv.ticks(QwtScaleDiv::MinorTick);
    if (minTicks.contains(value) && (d_minTicks == In || d_minTicks == None))
        return;

    QwtScaleDraw::drawTick(p, value, len);
}

/*****************************************************************************
 *
 * Class QwtTextScaleDraw
 *
 *****************************************************************************/

QwtTextScaleDraw::QwtTextScaleDraw(const QMap<int, QString> &list) : labels(list) { }

QwtText QwtTextScaleDraw::label(double value) const
{
    const QwtScaleDiv scDiv = scaleDiv();
    if (!scDiv.contains(value))
        return QwtText();

    int int_value = (int)(value + value / 1e6);
    if (labels.contains(int_value))
        return QwtText(labels[int_value]);
    else
        return QwtText();
}

/*****************************************************************************
 *
 * Class TimeScaleDraw
 *
 *****************************************************************************/

TimeScaleDraw::TimeScaleDraw(const QTime &t, QString format)
    : t_origin(t), t_format(std::move(format))
{
}

QString TimeScaleDraw::origin()
{
    return t_origin.toString("hh:mm:ss.zzz");
}

QwtText TimeScaleDraw::label(double value) const
{
    QTime t = t_origin.addMSecs((int)value);
    return QwtText(t.toString(t_format));
}

/*****************************************************************************
 *
 * Class DateScaleDraw
 *
 *****************************************************************************/

DateScaleDraw::DateScaleDraw(const QDate &t, QString format)
    : t_origin(t), t_format(std::move(format))
{
}

QString DateScaleDraw::origin()
{
    return t_origin.toString();
}

QwtText DateScaleDraw::label(double value) const
{
    QDate t;
    if (t_origin.isValid())
        t = t_origin.addDays((int)floor(value));
    else
        t = QDate::fromJulianDay((int)floor(value));
    return QwtText(t.toString(t_format));
}

/*****************************************************************************
 *
 * Class DateTimeScaleDraw
 *
 *****************************************************************************/

DateTimeScaleDraw::DateTimeScaleDraw(QDateTime origin, QString format)
    : d_origin(std::move(origin)), d_format(std::move(format))
{
}

QString DateTimeScaleDraw::origin()
{
    return d_origin.toString(d_format);
}

QwtText DateTimeScaleDraw::label(double value) const
{
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(round((value - 2440587.5) * 86400000.));
    return QwtText(dt.toString(d_format));
}

/*****************************************************************************
 *
 * Class WeekDayScaleDraw
 *
 *****************************************************************************/

WeekDayScaleDraw::WeekDayScaleDraw(NameFormat format) : d_format(format) { }

QwtText WeekDayScaleDraw::label(double value) const
{
    int val = int(transformValue(value)) % 7;

    if (val < 0)
        val = 7 - abs(val);
    else if (val == 0)
        val = 7;

    QString day;
    switch (d_format) {
    case ShortName:
        day = QLocale().dayName(val, QLocale::ShortFormat);
        break;

    case LongName:
        day = QLocale().dayName(val, QLocale::LongFormat);
        break;

    case Initial:
        day = (QLocale().dayName(val, QLocale::ShortFormat)).left(1);
        break;
    }
    return QwtText(day);
}

/*****************************************************************************
 *
 * Class MonthScaleDraw
 *
 *****************************************************************************/

MonthScaleDraw::MonthScaleDraw(NameFormat format) : d_format(format) { }

QwtText MonthScaleDraw::label(double value) const
{
    int val = int(transformValue(value)) % 12;

    if (val < 0)
        val = 12 - abs(val);
    else if (val == 0)
        val = 12;

    QString month;
    switch (d_format) {
    case ShortName:
        month = QLocale().monthName(val, QLocale::ShortFormat);
        break;

    case LongName:
        month = QLocale().monthName(val, QLocale::LongFormat);
        break;

    case Initial:
        month = (QLocale().monthName(val, QLocale::ShortFormat)).left(1);
        break;
    }
    return QwtText(month);
}

/*****************************************************************************
 *
 * Class QwtSupersciptsScaleDraw
 *
 *****************************************************************************/

QwtSupersciptsScaleDraw::QwtSupersciptsScaleDraw(const QString &s)
{
    setFormulaString(s);
}

QwtText QwtSupersciptsScaleDraw::label(double value) const
{
    char f = 0;
    int prec = 0;
    labelFormat(f, prec);

    QString txt = QLocale().toString(transformValue(value), 'e', prec);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList list = txt.split("e", Qt::SkipEmptyParts);
#else
    QStringList list = txt.split("e", QString::SkipEmptyParts);
#endif
    if (list[0].toDouble() == 0.0)
        return QString("0");

    QString s = list[1];
    int l = s.length();
    QChar sign = s[0];

    s.remove(sign);

    while (l > 1 && s.startsWith("0", Qt::CaseInsensitive)) {
        s.remove(0, 1);
        l = s.length();
    }

    if (sign == '-')
        s.prepend(sign);

    if (list[0] == "1")
        return QwtText("10<sup>" + s + "</sup>");
    else
        return QwtText(list[0] + "x10<sup>" + s + "</sup>");
}
