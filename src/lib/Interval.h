/***************************************************************************
    File                 : Interval.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Auxiliary class for interval based data

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

#ifndef INTERVAL_H
#define INTERVAL_H

#include <QList>
#include <QString>
#include <QtGlobal>

// forward declaration
template<class T>
class Interval;

template<class T>
class IntervalBase
{
public:
    IntervalBase() : d_minvalue(-1), d_maxvalue(-1) { }
    IntervalBase(T minValue, T maxValue)
    {
        d_minvalue = minValue;
        d_maxvalue = maxValue;
    }
    virtual ~IntervalBase() { }
    T minValue() const { return d_minvalue; }
    T maxValue() const { return d_maxvalue; }
    void setMinValue(T minValue) { d_minvalue = minValue; }
    void setMaxValue(T maxValue) { d_maxvalue = maxValue; }
    bool contains(const Interval<T> &other) const
    {
        return (d_minvalue <= other.minValue() && d_maxvalue >= other.maxValue());
    }
    bool contains(T value) const { return (d_minvalue <= value && d_maxvalue >= value); }
    bool intersects(const Interval<T> &other) const
    {
        return (contains(other.minValue()) || contains(other.maxValue()));
    }
    //! Return the intersection of two intervals
    /**
     * This function returns an invalid interval if the two intervals do not intersect.
     */
    static Interval<T> intersection(const Interval<T> &first, const Interval<T> &second)
    {
        return Interval<T>(qMax(first.minValue(), second.minValue()),
                           qMin(first.maxValue(), second.maxValue()));
    }
    void translate(T offset)
    {
        d_minvalue += offset;
        d_maxvalue += offset;
    }
    bool operator==(const IntervalBase<T> &other) const
    {
        return (d_minvalue == other.minValue() && d_maxvalue == other.maxValue());
    }
    //! Returns true if no gap is between two intervals.
    virtual bool touches(const Interval<T> &other) const = 0;
    //! Merge two intervals that touch or intersect
    static Interval<T> merge(const Interval<T> &a, const Interval<T> &b)
    {
        if (!(a.intersects(b) || a.touches(b)))
            return a;
        return Interval<T>(qMin(a.minValue(), b.minValue()), qMax(a.maxValue(), b.maxValue()));
    }
    //! Subtract an interval from another
    static QList<Interval<T>> subtract(const Interval<T> &src_iv, const Interval<T> &minus_iv)
    {
        QList<Interval<T>> list;
        if ((src_iv == minus_iv) || (minus_iv.contains(src_iv)))
            return list;

        if (!src_iv.intersects(minus_iv))
            list.append(src_iv);
        else if (src_iv.maxValue() <= minus_iv.maxValue())
            list.append(Interval<T>(src_iv.minValue(), minus_iv.minValue() - 1));
        else if (src_iv.minValue() >= minus_iv.minValue())
            list.append(Interval<T>(minus_iv.maxValue() + 1, src_iv.maxValue()));
        else {
            list.append(Interval<T>(src_iv.minValue(), minus_iv.minValue() - 1));
            list.append(Interval<T>(minus_iv.maxValue() + 1, src_iv.maxValue()));
        }

        return list;
    }
    //! Split an interval into two
    static QList<Interval<T>> split(const Interval<T> &i, T before)
    {
        QList<Interval<T>> list;
        if (before < i.minValue() || before > i.maxValue()) {
            list.append(i);
        } else {
            Interval<T> left(i.minValue(), before - 1);
            Interval<T> right(before, i.maxValue());
            if (left.isValid())
                list.append(left);
            if (right.isValid())
                list.append(right);
        }
        return list;
    }
    //! Merge an interval into a list
    /*
     * This function merges all intervals in the list until none of them
     * intersect or touch anymore.
     */
    static void mergeIntervalIntoList(QList<Interval<T>> *list, Interval<T> i)
    {
        for (int c = 0; c < list->size(); c++) {
            if (list->at(c).touches(i) || list->at(c).intersects(i)) {
                Interval<T> result = merge(list->takeAt(c), i);
                mergeIntervalIntoList(list, result);
                return;
            }
        }
        list->append(i);
    }
    //! Restrict all intervals in the list to their intersection with a given interval
    /**
     * Remark: This may decrease the list size.
     */
    static void restrictList(QList<Interval<T>> *list, Interval<T> i)
    {
        Interval<T> temp;
        for (int c = 0; c < list->size(); c++) {
            temp = intersection(list->at(c), i);
            if (!temp.isValid())
                list->removeAt(c--);
            else
                list->replace(c, temp);
        }
    }
    //! Subtract an interval from all intervals in the list
    /**
     * Remark: This may increase or decrease the list size.
     */
    static void subtractIntervalFromList(QList<Interval<T>> *list, Interval<T> i)
    {
        for (int c = 0; c < list->size(); c++) {
            QList<Interval<T>> temp_list = subtract(list->at(c), i);
            if (temp_list.isEmpty())
                list->removeAt(c--);
            else {
                list->replace(c, temp_list.at(0));
                if (temp_list.size() > 1)
                    list->insert(c, temp_list.at(1));
            }
        }
    }
    QList<Interval<T>> operator-(QList<Interval<T>> subtrahend)
    {
        QList<Interval<T>> *tmp1, *tmp2;
        tmp1 = new QList<Interval<T>>();
        *tmp1 << *static_cast<Interval<T> *>(this);
        for (Interval<T> i : subtrahend) {
            tmp2 = new QList<Interval<T>>();
            for (Interval<T> j : *tmp1)
                *tmp2 << subtract(j, i);
            delete tmp1;
            tmp1 = tmp2;
        }
        QList<Interval<T>> result = *tmp1;
        delete tmp1;
        return result;
    }
    //! Return a string in the format '[minValue,maxValue]'
    QString toString() const
    {
        return "[" + QString::number(d_minvalue) + "," + QString::number(d_maxvalue) + "]";
    }

protected:
    //! Interval minValue
    T d_minvalue;
    //! Interval maxValue
    T d_maxvalue;
};

//! Auxiliary class for interval based data
/**
 *	This class represents an interval of
 *	the type [minValue,maxValue]. It should be pretty
 *	self explanatory.
 *
 *	For the template argument (T), only numerical types ((unsigned) short, (unsigned) int,
 *	(unsigned) long, float, double, long double) are supported.
 */
template<class T>
class Interval : public IntervalBase<T>
{
public:
    Interval() { }
    Interval(T minValue, T maxValue) : IntervalBase<T>(minValue, maxValue) { }
    T size() const { return IntervalBase<T>::d_maxvalue - IntervalBase<T>::d_minvalue + 1; }
    bool isValid() const
    {
        return (IntervalBase<T>::d_minvalue >= 0 && IntervalBase<T>::d_maxvalue >= 0
                && IntervalBase<T>::d_minvalue <= IntervalBase<T>::d_maxvalue);
    }
    bool touches(const Interval<T> &other) const
    {
        return ((other.maxValue() == IntervalBase<T>::d_minvalue - 1)
                || (other.minValue() == IntervalBase<T>::d_maxvalue + 1));
    }
};

template<>
class Interval<float> : public IntervalBase<float>
{
public:
    Interval() { }
    Interval(float minValue, float maxValue) : IntervalBase<float>(minValue, maxValue) { }
    Interval(const Interval<float> &other) : IntervalBase<float>(other) { }
    float size() const { return IntervalBase<float>::d_maxvalue - IntervalBase<float>::d_minvalue; }
    bool isValid() const
    {
        return (IntervalBase<float>::d_minvalue <= IntervalBase<float>::d_maxvalue);
    }
    bool touches(const Interval<float> &other) const
    {
        return ((other.maxValue() == IntervalBase<float>::d_minvalue)
                || (other.minValue() == IntervalBase<float>::d_maxvalue));
    }
};

template<>
class Interval<double> : public IntervalBase<double>
{
public:
    Interval() { }
    Interval(double minValue, double maxValue) : IntervalBase<double>(minValue, maxValue) { }
    Interval(const Interval<double> &other) : IntervalBase<double>(other) { }
    double size() const
    {
        return IntervalBase<double>::d_maxvalue - IntervalBase<double>::d_minvalue;
    }
    bool isValid() const
    {
        return (IntervalBase<double>::d_minvalue <= IntervalBase<double>::d_maxvalue);
    }
    bool touches(const Interval<double> &other) const
    {
        return ((other.maxValue() == IntervalBase<double>::d_minvalue)
                || (other.minValue() == IntervalBase<double>::d_maxvalue));
    }
};

template<>
class Interval<long double> : public IntervalBase<long double>
{
public:
    Interval() { }
    Interval(long double minValue, long double maxValue)
        : IntervalBase<long double>(minValue, maxValue)
    {
    }
    Interval(const Interval<long double> &other) : IntervalBase<long double>(other) { }
    long double size() const
    {
        return IntervalBase<long double>::d_maxvalue - IntervalBase<long double>::d_minvalue;
    }
    bool isValid() const
    {
        return (IntervalBase<long double>::d_minvalue <= IntervalBase<long double>::d_maxvalue);
    }
    bool touches(const Interval<long double> &other) const
    {
        return ((other.maxValue() == IntervalBase<long double>::d_minvalue)
                || (other.minValue() == IntervalBase<long double>::d_maxvalue));
    }
};

#endif
