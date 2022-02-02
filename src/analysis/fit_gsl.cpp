/***************************************************************************
   File                 : fit_gsl.cpp
   Project              : Makhber
   Description          : Built-in data fit models for Makhber
   --------------------------------------------------------------------
   Copyright            : (C) 2004-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
                          (C) 2008-2009 Knut Franke (knut.franke*gmx.de)
                                                  (replace * with @ in the email address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  As a special exception, you may omit the above copyright notice when   *
 *  distributing modified copies of this file (for instance, when using it *
 *  as a template for your own fit plugin).                                *
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

#include "fit_gsl.h"

#include "analysis/Fit.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_math.h>

#include <QMessageBox>

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cmath>

int expd3_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double t1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double t2 = gsl_vector_get(x, 3);
    double A3 = gsl_vector_get(x, 4);
    double t3 = gsl_vector_get(x, 5);
    double y0 = gsl_vector_get(x, 6);

    size_t i = 0;
    for (i = 0; i < n; i++) {
        double Yi = A1 * exp(-X[i] * t1) + A2 * exp(-X[i] * t2) + A3 * exp(-X[i] * t3) + y0;
        gsl_vector_set(f, i, (Yi - Y[i]) / sigma[i]);
    }

    return GSL_SUCCESS;
}

double expd3_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double t1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double t2 = gsl_vector_get(x, 3);
    double A3 = gsl_vector_get(x, 4);
    double t3 = gsl_vector_get(x, 5);
    double y0 = gsl_vector_get(x, 6);

    size_t i = 0;
    double val = 0;
    for (i = 0; i < n; i++) {
        double dYi =
                ((A1 * exp(-X[i] * t1) + A2 * exp(-X[i] * t2) + A3 * exp(-X[i] * t3) + y0) - Y[i])
                / sigma[i];
        val += dYi * dYi;
    }

    return val;
}

int expd3_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double l1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double l2 = gsl_vector_get(x, 3);
    double A3 = gsl_vector_get(x, 4);
    double l3 = gsl_vector_get(x, 5);

    size_t i = 0;
    for (i = 0; i < n; i++) {
        /* Jacobian matrix J(i,j) = dfi / dxj, */
        /* where fi = (Yi - yi)/sigma[i],      */
        /*       Yi = A1 * exp(-xi*l1) + A2 * exp(-xi*l2) +y0  */
        /* and the xj are the parameters (A1,l1,A2,l2,y0) */
        double t = X[i];
        double s = sigma[i];
        double e1 = exp(-t * l1) / s;
        double e2 = exp(-t * l2) / s;
        double e3 = exp(-t * l3) / s;

        gsl_matrix_set(J, i, 0, e1);
        gsl_matrix_set(J, i, 1, -t * A1 * e1);
        gsl_matrix_set(J, i, 2, e2);
        gsl_matrix_set(J, i, 3, -t * A2 * e2);
        gsl_matrix_set(J, i, 4, e3);
        gsl_matrix_set(J, i, 5, -t * A3 * e3);
        gsl_matrix_set(J, i, 6, 1 / s);
    }
    return GSL_SUCCESS;
}

int expd3_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    expd3_f(x, params, f);
    expd3_df(x, params, J);

    return GSL_SUCCESS;
}

int expd2_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double t1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double t2 = gsl_vector_get(x, 3);
    double y0 = gsl_vector_get(x, 4);

    size_t i = 0;
    for (i = 0; i < n; i++) {
        double Yi = A1 * exp(-X[i] * t1) + A2 * exp(-X[i] * t2) + y0;
        gsl_vector_set(f, i, (Yi - Y[i]) / sigma[i]);
    }

    return GSL_SUCCESS;
}

double expd2_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double t1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double t2 = gsl_vector_get(x, 3);
    double y0 = gsl_vector_get(x, 4);

    size_t i = 0;
    double val = 0;
    for (i = 0; i < n; i++) {
        double dYi = ((A1 * exp(-X[i] * t1) + A2 * exp(-X[i] * t2) + y0) - Y[i]) / sigma[i];
        val += dYi * dYi;
    }

    return val;
}

int expd2_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double l1 = gsl_vector_get(x, 1);
    double A2 = gsl_vector_get(x, 2);
    double l2 = gsl_vector_get(x, 3);

    size_t i = 0;

    for (i = 0; i < n; i++) {
        /* Jacobian matrix J(i,j) = dfi / dxj, */
        /* where fi = (Yi - yi)/sigma[i],      */
        /*       Yi = A1 * exp(-xi*l1) + A2 * exp(-xi*l2) +y0  */
        /* and the xj are the parameters (A1,l1,A2,l2,y0) */
        double s = sigma[i];
        double t = X[i];
        double e1 = exp(-t * l1) / s;
        double e2 = exp(-t * l2) / s;

        gsl_matrix_set(J, i, 0, e1);
        gsl_matrix_set(J, i, 1, -t * A1 * e1);
        gsl_matrix_set(J, i, 2, e2);
        gsl_matrix_set(J, i, 3, -t * A2 * e2);
        gsl_matrix_set(J, i, 4, 1 / s);
    }
    return GSL_SUCCESS;
}

int expd2_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    expd2_f(x, params, f);
    expd2_df(x, params, J);
    return GSL_SUCCESS;
}

int exp_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A = gsl_vector_get(x, 0);
    double lambda = gsl_vector_get(x, 1);
    double b = gsl_vector_get(x, 2);
    size_t i = 0;
    for (i = 0; i < n; i++) {
        double Yi = A * exp(-lambda * X[i]) + b;
        gsl_vector_set(f, i, (Yi - Y[i]) / sigma[i]);
    }
    return GSL_SUCCESS;
}

double exp_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A = gsl_vector_get(x, 0);
    double lambda = gsl_vector_get(x, 1);
    double b = gsl_vector_get(x, 2);
    size_t i = 0;
    double val = 0;
    for (i = 0; i < n; i++) {
        double dYi = ((A * exp(-lambda * X[i]) + b) - Y[i]) / sigma[i];
        val += dYi * dYi;
    }
    return val;
}

int exp_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    double A = gsl_vector_get(x, 0);
    double lambda = gsl_vector_get(x, 1);
    size_t i = 0;
    for (i = 0; i < n; i++) {
        /* Jacobian matrix J(i,j) = dfi / dxj, */
        /* where fi = (Yi - yi)/sigma[i],      */
        /*       Yi = A * exp(-lambda * i) + b  */
        /* and the xj are the parameters (A,lambda,b) */

        double t = X[i];
        double s = sigma[i];
        double e = exp(-lambda * t);
        gsl_matrix_set(J, i, 0, e / s);
        gsl_matrix_set(J, i, 1, -t * A * e / s);
        gsl_matrix_set(J, i, 2, 1 / s);
    }
    return GSL_SUCCESS;
}

int exp_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    exp_f(x, params, f);
    exp_df(x, params, J);
    return GSL_SUCCESS;
}

int gauss_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double Y0 = gsl_vector_get(x, 0);
    double A = gsl_vector_get(x, 1);
    double C = gsl_vector_get(x, 2);
    double w = gsl_vector_get(x, 3);

    size_t i = 0;

    for (i = 0; i < n; i++) {
        double diff = X[i] - C;
        double Yi = A * exp(-0.5 * diff * diff / (w * w)) + Y0;
        gsl_vector_set(f, i, (Yi - Y[i]) / sigma[i]);
    }
    return GSL_SUCCESS;
}

double gauss_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double Y0 = gsl_vector_get(x, 0);
    double A = gsl_vector_get(x, 1);
    double C = gsl_vector_get(x, 2);
    double w = gsl_vector_get(x, 3);

    size_t i = 0;
    double val = 0;

    for (i = 0; i < n; i++) {
        double diff = X[i] - C;
        double dYi = ((A * exp(-0.5 * diff * diff / (w * w)) + Y0) - Y[i]) / sigma[i];
        val += dYi * dYi;
    }
    return val;
}

int gauss_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    double A = gsl_vector_get(x, 1);
    double C = gsl_vector_get(x, 2);
    double w = gsl_vector_get(x, 3);

    size_t i = 0;
    for (i = 0; i < n; i++) {
        /* Jacobian matrix J(i,j) = dfi / dxj,	 */
        /* where fi = Yi - yi,					*/
        /* Yi = y=A*exp[-(Xi-xc)^2/(2*w*w)]+B		*/
        /* and the xj are the parameters (B,A,C,w) */

        double s = sigma[i];
        double diff = X[i] - C;
        double e = exp(-0.5 * diff * diff / (w * w)) / s;

        gsl_matrix_set(J, i, 0, 1 / s);
        gsl_matrix_set(J, i, 1, e);
        gsl_matrix_set(J, i, 2, diff * A * e / (w * w));
        gsl_matrix_set(J, i, 3, diff * diff * A * e / (w * w * w));
    }
    return GSL_SUCCESS;
}

int gauss_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    gauss_f(x, params, f);
    gauss_df(x, params, J);

    return GSL_SUCCESS;
}

int gauss_multi_peak_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w2 = new double[peaks];
    double offset = gsl_vector_get(x, p - 1);

    size_t i = 0, j = 0;
    for (i = 0; i < peaks; i++) {
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        double wi = gsl_vector_get(x, 3 * i + 2);
        a[i] = sqrt(M_2_PI) * gsl_vector_get(x, 3 * i) / wi;
        w2[i] = wi * wi;
    }
    for (i = 0; i < n; i++) {
        double res = 0;
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            res += a[j] * exp(-2 * diff * diff / w2[j]);
        }
        gsl_vector_set(f, i, (res + offset - Y[i]) / sigma[i]);
    }
    delete[] a;
    delete[] xc;
    delete[] w2;
    return GSL_SUCCESS;
}

double gauss_multi_peak_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w2 = new double[peaks];
    double offset = gsl_vector_get(x, p - 1);

    size_t i = 0, j = 0;
    double val = 0;
    for (i = 0; i < peaks; i++) {
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        double wi = gsl_vector_get(x, 3 * i + 2);
        a[i] = sqrt(M_2_PI) * gsl_vector_get(x, 3 * i) / wi;
        w2[i] = wi * wi;
    }
    double t = NAN;
    for (i = 0; i < n; i++) {
        double res = 0;
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            res += a[j] * exp(-2 * diff * diff / w2[j]);
        }
        t = (res + offset - Y[i]) / sigma[i];
        val += t * t;
    }
    delete[] a;
    delete[] xc;
    delete[] w2;
    return val;
}

int gauss_multi_peak_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w = new double[peaks];

    size_t i = 0, j = 0;
    for (i = 0; i < peaks; i++) {
        a[i] = gsl_vector_get(x, 3 * i);
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        w[i] = gsl_vector_get(x, 3 * i + 2);
    }
    for (i = 0; i < n; i++) {
        double s = sigma[i];
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            double w2 = w[j] * w[j];
            double e = sqrt(M_2_PI) / s * exp(-2 * diff * diff / w2);

            gsl_matrix_set(J, i, 3 * j, e / w[j]);
            gsl_matrix_set(J, i, 3 * j + 1, 4 * diff * a[j] * e / (w2 * w[j]));
            gsl_matrix_set(J, i, 3 * j + 2, a[j] / w2 * e * (4 * diff * diff / w2 - 1));
        }
        gsl_matrix_set(J, i, p - 1, 1.0 / s);
    }
    delete[] a;
    delete[] xc;
    delete[] w;
    return GSL_SUCCESS;
}

int gauss_multi_peak_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    gauss_multi_peak_f(x, params, f);
    gauss_multi_peak_df(x, params, J);
    return GSL_SUCCESS;
}

int lorentz_multi_peak_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w = new double[peaks];
    double offset = gsl_vector_get(x, p - 1);

    size_t i = 0, j = 0;
    for (i = 0; i < peaks; i++) {
        a[i] = gsl_vector_get(x, 3 * i);
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        w[i] = gsl_vector_get(x, 3 * i + 2);
    }
    for (i = 0; i < n; i++) {
        double res = 0;
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            res += a[j] * w[j] / (4 * diff * diff + w[j] * w[j]);
        }
        gsl_vector_set(f, i, (res + offset - Y[i]) / sigma[i]);
    }
    delete[] a;
    delete[] xc;
    delete[] w;
    return GSL_SUCCESS;
}

double lorentz_multi_peak_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w = new double[peaks];
    double offset = gsl_vector_get(x, p - 1);

    size_t i = 0, j = 0;
    double val = 0, t = NAN;
    for (i = 0; i < peaks; i++) {
        a[i] = gsl_vector_get(x, 3 * i);
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        w[i] = gsl_vector_get(x, 3 * i + 2);
    }
    for (i = 0; i < n; i++) {
        double res = 0;
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            res += a[j] * w[j] / (4 * diff * diff + w[j] * w[j]);
        }
        t = (res + offset - Y[i]) / sigma[i];
        val += t * t;
    }
    delete[] a;
    delete[] xc;
    delete[] w;
    return GSL_SUCCESS;
}

int lorentz_multi_peak_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    size_t p = ((struct FitData *)params)->p;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    size_t peaks = (p - 1) / 3;
    auto *a = new double[peaks];
    auto *xc = new double[peaks];
    auto *w = new double[peaks];

    size_t i = 0, j = 0;
    for (i = 0; i < peaks; i++) {
        a[i] = gsl_vector_get(x, 3 * i);
        xc[i] = gsl_vector_get(x, 3 * i + 1);
        w[i] = gsl_vector_get(x, 3 * i + 2);
    }
    for (i = 0; i < n; i++) {
        double s = sigma[i];
        for (j = 0; j < peaks; j++) {
            double diff = X[i] - xc[j];
            double w2 = w[j] * w[j];
            double num = 1.0 / (4 * diff * diff + w2);
            double den = 4 * diff * diff - w2;

            gsl_matrix_set(J, i, 3 * j, w[j] * num / s);
            gsl_matrix_set(J, i, 3 * j + 1, 8 * diff * a[j] * w[j] * num * sqrt(num) / s);
            gsl_matrix_set(J, i, 3 * j + 2, den * a[j] * num * num / s);
        }
        gsl_matrix_set(J, i, p - 1, 1.0 / s);
    }
    delete[] a;
    delete[] xc;
    delete[] w;
    return GSL_SUCCESS;
}

int lorentz_multi_peak_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    lorentz_multi_peak_f(x, params, f);
    lorentz_multi_peak_df(x, params, J);
    return GSL_SUCCESS;
}

int user_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    return static_cast<struct FitData *>(params)->fit->evaluate_f(x, f);
}

double user_d(const gsl_vector *x, void *params)
{
    return static_cast<struct FitData *>(params)->fit->evaluate_d(x);
}

int user_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    return static_cast<struct FitData *>(params)->fit->evaluate_df(x, J);
}

int user_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    user_f(x, params, f);
    user_df(x, params, J);
    return GSL_SUCCESS;
}

int boltzmann_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double A2 = gsl_vector_get(x, 1);
    double x0 = gsl_vector_get(x, 2);
    double dx = gsl_vector_get(x, 3);
    size_t i = 0;
    for (i = 0; i < n; i++) {
        double Yi = (A1 - A2) / (1 + exp((X[i] - x0) / dx)) + A2;
        gsl_vector_set(f, i, (Yi - Y[i]) / sigma[i]);
    }

    return GSL_SUCCESS;
}

double boltzmann_d(const gsl_vector *x, void *params)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *Y = ((struct FitData *)params)->Y;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double A2 = gsl_vector_get(x, 1);
    double x0 = gsl_vector_get(x, 2);
    double dx = gsl_vector_get(x, 3);
    size_t i = 0;
    double val = 0;
    for (i = 0; i < n; i++) {
        double dYi = ((A1 - A2) / (1 + exp((X[i] - x0) / dx)) + A2 - Y[i]) / sigma[i];
        val += dYi * dYi;
    }
    return val;
}

int boltzmann_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
    size_t n = ((struct FitData *)params)->n;
    double *X = ((struct FitData *)params)->X;
    double *sigma = ((struct FitData *)params)->sigma;

    double A1 = gsl_vector_get(x, 0);
    double A2 = gsl_vector_get(x, 1);
    double x0 = gsl_vector_get(x, 2);
    double dx = gsl_vector_get(x, 3);
    size_t i = 0;
    for (i = 0; i < n; i++) {
        /* Jacobian matrix J(i,j) = dfi / dxj,		*/
        /* where fi = Yi - yi,						*/
        /* Yi = (A1-A2)/(1+exp((X[i]-x0)/dx)) + A2	*/
        /* and the xj are the parameters (A1,A2,x0,dx)*/
        double s = sigma[i];
        double diff = X[i] - x0;
        double e = exp(diff / dx);
        double r = 1 / (1 + e);
        double aux = (A1 - A2) * e * r * r / (dx * s);
        gsl_matrix_set(J, i, 0, r / s);
        gsl_matrix_set(J, i, 1, (1 - r) / s);
        gsl_matrix_set(J, i, 2, aux);
        gsl_matrix_set(J, i, 3, aux * diff / dx);
    }
    return GSL_SUCCESS;
}

int boltzmann_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
{
    boltzmann_f(x, params, f);
    boltzmann_df(x, params, J);
    return GSL_SUCCESS;
}
