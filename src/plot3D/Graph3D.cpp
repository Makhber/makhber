/***************************************************************************
    File                 : Graph3D.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : 3D graph widget

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
#include "Graph3D.h"

#ifdef _MSC_VER
#define NOMINMAX
#endif

#include "plot3D/Bar.h"
#include "plot3D/Cone3D.h"
#include "scripting/MyParser.h"
#include "core/ColorButton.h"
#include "aspects/column/Column.h"

#include <qwt3d_io_gl2ps.h>
#include <qwt3d_coordsys.h>

#include <gsl/gsl_vector.h>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QClipboard>
#include <QPixmap>
#include <QBitmap>
#include <QDateTime>
#include <QCursor>
#include <QImageWriter>

#include <fstream>
#include <stdexcept>
#include <cmath>

UserFunction::UserFunction(const QString &s, Qwt3D::SurfacePlot &pw) : Qwt3D::Function(pw)
{
    formula = s;
}

double UserFunction::operator()(double x, double y)
{
    if (formula.isEmpty())
        return 0.0;

    MyParser parser;
    double result = 0.0;
    try {
        parser.DefineVar(_T("x"), &x);
        parser.DefineVar(_T("y"), &y);

        parser.SetExpr(formula);
        result = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(nullptr, "Input function error", QStringFromString(e.GetMsg()));
    }
    return result;
}

UserFunction::~UserFunction() = default;

Graph3D::Graph3D(const QString &label, QWidget *parent, const char *name, Qt::WindowFlags f)
    : MyWidget(label, parent, name, f)
{
    initPlot();
}

void Graph3D::initPlot()
{
    worksheet = nullptr;
    d_matrix = nullptr;
    plotAssociation = QString();

    QDateTime dt = QDateTime::currentDateTime();
    setBirthDate(QLocale().toString(dt));

    color_map = QString();
    animation_redraw_wait = 50;
    d_timer = new QTimer(this);
    connect(d_timer, SIGNAL(timeout()), this, SLOT(rotate()));
    ignoreFonts = false;

    auto *d_main_widget = new QWidget();
    sp = new Qwt3D::SurfacePlot(d_main_widget);
    sp->resize(500, 400);
    sp->installEventFilter(this);
    sp->setRotation(30, 0, 15);
    sp->setScale(1, 1, 1);
    sp->setShift(0.15, 0, 0);
    sp->setZoom(0.9);
    sp->setOrtho(false);

    smoothMesh = true;
    sp->setSmoothMesh(smoothMesh);

    d_autoscale = true;

    title = QString();
    sp->setTitle(title);

    titleCol = QColor(Qt::black);
    sp->setTitleColor(Qwt3D::Qt2GL(titleCol));

    titleFnt = QFont("Times New Roman", 14);
    titleFnt.setBold(true);

    sp->setTitleFont(titleFnt.family(), titleFnt.pointSize(), titleFnt.weight(), titleFnt.italic());

    axesCol = QColor(Qt::black);
    labelsCol = QColor(Qt::black);
    numCol = QColor(Qt::black);
    meshCol = QColor(Qt::black);
    gridCol = QColor(Qt::black);
    bgCol = QColor(255, 255, 255);
    fromColor = QColor(Qt::red);
    toColor = QColor(Qt::blue);

    col_ = nullptr;

    legendOn = false;
    legendMajorTicks = 5;
    sp->showColorLegend(legendOn);
    sp->legend()->setAutoScale(true);
    sp->legend()->setMajors(legendMajorTicks);

    labelsDist = 0;

    scaleType = QVector<int>(3);
    for (int j = 0; j < 3; j++)
        scaleType[j] = 0;

    pointStyle = None;
    func = nullptr;
    alpha = 1.0;
    barsRad = 0.007;
    pointSize = 5;
    smooth = false;
    crossHairRad = 0.03, crossHairLineWidth = 2;
    crossHairSmooth = true, crossHairBoxed = false;
    conesQuality = 32;
    conesRad = 0.5;

    style_ = Qwt3D::NOPLOT;
    this->setWidget(d_main_widget);
    initCoord();

    connect(sp, SIGNAL(rotationChanged(double, double, double)), this,
            SLOT(rotationChanged(double, double, double)));
    connect(sp, SIGNAL(zoomChanged(double)), this, SLOT(zoomChanged(double)));
    connect(sp, SIGNAL(scaleChanged(double, double, double)), this,
            SLOT(scaleChanged(double, double, double)));
    connect(sp, SIGNAL(shiftChanged(double, double, double)), this,
            SLOT(shiftChanged(double, double, double)));
}

void Graph3D::initCoord()
{
    sp->makeCurrent();
    for (unsigned i = 0; i != sp->coordinates()->axes.size(); ++i) {
        sp->coordinates()->axes[i].setMajors(5);
        sp->coordinates()->axes[i].setMinors(5);
    }

    QString s = tr("X axis");
    sp->coordinates()->axes[Qwt3D::X1].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::X2].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::X3].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::X4].setLabelString(s);
    labels << s;

    s = tr("Y axis");
    sp->coordinates()->axes[Qwt3D::Y1].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Y2].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Y3].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Y4].setLabelString(s);
    labels << s;

    s = tr("Z axis");
    sp->coordinates()->axes[Qwt3D::Z1].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Z2].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Z3].setLabelString(s);
    sp->coordinates()->axes[Qwt3D::Z4].setLabelString(s);
    labels << s;

    sp->setCoordinateStyle(Qwt3D::BOX);
    sp->coordinates()->setAutoScale(false);
}

void Graph3D::addFunction(const QString &s, double xl, double xr, double yl, double yr, double zl,
                          double zr)
{
    sp->makeCurrent();
    sp->resize(this->size());

    func = new UserFunction(s, *sp);

    func->setMesh(41, 31);
    func->setDomain(xl, xr, yl, yr);
    func->setMinZ(zl);
    func->setMaxZ(zr);
    func->create();

    sp->legend()->setLimits(zl, zr);

    if (sp->plotStyle() == Qwt3D::NOPLOT) {
        sp->setPlotStyle(Qwt3D::FILLED);
        style_ = Qwt3D::FILLED;
        pointStyle = None;
    }
    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    findBestLayout();
}

void Graph3D::insertFunction(const QString &s, double xl, double xr, double yl, double yr,
                             double zl, double zr)
{
    addFunction(s, xl, xr, yl, yr, zl, zr);
    update();
}

void Graph3D::addData(Table *table, int xcol, int ycol)
{
    worksheet = table;
    int r = table->numRows();
    int i = 0, xmesh = 0, ymesh = 2;
    for (i = 0; i < r; i++) {
        if (!table->column(xcol)->isInvalid(i) && !table->column(ycol)->isInvalid(i))
            xmesh++;
    }

    if (xmesh == 0)
        xmesh++;

    double **data = Matrix::allocateMatrixData(xmesh, ymesh);
    gsl_vector *x = gsl_vector_alloc(xmesh);
    gsl_vector *y = gsl_vector_alloc(xmesh);

    for (int j = 0; j < ymesh; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xcol)->isInvalid(i) && !table->column(ycol)->isInvalid(i)) {
                gsl_vector_set(x, k, table->cell(i, xcol));

                double yv = table->cell(i, ycol);
                gsl_vector_set(y, k, yv);
                data[k][j] = yv;
                k++;
            }
        }
    }

    double maxy = gsl_vector_max(y);
    double maxz = 0.6 * maxy;
    sp->makeCurrent();
    sp->legend()->setLimits(gsl_vector_min(y), maxy);
    sp->loadFromData(data, xmesh, ymesh, gsl_vector_min(x), gsl_vector_max(x), 0, maxz);

    if (d_autoscale)
        findBestLayout();

    gsl_vector_free(x);
    gsl_vector_free(y);
    Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::addData(Table *table, const QString &colName)
{
    int ycol = table->colIndex(colName);
    int xcol = table->colX(ycol);
    plotAssociation = table->colName(xcol) + "(X)," + colName + "(Y)";

    addData(table, xcol, ycol);
}

void Graph3D::addData(Table *table, const QString &xColName, const QString &yColName)
{
    int xcol = table->colIndex(xColName);
    plotAssociation = table->colName(xcol) + "(X)," + yColName + "(Y)";

    addData(table, xcol, table->colIndex(yColName));
}

void Graph3D::changeMatrix(Matrix *m)
{
    if (d_matrix == m)
        return;

    addMatrixData(m);
}

void Graph3D::addMatrixData(Matrix *m)
{
    if (d_matrix == m)
        return;

    bool first_time = false;
    if (!d_matrix)
        first_time = true;

    d_matrix = m;
    plotAssociation = "matrix<" + QString(m->name()) + ">";

    int cols = m->numCols();
    int rows = m->numRows();
    double **data_matrix = Matrix::allocateMatrixData(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            data_matrix[i][j] = m->cell(i, j);
    }

    sp->makeCurrent();
    sp->loadFromData(data_matrix, rows, cols, m->xStart(), m->xEnd(), m->yStart(), m->yEnd());

    double start = NAN, end = NAN;
    sp->coordinates()->axes[Qwt3D::Z1].limits(start, end);
    sp->legend()->setLimits(start, end);
    sp->legend()->setMajors(legendMajorTicks);

    Matrix::freeMatrixData(data_matrix, rows);

    if (d_autoscale || first_time)
        findBestLayout();
    update();
}

void Graph3D::addMatrixData(Matrix *m, double xl, double xr, double yl, double yr, double zl,
                            double zr)
{
    d_matrix = m;
    plotAssociation = "matrix<" + QString(m->name()) + ">";

    updateScalesFromMatrix(xl, xr, yl, yr, zl, zr);
}

void Graph3D::addData(Table *table, const QString &xColName, const QString &yColName, double xl,
                      double xr, double yl, double yr, double zl, double zr)
{
    worksheet = table;
    int r = table->numRows();
    int xcol = table->colIndex(xColName);
    int ycol = table->colIndex(yColName);

    QString s = table->colName(xcol) + "(X),";
    s += yColName + "(Y)";
    plotAssociation = s;

    int i = 0, j = 0, xmesh = 0, ymesh = 2;
    double xv = NAN, yv = NAN;

    for (i = 0; i < r; i++) {
        if (!table->column(xcol)->isInvalid(i) && !table->column(ycol)->isInvalid(i)) {
            xv = table->cell(i, xcol);
            if (xv >= xl && xv <= xr)
                xmesh++;
        }
    }

    if (xmesh == 0)
        xmesh++;

    double **data = Matrix::allocateMatrixData(xmesh, ymesh);
    for (j = 0; j < ymesh; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xcol)->isInvalid(i) && !table->column(ycol)->isInvalid(i)) {
                xv = table->cell(i, xcol);
                if (xv >= xl && xv <= xr) {
                    yv = table->cell(i, ycol);
                    if (yv > zr)
                        data[k][j] = zr;
                    else if (yv < zl)
                        data[k][j] = zl;
                    else
                        data[k][j] = yv;
                    k++;
                }
            }
        }
    }
    sp->makeCurrent();
    sp->loadFromData(data, xmesh, ymesh, xl, xr, yl, yr);
    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    sp->legend()->setLimits(zl, zr);
    sp->legend()->setMajors(legendMajorTicks);

    Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::insertNewData(Table *table, const QString &colName)
{
    int zCol = table->colIndex(colName);
    int yCol = table->colY(zCol);
    int xCol = table->colX(zCol);

    addData(table, xCol, yCol, zCol, Trajectory);
    update();
}

void Graph3D::changeDataColumn(Table *table, const QString &colName)
{
    int zCol = table->colIndex(colName);
    int yCol = table->colY(zCol);
    int xCol = table->colX(zCol);

    plotAssociation = table->colName(xCol) + "(X)," + table->colName(yCol) + "(Y),";
    plotAssociation += colName + "(Z)";

    updateDataXYZ(table, xCol, yCol, zCol);
    if (d_autoscale)
        findBestLayout();
}

void Graph3D::addData(Table *table, int xCol, int yCol, int zCol, int type)
{
    worksheet = table;
    int r = table->numRows();

    QString s = table->colName(xCol) + "(X),";
    s += table->colName(yCol) + "(Y),";
    s += table->colName(zCol) + "(Z)";
    plotAssociation = s;

    int i = 0, j = 0, columns = 0;
    for (i = 0; i < r; i++) {
        if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
            && !table->column(zCol)->isInvalid(i))
            columns++;
    }

    if (columns == 0)
        columns++;

    Qwt3D::Triple **data = allocateData(columns, columns);
    for (j = 0; j < columns; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
                && !table->column(zCol)->isInvalid(i)) {
                double xv = table->cell(i, xCol);
                double yv = table->cell(i, yCol);
                double zv = table->cell(i, zCol);

                data[k][j] = Qwt3D::Triple(xv, yv, zv);
                k++;
            }
        }
    }

    sp->makeCurrent();
    sp->loadFromData(data, columns, columns, false, false);

    double start = NAN, end = NAN;
    sp->coordinates()->axes[Qwt3D::Z1].limits(start, end);
    sp->legend()->setLimits(start, end);
    sp->legend()->setMajors(legendMajorTicks);

    if (type == Scatter) {
        Qwt3D::Dot d(pointSize, smooth);
        sp->setPlotStyle(d);
        pointStyle = Dots;
        style_ = Qwt3D::USER;
    } else if (type == Trajectory) {
        legendOn = false;
        sp->showColorLegend(legendOn);
    } else {
        sp->setPlotStyle(Bar(barsRad));
        pointStyle = VerticalBars;
        style_ = Qwt3D::USER;
    }

    if (d_autoscale)
        findBestLayout();
    deleteData(data, columns);
}

void Graph3D::addData(Table *table, int xCol, int yCol, int zCol, double xl, double xr, double yl,
                      double yr, double zl, double zr)
{
    worksheet = table;
    int r = table->numRows();

    QString s = table->colName(xCol) + "(X),";
    s += table->colName(yCol) + "(Y),";
    s += table->colName(zCol) + "(Z)";
    plotAssociation = s;

    int i = 0, j = 0, columns = 0;
    double xv = NAN, yv = NAN;
    for (i = 0; i < r; i++) {
        if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
            && !table->column(zCol)->isInvalid(i)) {
            xv = table->cell(i, xCol);
            yv = table->cell(i, yCol);
            if (xv >= xl && xv <= xr && yv >= yl && yv <= yr)
                columns++;
        }
    }

    if (columns == 0)
        columns++;

    Qwt3D::Triple **data = allocateData(columns, columns);
    for (j = 0; j < columns; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
                && !table->column(zCol)->isInvalid(i)) {
                xv = table->cell(i, xCol);
                yv = table->cell(i, yCol);
                if (xv >= xl && xv <= xr && yv >= yl && yv <= yr) {
                    double zv = table->cell(i, zCol);
                    if (zv > zr)
                        data[k][j] = Qwt3D::Triple(xv, yv, zr);
                    else if (zv < zl)
                        data[k][j] = Qwt3D::Triple(xv, yv, zl);
                    else
                        data[k][j] = Qwt3D::Triple(xv, yv, zv);
                    k++;
                }
            }
        }
    }
    sp->makeCurrent();
    sp->loadFromData(data, columns, columns, false, false);
    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    sp->legend()->setLimits(zl, zr);
    sp->legend()->setMajors(legendMajorTicks);

    deleteData(data, columns);
}

void Graph3D::updateData(Table *table)
{
    if (func) // function plot
        return;

    QString name = plotAssociation;
    int pos = name.indexOf("_", 0);
    int posX = name.indexOf("(", pos);
    QString xColName = name.mid(pos + 1, posX - pos - 1);

    pos = name.indexOf(",", posX);
    posX = name.indexOf("(", pos);
    QString yColName = name.mid(pos + 1, posX - pos - 1);

    int xCol = table->colIndex(xColName);
    int yCol = table->colIndex(yColName);

    if (name.contains("(Z)", Qt::CaseSensitive)) {
        pos = name.indexOf(",", posX);
        posX = name.indexOf("(", pos);
        QString zColName = name.mid(pos + 1, posX - pos - 1);
        int zCol = table->colIndex(zColName);
        updateDataXYZ(table, xCol, yCol, zCol);
    } else
        updateDataXY(table, xCol, yCol);

    if (d_autoscale)
        findBestLayout();
    update();
}

void Graph3D::updateDataXY(Table *table, int xCol, int yCol)
{
    int r = table->numRows();
    int i = 0, j = 0, xmesh = 0, ymesh = 2;

    for (i = 0; i < r; i++) {
        if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i))
            xmesh++;
    }

    if (xmesh < 2) {
        sp->setPlotStyle(Qwt3D::NOPLOT);
        update();
        return;
    }

    double **data = Matrix::allocateMatrixData(xmesh, ymesh);
    gsl_vector *x = gsl_vector_alloc(xmesh);
    gsl_vector *y = gsl_vector_alloc(xmesh);

    for (j = 0; j < ymesh; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)) {
                double xv = table->cell(i, xCol);
                double yv = table->cell(i, yCol);

                gsl_vector_set(x, k, xv);
                gsl_vector_set(y, k, yv);

                data[k][j] = yv;
                k++;
            }
        }
    }

    double minx = gsl_vector_min(x);
    double maxx = gsl_vector_max(x);
    double minz = gsl_vector_min(y);
    double maxz = gsl_vector_max(y);
    double miny = NAN, maxy = NAN;

    sp->makeCurrent();
    resetNonEmptyStyle();
    sp->coordinates()->axes[Qwt3D::Y1].limits(miny, maxy); // actual Y scale limits
    sp->loadFromData(data, xmesh, ymesh, minx, maxx, miny, maxy);
    sp->legend()->setLimits(minz, maxz);
    sp->legend()->setMajors(legendMajorTicks);

    gsl_vector_free(x);
    gsl_vector_free(y);
    Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::updateDataXYZ(Table *table, int xCol, int yCol, int zCol)
{
    int r = table->numRows();
    int i = 0, j = 0, columns = 0;

    for (i = 0; i < r; i++) {
        if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
            && !table->column(zCol)->isInvalid(i))
            columns++;
    }

    if (columns < 2) {
        sp->setPlotStyle(Qwt3D::NOPLOT);
        update();
        return;
    }

    Qwt3D::Triple **data = allocateData(columns, columns);
    gsl_vector *z = gsl_vector_alloc(columns);

    for (j = 0; j < columns; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!table->column(xCol)->isInvalid(i) && !table->column(yCol)->isInvalid(i)
                && !table->column(zCol)->isInvalid(i)) {
                double xv = table->cell(i, xCol);
                double yv = table->cell(i, yCol);
                double zv = table->cell(i, zCol);

                gsl_vector_set(z, k, zv);
                data[k][j] = Qwt3D::Triple(xv, yv, zv);
                k++;
            }
        }
    }

    double minz = gsl_vector_min(z);
    double maxz = gsl_vector_max(z);
    gsl_vector_free(z);

    sp->makeCurrent();
    resetNonEmptyStyle();

    sp->loadFromData(data, columns, columns, false, false);
    sp->legend()->setLimits(minz, maxz);
    sp->legend()->setMajors(legendMajorTicks);

    deleteData(data, columns);
}

void Graph3D::updateMatrixData(Matrix *m)
{
    int cols = m->numCols();
    int rows = m->numRows();

    double **data = Matrix::allocateMatrixData(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            data[i][j] = m->cell(i, j);
    }

    sp->loadFromData(data, rows, cols, m->xStart(), m->xEnd(), m->yStart(), m->yEnd());

    Qwt3D::Axis z_axis = sp->coordinates()->axes[Qwt3D::Z1];
    double start = NAN, end = NAN;
    z_axis.limits(start, end);
    z_axis.setMajors(z_axis.majors());
    z_axis.setMajors(z_axis.minors());

    sp->legend()->setLimits(start, end);
    sp->legend()->setMajors(legendMajorTicks);

    Matrix::freeMatrixData(data, rows);
    if (d_autoscale)
        findBestLayout();
    update();
}

void Graph3D::resetNonEmptyStyle()
{
    if (sp->plotStyle() != Qwt3D::NOPLOT)
        return; // the plot was not previousely emptied

    if (style_ == Qwt3D::USER) { // reseting the right user plot style
        switch (pointStyle) {
        case None:
            break;

        case Dots:
            sp->setPlotStyle(Qwt3D::Dot(pointSize, smooth));
            break;

        case VerticalBars:
            sp->setPlotStyle(Bar(barsRad));
            break;

        case HairCross:
            sp->setPlotStyle(Qwt3D::CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth,
                                              crossHairBoxed));
            break;

        case Cones:
            sp->setPlotStyle(Cone3D(conesRad, conesQuality));
            break;
        }
    } else
        sp->setPlotStyle(style_);
}

UserFunction *Graph3D::userFunction()
{
    if (func)
        return func;
    else
        return nullptr;
}

void Graph3D::update()
{
    sp->makeCurrent();

    resetAxesLabels();

    sp->updateData();
    sp->update();
}

void Graph3D::adjustLabels(int val)
{
    if (labelsDist != val) {
        labelsDist = val;
        sp->coordinates()->adjustLabels(val);
        sp->makeCurrent();
        sp->update();
    }
    Q_EMIT modified();
}

QFont Graph3D::numbersFont()
{
    return sp->coordinates()->axes[Qwt3D::X1].numberFont();
}

void Graph3D::setNumbersFont(const QFont &font)
{
    sp->coordinates()->setNumberFont(font);
    sp->makeCurrent();
    sp->update();
}

void Graph3D::setNumbersFont(const QStringList &lst)
{
    QFont fnt = QFont(lst[1], lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
    sp->coordinates()->setNumberFont(fnt);
}

void Graph3D::setXAxisLabelFont(const QFont &fnt)
{
    sp->coordinates()->axes[Qwt3D::X1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X4].setLabelFont(fnt);
}

void Graph3D::setYAxisLabelFont(const QFont &fnt)
{
    sp->coordinates()->axes[Qwt3D::Y1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y4].setLabelFont(fnt);
}

void Graph3D::setZAxisLabelFont(const QFont &fnt)
{
    sp->coordinates()->axes[Qwt3D::Z1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z4].setLabelFont(fnt);
}

void Graph3D::setXAxisLabelFont(const QStringList &lst)
{
    QFont fnt = QFont(lst[1], lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
    sp->coordinates()->axes[Qwt3D::X1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::X4].setLabelFont(fnt);
}

void Graph3D::setYAxisLabelFont(const QStringList &lst)
{
    QFont fnt = QFont(lst[1], lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
    sp->coordinates()->axes[Qwt3D::Y1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Y4].setLabelFont(fnt);
}

void Graph3D::setZAxisLabelFont(const QStringList &lst)
{
    QFont fnt = QFont(lst[1], lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
    sp->coordinates()->axes[Qwt3D::Z1].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z2].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z3].setLabelFont(fnt);
    sp->coordinates()->axes[Qwt3D::Z4].setLabelFont(fnt);
}

QStringList Graph3D::axisTickLengths()
{
    QStringList lst;
    double majorl = NAN, minorl = NAN;

    sp->coordinates()->axes[Qwt3D::X1].ticLength(majorl, minorl);
    lst << QString::number(majorl);
    lst << QString::number(minorl);

    sp->coordinates()->axes[Qwt3D::Y1].ticLength(majorl, minorl);
    lst << QString::number(majorl);
    lst << QString::number(minorl);

    sp->coordinates()->axes[Qwt3D::Z1].ticLength(majorl, minorl);
    lst << QString::number(majorl);
    lst << QString::number(minorl);

    return lst;
}

void Graph3D::setTickLengths(const QStringList &lst)
{
    double majorl = NAN, minorl = NAN;
    QStringList tick_length = lst;
    if (int(lst.count()) > 6)
        tick_length.removeAll(tick_length.first());

    majorl = tick_length[0].toDouble();
    minorl = tick_length[1].toDouble();
    sp->coordinates()->axes[Qwt3D::X1].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::X2].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::X3].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::X4].setTicLength(majorl, minorl);

    majorl = tick_length[2].toDouble();
    minorl = tick_length[3].toDouble();
    sp->coordinates()->axes[Qwt3D::Y1].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Y2].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Y3].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Y4].setTicLength(majorl, minorl);

    majorl = tick_length[4].toDouble();
    minorl = tick_length[5].toDouble();
    sp->coordinates()->axes[Qwt3D::Z1].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Z2].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Z3].setTicLength(majorl, minorl);
    sp->coordinates()->axes[Qwt3D::Z4].setTicLength(majorl, minorl);
}

void Graph3D::updateTickLength(int axis, double majorLength, double minorLength)
{
    double majorl = NAN, minorl = NAN;
    switch (axis) {
    case 0:
        sp->coordinates()->axes[Qwt3D::X1].ticLength(majorl, minorl);
        if (majorl != majorLength || minorl != minorLength) {
            sp->coordinates()->axes[Qwt3D::X1].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::X2].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::X3].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::X4].setTicLength(majorLength, minorLength);
        }
        break;

    case 1:
        sp->coordinates()->axes[Qwt3D::Y1].ticLength(majorl, minorl);
        if (majorl != majorLength || minorl != minorLength) {
            sp->coordinates()->axes[Qwt3D::Y1].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Y2].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Y3].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Y4].setTicLength(majorLength, minorLength);
        }
        break;

    case 2:
        sp->coordinates()->axes[Qwt3D::Z1].ticLength(majorl, minorl);
        if (majorl != majorLength || minorl != minorLength) {
            sp->coordinates()->axes[Qwt3D::Z1].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Z2].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Z3].setTicLength(majorLength, minorLength);
            sp->coordinates()->axes[Qwt3D::Z4].setTicLength(majorLength, minorLength);
        }
        break;
    }
    sp->update();
}

void Graph3D::rotationChanged(double, double, double)
{
    Q_EMIT modified();
}

void Graph3D::scaleChanged(double, double, double)
{
    Q_EMIT modified();
}

void Graph3D::shiftChanged(double, double, double)
{
    Q_EMIT modified();
}

void Graph3D::zoomChanged(double)
{
    Q_EMIT modified();
}

void Graph3D::resetAxesLabels()
{
    sp->coordinates()->axes[Qwt3D::X1].setLabelString(labels[0]);
    sp->coordinates()->axes[Qwt3D::X2].setLabelString(labels[0]);
    sp->coordinates()->axes[Qwt3D::X3].setLabelString(labels[0]);
    sp->coordinates()->axes[Qwt3D::X4].setLabelString(labels[0]);

    sp->coordinates()->axes[Qwt3D::Y1].setLabelString(labels[1]);
    sp->coordinates()->axes[Qwt3D::Y2].setLabelString(labels[1]);
    sp->coordinates()->axes[Qwt3D::Y3].setLabelString(labels[1]);
    sp->coordinates()->axes[Qwt3D::Y4].setLabelString(labels[1]);

    sp->coordinates()->axes[Qwt3D::Z1].setLabelString(labels[2]);
    sp->coordinates()->axes[Qwt3D::Z2].setLabelString(labels[2]);
    sp->coordinates()->axes[Qwt3D::Z3].setLabelString(labels[2]);
    sp->coordinates()->axes[Qwt3D::Z4].setLabelString(labels[2]);
}

void Graph3D::setAxesLabels(const QStringList &l)
{
    QString label = l[0];
    sp->coordinates()->axes[Qwt3D::X1].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::X2].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::X3].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::X4].setLabelString(label);

    label = l[1];
    sp->coordinates()->axes[Qwt3D::Y1].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Y2].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Y3].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Y4].setLabelString(label);

    label = l[2];
    sp->coordinates()->axes[Qwt3D::Z1].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Z2].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Z3].setLabelString(label);
    sp->coordinates()->axes[Qwt3D::Z4].setLabelString(label);

    labels = l;
}

void Graph3D::updateLabel(int axis, const QString &label, const QFont &f)
{
    switch (axis) {
    case 0:
        if (labels[0] != label) {
            sp->coordinates()->axes[Qwt3D::X1].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::X2].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::X3].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::X4].setLabelString(label);
            labels[0] = label;
        }
        if (sp->coordinates()->axes[Qwt3D::X1].labelFont() != f) {
            sp->coordinates()->axes[Qwt3D::X1].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::X2].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::X3].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::X4].setLabelFont(f);
        }
        break;

    case 1:
        if (labels[1] != label) {
            sp->coordinates()->axes[Qwt3D::Y1].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Y2].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Y3].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Y4].setLabelString(label);

            labels[1] = label;
        }
        if (sp->coordinates()->axes[Qwt3D::Y1].labelFont() != f) {
            sp->coordinates()->axes[Qwt3D::Y1].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Y2].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Y3].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Y4].setLabelFont(f);
        }
        break;

    case 2:
        if (labels[2] != label) {
            sp->coordinates()->axes[Qwt3D::Z1].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Z2].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Z3].setLabelString(label);
            sp->coordinates()->axes[Qwt3D::Z4].setLabelString(label);
            labels[2] = label;
        }
        if (sp->coordinates()->axes[Qwt3D::Z1].labelFont() != f) {
            sp->coordinates()->axes[Qwt3D::Z1].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Z2].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Z3].setLabelFont(f);
            sp->coordinates()->axes[Qwt3D::Z4].setLabelFont(f);
        }
        break;
    }

    sp->makeCurrent();
    sp->update();
    Q_EMIT modified();
}

QFont Graph3D::xAxisLabelFont()
{
    return sp->coordinates()->axes[Qwt3D::X1].labelFont();
}

QFont Graph3D::yAxisLabelFont()
{
    return sp->coordinates()->axes[Qwt3D::Y1].labelFont();
}

QFont Graph3D::zAxisLabelFont()
{
    return sp->coordinates()->axes[Qwt3D::Z1].labelFont();
}

double Graph3D::xStart()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::X1].limits(start, stop);
    return start;
}

double Graph3D::xStop()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::X1].limits(start, stop);
    return stop;
}

double Graph3D::yStart()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::Y1].limits(start, stop);
    return start;
}

double Graph3D::yStop()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::Y1].limits(start, stop);
    return stop;
}

double Graph3D::zStart()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::Z1].limits(start, stop);
    return start;
}

double Graph3D::zStop()
{
    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::Z1].limits(start, stop);
    return stop;
}

QStringList Graph3D::scaleLimits()
{
    QStringList limits;
    double start = NAN, stop = NAN;
    int majors = 0, minors = 0;

    sp->coordinates()->axes[Qwt3D::X1].limits(start, stop);
    majors = sp->coordinates()->axes[Qwt3D::X1].majors();
    minors = sp->coordinates()->axes[Qwt3D::X1].minors();

    limits << QString::number(start);
    limits << QString::number(stop);
    limits << QString::number(majors);
    limits << QString::number(minors);
    limits << QString::number(scaleType[0]);

    sp->coordinates()->axes[Qwt3D::Y1].limits(start, stop);
    majors = sp->coordinates()->axes[Qwt3D::Y1].majors();
    minors = sp->coordinates()->axes[Qwt3D::Y1].minors();

    limits << QString::number(start);
    limits << QString::number(stop);
    limits << QString::number(majors);
    limits << QString::number(minors);
    limits << QString::number(scaleType[1]);

    sp->coordinates()->axes[Qwt3D::Z1].limits(start, stop);
    majors = sp->coordinates()->axes[Qwt3D::Z1].majors();
    minors = sp->coordinates()->axes[Qwt3D::Z1].minors();

    limits << QString::number(start);
    limits << QString::number(stop);
    limits << QString::number(majors);
    limits << QString::number(minors);
    limits << QString::number(scaleType[2]);

    return limits;
}

QStringList Graph3D::scaleTicks()
{
    QStringList limits;
    int majors = 0, minors = 0;

    majors = sp->coordinates()->axes[Qwt3D::X1].majors();
    minors = sp->coordinates()->axes[Qwt3D::X1].minors();
    limits << QString::number(majors);
    limits << QString::number(minors);

    majors = sp->coordinates()->axes[Qwt3D::Y1].majors();
    minors = sp->coordinates()->axes[Qwt3D::Y1].minors();
    limits << QString::number(majors);
    limits << QString::number(minors);

    majors = sp->coordinates()->axes[Qwt3D::Z1].majors();
    minors = sp->coordinates()->axes[Qwt3D::Z1].minors();
    limits << QString::number(majors);
    limits << QString::number(minors);

    return limits;
}

void Graph3D::updateScale(int axis, const QStringList &options)
{
    double xMin = NAN, xMax = NAN, yMin = NAN, yMax = NAN, zMin = NAN, zMax = NAN;
    double *min = nullptr, *max = nullptr;
    int majors = 0, minors = 0, newMaj = 0, newMin = 0;
    std::array<Qwt3D::Axis *, 4> targetAxes;

    sp->makeCurrent();

    switch (axis) {
    case 0:
        targetAxes[0] = &sp->coordinates()->axes[Qwt3D::X1];
        targetAxes[1] = &sp->coordinates()->axes[Qwt3D::X2];
        targetAxes[2] = &sp->coordinates()->axes[Qwt3D::X3];
        targetAxes[3] = &sp->coordinates()->axes[Qwt3D::X4];
        min = &xMin;
        max = &xMax;
        break;
    case 1:
        targetAxes[0] = &sp->coordinates()->axes[Qwt3D::Y1];
        targetAxes[1] = &sp->coordinates()->axes[Qwt3D::Y2];
        targetAxes[2] = &sp->coordinates()->axes[Qwt3D::Y3];
        targetAxes[3] = &sp->coordinates()->axes[Qwt3D::Y4];
        min = &yMin;
        max = &yMax;
        break;
    case 2:
        targetAxes[0] = &sp->coordinates()->axes[Qwt3D::Z1];
        targetAxes[1] = &sp->coordinates()->axes[Qwt3D::Z2];
        targetAxes[2] = &sp->coordinates()->axes[Qwt3D::Z3];
        targetAxes[3] = &sp->coordinates()->axes[Qwt3D::Z4];
        min = &zMin;
        max = &zMax;
        break;
    default:
        throw std::runtime_error("invalid axis");
    }

    majors = targetAxes[0]->majors();
    minors = targetAxes[0]->minors();

    sp->coordinates()->axes[Qwt3D::X1].limits(xMin, xMax);
    sp->coordinates()->axes[Qwt3D::Y1].limits(yMin, yMax);
    sp->coordinates()->axes[Qwt3D::Z1].limits(zMin, zMax);

    if (*min != options[0].toDouble() || *max != options[1].toDouble()) {
        *min = options[0].toDouble();
        *max = options[1].toDouble();
        if (func) {
            func->setDomain(xMin, xMax, yMin, yMax);
            func->setMinZ(zMin);
            func->setMaxZ(zMax);
            func->create();
            sp->createCoordinateSystem(Qwt3D::Triple(xMin, yMin, zMin),
                                       Qwt3D::Triple(xMax, yMax, zMax));
        } else
            updateScales(xMin, xMax, yMin, yMax, zMin, zMax);
        sp->legend()->setLimits(zMin, zMax);
    }

    if (QString::number(scaleType[axis]) != options[4]) {
        if (options[4] == "0") {
            targetAxes[0]->setScale(Qwt3D::LINEARSCALE);
            if (axis == 2)
                sp->legend()->setScale(Qwt3D::LINEARSCALE);
            scaleType[axis] = 0;
        } else {
            targetAxes[0]->setScale(Qwt3D::LOG10SCALE);
            if (axis == 2)
                sp->legend()->setScale(Qwt3D::LOG10SCALE);
            scaleType[axis] = 1;
        }
    }

    newMaj = options[2].toInt();
    if (majors != newMaj)
        for (auto &targetAxe : targetAxes)
            targetAxe->setMajors(newMaj);

    newMin = options[3].toInt();
    if (minors != newMin)
        for (auto &targetAxe : targetAxes)
            targetAxe->setMinors(newMin);

    update();
    Q_EMIT modified();
}

void Graph3D::updateScales(double xl, double xr, double yl, double yr, double zl, double zr)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (d_matrix)
        updateScalesFromMatrix(xl, xr, yl, yr, zl, zr);
    else {
        QString name = plotAssociation;

        int pos = name.indexOf("_", 0);
        int posX = name.indexOf("(", pos);
        QString xColName = name.mid(pos + 1, posX - pos - 1);
        int xCol = worksheet->colIndex(xColName);

        pos = name.indexOf(",", posX);
        posX = name.indexOf("(", pos);
        QString yColName = name.mid(pos + 1, posX - pos - 1);
        int yCol = worksheet->colIndex(yColName);

        if (name.endsWith("(Z)", Qt::CaseSensitive)) {
            pos = name.indexOf(",", posX);
            posX = name.indexOf("(", pos);
            QString zColName = name.mid(pos + 1, posX - pos - 1);
            int zCol = worksheet->colIndex(zColName);

            updateScales(xl, xr, yl, yr, zl, zr, xCol, yCol, zCol);
        } else if (name.endsWith("(Y)", Qt::CaseSensitive))
            updateScales(xl, xr, yl, yr, zl, zr, xCol, yCol);
    }

    QApplication::restoreOverrideCursor();
}

void Graph3D::updateScalesFromMatrix(double xl, double xr, double yl, double yr, double zl,
                                     double zr)
{
    double xStart = qMin(d_matrix->xStart(), d_matrix->xEnd());
    double xEnd = qMax(d_matrix->xStart(), d_matrix->xEnd());
    double yStart = qMin(d_matrix->yStart(), d_matrix->yEnd());
    double yEnd = qMax(d_matrix->yStart(), d_matrix->yEnd());

    double dx = fabs((xEnd - xStart) / double(d_matrix->numCols() - 1));
    double dy = fabs((yEnd - yStart) / double(d_matrix->numRows() - 1));

    int nc = int(fabs(xr - xl) / dx) + 1;
    int nr = int(fabs(yr - yl) / dy) + 1;

    double x_begin = qMin(xl, xr);
    double y_begin = qMin(yl, yr);

    double **data_matrix = Matrix::allocateMatrixData(nc, nr);
    for (int i = 0; i < nc; i++) {
        double x = x_begin + i * dx;
        for (int j = 0; j < nr; j++) {
            double y = y_begin + j * dy;
            if (x >= xStart && x <= xEnd && y >= yStart && y <= yEnd) {
                int k = abs((y - yStart) / dy);
                int l = abs((x - xStart) / dx);
                double val = d_matrix->cell(k, l);
                if (val > zr)
                    data_matrix[i][j] = zr;
                else if (val < zl)
                    data_matrix[i][j] = zl;
                else
                    data_matrix[i][j] = val;
            } else
                data_matrix[i][j] = 0.0;
        }
    }
    sp->loadFromData(data_matrix, nc, nr, xl, xr, yl, yr);
    Matrix::freeMatrixData(data_matrix, nc);

    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    sp->legend()->setLimits(zl, zr);
    sp->legend()->setMajors(legendMajorTicks);
    update();
}

void Graph3D::updateScales(double xl, double xr, double yl, double yr, double zl, double zr,
                           int xcol, int ycol)
{
    int r = worksheet->numRows();
    int i = 0, j = 0, xmesh = 0, ymesh = 2;
    double xv = NAN, yv = NAN;

    for (i = 0; i < r; i++) {
        if (!worksheet->column(xcol)->isInvalid(i) && !worksheet->column(ycol)->isInvalid(i)) {
            xv = worksheet->cell(i, xcol);
            if (xv >= xl && xv <= xr)
                xmesh++;
        }
    }

    if (xmesh == 0)
        xmesh++;

    double **data = Matrix::allocateMatrixData(xmesh, ymesh);

    for (j = 0; j < ymesh; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!worksheet->column(xcol)->isInvalid(i) && !worksheet->column(ycol)->isInvalid(i)) {
                xv = worksheet->cell(i, xcol);
                if (xv >= xl && xv <= xr) {
                    yv = worksheet->cell(i, ycol);
                    if (yv > zr)
                        data[k][j] = zr;
                    else if (yv < zl)
                        data[k][j] = zl;
                    else
                        data[k][j] = yv;
                    k++;
                }
            }
        }
    }

    sp->loadFromData(data, xmesh, ymesh, xl, xr, yl, yr);
    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    Matrix::freeMatrixData(data, xmesh);
}

void Graph3D::updateScales(double xl, double xr, double yl, double yr, double zl, double zr,
                           int xCol, int yCol, int zCol)
{
    int r = worksheet->numRows();
    int i = 0, j = 0, columns = 0;
    double xv = NAN, yv = NAN, zv = NAN;
    for (i = 0; i < r; i++) {
        if (!worksheet->column(xCol)->isInvalid(i) && !worksheet->column(yCol)->isInvalid(i)
            && !worksheet->column(zCol)->isInvalid(i)) {
            xv = worksheet->cell(i, xCol);
            yv = worksheet->cell(i, yCol);
            if (xv >= xl && xv <= xr && yv >= yl && yv <= yr)
                columns++;
        }
    }

    if (columns == 0)
        columns++;

    Qwt3D::Triple **data = allocateData(columns, columns);
    for (j = 0; j < columns; j++) {
        int k = 0;
        for (i = 0; i < r; i++) {
            if (!worksheet->column(xCol)->isInvalid(i) && !worksheet->column(yCol)->isInvalid(i)
                && !worksheet->column(zCol)->isInvalid(i)) {
                xv = worksheet->cell(i, xCol);
                yv = worksheet->cell(i, yCol);
                if (xv >= xl && xv <= xr && yv >= yl && yv <= yr) {
                    zv = worksheet->cell(i, zCol);
                    if (zv > zr)
                        data[k][j] = Qwt3D::Triple(xv, yv, zr);
                    else if (zv < zl)
                        data[k][j] = Qwt3D::Triple(xv, yv, zl);
                    else
                        data[k][j] = Qwt3D::Triple(xv, yv, zv);
                    k++;
                }
            }
        }
    }
    sp->loadFromData(data, columns, columns, false, false);
    sp->createCoordinateSystem(Qwt3D::Triple(xl, yl, zl), Qwt3D::Triple(xr, yr, zr));
    deleteData(data, columns);
}

void Graph3D::setTicks(const QStringList &options)
{
    int min = 0, maj = 0;
    if (int(options.count()) == 6) {
        maj = options[0].toInt();
        sp->coordinates()->axes[Qwt3D::X1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X4].setMajors(maj);

        min = options[1].toInt();
        sp->coordinates()->axes[Qwt3D::X1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X4].setMinors(min);

        maj = options[2].toInt();
        sp->coordinates()->axes[Qwt3D::Y1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y4].setMajors(maj);

        min = options[3].toInt();
        sp->coordinates()->axes[Qwt3D::Y1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y4].setMinors(min);

        maj = options[4].toInt();
        sp->coordinates()->axes[Qwt3D::Z1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z4].setMajors(maj);

        min = options[5].toInt();
        sp->coordinates()->axes[Qwt3D::Z1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z4].setMinors(min);
    } else {
        maj = options[1].toInt();
        sp->coordinates()->axes[Qwt3D::X1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::X4].setMajors(maj);

        min = options[2].toInt();
        sp->coordinates()->axes[Qwt3D::X1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::X4].setMinors(min);

        maj = options[3].toInt();
        sp->coordinates()->axes[Qwt3D::Y1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Y4].setMajors(maj);

        min = options[4].toInt();
        sp->coordinates()->axes[Qwt3D::Y1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Y4].setMinors(min);

        maj = options[5].toInt();
        sp->coordinates()->axes[Qwt3D::Z1].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z2].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z3].setMajors(maj);
        sp->coordinates()->axes[Qwt3D::Z4].setMajors(maj);

        min = options[6].toInt();
        sp->coordinates()->axes[Qwt3D::Z1].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z2].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z3].setMinors(min);
        sp->coordinates()->axes[Qwt3D::Z4].setMinors(min);
    }
}

void Graph3D::setColors(const QColor &meshColor, const QColor &axesColor, const QColor &numColor,
                        const QColor &labelColor, const QColor &bgColor, const QColor &gridColor)
{
    if (meshCol != meshColor) {
        sp->setMeshColor(Qwt3D::Qt2GL(meshColor));
        meshCol = meshColor;
    }

    if (axesCol != axesColor) {
        sp->coordinates()->setAxesColor(Qwt3D::Qt2GL(axesColor));
        axesCol = axesColor;
    }

    if (numCol != numColor) {
        sp->coordinates()->setNumberColor(Qwt3D::Qt2GL(numColor));
        numCol = numColor;
    }

    if (labelsCol != labelColor) {
        sp->coordinates()->setLabelColor(Qwt3D::Qt2GL(labelColor));
        labelsCol = labelColor;
    }

    if (bgCol != bgColor) {
        sp->setBackgroundColor(Qwt3D::Qt2GL(bgColor));
        bgCol = bgColor;
    }

    if (gridCol != gridColor) {
        sp->coordinates()->setGridLinesColor(Qwt3D::Qt2GL(gridColor));
        gridCol = gridColor;
    }
}

void Graph3D::setColors(const QStringList &colors)
{
    meshCol = QColor(COLORVALUE(colors[1]));
    sp->setMeshColor(Qwt3D::Qt2GL(meshCol));

    axesCol = QColor(COLORVALUE(colors[2]));
    sp->coordinates()->setAxesColor(Qwt3D::Qt2GL(axesCol));

    numCol = QColor(COLORVALUE(colors[3]));
    sp->coordinates()->setNumberColor(Qwt3D::Qt2GL(numCol));

    labelsCol = QColor(COLORVALUE(colors[4]));
    sp->coordinates()->setLabelColor(Qwt3D::Qt2GL(labelsCol));

    bgCol = QColor(COLORVALUE(colors[5]));
    sp->setBackgroundColor(Qwt3D::Qt2GL(bgCol));

    gridCol = QColor(COLORVALUE(colors[6]));
    sp->coordinates()->setGridLinesColor(Qwt3D::Qt2GL(gridCol));

    if ((int)colors.count() > 7) {
        QColor min = QColor(COLORVALUE(colors[7]));
        QColor max = QColor(COLORVALUE(colors[8]));
        alpha = colors[9].toDouble();
        if ((int)colors.count() == 11)
            setDataColorMap(colors[10]);
        else
            setDataColors(min, max);
    }
}

void Graph3D::updateColors(const QColor &meshColor, const QColor &axesColor, const QColor &numColor,
                           const QColor &labelColor, const QColor &bgColor, const QColor &gridColor)
{
    if (meshCol != meshColor) {
        sp->setMeshColor(Qwt3D::Qt2GL(meshColor));
        meshCol = meshColor;
    }

    if (axesCol != axesColor) {
        sp->coordinates()->setAxesColor(Qwt3D::Qt2GL(axesColor));
        axesCol = axesColor;
    }

    if (numCol != numColor) {
        sp->coordinates()->setNumberColor(Qwt3D::Qt2GL(numColor));
        numCol = numColor;
    }

    if (labelsCol != labelColor) {
        sp->coordinates()->setLabelColor(Qwt3D::Qt2GL(labelColor));
        labelsCol = labelColor;
    }

    if (bgCol != bgColor) {
        sp->setBackgroundColor(Qwt3D::Qt2GL(bgColor));
        bgCol = bgColor;
    }

    if (gridCol != gridColor) {
        sp->coordinates()->setGridLinesColor(Qwt3D::Qt2GL(gridColor));
        gridCol = gridColor;
    }

    sp->updateData();
    sp->update();
    Q_EMIT modified();
}

void Graph3D::scaleFonts(double factor)
{
    QFont font = sp->coordinates()->axes[Qwt3D::X1].numberFont();
    font.setPointSizeF(font.pointSizeF() * factor);
    sp->coordinates()->setNumberFont(font);

    titleFnt.setPointSizeF(factor * titleFnt.pointSizeF());
    sp->setTitleFont(titleFnt.family(), titleFnt.pointSize(), titleFnt.weight(), titleFnt.italic());

    font = xAxisLabelFont();
    font.setPointSizeF(factor * font.pointSizeF());
    setXAxisLabelFont(font);

    font = yAxisLabelFont();
    font.setPointSizeF(factor * font.pointSizeF());
    setYAxisLabelFont(font);

    font = zAxisLabelFont();
    font.setPointSizeF(factor * font.pointSizeF());
    setZAxisLabelFont(font);
}

void Graph3D::resizeEvent(QResizeEvent *e)
{
    sp->makeCurrent();
    sp->resize(e->size());

    if (!ignoreFonts && this->isVisible()) {
        double ratio = (double)e->size().height() / (double)e->oldSize().height();
        scaleFonts(ratio);
    }

    sp->update();
    Q_EMIT resizedWindow(this);
    Q_EMIT modified();
    QMdiSubWindow::resizeEvent(e);
}

void Graph3D::contextMenuEvent(QContextMenuEvent *e)
{
    if (widget()->geometry().contains(e->pos())) {
        Q_EMIT showContextMenu();
    } else {
        Q_EMIT showTitleBarMenu();
    }
    e->accept();
}

void Graph3D::setFramed()
{
    if (sp->coordinates()->style() == Qwt3D::FRAME)
        return;

    sp->makeCurrent();
    sp->setCoordinateStyle(Qwt3D::FRAME);
}

void Graph3D::setBoxed()
{
    if (sp->coordinates()->style() == Qwt3D::BOX)
        return;

    sp->makeCurrent();
    sp->setCoordinateStyle(Qwt3D::BOX);
}

void Graph3D::setNoAxes()
{
    if (sp->coordinates()->style() == Qwt3D::NOCOORD)
        return;

    sp->makeCurrent();
    sp->setCoordinateStyle(Qwt3D::NOCOORD);
}

void Graph3D::setNoGrid()
{
    if (sp->plotStyle() == Qwt3D::FILLED)
        return;

    sp->makeCurrent();
    sp->setPlotStyle(Qwt3D::FILLED);
    sp->updateData();
    sp->update();

    style_ = Qwt3D::FILLED;
    pointStyle = None;
}

void Graph3D::setFilledMesh()
{
    if (sp->plotStyle() == Qwt3D::FILLEDMESH)
        return;

    sp->makeCurrent();
    sp->setPlotStyle(Qwt3D::FILLEDMESH);
    sp->updateData();
    sp->update();

    style_ = Qwt3D::FILLEDMESH;
    pointStyle = None;
}

void Graph3D::setHiddenLineGrid()
{
    if (sp->plotStyle() == Qwt3D::HIDDENLINE)
        return;

    sp->makeCurrent();
    sp->setPlotStyle(Qwt3D::HIDDENLINE);
    sp->showColorLegend(false);
    sp->updateData();
    sp->update();

    style_ = Qwt3D::HIDDENLINE;
    pointStyle = None;
    legendOn = false;
}

void Graph3D::setLineGrid()
{
    if (sp->plotStyle() == Qwt3D::WIREFRAME)
        return;

    sp->makeCurrent();
    sp->setPlotStyle(Qwt3D::WIREFRAME);
    sp->showColorLegend(false);
    sp->updateData();
    sp->update();

    pointStyle = None;
    style_ = Qwt3D::WIREFRAME;
    legendOn = false;
}

void Graph3D::setPointsMesh()
{
    if (!sp || pointStyle == Dots)
        return;

    pointStyle = Dots;
    style_ = Qwt3D::USER;

    sp->makeCurrent();
    sp->setPlotStyle(Qwt3D::Dot(pointSize, smooth));
    sp->updateData();
    sp->update();
}

void Graph3D::setConesMesh()
{
    if (!sp || pointStyle == Cones)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    pointStyle = Cones;
    style_ = Qwt3D::USER;

    sp->makeCurrent();
    sp->setPlotStyle(Cone3D(conesRad, conesQuality));
    sp->updateData();
    sp->update();

    QApplication::restoreOverrideCursor();
}

void Graph3D::setCrossMesh()
{
    if (!sp || pointStyle == HairCross)
        return;

    pointStyle = HairCross;
    style_ = Qwt3D::USER;

    sp->makeCurrent();
    sp->setPlotStyle(
            Qwt3D::CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth, crossHairBoxed));
    sp->updateData();
    sp->update();
}

void Graph3D::clearData()
{
    if (d_matrix)
        d_matrix = nullptr;
    else if (worksheet)
        worksheet = nullptr;
    else if (func) {
        delete func;
        func = nullptr;
    }
    plotAssociation = QString();

    sp->makeCurrent();
    sp->loadFromData(nullptr, 0, 0, false, false);
    sp->updateData();
    sp->update();
}

void Graph3D::setBarsPlot()
{
    if (pointStyle == VerticalBars)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    pointStyle = VerticalBars;
    style_ = Qwt3D::USER;

    sp->makeCurrent();
    sp->setPlotStyle(Bar(barsRad));
    sp->updateData();
    sp->update();
    QApplication::restoreOverrideCursor();
}

void Graph3D::setFloorData()
{
    if (sp->floorStyle() == Qwt3D::FLOORDATA)
        return;

    sp->makeCurrent();
    sp->setFloorStyle(Qwt3D::FLOORDATA);
    sp->updateData();
    sp->update();
}

void Graph3D::setFloorIsolines()
{
    if (sp->floorStyle() == Qwt3D::FLOORISO)
        return;

    sp->makeCurrent();
    sp->setFloorStyle(Qwt3D::FLOORISO);
    sp->updateData();
    sp->update();
}

void Graph3D::setEmptyFloor()
{
    if (sp->floorStyle() == Qwt3D::NOFLOOR)
        return;

    sp->makeCurrent();
    sp->setFloorStyle(Qwt3D::NOFLOOR);
    sp->updateData();
    sp->update();
}

void Graph3D::setMeshLineWidth(int lw)
{
    if ((int)sp->meshLineWidth() == lw)
        return;

    sp->makeCurrent();
    sp->setMeshLineWidth((double)lw);
    sp->updateData();
    sp->update();
}

int Graph3D::grids()
{
    return sp->coordinates()->grids();
}

void Graph3D::setGrid(Qwt3D::SIDE s, bool b)
{
    if (!sp)
        return;

    int sum = sp->coordinates()->grids();

    if (b)
        sum |= s;
    else
        sum &= ~s;

    sp->coordinates()->setGridLines(sum != Qwt3D::NOSIDEGRID, false, sum);
    sp->update();
    Q_EMIT modified();
}

void Graph3D::setGrid(int grids)
{
    if (!sp)
        return;

    sp->coordinates()->setGridLines(true, false, grids);
}

void Graph3D::setLeftGrid(bool b)
{
    setGrid(Qwt3D::LEFT, b);
}
void Graph3D::setRightGrid(bool b)
{
    setGrid(Qwt3D::RIGHT, b);
}
void Graph3D::setCeilGrid(bool b)
{
    setGrid(Qwt3D::CEIL, b);
}
void Graph3D::setFloorGrid(bool b)
{
    setGrid(Qwt3D::FLOOR, b);
}
void Graph3D::setFrontGrid(bool b)
{
    setGrid(Qwt3D::FRONT, b);
}
void Graph3D::setBackGrid(bool b)
{
    setGrid(Qwt3D::BACK, b);
}

void Graph3D::print()
{
    QPrinter printer;
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(false);

    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        if (Qwt3D::IO::save(sp, "makhber.png", "PNG")) {
            QPixmap p;
            p.load("makhber.png", "PNG", Qt::ColorOnly);

            QPainter paint(&printer);
            paint.drawPixmap(QPoint(0, 0), p);
            paint.end();

            QFile f("makhber.png");
            f.remove();
        } else
            QMessageBox::about(nullptr, tr("IO Error"),
                               tr("Could not print: <h4>") + QString(name()) + "</h4>.");
    }
}

void Graph3D::copyImage()
{
    QApplication::clipboard()->setPixmap(QPixmap::fromImage(sp->grabFramebuffer()),
                                         QClipboard::Clipboard);
    sp->updateData();
}

void Graph3D::exportImage(const QString &fileName, int quality, bool transparent)
{
    if (transparent) {
        QPixmap pic = QPixmap::fromImage(sp->grabFramebuffer());
        sp->updateData();

        QBitmap mask(pic.size());
        mask.fill(Qt::color1);
        QPainter p;
        p.begin(&mask);
        p.setPen(Qt::color0);

        auto background = QColor(Qt::white);
        QRgb backgroundPixel = background.rgb();
        QImage image = pic.toImage();
        for (int y = 0; y < image.height(); y++) {
            for (int x = 0; x < image.width(); x++) {
                QRgb rgb = image.pixel(x, y);
                if (rgb == backgroundPixel) // we want the frame transparent
                    p.drawPoint(x, y);
            }
        }
        p.end();
        pic.setMask(mask);
        pic.save(fileName, nullptr, quality);
    } else {
        QImage im = sp->grabFramebuffer();
        QImageWriter iw(fileName);
        iw.setQuality(quality);
        iw.write(im);
    }
}

void Graph3D::exportPDF(const QString &fileName)
{
    exportVector(fileName);
}

void Graph3D::exportVector(const QString &fileName, const QString &fileType)
{
    if (fileName.isEmpty()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Please provide a valid file name!"));
        return;
    }

    QString format = fileType;
    format = format.toUpper();

    auto *gl2ps = dynamic_cast<Qwt3D::VectorWriter *>(Qwt3D::IO::outputHandler(format));
    if (gl2ps) {
        gl2ps->setTextMode(Qwt3D::VectorWriter::NATIVE);
    }
    if (!Qwt3D::IO::save(sp, fileName, format))
        throw std::runtime_error((fileType + tr(" not supported")).toStdString());
}

bool Graph3D::eventFilter(QObject *object, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonDblClick && object == (QObject *)this->sp) {
        Q_EMIT showOptionsDialog();
        return true;
    }
    return MyWidget::eventFilter(object, e);
}

void Graph3D::setPointOptions(double size, bool s)
{
    if (pointSize == size && smooth == s)
        return;

    pointSize = size;
    smooth = s;
}

double Graph3D::barsRadius()
{
    if (sp->plotStyle() == Qwt3D::USER && sp->plotStyle() != Qwt3D::POINTS)
        return barsRad;
    else
        return 0.0;
}

void Graph3D::setBarsRadius(double rad)
{
    if (barsRad == rad)
        return;

    barsRad = rad;
}

void Graph3D::updateBars(double rad)
{
    if (barsRad == rad)
        return;

    barsRad = rad;
    sp->setPlotStyle(Bar(barsRad));
    update();
}

void Graph3D::updatePoints(double size, bool sm)
{
    if (pointStyle == Dots && pointSize == size && smooth == sm)
        return;

    pointSize = size;
    smooth = sm;
    pointStyle = Dots;

    Qwt3D::Dot d(pointSize, smooth);
    sp->setPlotStyle(d);

    update();
    Q_EMIT modified();
    Q_EMIT custom3DActions(this);
}

void Graph3D::updateCones(double rad, int quality)
{
    if (pointStyle == Cones && conesRad == rad && conesQuality == quality)
        return;

    conesRad = rad;
    conesQuality = quality;
    pointStyle = Cones;
    sp->setPlotStyle(Cone3D(conesRad, conesQuality));
    update();
    Q_EMIT modified();
    Q_EMIT custom3DActions(this);
}

void Graph3D::setConesOptions(double rad, int quality)
{
    conesRad = rad;
    conesQuality = quality;
}

void Graph3D::updateCross(double rad, double linewidth, bool smooth, bool boxed)
{
    if (pointStyle == HairCross && crossHairRad == rad && crossHairSmooth == smooth
        && crossHairBoxed == boxed && crossHairLineWidth == linewidth)
        return;

    crossHairRad = rad;
    crossHairLineWidth = linewidth;
    crossHairSmooth = smooth;
    crossHairBoxed = boxed;
    pointStyle = HairCross;

    sp->setPlotStyle(Qwt3D::CrossHair(rad, linewidth, smooth, boxed));
    update();
    Q_EMIT modified();
    Q_EMIT custom3DActions(this);
}

void Graph3D::setCrossOptions(double rad, double linewidth, bool smooth, bool boxed)
{
    crossHairRad = rad;
    crossHairLineWidth = linewidth;
    crossHairSmooth = smooth;
    crossHairBoxed = boxed;
}

void Graph3D::setStyle(Qwt3D::COORDSTYLE coord, Qwt3D::FLOORSTYLE floor, Qwt3D::PLOTSTYLE plot,
                       Graph3D::PointStyle point)
{
    sp->setCoordinateStyle(coord);
    sp->setFloorStyle(floor);

    if (point == None)
        sp->setPlotStyle(plot);
    else if (point == VerticalBars)
        sp->setPlotStyle(Bar(barsRad));
    else if (point == Dots)
        sp->setPlotStyle(Qwt3D::Dot(pointSize, smooth));
    else if (point == HairCross)
        sp->setPlotStyle(Qwt3D::CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth,
                                          crossHairBoxed));
    else if (point == Cones)
        sp->setPlotStyle(Cone3D(conesRad, conesQuality));

    pointStyle = point;
    style_ = sp->plotStyle();
}

void Graph3D::customPlotStyle(int style)
{
    sp->makeCurrent();
    if (sp->plotStyle() == style)
        return;

    switch (style) {
    case Qwt3D::WIREFRAME: {
        sp->setPlotStyle(Qwt3D::WIREFRAME);
        style_ = Qwt3D::WIREFRAME;
        pointStyle = None;

        legendOn = false;
        sp->showColorLegend(legendOn);
        break;
    }

    case Qwt3D::FILLED: {
        sp->setPlotStyle(Qwt3D::FILLED);
        style_ = Qwt3D::FILLED;
        pointStyle = None;
        break;
    }

    case Qwt3D::FILLEDMESH: {
        sp->setPlotStyle(Qwt3D::FILLEDMESH);
        style_ = Qwt3D::FILLEDMESH;
        pointStyle = None;
        break;
    }

    case Qwt3D::HIDDENLINE: {
        sp->setPlotStyle(Qwt3D::HIDDENLINE);
        style_ = Qwt3D::HIDDENLINE;
        pointStyle = None;
        legendOn = false;
        sp->showColorLegend(legendOn);
        break;
    }

    case Qwt3D::POINTS: {
        pointSize = 0.5;
        smooth = true;
        pointStyle = Dots;
        style_ = Qwt3D::USER;

        Qwt3D::Dot d(pointSize, smooth);
        sp->setPlotStyle(d);
        break;
    }

    case Qwt3D::USER: {
        pointStyle = VerticalBars;
        style_ = Qwt3D::USER;
        sp->setPlotStyle(Bar(barsRad));
        break;
    }
    }

    sp->updateData();
    sp->update();
}

void Graph3D::setStyle(const QStringList &st)
{
    if (st[1] == "nocoord")
        sp->setCoordinateStyle(Qwt3D::NOCOORD);
    else if (st[1] == "frame")
        sp->setCoordinateStyle(Qwt3D::FRAME);
    else if (st[1] == "box")
        sp->setCoordinateStyle(Qwt3D::BOX);

    if (st[2] == "nofloor")
        sp->setFloorStyle(Qwt3D::NOFLOOR);
    else if (st[2] == "flooriso")
        sp->setFloorStyle(Qwt3D::FLOORISO);
    else if (st[2] == "floordata")
        sp->setFloorStyle(Qwt3D::FLOORDATA);

    if (st[3] == "filledmesh")
        sp->setPlotStyle(Qwt3D::FILLEDMESH);
    else if (st[3] == "filled")
        sp->setPlotStyle(Qwt3D::FILLED);
    else if (st[3] == "points") {
        pointSize = st[4].toDouble();

        smooth = false;
        if (st[5] == "1")
            smooth = true;

        sp->setPlotStyle(Qwt3D::Dot(pointSize, smooth));
        pointStyle = Dots;
    } else if (st[3] == "wireframe")
        sp->setPlotStyle(Qwt3D::WIREFRAME);
    else if (st[3] == "hiddenline")
        sp->setPlotStyle(Qwt3D::HIDDENLINE);
    else if (st[3] == "bars") {
        barsRad = (st[4]).toDouble();
        sp->setPlotStyle(Bar(barsRad));
        pointStyle = VerticalBars;
    } else if (st[3] == "cones") {
        conesRad = (st[4]).toDouble();
        conesQuality = (st[5]).toInt();

        sp->setPlotStyle(Cone3D(conesRad, conesQuality));
        pointStyle = Cones;
    } else if (st[3] == "cross") {
        crossHairRad = (st[4]).toDouble();
        crossHairLineWidth = (st[5]).toDouble();

        crossHairSmooth = false;
        if (st[6] == "1")
            crossHairSmooth = true;

        crossHairBoxed = false;
        if (st[7] == "1")
            crossHairBoxed = true;

        sp->setPlotStyle(Qwt3D::CrossHair(crossHairRad, crossHairLineWidth, crossHairSmooth,
                                          crossHairBoxed));
        pointStyle = HairCross;
    }

    style_ = sp->plotStyle();
}

void Graph3D::setRotation(double xVal, double yVal, double zVal)
{
    sp->setRotation(xVal, yVal, zVal);
}

void Graph3D::setScale(double xVal, double yVal, double zVal)
{
    sp->setScale(xVal, yVal, zVal);
}

void Graph3D::setShift(double xVal, double yVal, double zVal)
{
    sp->setShift(xVal, yVal, zVal);
}

void Graph3D::setZoom(double val)
{
    sp->setZoom(val);
}

void Graph3D::updateZoom(double val)
{
    if (sp->zoom() == val)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    sp->makeCurrent();
    sp->setZoom(val);
    sp->updateData();
    sp->update();

    QApplication::restoreOverrideCursor();
}

void Graph3D::updateScaling(double xVal, double yVal, double zVal)
{
    if (sp->xScale() == xVal && sp->yScale() == yVal && sp->zScale() == zVal)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    sp->setScale(xVal, yVal, zVal);
    sp->updateData();
    sp->update();

    QApplication::restoreOverrideCursor();
}

Qwt3D::PLOTSTYLE Graph3D::plotStyle()
{
    return sp->plotStyle();
}

Qwt3D::FLOORSTYLE Graph3D::floorStyle()
{
    return sp->floorStyle();
}

Qwt3D::COORDSTYLE Graph3D::coordStyle()
{
    return sp->coordinates()->style();
}

QString Graph3D::formula()
{
    if (func)
        return func->function();
    else
        return plotAssociation;
}

QString Graph3D::saveToString(const QString &geometry)
{
    QString s = "<SurfacePlot>\n";
    s += QString(name()) + "\t";
    s += birthDate() + "\n";
    s += geometry;
    s += "SurfaceFunction\t";

    sp->makeCurrent();
    if (func)
        s += func->function() + "\t";
    else {
        s += plotAssociation;
        s += "\t";
    }

    double start = NAN, stop = NAN;
    sp->coordinates()->axes[Qwt3D::X1].limits(start, stop);
    s += QString::number(start) + "\t";
    s += QString::number(stop) + "\t";
    sp->coordinates()->axes[Qwt3D::Y1].limits(start, stop);
    s += QString::number(start) + "\t";
    s += QString::number(stop) + "\t";
    sp->coordinates()->axes[Qwt3D::Z1].limits(start, stop);
    s += QString::number(start) + "\t";
    s += QString::number(stop) + "\n";

    QString st;
    if (sp->coordinates()->style() == Qwt3D::NOCOORD)
        st = "nocoord";
    else if (sp->coordinates()->style() == Qwt3D::BOX)
        st = "box";
    else
        st = "frame";
    s += "Style\t" + st + "\t";

    switch (sp->floorStyle()) {
    case Qwt3D::NOFLOOR:
        st = "nofloor";
        break;

    case Qwt3D::FLOORISO:
        st = "flooriso";
        break;

    case Qwt3D::FLOORDATA:
        st = "floordata";
        break;
    }
    s += st + "\t";

    switch (sp->plotStyle()) {
    case Qwt3D::USER:
        if (pointStyle == VerticalBars)
            st = "bars\t" + QString::number(barsRad);
        else if (pointStyle == Dots) {
            st = "points\t" + QString::number(pointSize);
            st += "\t" + QString::number(smooth);
        } else if (pointStyle == Cones) {
            st = "cones\t" + QString::number(conesRad);
            st += "\t" + QString::number(conesQuality);
        } else if (pointStyle == HairCross) {
            st = "cross\t" + QString::number(crossHairRad);
            st += "\t" + QString::number(crossHairLineWidth);
            st += "\t" + QString::number(crossHairSmooth);
            st += "\t" + QString::number(crossHairBoxed);
        }
        break;

    case Qwt3D::WIREFRAME:
        st = "wireframe";
        break;

    case Qwt3D::HIDDENLINE:
        st = "hiddenline";
        break;

    case Qwt3D::FILLED:
        st = "filled";
        break;

    case Qwt3D::FILLEDMESH:
        st = "filledmesh";
        break;

    default:;
    }
    s += st + "\n";

    s += "grids\t";
    s += QString::number(sp->coordinates()->grids()) + "\n";

    s += "title\t";
    s += title + "\t";
    s += COLORNAME(titleCol) + "\t";
    s += titleFnt.family() + "\t";
    s += QString::number(titleFnt.pointSize()) + "\t";
    s += QString::number(titleFnt.weight()) + "\t";
    s += QString::number(titleFnt.italic()) + "\n";

    s += "colors\t";
    s += COLORNAME(meshCol) + "\t";
    s += COLORNAME(axesCol) + "\t";
    s += COLORNAME(numCol) + "\t";
    s += COLORNAME(labelsCol) + "\t";
    s += COLORNAME(bgCol) + "\t";
    s += COLORNAME(gridCol) + "\t";
    s += COLORNAME(fromColor) + "\t";
    s += COLORNAME(toColor) + "\t";
    s += QString::number(alpha) + "\t" + color_map + "\n";

    s += "axesLabels\t";
    s += labels.join("\t") + "\n";

    s += "tics\t";
    QStringList tl = scaleTicks();
    s += tl.join("\t") + "\n";

    s += "tickLengths\t";
    tl = axisTickLengths();
    s += tl.join("\t") + "\n";

    s += "options\t";
    s += QString::number(legendOn) + "\t";
    s += QString::number(sp->resolution()) + "\t";
    s += QString::number(labelsDist) + "\n";

    s += "numbersFont\t";
    QFont fnt = sp->coordinates()->axes[Qwt3D::X1].numberFont();
    s += fnt.family() + "\t";
    s += QString::number(fnt.pointSize()) + "\t";
    s += QString::number(fnt.weight()) + "\t";
    s += QString::number(fnt.italic()) + "\n";

    s += "xAxisLabelFont\t";
    fnt = sp->coordinates()->axes[Qwt3D::X1].labelFont();
    s += fnt.family() + "\t";
    s += QString::number(fnt.pointSize()) + "\t";
    s += QString::number(fnt.weight()) + "\t";
    s += QString::number(fnt.italic()) + "\n";

    s += "yAxisLabelFont\t";
    fnt = sp->coordinates()->axes[Qwt3D::Y1].labelFont();
    s += fnt.family() + "\t";
    s += QString::number(fnt.pointSize()) + "\t";
    s += QString::number(fnt.weight()) + "\t";
    s += QString::number(fnt.italic()) + "\n";

    s += "zAxisLabelFont\t";
    fnt = sp->coordinates()->axes[Qwt3D::Z1].labelFont();
    s += fnt.family() + "\t";
    s += QString::number(fnt.pointSize()) + "\t";
    s += QString::number(fnt.weight()) + "\t";
    s += QString::number(fnt.italic()) + "\n";

    s += "rotation\t";
    s += QString::number(sp->xRotation()) + "\t";
    s += QString::number(sp->yRotation()) + "\t";
    s += QString::number(sp->zRotation()) + "\n";

    s += "zoom\t";
    s += QString::number(sp->zoom()) + "\n";

    s += "scaling\t";
    s += QString::number(sp->xScale()) + "\t";
    s += QString::number(sp->yScale()) + "\t";
    s += QString::number(sp->zScale()) + "\n";

    s += "shift\t";
    s += QString::number(sp->xShift()) + "\t";
    s += QString::number(sp->yShift()) + "\t";
    s += QString::number(sp->zShift()) + "\n";

    s += "LineWidth\t";
    s += QString::number(sp->meshLineWidth()) + "\n";
    s += "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
    s += "Orthogonal\t" + QString::number(sp->ortho()) + "\n";
    s += "</SurfacePlot>\n";
    return s;
}

void Graph3D::showColorLegend(bool show)
{
    if (legendOn == show)
        return;

    sp->makeCurrent();
    sp->showColorLegend(show);

    legendOn = show;
    sp->update();
    Q_EMIT modified();
}

void Graph3D::setResolution(int r)
{
    if (sp->resolution() == r)
        return;

    sp->makeCurrent();
    sp->setResolution(r);
    sp->updateData();
    sp->update();
    Q_EMIT modified();
}

void Graph3D::setTitle(const QStringList &lst)
{
    title = lst[1];
    sp->setTitle(title);

    titleCol = QColor(COLORVALUE(lst[2]));
    sp->setTitleColor(Qwt3D::Qt2GL(titleCol));

    titleFnt = QFont(lst[3], lst[4].toInt(), lst[5].toInt(), lst[6].toInt());
    sp->setTitleFont(titleFnt.family(), titleFnt.pointSize(), titleFnt.weight(), titleFnt.italic());
}

void Graph3D::setTitle(const QString &s, const QColor &color, const QFont &font)
{
    if (title != s) {
        title = s;
        sp->setTitle(title);
    }

    titleCol = color;
    sp->setTitleColor(Qwt3D::Qt2GL(color));

    if (titleFnt != font) {
        titleFnt = font;
        sp->setTitleFont(font.family(), font.pointSize(), font.weight(), font.italic());
    }
}

void Graph3D::updateTitle(const QString &s, const QColor &color, const QFont &font)
{
    if (title != s) {
        title = s;
        sp->setTitle(title);
    }

    titleCol = color;
    sp->setTitleColor(Qwt3D::Qt2GL(color));

    if (titleFnt != font) {
        titleFnt = font;
        sp->setTitleFont(font.family(), font.pointSize(), font.weight(), font.italic());
    }

    sp->makeCurrent();
    sp->update();
    Q_EMIT modified();
}

void Graph3D::setTitleFont(const QFont &font)
{
    if (titleFnt != font) {
        titleFnt = font;
        sp->setTitleFont(font.family(), font.pointSize(), font.weight(), font.italic());
    }
}

void Graph3D::setOptions(const QStringList &lst)
{
    legendOn = false;
    if (lst[1].toInt() == 1)
        legendOn = true;
    sp->showColorLegend(legendOn);
    sp->setResolution(lst[2].toInt());
    adjustLabels(lst[3].toInt());
}

void Graph3D::setOptions(bool legend, int r, int dist)
{
    sp->showColorLegend(legend);
    legendOn = legend;
    sp->setResolution(r);
    adjustLabels(dist);
}

Qwt3D::Triple **Graph3D::allocateData(int columns, int rows)
{
    auto **data = new Qwt3D::Triple *[columns];

    for (int i = 0; i < columns; ++i) {
        data[i] = new Qwt3D::Triple[rows];
    }
    return data;
}

void Graph3D::deleteData(Qwt3D::Triple **data, int columns)
{
    for (int i = 0; i < columns; i++) {
        delete[] data[i];
    }
    delete[] data;
}

QColor Graph3D::minDataColor()
{
    return fromColor;
}

QColor Graph3D::maxDataColor()
{
    return toColor;
}

void Graph3D::setDataColors(const QColor &cMin, const QColor &cMax)
{
    if (cMin == fromColor && cMax == toColor)
        return;

    fromColor = cMin;
    toColor = cMax;

    Qwt3D::ColorVector cv;

    int size = 255;
    double dsize = size;

    double r1 = cMax.red() / dsize;
    double r2 = cMin.red() / dsize;

    double stepR = (r1 - r2) / dsize;

    double g1 = cMax.green() / dsize;
    double g2 = cMin.green() / dsize;

    double stepG = (g1 - g2) / dsize;

    double b1 = cMax.blue() / dsize;
    double b2 = cMin.blue() / dsize;

    double stepB = (b1 - b2) / dsize;

    Qwt3D::RGBA rgb;
    for (int i = 0; i < size; i++) {
        rgb.r = r1 - i * stepR;
        rgb.g = g1 - i * stepG;
        rgb.b = b1 - i * stepB;
        rgb.a = alpha;

        cv.push_back(rgb);
    }

    col_ = new Qwt3D::StandardColor(sp);
    col_->setColorVector(cv);
    sp->setDataColor(col_);

    if (legendOn) {
        sp->showColorLegend(false);
        sp->showColorLegend(legendOn);
    }
}

void Graph3D::changeTransparency(double t)
{
    if (alpha == t)
        return;

    alpha = t;

    auto *color = (Qwt3D::StandardColor *)sp->dataColor();
    color->setAlpha(t);

    sp->showColorLegend(legendOn);
    sp->updateData();
    sp->update();
    Q_EMIT modified();
}

void Graph3D::setTransparency(double t)
{
    if (alpha == t)
        return;

    alpha = t;

    auto *color = (Qwt3D::StandardColor *)sp->dataColor();
    color->setAlpha(t);
}

void Graph3D::showWorksheet()
{
    if (worksheet)
        worksheet->showMaximized();
    else if (d_matrix)
        d_matrix->showMaximized();
}

void Graph3D::setSmoothMesh(bool smooth)
{
    if (smoothMesh == smooth)
        return;

    smoothMesh = smooth;
    sp->setSmoothMesh(smoothMesh);
    sp->coordinates()->setLineSmooth(smoothMesh);
    sp->updateData();
    sp->update();
}

QString Graph3D::saveAsTemplate(const QString &geometryInfo)
{
    QString s = saveToString(geometryInfo);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lst = s.split("\n", Qt::SkipEmptyParts);
#else
    QStringList lst = s.split("\n", QString::SkipEmptyParts);
#endif
    QStringList l = lst[3].split("\t");
    l[1] = QString();
    lst[3] = l.join("\t");
    return lst.join("\n");
}

/*!
Turns 3D animation on or off
*/
void Graph3D::animate(bool on)
{
    if (on)
        d_timer->start(animation_redraw_wait); // Wait this many msecs before redraw
    else
        d_timer->stop();
}

void Graph3D::rotate()
{
    if (!sp)
        return;

    sp->setRotation(int(sp->xRotation() + 1) % 360, int(sp->yRotation() + 1) % 360,
                    int(sp->zRotation() + 1) % 360);
}

void Graph3D::setDataColorMap(const QString &fileName)
{
    if (color_map == fileName)
        return;

    Qwt3D::ColorVector cv;
    if (!openColorMap(cv, fileName))
        return;

    color_map = fileName;

    col_ = new Qwt3D::StandardColor(sp);
    col_->setColorVector(cv);

    sp->setDataColor(col_);
    sp->updateData();
    sp->showColorLegend(legendOn);
    sp->update();
}

bool Graph3D::openColorMap(Qwt3D::ColorVector &cv, QString fname)
{
    if (fname.isEmpty())
        return false;

    using std::ifstream;
    ifstream file(QWT3DLOCAL8BIT(fname));
    if (!file)
        return false;

    Qwt3D::RGBA rgb;
    cv.clear();

    while (file) {
        file >> rgb.r >> rgb.g >> rgb.b;
        file.ignore(10000, '\n');
        if (!file.good())
            break;
        else {
            rgb.a = 1;
            rgb.r /= 255;
            rgb.g /= 255;
            rgb.b /= 255;
            cv.push_back(rgb);
        }
    }
    return true;
}

void Graph3D::findBestLayout()
{
    double start = NAN, end = NAN;
    sp->coordinates()->axes[Qwt3D::X1].limits(start, end);
    double xScale = 1 / fabs(end - start);

    sp->coordinates()->axes[Qwt3D::Y1].limits(start, end);
    double yScale = 1 / fabs(end - start);

    sp->coordinates()->axes[Qwt3D::Z1].limits(start, end);
    double zScale = 1 / fabs(end - start);

    double d = (sp->hull().maxVertex - sp->hull().minVertex).length();
    sp->setScale(xScale, yScale, zScale);
    sp->setZoom(d / sqrt(3));

    double majl = 0.1 / yScale;
    updateTickLength(0, majl, 0.6 * majl);
    majl = 0.1 / xScale;
    updateTickLength(1, majl, 0.6 * majl);
    updateTickLength(2, majl, 0.6 * majl);
}

void Graph3D::copy(Graph3D *g)
{
    if (!g)
        return;

    Graph3D::PointStyle pt = g->pointType();
    if (g->plotStyle() == Qwt3D::USER) {
        switch (pt) {
        case Graph3D::None:
            break;

        case Graph3D::Dots:
            setPointOptions(g->pointsSize(), g->smoothPoints());
            break;

        case Graph3D::VerticalBars:
            setBarsRadius(g->barsRadius());
            break;

        case Graph3D::HairCross:
            setCrossOptions(g->crossHairRadius(), g->crossHairLinewidth(), g->smoothCrossHair(),
                            g->boxedCrossHair());
            break;

        case Graph3D::Cones:
            setConesOptions(g->coneRadius(), g->coneQuality());
            break;
        }
    }
    setStyle(g->coordStyle(), g->floorStyle(), g->plotStyle(), pt);
    setGrid(g->grids());
    setTitle(g->plotTitle(), g->titleColor(), g->titleFont());
    setTransparency(g->transparency());
    if (!g->colorMap().isEmpty())
        setDataColorMap(g->colorMap());
    else
        setDataColors(g->minDataColor(), g->maxDataColor());

    setColors(g->meshColor(), g->axesColor(), g->numColor(), g->labelColor(), g->bgColor(),
              g->gridColor());
    setAxesLabels(g->axesLabels());
    setTicks(g->scaleTicks());
    setTickLengths(g->axisTickLengths());
    setOptions(g->isLegendOn(), g->resolution(), g->labelsDistance());
    setNumbersFont(g->numbersFont());
    setXAxisLabelFont(g->xAxisLabelFont());
    setYAxisLabelFont(g->yAxisLabelFont());
    setZAxisLabelFont(g->zAxisLabelFont());
    setRotation(g->xRotation(), g->yRotation(), g->zRotation());
    setZoom(g->zoom());
    setScale(g->xScale(), g->yScale(), g->zScale());
    setShift(g->xShift(), g->yShift(), g->zShift());
    setMeshLineWidth((int)g->meshLineWidth());
    setOrtho(g->isOrthogonal());
    update();
    animate(g->isAnimated());
}

Graph3D::~Graph3D()
{
    if (func)
        delete func;

    delete sp;
}
