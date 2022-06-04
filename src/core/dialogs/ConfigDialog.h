/***************************************************************************
    File                 : ConfigDialog.h
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Preferences dialog

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
#ifndef ConfigDialog_H
#define ConfigDialog_H

#include "core/MakhberDefs.h"

#include <QDialog>
#include <QCheckBox>

class QGroupBox;
class QPushButton;
class QTabWidget;
class QStackedWidget;
class QWidget;
class QComboBox;
class QSpinBox;
class QLabel;
class QRadioButton;
class QListWidget;
class ColorButton;

//! Preferences dialog
class MAKHBER_EXPORT ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor
    /**
     * \param parent parent widget (must be the application window!=
     * \param fl window flags
     */
    explicit ConfigDialog(QWidget *parent, Qt::WindowFlags fl = Qt::Widget);

public Q_SLOTS:
    void languageChange();
    void insertLanguagesList();

    void accept();
    void apply();

    void setCurrentPage(int index);
    void setColumnSeparator(const QString &sep);
    // table colors
    void pickBgColor();
    void pickTextColor();
    void pickHeaderColor();

    // table fonts
    void pickTextFont();
    void pickHeaderFont();

    // graph fonts
    void pickAxesFont();
    void pickNumbersFont();
    void pickLegendFont();
    void pickTitleFont();

    void enableScaleFonts();
    void showFrameWidth(bool ok);

    // application
    void pickApplicationFont();
    void pickPanelsTextColor();
    void pickPanelsColor();
    void pickWorkspaceColor();

    // 2D curves
    int curveStyle();

    void pickDataMaxColor();
    void pickDataMinColor();
    void pick3DBackgroundColor();
    void pickMeshColor();
    void pickGridColor();
    void pick3DAxesColor();
    void pick3DNumbersColor();
    void pick3DLabelsColor();
    void pick3DTitleFont();
    void pick3DNumbersFont();
    void pick3DAxesFont();

    // Fitting
    void showPointsBox(bool);

    void switchToLanguage(int param);

private Q_SLOTS:
    //! Update the decimal separator preview
    void updateDecSepPreview();

private:
    void initPlotsPage();
    void initAppPage();
    void initCurvesPage();
    void initPlots3DPage();
    void initTablesPage();
    void initConfirmationsPage();
    void initFittingPage();

    QFont textFont, headerFont, axesFont, numbersFont, legendFont, titleFont, appFont;
    QFont plot3DTitleFont, plot3DNumbersFont, plot3DAxesFont;
    QStringList plot3DColors;

    QCheckBox *boxScaleLayersOnPrint {}, *boxPrintCropmarks {}, *linearFit2PointsBox {};
    QTabWidget *plotsTabWidget {}, *appTabWidget {};
    QPushButton *btnBackground3D {}, *btnMesh {}, *btnAxes {}, *btnLabels {}, *btnNumbers {};
    QPushButton *btnFromColor {}, *btnToColor {}, *btnGrid {};
    QPushButton *btnTitleFnt {}, *btnLabelsFnt {}, *btnNumFnt {};
    ColorButton *buttonBackground {}, *buttonText {}, *buttonHeader {};
    QPushButton *buttonOk, *buttonCancel, *buttonApply;
    QPushButton *buttonTextFont {}, *buttonHeaderFont {};
    QStackedWidget *generalDialog;
    QWidget *appColors {}, *tables {}, *plotOptions {}, *plotTicks {}, *plotFonts {}, *confirm {},
            *plotPrint {};
    QWidget *application {}, *curves {}, *plots3D {}, *fitPage {}, *numericFormatPage {};
    QPushButton *buttonAxesFont {}, *buttonNumbersFont {}, *buttonLegendFont {},
            *buttonTitleFont {}, *fontsBtn {};
    QCheckBox *boxSearchUpdates {}, *boxOrthogonal {}, *logBox {}, *plotLabelBox {},
            *scaleErrorsBox {};
    QCheckBox *boxTitle {}, *boxFrame {}, *boxPlots3D {}, *boxPlots2D {}, *boxTables {},
            *boxNotes {}, *boxFolders {};
    QCheckBox *boxSave {}, *boxBackbones {}, *boxAllAxes {}, *boxShowLegend {}, *boxSmoothMesh {};
    QCheckBox *boxAutoscaling {}, *boxShowProjection {}, *boxMatrices {}, *boxScaleFonts {},
            *boxResize {}, *boxUseGroupSeparator {}, *boxUseForeignSeparator {},
            *boxConvertToTextColumn {};
    QComboBox *boxMajTicks {}, *boxMinTicks {}, *boxStyle {}, *boxCurveStyle {}, *boxSeparator {},
            *boxLanguage {}, *boxDecimalSeparator {};
    QLabel *lblDefaultNumericFormat {}, *lblForeignSeparator {}, *lblConvertToTextColumn {};
    QComboBox *boxDefaultNumericFormat {};
    QLabel *boxSeparatorPreview {};
    QLabel *lblTableRowHeight {};
    QSpinBox *boxTableRowHeight {};
    QSpinBox *boxMinutes {}, *boxLineWidth {}, *boxFrameWidth {}, *boxResolution {}, *boxMargin {},
            *boxPrecision {}, *boxAppPrecision {};
    QSpinBox *boxCurveLineWidth {}, *boxSymbolSize {}, *boxMinTicksLength {}, *boxMajTicksLength {},
            *generatePointsBox {};
    QSpinBox *boxUndoLimit {};
    ColorButton *btnWorkspace {}, *btnPanels {}, *btnPanelsText {};
    QListWidget *itemsList;
    QLabel *labelFrameWidth {}, *lblLanguage {}, *lblWorkspace {}, *lblPanels {}, *lblPageHeader;
    QLabel *lblPanelsText {}, *lblFonts {}, *lblStyle {}, *lblDecimalSeparator {},
            *lblAppPrecision {};
    QGroupBox *groupBoxConfirm {};
    QGroupBox *groupBoxTableFonts {}, *groupBoxTableCol {};
    QLabel *lblSeparator {}, *lblTableBackground {}, *lblTextColor {}, *lblHeaderColor {};
    QLabel *lblSymbSize {}, *lblAxesLineWidth {}, *lblCurveStyle {}, *lblResolution {},
            *lblPrecision {};
    QGroupBox *groupBox3DFonts {}, *groupBox3DCol {};
    QLabel *lblMargin {}, *lblMajTicks {}, *lblMajTicksLength {}, *lblLineWidth {}, *lblMinTicks {},
            *lblMinTicksLength {}, *lblPoints {}, *lblPeaksColor {};
    QLabel *lblUndoLimit {};
    QGroupBox *groupBoxFittingCurve {}, *groupBoxFitParameters {};
    QRadioButton *samePointsBtn {}, *generatePointsBtn {};
    QGroupBox *groupBoxMultiPeak {};
    ColorButton *boxPeaksColor {};
    QLabel *lblScriptingLanguage {};
    QComboBox *boxScriptingLanguage {};
    QCheckBox *boxAntialiasing {}, *boxAutoscale3DPlots {}, *boxTableComments {};
};

#endif // CONFIGDIALOG_H
