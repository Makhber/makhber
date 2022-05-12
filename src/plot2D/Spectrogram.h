/***************************************************************************
        File                 : Spectrogram.h
        Project              : Makhber
--------------------------------------------------------------------
        Copyright            : (C) 2006 by Ion Vasilief
        Email (use @ for *)  : ion_vasilief*yahoo.fr
        Description          : Makhber's Spectrogram Class
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

#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include "matrix/Matrix.h"

#include <qwt_matrix_raster_data.h>
#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_interval.h>

#include <limits>
#include <cmath>

class MatrixData;

class MAKHBER_EXPORT Spectrogram : public QwtPlotSpectrogram
{
public:
    Spectrogram();
    Spectrogram(Matrix *m);

    enum ColorMapPolicy { GrayScale, Default, Custom };

    Spectrogram *copy();
    Matrix *matrix() { return d_matrix; };

    int levels() { return (int)contourLevels().size() + 1; };
    void setLevelsNumber(int levels);

    bool hasColorScale();
    int colorScaleAxis() { return color_axis; };
    void showColorScale(int axis, bool on = true);

    int colorBarWidth();
    void setColorBarWidth(int width);

    void setGrayScale();
    void setDefaultColorMap();
    static QwtLinearColorMap *defaultColorMap();

    void setCustomColorMap(QwtLinearColorMap *map);
    void updateData(Matrix *m);

    //! Used when saving a project file
    void saveToJson(QJsonObject *);

    ColorMapPolicy colorMapPolicy() { return color_map_policy; };

protected:
    //! Pointer to the source data matrix
    Matrix *d_matrix;

    //! Axis used to display the color scale
    int color_axis;

    //! Flags
    ColorMapPolicy color_map_policy;

    QwtLinearColorMap *color_map;
};

class MAKHBER_EXPORT MatrixData : public QwtMatrixRasterData
{
public:
    MatrixData(Matrix *m);

    ~MatrixData() {};

    virtual QwtMatrixRasterData *copy() const { return new MatrixData(d_matrix); }

private:
    //! Pointer to the source data matrix
    Matrix *d_matrix;
};

#endif
