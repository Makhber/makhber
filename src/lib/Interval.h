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
    IntervalBase() : d_start(-1), d_end(-1) { }
    IntervalBase(T start, T end)
    {
        d_start = start;
        d_end = end;
    }
    virtual ~IntervalBase() { }
    T start() const { return d_start; }
    T end() const { return d_end; }
    void setStart(T start) { d_start = start; }
    void setEnd(T end) { d_end = end; }
    bool contains(const Interval<T> &other) const
    {
        return (d_start <= other.start() && d_end >= other.end());
    }
    bool contains(T value) const { return (d_start <= value && d_end >= value); }
    bool intersects(const Interval<T> &other) const
    {
        return (contains(other.start()) || contains(other.end()));
    }
    //! Return the intersection of two intervals
    /**
     * This function returns an invalid interval if the two intervals do not intersect.
     */
    static Interval<T> intersection(const Interval<T> &first, const Interval<T> &second)
    {
        return Interval<T>(qMax(first.start(), second.start()), qMin(first.end(), second.end()));
    }
    void translate(T offset)
    {
        d_start += offset;
        d_end += offset;
    }
    bool operator==(const IntervalBase<T> &other) const
    {
        return (d_start == other.start() && d_end == other.end());
    }
    //! Returns true if no gap is between two intervals.
    virtual bool touches(const Interval<T> &other) const = 0;
    //! Merge two intervals that touch or intersect
    static Interval<T> merge(const Interval<T> &a, const Interval<T> &b)
    {
        if (!(a.intersects(b) || a.touches(b)))
            return a;
        return Interval<T>(qMin(a.start(), b.start()), qMax(a.end(), b.end()));
    }
    //! Subtract an interval from another
    static QList<Interval<T>> subtract(const Interval<T> &src_iv, const Interval<T> &minus_iv)
    {
        QList<Interval<T>> list;
        if ((src_iv == minus_iv) || (minus_iv.contains(src_iv)))
            return list;

        if (!src_iv.intersects(minus_iv))
            list.append(src_iv);
        else if (src_iv.end() <= minus_iv.end())
            list.append(Interval<T>(src_iv.start(), minus_iv.start() - 1));
        else if (src_iv.start() >= minus_iv.start())
            list.append(Interval<T>(minus_iv.end() + 1, src_iv.end()));
        else {
            list.append(Interval<T>(src_iv.start(), minus_iv.start() - 1));
            list.append(Interval<T>(minus_iv.end() + 1, src_iv.end()));
        }

        return list;
    }
    //! Split an interval into two
    static QList<Interval<T>> split(const Interval<T> &i, T before)
    {
        QList<Interval<T>> list;
        if (before < i.start() || before > i.end()) {
            list.append(i);
        } else {
            Interval<T> left(i.start(), before - 1);
            Interval<T> right(before, i.end());
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
        QList<Interval<T>> temp_list;
        for (int c = 0; c < list->size(); c++) {
            temp_list = subtract(list->at(c), i);
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
    //! Return a string in the format '[start,end]'
    QString toString() const
    {
        return "[" + QString::number(d_start) + "," + QString::number(d_end) + "]";
    }

protected:
    //! Interval start
    T d_start;
    //! Interval end
    T d_end;
};

//! Auxiliary class for interval based data
/**
 *	This class represents an interval of
 *	the type [start,end]. It should be pretty
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
    Interval(T start, T end) : IntervalBase<T>(start, end) { }
    T size() const { return IntervalBase<T>::d_end - IntervalBase<T>::d_start + 1; }
    bool isValid() const
    {
        return (IntervalBase<T>::d_start >= 0 && IntervalBase<T>::d_end >= 0
                && IntervalBase<T>::d_start <= IntervalBase<T>::d_end);
    }
    bool touches(const Interval<T> &other) const
    {
        return ((other.end() == IntervalBase<T>::d_start - 1)
                || (other.start() == IntervalBase<T>::d_end + 1));
    }
};

template<>
class Interval<float> : public IntervalBase<float>
{
    Interval() { }
    Interval(float start, float end) : IntervalBase<float>(start, end) { }
    Interval(const Interval<float> &other) : IntervalBase<float>(other) { }
    float size() const { return IntervalBase<float>::d_end - IntervalBase<float>::d_start; }
    bool isValid() const { return (IntervalBase<float>::d_start <= IntervalBase<float>::d_end); }
    bool touches(const Interval<float> &other) const
    {
        return ((other.end() == IntervalBase<float>::d_start)
                || (other.start() == IntervalBase<float>::d_end));
    }
};

template<>
class Interval<double> : public IntervalBase<double>
{
    Interval() { }
    Interval(double start, double end) : IntervalBase<double>(start, end) { }
    Interval(const Interval<double> &other) : IntervalBase<double>(other) { }
    double size() const { return IntervalBase<double>::d_end - IntervalBase<double>::d_start; }
    bool isValid() const { return (IntervalBase<double>::d_start <= IntervalBase<double>::d_end); }
    bool touches(const Interval<double> &other) const
    {
        return ((other.end() == IntervalBase<double>::d_start)
                || (other.start() == IntervalBase<double>::d_end));
    }
};

template<>
class Interval<long double> : public IntervalBase<long double>
{
    Interval() { }
    Interval(long double start, long double end) : IntervalBase<long double>(start, end) { }
    Interval(const Interval<long double> &other) : IntervalBase<long double>(other) { }
    long double size() const
    {
        return IntervalBase<long double>::d_end - IntervalBase<long double>::d_start;
    }
    bool isValid() const
    {
        return (IntervalBase<long double>::d_start <= IntervalBase<long double>::d_end);
    }
    bool touches(const Interval<long double> &other) const
    {
        return ((other.end() == IntervalBase<long double>::d_start)
                || (other.start() == IntervalBase<long double>::d_end));
    }
};

#endif
