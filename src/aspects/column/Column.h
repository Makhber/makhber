/***************************************************************************
    File                 : Column.h
    Project              : Makhber
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef COLUMN_H
#define COLUMN_H

#include "aspects/AbstractAspect.h"
#include "aspects/AbstractSimpleFilter.h"
#include "aspects/datatypes/NumericDateTimeBaseFilter.h"
#include "lib/IntervalAttribute.h"
#include "lib/XmlStreamReader.h"

#include <memory>

class QString;

class ColumnStringIO;

//! Aspect that manages a column
/**
  This class represents a column, i.e., (mathematically) a 1D vector of
  values with a header. It provides a public reading and (undo aware) writing
  interface as defined in AbstractColumn. It manages special attributes
  of column rows such as masking and a validity flag. A column
  can have one of currently three data types: double, QString, or
  QDateTime. The string representation of the values can differ depending
  on the mode of the column.

  Column inherits from AbstractAspect and is intended to be a child
  of the corresponding Table in the aspect hierarchy. Columns don't
  have a view as they are intended to be displayed inside a table.
 */
class MAKHBER_EXPORT Column : public AbstractColumn
{
    Q_OBJECT

public:
    class Private;
    friend class Private;
    //! Ctor
    /**
     * \param name the column name (= aspect name)
     * \param mode initial column mode
     */
    Column(const QString &name, Makhber::ColumnMode mode);
    //! Ctor
    /** @{
     * \param name the column name (= aspect name)
     * \param data initial data vector
     * \param validity a list of invalid intervals (optional)
     */
    template<class D>
    Column(const QString &name, const D &data,
           IntervalAttribute<bool> validity = IntervalAttribute<bool>())
        : AbstractColumn(name)
    {
        initPrivate(std::unique_ptr<D>(new D(data)), validity);
        init();
    }

    template<class D>
    Column(const QString &name, std::unique_ptr<D> data,
           IntervalAttribute<bool> validity = IntervalAttribute<bool>())
        : AbstractColumn(name)
    {
        initPrivate(std::move(data), validity);
        init();
    }
    /// @}

    ~Column();

    //! \name aspect related functions
    //@{
    //! Return an icon to be used for decorating the views and table column headers
    virtual QIcon icon() const override;
    //@}

    //! Return the data type of the column
    Makhber::ColumnDataType dataType() const override;
    //! Return whether the object is read-only
    bool isReadOnly() const override { return false; };
    //! Return the column mode
    /**
     * This function is most used by tables but can also be used
     * by plots. The column mode specifies how to interpret
     * the values in the column additional to the data type.
     */
    Makhber::ColumnMode columnMode() const override;
    //! Set the column mode
    /**
     * This sets the column mode and, if
     * necessary, converts it to another datatype.
     */
    void setColumnMode(Makhber::ColumnMode mode, AbstractFilter *conversion_filter = 0) override;
    //! Copy another column of the same type
    /**
     * This function will return false if the data type
     * of 'other' is not the same as the type of 'this'.
     * The validity information for the rows is also copied.
     * Use a filter to convert a column to another type.
     */
    bool copy(const AbstractColumn *other) override;
    //! Copies a part of another column of the same type
    /**
     * This function will return false if the data type
     * of 'other' is not the same as the type of 'this'.
     * The validity information for the rows is also copied.
     * \param other pointer to the column to copy
     * \param src_start first row to copy in the column to copy
     * \param dest_start first row to copy in
     * \param num_rows the number of rows to copy
     */
    bool copy(const AbstractColumn *source, int source_start, int dest_start,
              int num_rows) override;
    //! Return the data vector size
    /**
     * This returns the number of rows that actually contain data.
     * Rows beyond this can be masked etc. but should be ignored by filters,
     * plots etc.
     */
    int rowCount() const override;
    //! Insert some empty (or initialized with zero) rows
    void insertRows(int before, int count) override;
    //! Remove 'count' rows starting from row 'first'
    void removeRows(int first, int count) override;
    //! Return the column plot designation
    Makhber::PlotDesignation plotDesignation() const override;
    //! Set the column plot designation
    void setPlotDesignation(Makhber::PlotDesignation pd);
    //! Clear the whole column
    void clear() override;
    //! This must be called before the column is replaced by another
    void notifyReplacement(const AbstractColumn *replacement) override;
    //! Return the converter filter (for double <-> datetime  conversion)
    NumericDateTimeBaseFilter *numericDateTimeBaseFilter() const;
    //! Return the output filter (for data type -> string  conversion)
    /**
     * This method is mainly used to get a filter that can convert
     * the column's data type to strings (usualy to display in a view).
     */
    AbstractSimpleFilter *outputFilter() const;

    //! Return the input filter (for string -> data type conversion)
    /**
     * This method is mainly used to get a filter that can convert
     * the column's strings to a data-type (usually to check the input).
     */
    AbstractSimpleFilter *inputFilter() const;

    //! Return a wrapper column object used for String I/O.
    ColumnStringIO *asStringColumn() const { return d_string_io; }

    //! \name IntervalAttribute related functions
    //@{
    //! Return whether a certain row contains an invalid value
    bool isInvalid(int row) const override;
    //! Return whether a certain interval of rows contains only invalid values
    bool isInvalid(Interval<int> i) const override;
    //! Return all intervals of invalid rows
    QList<Interval<int>> invalidIntervals() const override;
    //! Return whether a certain row is masked
    bool isMasked(int row) const override;
    //! Return whether a certain interval of rows rows is fully masked
    bool isMasked(Interval<int> i) const override;
    //! Return all intervals of masked rows
    QList<Interval<int>> maskedIntervals() const override;
    //! Clear all validity information
    void clearValidity() override;
    //! Clear all masking information
    void clearMasks() override;
    //! Set an interval invalid or valid
    /**
     * \param i the interval
     * \param invalid true: set invalid, false: set valid
     */
    void setInvalid(Interval<int> i, bool invalid = true);
    //! Overloaded function for convenience
    void setInvalid(int row, bool invalid = true);
    //! Set an interval masked
    /**
     * \param i the interval
     * \param mask true: mask, false: unmask
     */
    void setMasked(Interval<int> i, bool mask = true);
    //! Overloaded function for convenience
    void setMasked(int row, bool mask = true);
    //@}

    //! \name Formula related functions
    //@{
    //! Return the formula associated with row 'row'
    QString formula(int row) const override;
    //! Return the intervals that have associated formulas
    /**
     * This can be used to make a list of formulas with their intervals.
     * Here is some example code:
     *
     * \code
     * QStringList list;
     * QList< Interval<int> > intervals = my_column.formulaIntervals();
     * foreach(Interval<int> interval, intervals)
     * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
     * \endcode
     */
    QList<Interval<int>> formulaIntervals() const override;
    //! Set a formula string for an interval of rows
    void setFormula(Interval<int> i, QString formula);
    //! Overloaded function for convenience
    void setFormula(int row, QString formula);
    //! Clear all formulas
    void clearFormulas() override;
    //@}

    //! \name type specific functions
    //@{
    //! Return the content of row 'row'.
    /**
     * Use this only when dataType() is QString
     */
    QString textAt(int row) const override;
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is QString
     */
    void setTextAt(int row, const QString &new_value);
    //! Replace a range of values
    /**
     * Use this only when dataType() is QString
     */
    void replaceTexts(int first, const QStringList &new_values) override;
    //! Return the date part of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    QDate dateAt(int row) const override;
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    void setDateAt(int row, const QDate &new_value);
    //! Return the time part of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    QTime timeAt(int row) const override;
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    void setTimeAt(int row, const QTime &new_value);
    //! Return the QDateTime in row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    QDateTime dateTimeAt(int row) const override;
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    void setDateTimeAt(int row, const QDateTime &new_value);
    //! Replace a range of values
    /**
     * Use this only when dataType() is QDateTime
     */
    void replaceDateTimes(int first, const QList<QDateTime> &new_values) override;
    //! Return the double value in row 'row'
    double valueAt(int row) const override;
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is double
     */
    void setValueAt(int row, double new_value);
    //! Replace a range of values
    /**
     * Use this only when dataType() is double
     */
    virtual void replaceValues(int first, const QVector<qreal> &new_values) override;
    //@}

    //! \name XML related functions
    //@{
    //! Save the column as XML
    void save(QJsonObject *) const override;
    //! Load the column from XML
    bool load(XmlStreamReader *reader) override;

private:
    //! Read XML numerical <-> date time filter element
    bool XmlReadNumericDateTimeFilter(XmlStreamReader *reader);
    //! Read XML input filter element
    bool XmlReadInputFilter(XmlStreamReader *reader);
    //! Read XML output filter element
    bool XmlReadOutputFilter(XmlStreamReader *reader);
    //! Read XML mask element
    bool XmlReadMask(XmlStreamReader *reader);
    //! Read XML formula element
    bool XmlReadFormula(XmlStreamReader *reader);
    //! Read XML row element
    bool XmlReadRow(XmlStreamReader *reader);
    //@}

private Q_SLOTS:
    void notifyDisplayChange();

private:
    //! Pointer to the private data object
    Private *d_column_private;
    ColumnStringIO *d_string_io;

    void init();
    template<class D>
    void initPrivate(std::unique_ptr<D>, IntervalAttribute<bool>);

    friend class ColumnStringIO;
};

//! String-IO interface of Column.
class MAKHBER_EXPORT ColumnStringIO : public AbstractColumn
{
    Q_OBJECT

public:
    ColumnStringIO(Column *owner)
        : AbstractColumn(tr("as string")), d_owner(owner), d_setting(false)
    {
    }
    virtual Makhber::ColumnMode columnMode() const { return Makhber::ColumnMode::Text; }
    virtual Makhber::ColumnDataType dataType() const { return Makhber::TypeQString; }
    virtual Makhber::PlotDesignation plotDesignation() const { return d_owner->plotDesignation(); }
    virtual int rowCount() const { return d_owner->rowCount(); }
    virtual QString textAt(int row) const;
    virtual void setTextAt(int row, const QString &value);
    virtual bool isInvalid(int row) const
    {
        if (d_setting)
            return false;
        else
            return d_owner->isInvalid(row);
    }
    virtual bool copy(const AbstractColumn *other);
    virtual bool copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows);
    virtual void replaceTexts(int start_row, const QStringList &texts);

private:
    Column *d_owner;
    bool d_setting;
    QString d_to_set;
};

#endif
