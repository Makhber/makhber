/***************************************************************************
    File                 : AbstractSimpleFilter.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Simplified filter interface for filters with
                           only one output port.

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
#ifndef ABSTRACT_SIMPLE_FILTER
#define ABSTRACT_SIMPLE_FILTER

#include "aspects/AbstractFilter.h"
#include "aspects/AbstractColumn.h"
#include "lib/IntervalAttribute.h"

#include <QUndoCommand>

// forward declaration - class follows
class SimpleFilterColumn;

/**
 * \brief Simplified filter interface for filters with only one output port.
 *
 * This class is only meant to simplify implementation of a restricted subtype of filter.
 * It should not be instantiated directly. You should always either derive from
 * AbstractFilter or (if necessary) provide an actual (non-abstract) implementation.
 *
 * The trick here is that, in a sense, the filter is its own output port. This means you
 * can implement a complete filter in only one class and don't have to coordinate data
 * transfer between a filter class and a data source class.
 * Additionaly, AbstractSimpleFilter offers some useful convenience methods which make writing
 * filters as painless as possible.
 *
 * For the data type of the output, all types supported by AbstractColumn (currently double, QString
 * and QDateTime) are supported.
 *
 * \section tutorial1 Tutorial, Step 1
 * The simplest filter you can write assumes there's also only one input port and rows on the
 * input correspond 1:1 to rows in the output. All you need to specify is what data type you
 * want to have (in this example double) on the input port and how to compute the output values:
 *
 * \code
 * 01 #include "AbstractSimpleFilter.h"
 * 02 class TutorialFilter1 : public AbstractSimpleFilter
 * 03 {
 * 04	protected:
 * 05		virtual bool inputAcceptable(int, AbstractColumn *source) {
 * 06			return (source->dataType() == Makhber::TypeDouble);
 * 07		}
 * 08	public:
 * 09		virtual Makhber::ColumnDataType dataType() const { return Makhber::TypeDouble; }
 * 10
 * 11		virtual double valueAt(int row) const {
 * 12			if (!d_inputs.value(0)) return 0.0;
 * 13			double input_value = d_inputs.value(0)->valueAt(row);
 * 14			return input_value * input_value;
 * 15		}
 * 16 };
 * \endcode
 *
 * This filter reads an input value (line 13) and returns its square (line 14).
 * Reimplementing inputAcceptable() makes sure that the data source really is of type
 * double (lines 5 to 7). Otherwise, the source will be rejected by AbstractFilter::input().
 * The output type is repoted by reimplementing dataType() (line 09).
 * Before you actually use d_inputs.value(0), make sure that the input port has
 * been connected to a data source (line 12).
 * Otherwise line 13 would result in a crash. That's it, we've already written a
 * fully-functional filter!
 *
 * Equivalently, you can write 1:1-filters for QString or QDateTime inputs by checking for
 * Makhber::TypeQString or Makhber::TypeQDateTime in line 6. You would then use
 * AbstractColumn::textAt(row) or AbstractColumn::dateTimeAt(row) in line 13 to access the input
 * data. For QString output, you need to implement AbstractColumn::textAt(row). For QDateTime
 * output, you have to implement three methods: \code virtual QDateTime dateTimeAt(int row) const;
 * virtual QDate dateAt(int row) const;
 * virtual QTime timeAt(int row) const;
 * \endcode
 *
 * \section tutorial2 Tutorial, Step 2
 * Now for something slightly more interesting: a filter that uses only every second row of its
 * input. We no longer have a 1:1 correspondence between input and output rows, so we'll have
 * to do a bit more work in order to have everything come out as expected.
 * We'll use double-typed input and output again:
 * \code
 * 01 #include "AbstractSimpleFilter.h"
 * 02 class TutorialFilter2 : public AbstractSimpleFilter
 * 03 {
 * 04	protected:
 * 05		virtual bool inputAcceptable(int, AbstractColumn *source) {
 * 06			return (source->dataType() == Makhber::TypeDouble);
 * 07		}
 * 08	public:
 * 09		virtual Makhber::ColumnDataType dataType() const { return Makhber::TypeDouble; }
 * \endcode
 * Even rows (including row 0) get dropped, odd rows are renumbered:
 * \code
 * 10	public:
 * 11 	virtual double valueAt(int row) const {
 * 12		if (!d_inputs.value(0)) return 0.0;
 * 13		return d_inputs.value(0)->valueAt(2*row + 1);
 * 14 	}
 * \endcode
 */
class MAKHBER_EXPORT AbstractSimpleFilter : public AbstractFilter
{
    Q_OBJECT

public:
    //! Ctor
    AbstractSimpleFilter();
    //! Default to one input port.
    virtual int inputCount() const override { return 1; }
    //! We manage only one output port (don't override unless you really know what you are doing).
    virtual int outputCount() const override { return 1; }
    //! Return a pointer to #d_output_column on port 0 (don't override unless you really know what you are doing).
    virtual AbstractColumn *output(int port) override;
    virtual const AbstractColumn *output(int port) const override;
    //! Copy plot designation of input port 0.
    virtual Makhber::PlotDesignation plotDesignation() const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->plotDesignation() : Makhber::noDesignation;
    }
    //! Return the data type of the input
    virtual Makhber::ColumnDataType dataType() const
    {
        // calling this function while d_input is empty is a sign of very bad code
        // nevertheless it will return some rather meaningless value to
        // avoid crashes
        return d_inputs.value(0) ? d_inputs.at(0)->dataType() : Makhber::TypeQString;
    }
    //! Return the column mode
    /**
     * This function is most used by tables but can also be used
     * by plots. The column mode specifies how to interpret
     * the values in the column additional to the data type.
     */
    virtual Makhber::ColumnMode columnMode() const
    {
        // calling this function while d_input is empty is a sign of very bad code
        // nevertheless it will return some rather meaningless value to
        // avoid crashes
        return d_inputs.value(0) ? d_inputs.at(0)->columnMode() : Makhber::ColumnMode::Text;
    }
    //! Return the content of row 'row'.
    /**
     * Use this only when dataType() is QString
     */
    virtual QString textAt(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->textAt(row) : QString();
    }
    //! Return the date part of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    virtual QDate dateAt(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->dateAt(row) : QDate();
    }
    //! Return the time part of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    virtual QTime timeAt(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->timeAt(row) : QTime();
    }
    //! Set the content of row 'row'
    /**
     * Use this only when dataType() is QDateTime
     */
    virtual QDateTime dateTimeAt(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->dateTimeAt(row) : QDateTime();
    }
    //! Return the double value in row 'row'
    /**
     * Use this only when dataType() is double
     */
    virtual double valueAt(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->valueAt(row) : 0.0;
    }

    //!\name assuming a 1:1 correspondence between input and output rows
    //@{
    virtual int rowCount() const { return d_inputs.value(0) ? d_inputs.at(0)->rowCount() : 0; }
    virtual QList<Interval<int>> dependentRows(Interval<int> input_range) const
    {
        return QList<Interval<int>>() << input_range;
    }
    //@}

    //!\name Masking
    //@{
    //! Return whether a certain row is masked
    virtual bool isMasked(int row) const { return d_masking.isSet(row); }
    //! Return whether a certain interval of rows rows is fully masked
    virtual bool isMasked(Interval<int> i) const { return d_masking.isSet(i); }
    //! Return all intervals of masked rows
    virtual QList<Interval<int>> maskedIntervals() const { return d_masking.intervals(); }
    //! Clear all masking information
    virtual void clearMasks();
    //! Set an interval masked
    /**
     * \param i the interval
     * \param mask true: mask, false: unmask
     */
    virtual void setMasked(Interval<int> i, bool mask = true);
    //! Overloaded function for convenience
    virtual void setMasked(int row, bool mask = true) { setMasked(Interval<int>(row, row), mask); }
    //@}

    //! Return whether a certain row contains an invalid value
    virtual bool isInvalid(int row) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->isInvalid(row) : false;
    }
    //! Return whether a certain interval of rows contains only invalid values
    virtual bool isInvalid(Interval<int> i) const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->isInvalid(i) : false;
    }
    //! Return all intervals of invalid rows
    virtual QList<Interval<int>> invalidIntervals() const
    {
        return d_inputs.value(0) ? d_inputs.at(0)->invalidIntervals() : QList<Interval<int>>();
    }

    //! \name Json related functions
    //@{
    //! Save to Json
    virtual void save(QJsonObject *) const override;
    //! Load from Json
    virtual bool load(QJsonObject *reader) override;
    //! Override this in derived classes if they have other attributes than filter_name
    virtual void writeExtraAttributes(QJsonObject *jsObject) const { Q_UNUSED(jsObject) }
    //@}

protected:
    IntervalAttribute<bool> d_masking;

    //!\name signal handlers
    //@{
    virtual void inputPlotDesignationAboutToChange(const AbstractColumn *) override;
    virtual void inputPlotDesignationChanged(const AbstractColumn *) override;
    virtual void inputModeAboutToChange(const AbstractColumn *) override;
    virtual void inputModeChanged(const AbstractColumn *) override;
    virtual void inputDataAboutToChange(const AbstractColumn *) override;
    virtual void inputDataChanged(const AbstractColumn *) override;

    virtual void inputRowsAboutToBeInserted(const AbstractColumn *source, int before,
                                            int count) override;
    virtual void inputRowsInserted(const AbstractColumn *source, int before, int count) override;
    virtual void inputRowsAboutToBeRemoved(const AbstractColumn *source, int first,
                                           int count) override;
    virtual void inputRowsRemoved(const AbstractColumn *source, int first, int count) override;
    //@}

    SimpleFilterColumn *d_output_column;
};

class MAKHBER_EXPORT SimpleFilterColumn : public AbstractColumn
{
    Q_OBJECT

public:
    explicit SimpleFilterColumn(AbstractSimpleFilter *owner)
        : AbstractColumn(owner->name()), d_owner(owner)
    {
    }

    virtual Makhber::ColumnDataType dataType() const override { return d_owner->dataType(); }
    virtual Makhber::ColumnMode columnMode() const override { return d_owner->columnMode(); }
    virtual int rowCount() const override { return d_owner->rowCount(); }
    virtual Makhber::PlotDesignation plotDesignation() const override
    {
        return d_owner->plotDesignation();
    }
    virtual bool isInvalid(int row) const override { return d_owner->isInvalid(row); }
    virtual bool isInvalid(Interval<int> i) const override { return d_owner->isInvalid(i); }
    virtual QList<Interval<int>> invalidIntervals() const override
    {
        return d_owner->invalidIntervals();
    }
    virtual bool isMasked(int row) const override { return d_owner->isMasked(row); }
    virtual bool isMasked(Interval<int> i) const override { return d_owner->isMasked(i); }
    virtual QList<Interval<int>> maskedIntervals() const override
    {
        return d_owner->maskedIntervals();
    }
    virtual void clearMasks() override { d_owner->clearMasks(); }
    virtual QString textAt(int row) const override { return d_owner->textAt(row); }
    virtual QDate dateAt(int row) const override { return d_owner->dateAt(row); }
    virtual QTime timeAt(int row) const override { return d_owner->timeAt(row); }
    virtual QDateTime dateTimeAt(int row) const override { return d_owner->dateTimeAt(row); }
    virtual double valueAt(int row) const override { return d_owner->valueAt(row); }

private:
    AbstractSimpleFilter *d_owner;

    friend class AbstractSimpleFilter;
};

#endif // ifndef ABSTRACT_SIMPLE_FILTER
