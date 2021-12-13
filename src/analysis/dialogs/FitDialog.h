/***************************************************************************
    File                 : FitDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fit Wizard

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
#ifndef FITDIALOG_H
#define FITDIALOG_H

#include "plot2D/Graph.h"

class QPushButton;
class QLineEdit;
class QComboBox;
class QStackedWidget;
class QWidget;
class QTextEdit;
class QListWidget;
class QCheckBox;
class QTableWidget;
class QSpinBox;
class QLabel;
class QRadioButton;
class QLineEdit;
class ColorButton;
class Fit;

//! Fit Wizard
class MAKHBER_EXPORT FitDialog : public QDialog
{
    Q_OBJECT

public:
    FitDialog(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget);

protected:
    //! On closing show the fit function in the plot window if that option is selected
    void closeEvent(QCloseEvent *e);
    //! Initialized the widget for the first dialog page
    void initFitPage();
    //! Initialized the widget for the second dialog page
    void initEditPage();
    //! Initialized the widget for the third dialog page
    void initAdvancedPage();

public Q_SLOTS:
    //! Start the actual fitting
    void accept();
    //! Add a list of user defined functions
    void addUserFunctions(const QStringList &list);
    //! Clear the list of user defined functions
    void clearUserFunctions();
    //! Clears the function editor, the parameter names and the function name
    void resetFunction();
    //! Show the fit page
    void showFitPage();
    //! Show the edit function page
    void showEditPage();
    //! Show the advanced options page
    void showAdvancedPage();
    //! Populate the functions list with functions from the given 'category'
    void showFunctionsList(int category);
    //! Populate the functions list with functions from MyParser
    void showParseFunctions();
    //! Populate the functions list with user-defined functions
    void showUserFunctions();
    //! Load the fit plugins
    void loadPlugins();
    //! Populate the expression text box according to the given function
    void showExpression(int function);
    //! Paste the expression of the current function into the editing text box
    void pasteExpression();
    //! Paste the name of the current function into the editing text box
    void pasteFunctionName();
    //! Toggle between predefined function and function editing
    void setFunction(bool ok);
    //! Save the user-defined function
    void saveUserFunction();
    //! Remove a user-defined function
    void removeUserFunction();
    //! Populate the list of build-in function names
    void setBuiltInFunctionNames();
    //! Populate the list of build-in function
    void setBuiltInFunctions();
    //! Check whether the given function contains a user-defined function
    bool containsUserFunctionName(const QString &function);
    //! Set the graph that is to be fitted
    void setGraph(Graph *g);
    //! Read the range of curve 'curveName' and set the from/to fields
    void activateCurve(const QString &curveName);
    //! Let the user select the Plugin folder
    void choosePluginsFolder();
    //! Check the validity of the initial values
    bool validInitialValues();
    //! Read the selected data range from the graph
    void changeDataRange();
    //! Fit using a built-in function
    void fitBuiltInFunction(const QString &function, double *initVal);

    //! Populate the list of tables containing data displayed in the corresponding graph
    void setSrcTables(QList<MyWidget *> *tables);
    //! Select a table from the source tables list
    void selectSrcTable(int tabnr);
    //! Enable/disable widgets for selection of custom error source.
    void yErrorSourceChanged(int index);
    //! Enable the X points spin box
    void showPointsBox(bool);
    //! Display the parameters table
    void showParametersTable();
    //! Display the covariance matrix
    void showCovarianceMatrix();

    //! Applies the user changes to the numerical format of the output results
    void applyChanges();

    //! Deletes the result fit curves from the plot
    void deleteFitCurves();

private Q_SLOTS:
    //! Enable the "Apply" button
    void enableApplyChanges(int = 0);

Q_SIGNALS:
    void clearFunctionsList();
    void saveFunctionsList(const QStringList &);

private:
    Fit *d_fitter;
    Graph *d_graph {};
    QStringList d_user_functions, d_user_function_names, d_user_function_params;
    QStringList d_built_in_function_names, d_built_in_functions;
    QStringList d_plugin_function_names, d_plugin_functions, d_plugin_files_list, d_plugin_params;
    QList<MyWidget *> *d_src_table {};

    QCheckBox *boxUseBuiltIn {};
    QStackedWidget *tw;
    QPushButton *buttonOk {};
    QPushButton *buttonCancel1 {};
    QPushButton *buttonCancel2 {};
    QPushButton *buttonCancel3 {};
    QPushButton *buttonAdvanced {};
    QPushButton *buttonClear {};
    QPushButton *buttonClearUsrList {};
    QPushButton *buttonPlugins {};
    QPushButton *btnBack {};
    QComboBox *boxCurve {};
    QComboBox *boxAlgorithm {};
    QTableWidget *boxParams {};
    QLineEdit *boxFrom {};
    QLineEdit *boxTo {};
    QLineEdit *boxTolerance {};
    QSpinBox *boxPoints {}, *generatePointsBox {}, *boxPrecision {}, *polynomOrderBox {};
    QWidget *fitPage {}, *editPage {}, *advancedPage {};
    QTextEdit *editBox {}, *explainBox {}, *boxFunction {};
    QListWidget *categoryBox {}, *funcBox {};
    QLineEdit *boxName {}, *boxParam {};
    QLabel *lblFunction {}, *lblPoints {}, *polynomOrderLabel {};
    QPushButton *btnAddFunc {}, *btnDelFunc {}, *btnContinue {}, *btnApply {};
    QPushButton *buttonEdit {}, *btnAddTxt {}, *btnAddName {}, *btnDeleteFitCurves {};
    ColorButton *btnColor {};
    QComboBox *boxYErrorSource {}, *tableNamesBox {}, *colNamesBox {};
    QRadioButton *generatePointsBtn {}, *samePointsBtn {};
    QPushButton *btnParamTable {}, *btnCovMatrix {};
    QLineEdit *covMatrixName {}, *paramTableName {};
    QCheckBox *plotLabelBox {}, *logBox {}, *scaleErrorsBox {};
};

#endif // FITDIALOG_H
