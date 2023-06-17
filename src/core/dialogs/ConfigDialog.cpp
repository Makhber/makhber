/***************************************************************************
    File                 : ConfigDialog.cpp
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
#include "ConfigDialog.h"

#include "core/ApplicationWindow.h"
#include "core/ColorButton.h"
#include "plot2D/MultiLayer.h"
#include "plot2D/Graph.h"
#include "matrix/Matrix.h"

#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStyleFactory>
#include <QRegularExpression>
#include <QMessageBox>
#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QFontMetrics>

ConfigDialog::ConfigDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    // get current values from app window
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    plot3DColors = app->plot3DColors;
    plot3DTitleFont = app->plot3DTitleFont;
    plot3DNumbersFont = app->plot3DNumbersFont;
    plot3DAxesFont = app->plot3DAxesFont;
    textFont = app->tableTextFont;
    headerFont = app->tableHeaderFont;
    appFont = app->appFont;
    axesFont = app->plotAxesFont;
    numbersFont = app->plotNumbersFont;
    legendFont = app->plotLegendFont;
    titleFont = app->plotTitleFont;

    QPalette pal;
    pal.setColor(QPalette::Window, app->palette().color(QPalette::Window));

    // create the GUI
    generalDialog = new QStackedWidget();
    itemsList = new QListWidget();
    itemsList->setSpacing(10);
    itemsList->setAlternatingRowColors(true);

    initAppPage();
    initTablesPage();
    initPlotsPage();
    initPlots3DPage();
    initFittingPage();

    generalDialog->addWidget(appTabWidget);
    generalDialog->addWidget(tables);
    generalDialog->addWidget(plotsTabWidget);
    generalDialog->addWidget(plots3D);
    generalDialog->addWidget(fitPage);

    auto *rightLayout = new QVBoxLayout();
    lblPageHeader = new QLabel();
    QFont fnt = this->font();
    fnt.setPointSize(fnt.pointSize() + 3);
    fnt.setBold(true);
    lblPageHeader->setFont(fnt);

    lblPageHeader->setPalette(pal);
    lblPageHeader->setAutoFillBackground(true);

    rightLayout->setSpacing(10);
    rightLayout->addWidget(lblPageHeader);
    rightLayout->addWidget(generalDialog);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(5);
    topLayout->setContentsMargins(5, 5, 5, 5);
    topLayout->addWidget(itemsList);
    topLayout->addLayout(rightLayout);

    auto *bottomButtons = new QHBoxLayout();
    bottomButtons->addStretch();
    buttonApply = new QPushButton();
    buttonApply->setAutoDefault(true);
    bottomButtons->addWidget(buttonApply);

    buttonOk = new QPushButton();
    buttonOk->setAutoDefault(true);
    buttonOk->setDefault(true);
    bottomButtons->addWidget(buttonOk);

    buttonCancel = new QPushButton();
    buttonCancel->setAutoDefault(true);
    bottomButtons->addWidget(buttonCancel);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomButtons);

    languageChange();

    // signals and slots connections
    connect(itemsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentPage(int)));
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(buttonBackground, SIGNAL(clicked()), this, SLOT(pickBgColor()));
    connect(buttonText, SIGNAL(clicked()), this, SLOT(pickTextColor()));
    connect(buttonHeader, SIGNAL(clicked()), this, SLOT(pickHeaderColor()));
    connect(buttonTextFont, SIGNAL(clicked()), this, SLOT(pickTextFont()));
    connect(buttonHeaderFont, SIGNAL(clicked()), this, SLOT(pickHeaderFont()));

    setCurrentPage(0);
}

void ConfigDialog::setCurrentPage(int index)
{
    generalDialog->setCurrentIndex(index);
    if (itemsList->currentItem())
        lblPageHeader->setText(itemsList->currentItem()->text());
}

void ConfigDialog::initTablesPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    tables = new QWidget();

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(5);

    lblSeparator = new QLabel();
    topLayout->addWidget(lblSeparator);
    boxSeparator = new QComboBox();
    boxSeparator->setEditable(true);
    topLayout->addWidget(boxSeparator);

    QString help = tr("The column separator can be customized. \nThe following special codes can "
                      "be used:\n\\t for a TAB character \n\\s for a SPACE");
    help += "\n" + tr("The separator must not contain the following characters: \n0-9eE.+-");

    boxSeparator->setWhatsThis(help);
    boxSeparator->setToolTip(help);
    lblSeparator->setWhatsThis(help);
    lblSeparator->setToolTip(help);

    groupBoxTableCol = new QGroupBox();
    auto *colorsLayout = new QGridLayout(groupBoxTableCol);

    lblTableBackground = new QLabel();
    colorsLayout->addWidget(lblTableBackground, 0, 0);
    buttonBackground = new ColorButton();
    buttonBackground->setColor(app->tableBkgdColor);
    colorsLayout->addWidget(buttonBackground, 0, 1);

    lblTextColor = new QLabel();
    colorsLayout->addWidget(lblTextColor, 1, 0);
    buttonText = new ColorButton();
    buttonText->setColor(app->tableTextColor);
    colorsLayout->addWidget(buttonText, 1, 1);

    lblHeaderColor = new QLabel();
    colorsLayout->addWidget(lblHeaderColor, 2, 0);
    buttonHeader = new ColorButton();
    buttonHeader->setColor(app->tableHeaderColor);
    colorsLayout->addWidget(buttonHeader, 2, 1);

    groupBoxTableFonts = new QGroupBox();
    auto *bottomLayout = new QHBoxLayout(groupBoxTableFonts);

    buttonTextFont = new QPushButton();
    bottomLayout->addWidget(buttonTextFont);
    buttonHeaderFont = new QPushButton();
    bottomLayout->addWidget(buttonHeaderFont);

    boxTableComments = new QCheckBox();
    boxTableComments->setChecked(app->d_show_table_comments);

    // Set table row height
    auto *tableRowHeightLayout = new QHBoxLayout();
    lblTableRowHeight = new QLabel();
    lblTableRowHeight->setText(tr("Default Row Height"));
    tableRowHeightLayout->addWidget(lblTableRowHeight);
    boxTableRowHeight = new QSpinBox();
    boxTableRowHeight->setRange(15, 100);
    tableRowHeightLayout->addWidget(boxTableRowHeight);

    auto &settings = ApplicationWindow::getSettings();
    settings.beginGroup("[Table]");
    boxTableRowHeight->setValue(settings.value("DefaultRowHeight", 20).toInt());
    settings.endGroup();

    auto *tablesPageLayout = new QVBoxLayout(tables);
    tablesPageLayout->addWidget(boxTableComments);
    tablesPageLayout->addLayout(tableRowHeightLayout, 1);
    tablesPageLayout->addLayout(topLayout, 1);
    tablesPageLayout->addWidget(groupBoxTableCol);
    tablesPageLayout->addWidget(groupBoxTableFonts);
    tablesPageLayout->addStretch();
}

void ConfigDialog::initPlotsPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());

    plotsTabWidget = new QTabWidget();
    plotOptions = new QWidget();

    auto *optionsTabLayout = new QVBoxLayout(plotOptions);
    optionsTabLayout->setSpacing(5);

    auto *groupBoxOptions = new QGroupBox();
    optionsTabLayout->addWidget(groupBoxOptions);

    auto *optionsLayout = new QGridLayout(groupBoxOptions);

    boxAutoscaling = new QCheckBox();
    boxAutoscaling->setChecked(app->autoscale2DPlots);
    optionsLayout->addWidget(boxAutoscaling, 0, 0);

    boxScaleFonts = new QCheckBox();
    boxScaleFonts->setChecked(app->autoScaleFonts);
    optionsLayout->addWidget(boxScaleFonts, 0, 1);

    boxTitle = new QCheckBox();
    boxTitle->setChecked(app->titleOn);
    optionsLayout->addWidget(boxTitle, 1, 0);

    boxAllAxes = new QCheckBox();
    boxAllAxes->setChecked(app->allAxesOn);
    optionsLayout->addWidget(boxAllAxes, 1, 1);

    boxAntialiasing = new QCheckBox();
    boxAntialiasing->setChecked(app->antialiasing2DPlots);
    optionsLayout->addWidget(boxAntialiasing, 2, 0);

    boxBackbones = new QCheckBox();
    boxBackbones->setChecked(app->drawBackbones);
    optionsLayout->addWidget(boxBackbones, 2, 1);

    boxFrame = new QCheckBox();
    boxFrame->setChecked(app->canvasFrameOn);
    optionsLayout->addWidget(boxFrame, 3, 0);

    labelFrameWidth = new QLabel();
    optionsLayout->addWidget(labelFrameWidth, 4, 0);
    boxFrameWidth = new QSpinBox();
    optionsLayout->addWidget(boxFrameWidth, 4, 1);
    boxFrameWidth->setRange(1, 100);
    boxFrameWidth->setValue(app->canvasFrameWidth);
    if (!app->canvasFrameOn) {
        labelFrameWidth->hide();
        boxFrameWidth->hide();
    }

    lblAxesLineWidth = new QLabel();
    optionsLayout->addWidget(lblAxesLineWidth, 5, 0);
    boxLineWidth = new QSpinBox();
    boxLineWidth->setRange(0, 100);
    boxLineWidth->setValue(app->axesLineWidth);
    optionsLayout->addWidget(boxLineWidth, 5, 1);

    lblMargin = new QLabel();
    optionsLayout->addWidget(lblMargin, 6, 0);
    boxMargin = new QSpinBox();
    boxMargin->setRange(0, 1000);
    boxMargin->setSingleStep(5);
    boxMargin->setValue(app->defaultPlotMargin);
    optionsLayout->addWidget(boxMargin, 6, 1);

    optionsLayout->setRowStretch(7, 1);

    boxResize = new QCheckBox();
    boxResize->setChecked(!app->autoResizeLayers);
    if (boxResize->isChecked())
        boxScaleFonts->setEnabled(false);

    optionsTabLayout->addWidget(boxResize);

    plotsTabWidget->addTab(plotOptions, QString());

    initCurvesPage();

    plotsTabWidget->addTab(curves, QString());

    plotTicks = new QWidget();
    auto *plotTicksLayout = new QVBoxLayout(plotTicks);

    auto *ticksGroupBox = new QGroupBox();
    auto *ticksLayout = new QGridLayout(ticksGroupBox);
    plotTicksLayout->addWidget(ticksGroupBox);

    lblMajTicks = new QLabel();
    ticksLayout->addWidget(lblMajTicks, 0, 0);
    boxMajTicks = new QComboBox();
    ticksLayout->addWidget(boxMajTicks, 0, 1);

    lblMajTicksLength = new QLabel();
    ticksLayout->addWidget(lblMajTicksLength, 0, 2);
    boxMajTicksLength = new QSpinBox();
    boxMajTicksLength->setRange(0, 100);
    boxMajTicksLength->setValue(app->majTicksLength);
    ticksLayout->addWidget(boxMajTicksLength, 0, 3);

    lblMinTicks = new QLabel();
    ticksLayout->addWidget(lblMinTicks, 1, 0);
    boxMinTicks = new QComboBox();
    ticksLayout->addWidget(boxMinTicks, 1, 1);

    lblMinTicksLength = new QLabel();
    ticksLayout->addWidget(lblMinTicksLength, 1, 2);
    boxMinTicksLength = new QSpinBox();
    boxMinTicksLength->setRange(0, 100);
    boxMinTicksLength->setValue(app->minTicksLength);
    ticksLayout->addWidget(boxMinTicksLength, 1, 3);

    ticksLayout->setRowStretch(4, 1);

    plotsTabWidget->addTab(plotTicks, QString());

    plotFonts = new QWidget();
    auto *plotFontsLayout = new QVBoxLayout(plotFonts);

    auto *groupBox2DFonts = new QGroupBox();
    plotFontsLayout->addWidget(groupBox2DFonts);
    auto *fontsLayout = new QVBoxLayout(groupBox2DFonts);
    buttonTitleFont = new QPushButton();
    fontsLayout->addWidget(buttonTitleFont);
    buttonLegendFont = new QPushButton();
    fontsLayout->addWidget(buttonLegendFont);
    buttonAxesFont = new QPushButton();
    fontsLayout->addWidget(buttonAxesFont);
    buttonNumbersFont = new QPushButton();
    fontsLayout->addWidget(buttonNumbersFont);
    fontsLayout->addStretch();

    plotsTabWidget->addTab(plotFonts, QString());

    plotPrint = new QWidget();
    auto *printLayout = new QVBoxLayout(plotPrint);

    boxScaleLayersOnPrint = new QCheckBox();
    boxScaleLayersOnPrint->setChecked(app->d_scale_plots_on_print);
    printLayout->addWidget(boxScaleLayersOnPrint);

    boxPrintCropmarks = new QCheckBox();
    boxPrintCropmarks->setChecked(app->d_print_cropmarks);
    printLayout->addWidget(boxPrintCropmarks);
    printLayout->addStretch();
    plotsTabWidget->addTab(plotPrint, QString());

    connect(boxResize, SIGNAL(clicked()), this, SLOT(enableScaleFonts()));
    connect(boxFrame, SIGNAL(toggled(bool)), this, SLOT(showFrameWidth(bool)));
    connect(buttonAxesFont, SIGNAL(clicked()), this, SLOT(pickAxesFont()));
    connect(buttonNumbersFont, SIGNAL(clicked()), this, SLOT(pickNumbersFont()));
    connect(buttonLegendFont, SIGNAL(clicked()), this, SLOT(pickLegendFont()));
    connect(buttonTitleFont, SIGNAL(clicked()), this, SLOT(pickTitleFont()));
}

void ConfigDialog::enableScaleFonts()
{
    if (boxResize->isChecked())
        boxScaleFonts->setEnabled(false);
    else
        boxScaleFonts->setEnabled(true);
}

void ConfigDialog::showFrameWidth(bool ok)
{
    if (!ok) {
        boxFrameWidth->hide();
        labelFrameWidth->hide();
    } else {
        boxFrameWidth->show();
        labelFrameWidth->show();
    }
}

void ConfigDialog::initPlots3DPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    plots3D = new QWidget();

    auto *topBox = new QGroupBox();
    auto *topLayout = new QGridLayout(topBox);
    topLayout->setSpacing(5);

    lblResolution = new QLabel();
    topLayout->addWidget(lblResolution, 0, 0);
    boxResolution = new QSpinBox();
    boxResolution->setRange(1, 100);
    boxResolution->setValue(app->plot3DResolution);
    topLayout->addWidget(boxResolution, 0, 1);

    boxShowLegend = new QCheckBox();
    boxShowLegend->setChecked(app->showPlot3DLegend);
    topLayout->addWidget(boxShowLegend, 1, 0);

    boxShowProjection = new QCheckBox();
    boxShowProjection->setChecked(app->showPlot3DProjection);
    topLayout->addWidget(boxShowProjection, 1, 1);

    boxSmoothMesh = new QCheckBox();
    boxSmoothMesh->setChecked(app->smooth3DMesh);
    topLayout->addWidget(boxSmoothMesh, 2, 0);

    boxOrthogonal = new QCheckBox();
    boxOrthogonal->setChecked(app->orthogonal3DPlots);
    topLayout->addWidget(boxOrthogonal, 2, 1);

    boxAutoscale3DPlots = new QCheckBox();
    boxAutoscale3DPlots->setChecked(app->autoscale3DPlots);
    topLayout->addWidget(boxAutoscale3DPlots, 3, 0);

    groupBox3DCol = new QGroupBox();
    auto *middleLayout = new QGridLayout(groupBox3DCol);

    btnFromColor = new QPushButton();
    middleLayout->addWidget(btnFromColor, 0, 0);
    btnLabels = new QPushButton();
    middleLayout->addWidget(btnLabels, 0, 1);
    btnMesh = new QPushButton();
    middleLayout->addWidget(btnMesh, 0, 2);
    btnGrid = new QPushButton();
    middleLayout->addWidget(btnGrid, 0, 3);
    btnToColor = new QPushButton();
    middleLayout->addWidget(btnToColor, 1, 0);
    btnNumbers = new QPushButton();
    middleLayout->addWidget(btnNumbers, 1, 1);
    btnAxes = new QPushButton();
    middleLayout->addWidget(btnAxes, 1, 2);
    btnBackground3D = new QPushButton();
    middleLayout->addWidget(btnBackground3D, 1, 3);

    groupBox3DFonts = new QGroupBox();
    auto *bottomLayout = new QHBoxLayout(groupBox3DFonts);

    btnTitleFnt = new QPushButton();
    bottomLayout->addWidget(btnTitleFnt);
    btnLabelsFnt = new QPushButton();
    bottomLayout->addWidget(btnLabelsFnt);
    btnNumFnt = new QPushButton();
    bottomLayout->addWidget(btnNumFnt);

    auto *plots3DPageLayout = new QVBoxLayout(plots3D);
    plots3DPageLayout->addWidget(topBox);
    plots3DPageLayout->addWidget(groupBox3DCol);
    plots3DPageLayout->addWidget(groupBox3DFonts);
    plots3DPageLayout->addStretch();

    connect(btnAxes, SIGNAL(clicked()), this, SLOT(pick3DAxesColor()));
    connect(btnLabels, SIGNAL(clicked()), this, SLOT(pick3DLabelsColor()));
    connect(btnNumbers, SIGNAL(clicked()), this, SLOT(pick3DNumbersColor()));
    connect(btnBackground3D, SIGNAL(clicked()), this, SLOT(pick3DBackgroundColor()));
    connect(btnFromColor, SIGNAL(clicked()), this, SLOT(pickDataMinColor()));
    connect(btnToColor, SIGNAL(clicked()), this, SLOT(pickDataMaxColor()));
    connect(btnGrid, SIGNAL(clicked()), this, SLOT(pickGridColor()));
    connect(btnMesh, SIGNAL(clicked()), this, SLOT(pickMeshColor()));

    connect(btnNumFnt, SIGNAL(clicked()), this, SLOT(pick3DNumbersFont()));
    connect(btnTitleFnt, SIGNAL(clicked()), this, SLOT(pick3DTitleFont()));
    connect(btnLabelsFnt, SIGNAL(clicked()), this, SLOT(pick3DAxesFont()));
}

void ConfigDialog::initAppPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());

    appTabWidget = new QTabWidget(generalDialog);

    application = new QWidget();
    auto *applicationLayout = new QVBoxLayout(application);
    auto *groupBoxApp = new QGroupBox();
    applicationLayout->addWidget(groupBoxApp);
    auto *topBoxLayout = new QGridLayout(groupBoxApp);

    lblLanguage = new QLabel();
    topBoxLayout->addWidget(lblLanguage, 0, 0);
    boxLanguage = new QComboBox();
    insertLanguagesList();
    topBoxLayout->addWidget(boxLanguage, 0, 1);

    lblStyle = new QLabel();
    topBoxLayout->addWidget(lblStyle, 1, 0);
    boxStyle = new QComboBox();
    topBoxLayout->addWidget(boxStyle, 1, 1);
    QStringList styles = QStyleFactory::keys();
    styles.sort();
    boxStyle->addItems(styles);
    boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle, Qt::MatchWildcard));

    lblFonts = new QLabel();
    topBoxLayout->addWidget(lblFonts, 2, 0);
    fontsBtn = new QPushButton();
    topBoxLayout->addWidget(fontsBtn, 2, 1);

    lblScriptingLanguage = new QLabel();
    topBoxLayout->addWidget(lblScriptingLanguage, 3, 0);
    boxScriptingLanguage = new QComboBox();
    QStringList llist = ScriptingLangManager::languages();
    boxScriptingLanguage->addItems(llist);
    boxScriptingLanguage->setCurrentIndex(llist.indexOf(app->defaultScriptingLang));
    topBoxLayout->addWidget(boxScriptingLanguage, 3, 1);

    boxSave = new QCheckBox();
    boxSave->setChecked(app->autoSave);
    topBoxLayout->addWidget(boxSave, 4, 0);

    boxMinutes = new QSpinBox();
    boxMinutes->setRange(1, 100);
    boxMinutes->setValue(app->autoSaveTime);
    boxMinutes->setEnabled(app->autoSave);
    topBoxLayout->addWidget(boxMinutes, 4, 1);

    lblUndoLimit = new QLabel();
    topBoxLayout->addWidget(lblUndoLimit, 5, 0);
    boxUndoLimit = new QSpinBox();
    boxUndoLimit->setRange(1, 10000);
    boxUndoLimit->setValue(app->undoLimit);
    topBoxLayout->addWidget(boxUndoLimit, 5, 1);

#ifdef SEARCH_FOR_UPDATES
    boxSearchUpdates = new QCheckBox();
    boxSearchUpdates->setChecked(app->autoSearchUpdates);
    topBoxLayout->addWidget(boxSearchUpdates, 6, 0, 1, 2);
#endif

    topBoxLayout->setRowStretch(7, 1);

    appTabWidget->addTab(application, QString());

    initConfirmationsPage();

    appTabWidget->addTab(confirm, QString());

    appColors = new QWidget();
    auto *appColorsLayout = new QVBoxLayout(appColors);
    auto *groupBoxAppCol = new QGroupBox();
    appColorsLayout->addWidget(groupBoxAppCol);
    auto *colorsBoxLayout = new QGridLayout(groupBoxAppCol);

    lblWorkspace = new QLabel();
    colorsBoxLayout->addWidget(lblWorkspace, 0, 0);
    btnWorkspace = new ColorButton();
    btnWorkspace->setColor(app->workspaceColor);
    colorsBoxLayout->addWidget(btnWorkspace, 0, 1);

    lblPanels = new QLabel();
    colorsBoxLayout->addWidget(lblPanels, 1, 0);
    btnPanels = new ColorButton();
    colorsBoxLayout->addWidget(btnPanels, 1, 1);
    btnPanels->setColor(app->panelsColor);

    lblPanelsText = new QLabel();
    colorsBoxLayout->addWidget(lblPanelsText, 2, 0);
    btnPanelsText = new ColorButton();
    colorsBoxLayout->addWidget(btnPanelsText, 2, 1);
    btnPanelsText->setColor(app->panelsTextColor);

    colorsBoxLayout->setRowStretch(3, 1);

    appTabWidget->addTab(appColors, QString());

    numericFormatPage = new QWidget();
    auto *numLayout = new QVBoxLayout(numericFormatPage);
    auto *numericFormatBox = new QGroupBox();
    numLayout->addWidget(numericFormatBox);
    auto *numericFormatLayout = new QGridLayout(numericFormatBox);

    lblAppPrecision = new QLabel();
    numericFormatLayout->addWidget(lblAppPrecision, 0, 0);
    boxAppPrecision = new QSpinBox();
    boxAppPrecision->setRange(0, 16);
    boxAppPrecision->setValue(app->d_decimal_digits);
    numericFormatLayout->addWidget(boxAppPrecision, 0, 1);

    lblDecimalSeparator = new QLabel();
    numericFormatLayout->addWidget(lblDecimalSeparator, 1, 0);
    boxDecimalSeparator = new QComboBox();

    numericFormatLayout->addWidget(boxDecimalSeparator, 1, 1);

    boxUseGroupSeparator = new QCheckBox();
    boxUseGroupSeparator->setChecked(!(QLocale().numberOptions() & QLocale::OmitGroupSeparator));

    numericFormatLayout->addWidget(boxUseGroupSeparator, 2, 0);

    boxSeparatorPreview = new QLabel();
    boxSeparatorPreview->setFrameStyle(static_cast<int>(QFrame::Panel)
                                       | static_cast<int>(QFrame::Sunken));
    numericFormatLayout->addWidget(boxSeparatorPreview, 2, 1);

    lblDefaultNumericFormat = new QLabel();
    boxDefaultNumericFormat = new QComboBox();

    numericFormatLayout->addWidget(lblDefaultNumericFormat, 3, 0);
    numericFormatLayout->addWidget(boxDefaultNumericFormat, 3, 1);

    numericFormatLayout->setRowStretch(4, 1);

    const auto &settings = ApplicationWindow::getSettings();
    lblForeignSeparator = new QLabel();
    numericFormatLayout->addWidget(lblForeignSeparator, 4, 0);
    boxUseForeignSeparator = new QCheckBox();
    boxUseForeignSeparator->setChecked(settings.value("/General/UseForeignSeparator").toBool());
    numericFormatLayout->addWidget(boxUseForeignSeparator, 4, 1);

    lblConvertToTextColumn = new QLabel();
    numericFormatLayout->addWidget(lblConvertToTextColumn, 5, 0);

    boxConvertToTextColumn = new QCheckBox();
    boxConvertToTextColumn->setChecked(
            settings.value("/General/SetColumnTypeToTextOnInvalidInput", true).toBool());
    numericFormatLayout->addWidget(boxConvertToTextColumn, 5, 1);

    appTabWidget->addTab(numericFormatPage, QString());

    connect(boxLanguage, SIGNAL(activated(int)), this, SLOT(switchToLanguage(int)));
    connect(fontsBtn, SIGNAL(clicked()), this, SLOT(pickApplicationFont()));
    connect(boxSave, SIGNAL(toggled(bool)), boxMinutes, SLOT(setEnabled(bool)));
    connect(btnWorkspace, SIGNAL(clicked()), this, SLOT(pickWorkspaceColor()));
    connect(btnPanels, SIGNAL(clicked()), this, SLOT(pickPanelsColor()));
    connect(btnPanelsText, SIGNAL(clicked()), this, SLOT(pickPanelsTextColor()));
    connect(boxUseGroupSeparator, SIGNAL(toggled(bool)), this, SLOT(updateDecSepPreview()));
    connect(boxDecimalSeparator, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateDecSepPreview()));
    connect(boxAppPrecision, SIGNAL(valueChanged(int)), this, SLOT(updateDecSepPreview()));
}

void ConfigDialog::initFittingPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    fitPage = new QWidget();

    groupBoxFittingCurve = new QGroupBox();
    auto *fittingCurveLayout = new QGridLayout(groupBoxFittingCurve);
    fittingCurveLayout->setSpacing(5);

    generatePointsBtn = new QRadioButton();
    generatePointsBtn->setChecked(app->generateUniformFitPoints);
    fittingCurveLayout->addWidget(generatePointsBtn, 0, 0);

    lblPoints = new QLabel();
    fittingCurveLayout->addWidget(lblPoints, 0, 1);
    generatePointsBox = new QSpinBox();
    generatePointsBox->setRange(0, 1000000);
    generatePointsBox->setSingleStep(10);
    generatePointsBox->setValue(app->fitPoints);
    fittingCurveLayout->addWidget(generatePointsBox, 0, 2);

    linearFit2PointsBox = new QCheckBox();
    linearFit2PointsBox->setChecked(app->d_2_linear_fit_points);
    fittingCurveLayout->addWidget(linearFit2PointsBox, 0, 3);

    showPointsBox(!app->generateUniformFitPoints);

    samePointsBtn = new QRadioButton();
    samePointsBtn->setChecked(!app->generateUniformFitPoints);
    fittingCurveLayout->addWidget(samePointsBtn, 1, 0);

    groupBoxMultiPeak = new QGroupBox();
    groupBoxMultiPeak->setCheckable(true);
    groupBoxMultiPeak->setChecked(app->generatePeakCurves);

    auto *multiPeakLayout = new QHBoxLayout(groupBoxMultiPeak);

    lblPeaksColor = new QLabel();
    multiPeakLayout->addWidget(lblPeaksColor);
    boxPeaksColor = new ColorButton();
    boxPeaksColor->setColor(app->peakCurvesColor);
    multiPeakLayout->addWidget(boxPeaksColor);

    groupBoxFitParameters = new QGroupBox();
    auto *fitParamsLayout = new QGridLayout(groupBoxFitParameters);

    lblPrecision = new QLabel();
    fitParamsLayout->addWidget(lblPrecision, 0, 0);
    boxPrecision = new QSpinBox();
    fitParamsLayout->addWidget(boxPrecision, 0, 1);
    boxPrecision->setValue(app->fit_output_precision);

    logBox = new QCheckBox();
    logBox->setChecked(app->writeFitResultsToLog);
    fitParamsLayout->addWidget(logBox, 1, 0);

    plotLabelBox = new QCheckBox();
    plotLabelBox->setChecked(app->pasteFitResultsToPlot);
    fitParamsLayout->addWidget(plotLabelBox, 2, 0);

    scaleErrorsBox = new QCheckBox();
    fitParamsLayout->addWidget(scaleErrorsBox, 3, 0);
    scaleErrorsBox->setChecked(app->fit_scale_errors);

    auto *fitPageLayout = new QVBoxLayout(fitPage);
    fitPageLayout->addWidget(groupBoxFittingCurve);
    fitPageLayout->addWidget(groupBoxMultiPeak);
    fitPageLayout->addWidget(groupBoxFitParameters);
    fitPageLayout->addStretch();

    connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
    connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
}

void ConfigDialog::initCurvesPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());

    curves = new QWidget();

    auto *curvesGroupBox = new QGroupBox();
    auto *curvesBoxLayout = new QGridLayout(curvesGroupBox);

    lblCurveStyle = new QLabel();
    curvesBoxLayout->addWidget(lblCurveStyle, 0, 0);
    boxCurveStyle = new QComboBox();
    curvesBoxLayout->addWidget(boxCurveStyle, 0, 1);

    lblLineWidth = new QLabel();
    curvesBoxLayout->addWidget(lblLineWidth, 1, 0);
    boxCurveLineWidth = new QSpinBox();
    boxCurveLineWidth->setRange(1, 100);
    boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
    curvesBoxLayout->addWidget(boxCurveLineWidth, 1, 1);

    lblSymbSize = new QLabel();
    curvesBoxLayout->addWidget(lblSymbSize, 2, 0);
    boxSymbolSize = new QSpinBox();
    boxSymbolSize->setRange(1, 100);
    boxSymbolSize->setValue(app->defaultSymbolSize / 2);
    curvesBoxLayout->addWidget(boxSymbolSize, 2, 1);

    curvesBoxLayout->setRowStretch(3, 1);

    auto *curvesPageLayout = new QHBoxLayout(curves);
    curvesPageLayout->addWidget(curvesGroupBox);
}

void ConfigDialog::initConfirmationsPage()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    confirm = new QWidget();

    groupBoxConfirm = new QGroupBox();
    auto *layout = new QVBoxLayout(groupBoxConfirm);

    boxFolders = new QCheckBox();
    boxFolders->setChecked(app->confirmCloseFolder);
    layout->addWidget(boxFolders);

    boxTables = new QCheckBox();
    boxTables->setChecked(app->confirmCloseTable);
    layout->addWidget(boxTables);

    boxMatrices = new QCheckBox();
    boxMatrices->setChecked(app->confirmCloseMatrix);
    layout->addWidget(boxMatrices);

    boxPlots2D = new QCheckBox();
    boxPlots2D->setChecked(app->confirmClosePlot2D);
    layout->addWidget(boxPlots2D);

    boxPlots3D = new QCheckBox();
    boxPlots3D->setChecked(app->confirmClosePlot3D);
    layout->addWidget(boxPlots3D);

    boxNotes = new QCheckBox();
    boxNotes->setChecked(app->confirmCloseNotes);
    layout->addWidget(boxNotes);

    layout->addStretch();

    auto *confirmPageLayout = new QHBoxLayout(confirm);
    confirmPageLayout->addWidget(groupBoxConfirm);
}

void ConfigDialog::languageChange()
{
    setWindowTitle(tr("Preferences"));
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());

    // pages list
    itemsList->clear();
    itemsList->addItem(tr("General"));
    itemsList->addItem(tr("Tables"));
    itemsList->addItem(tr("2D Plots"));
    itemsList->addItem(tr("3D Plots"));
    itemsList->addItem(tr("Fitting"));
    itemsList->setCurrentRow(0);
    itemsList->item(0)->setIcon(QIcon(QPixmap(":/general.xpm")));
    itemsList->item(1)->setIcon(QIcon(QPixmap(":/configTable.xpm")));
    itemsList->item(2)->setIcon(QIcon(QPixmap(":/config_curves.xpm")));
    itemsList->item(3)->setIcon(QIcon(QPixmap(":/3dplots.xpm")));
    itemsList->item(4)->setIcon(QIcon(QPixmap(":/fit.xpm")));
    itemsList->setIconSize(QSize(32, 32));
    // calculate a sensible width for the items list
    // (default QListWidget size is 256 which looks too big)
    QFontMetrics fm(itemsList->font());
    int width = 32, i = 0;
    for (i = 0; i < itemsList->count(); i++) {
        auto newWidth = fm.horizontalAdvance(itemsList->item(i)->text());
        if (newWidth > width)
            width = newWidth;
    }
    itemsList->setMaximumWidth(itemsList->iconSize().width() + width + 50);
    // resize the list to the maximum width
    itemsList->resize(itemsList->maximumWidth(), itemsList->height());

    // plots 2D page
    plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotOptions), tr("Options"));
    plotsTabWidget->setTabText(plotsTabWidget->indexOf(curves), tr("Curves"));
    plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotTicks), tr("Ticks"));
    plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotFonts), tr("Fonts"));

    boxResize->setText(tr("Do not &resize layers when window size changes"));

    lblMinTicksLength->setText(tr("Length"));

    lblAxesLineWidth->setText(tr("Axes linewidth"));
    lblMajTicksLength->setText(tr("Length"));
    lblMajTicks->setText(tr("Major Ticks"));
    lblMinTicks->setText(tr("Minor Ticks"));

    lblMargin->setText(tr("Margin"));
    labelFrameWidth->setText(tr("Frame width"));
    boxBackbones->setText(tr("Axes &backbones"));
    boxFrame->setText(tr("Canvas Fra&me"));
    boxAllAxes->setText(tr("Sho&w all axes"));
    boxTitle->setText(tr("Show &Title"));
    boxScaleFonts->setText(tr("Scale &Fonts"));
    boxAutoscaling->setText(tr("Auto&scaling"));
    boxAntialiasing->setText(tr("Antia&liasing"));

    boxMajTicks->clear();
    boxMajTicks->addItem(tr("None"));
    boxMajTicks->addItem(tr("Out"));
    boxMajTicks->addItem(tr("In & Out"));
    boxMajTicks->addItem(tr("In"));

    boxMinTicks->clear();
    boxMinTicks->addItem(tr("None"));
    boxMinTicks->addItem(tr("Out"));
    boxMinTicks->addItem(tr("In & Out"));
    boxMinTicks->addItem(tr("In"));

    boxMajTicks->setCurrentIndex(app->majTicksStyle);
    boxMinTicks->setCurrentIndex(app->minTicksStyle);

    plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotPrint), tr("Print"));
    boxPrintCropmarks->setText(tr("Print Crop &Marks"));
    boxScaleLayersOnPrint->setText(tr("&Scale layers to paper size"));

    // confirmations page
    groupBoxConfirm->setTitle(tr("Prompt on closing"));
    boxFolders->setText(tr("Folders"));
    boxTables->setText(tr("Tables"));
    boxPlots3D->setText(tr("3D Plots"));
    boxPlots2D->setText(tr("2D Plots"));
    boxMatrices->setText(tr("Matrices"));
    boxNotes->setText(tr("&Notes"));

    buttonOk->setText(tr("&OK"));
    buttonCancel->setText(tr("&Cancel"));
    buttonApply->setText(tr("&Apply"));
    buttonTextFont->setText(tr("&Text Font"));
    buttonHeaderFont->setText(tr("&Labels Font"));
    buttonAxesFont->setText(tr("A&xes Labels"));
    buttonNumbersFont->setText(tr("Axes &Numbers"));
    buttonLegendFont->setText(tr("&Legend"));
    buttonTitleFont->setText(tr("T&itle"));

    // application page
    appTabWidget->setTabText(appTabWidget->indexOf(application), tr("Application"));
    appTabWidget->setTabText(appTabWidget->indexOf(confirm), tr("Confirmations"));
    appTabWidget->setTabText(appTabWidget->indexOf(appColors), tr("Colors"));
    appTabWidget->setTabText(appTabWidget->indexOf(numericFormatPage), tr("Numeric Format"));

    lblLanguage->setText(tr("Language"));
    lblStyle->setText(tr("Style"));
    lblFonts->setText(tr("Main Font"));
    fontsBtn->setText(tr("Choose &font"));
    lblWorkspace->setText(tr("Workspace"));
    lblPanelsText->setText(tr("Panels text"));
    lblPanels->setText(tr("Panels"));
    lblUndoLimit->setText(tr("Undo/Redo History limit"));
    boxSave->setText(tr("Save every"));
#ifdef SEARCH_FOR_UPDATES
    boxSearchUpdates->setText(tr("Check for new versions at startup"));
#endif
    boxMinutes->setSuffix(tr(" minutes"));
    lblScriptingLanguage->setText(tr("Default scripting language"));

    lblDefaultNumericFormat->setText(tr("Default numeric format"));
    lblForeignSeparator->setText(
            tr("Consider ',' and '.' interchangeable on input in numerical columns"));
    lblConvertToTextColumn->setText(
            tr("Convert numerical columns to text columns when pasting non-numeric values"));
    boxDefaultNumericFormat->clear();
    boxDefaultNumericFormat->addItem(tr("Decimal"), QVariant('f'));
    boxDefaultNumericFormat->addItem(tr("Scientific (e)"), QVariant('e'));
    boxDefaultNumericFormat->addItem(tr("Scientific (E)"), QVariant('E'));
    boxDefaultNumericFormat->addItem(tr("Automatic (e)"), QVariant('g'));
    boxDefaultNumericFormat->addItem(tr("Automatic (E)"), QVariant('G'));
    int format_index = boxDefaultNumericFormat->findData(app->d_default_numeric_format);
    boxDefaultNumericFormat->setCurrentIndex(format_index);

    boxUseGroupSeparator->setText(
            tr("Use group separator", "option: use separator every 3 digits"));
    lblAppPrecision->setText(tr("Default Number of Decimal Digits"));
    lblDecimalSeparator->setText(tr("Decimal Separators"));
    boxDecimalSeparator->clear();
    boxDecimalSeparator->addItem(tr("default") + " (" + QLocale::system().toString(1000.0, 'f', 1)
                                 + ")");
    boxDecimalSeparator->addItem(QLocale::c().toString(1000.0, 'f', 1));
    boxDecimalSeparator->addItem(QLocale(QLocale::German).toString(1000.0, 'f', 1));
    boxDecimalSeparator->addItem(QLocale(QLocale::French).toString(1000.0, 'f', 1));

    if (QLocale().name() == QLocale::c().name())
        boxDecimalSeparator->setCurrentIndex(1);
    else if (QLocale().name() == QLocale(QLocale::German).name())
        boxDecimalSeparator->setCurrentIndex(2);
    else if (QLocale().name() == QLocale(QLocale::French).name())
        boxDecimalSeparator->setCurrentIndex(3);

    boxSeparatorPreview->setText(
            tr("Preview:", "preview of the decimal separator") + " "
            + QLocale().toString(1000.1234567890123456, 'f', boxAppPrecision->value()));

    // tables page
    boxTableComments->setText(tr("&Display Comments in Header"));
    groupBoxTableCol->setTitle(tr("Colors"));
    lblSeparator->setText(tr("Default Column Separator"));
    boxSeparator->clear();
    boxSeparator->addItem(tr("TAB"));
    boxSeparator->addItem(tr("SPACE"));
    boxSeparator->addItem(";" + tr("TAB"));
    boxSeparator->addItem("," + tr("TAB"));
    boxSeparator->addItem(";" + tr("SPACE"));
    boxSeparator->addItem("," + tr("SPACE"));
    boxSeparator->addItem(";");
    boxSeparator->addItem(",");
    setColumnSeparator(app->columnSeparator);

    lblTableBackground->setText(tr("Background"));
    lblTextColor->setText(tr("Text"));
    lblHeaderColor->setText(tr("Labels"));
    groupBoxTableFonts->setTitle(tr("Fonts"));

    // curves page
    lblCurveStyle->setText(tr("Default curve style"));
    lblLineWidth->setText(tr("Line width"));
    lblSymbSize->setText(tr("Symbol size"));

    boxCurveStyle->clear();
    boxCurveStyle->addItem(QPixmap(":/lPlot.xpm"), tr(" Line"));
    boxCurveStyle->addItem(QPixmap(":/pPlot.xpm"), tr(" Scatter"));
    boxCurveStyle->addItem(QPixmap(":/lpPlot.xpm"), tr(" Line + Symbol"));
    boxCurveStyle->addItem(QPixmap(":/dropLines.xpm"), tr(" Vertical drop lines"));
    boxCurveStyle->addItem(QPixmap(":/spline.xpm"), tr(" Spline"));
    boxCurveStyle->addItem(QPixmap(":/vert_steps.xpm"), tr(" Vertical steps"));
    boxCurveStyle->addItem(QPixmap(":/hor_steps.xpm"), tr(" Horizontal steps"));
    boxCurveStyle->addItem(QPixmap(":/area.xpm"), tr(" Area"));
    boxCurveStyle->addItem(QPixmap(":/vertBars.xpm"), tr(" Vertical Bars"));
    boxCurveStyle->addItem(QPixmap(":/hBars.xpm"), tr(" Horizontal Bars"));

    int style = app->defaultCurveStyle;
    if (style == Graph::Line)
        boxCurveStyle->setCurrentIndex(0);
    else if (style == Graph::Scatter)
        boxCurveStyle->setCurrentIndex(1);
    else if (style == Graph::LineSymbols)
        boxCurveStyle->setCurrentIndex(2);
    else if (style == Graph::VerticalDropLines)
        boxCurveStyle->setCurrentIndex(3);
    else if (style == Graph::Spline)
        boxCurveStyle->setCurrentIndex(4);
    else if (style == Graph::VerticalSteps)
        boxCurveStyle->setCurrentIndex(5);
    else if (style == Graph::HorizontalSteps)
        boxCurveStyle->setCurrentIndex(6);
    else if (style == Graph::Area)
        boxCurveStyle->setCurrentIndex(7);
    else if (style == Graph::VerticalBars)
        boxCurveStyle->setCurrentIndex(8);
    else if (style == Graph::HorizontalBars)
        boxCurveStyle->setCurrentIndex(9);

    // plots 3D
    lblResolution->setText(tr("Resolution"));
    boxResolution->setSpecialValueText("1 " + tr("(all data shown)"));
    boxShowLegend->setText(tr("&Show Legend"));
    boxShowProjection->setText(tr("Show &Projection"));
    btnFromColor->setText(tr("&Data Max"));
    boxSmoothMesh->setText(tr("Smoot&h Line"));
    boxOrthogonal->setText(tr("O&rthogonal"));
    btnLabels->setText(tr("Lab&els"));
    btnMesh->setText(tr("Mesh &Line"));
    btnGrid->setText(tr("&Grid"));
    btnToColor->setText(tr("Data &Min"));
    btnNumbers->setText(tr("&Numbers"));
    btnAxes->setText(tr("A&xes"));
    btnBackground3D->setText(tr("&Background"));
    groupBox3DCol->setTitle(tr("Colors"));
    groupBox3DFonts->setTitle(tr("Fonts"));
    btnTitleFnt->setText(tr("&Title"));
    btnLabelsFnt->setText(tr("&Axes Labels"));
    btnNumFnt->setText(tr("&Numbers"));
    boxAutoscale3DPlots->setText(tr("Autosca&ling"));

    // Fitting page
    groupBoxFittingCurve->setTitle(tr("Generated Fit Curve"));
    generatePointsBtn->setText(tr("Uniform X Function"));
    lblPoints->setText(tr("Points"));
    samePointsBtn->setText(tr("Same X as Fitting Data"));
    linearFit2PointsBox->setText(tr("2 points for linear fits"));

    groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));

    groupBoxFitParameters->setTitle(tr("Parameters Output"));
    lblPrecision->setText(tr("Significant Digits"));
    logBox->setText(tr("Write Parameters to Result Log"));
    plotLabelBox->setText(tr("Paste Parameters to Plot"));
    scaleErrorsBox->setText(tr("Scale Errors with sqrt(Chi^2/doF)"));
    groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));
    lblPeaksColor->setText(tr("Peaks Color"));
}

void ConfigDialog::accept()
{
    apply();
    close();
}

void ConfigDialog::apply()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    if (!app)
        return;

    // tables page
    QString sep = boxSeparator->currentText();
    sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);
    sep.replace("\\t", "\t");
    sep.replace(tr("SPACE"), " ");
    sep.replace("\\s", " ");

    if (sep.contains(QRegularExpression("[0-9.eE+-]")) != 0) {
        QMessageBox::warning(
                nullptr, tr("Import options error"),
                tr("The separator must not contain the following characters: 0-9eE.+-"));
        return;
    }

    app->columnSeparator = sep;
    app->customizeTables(buttonBackground->color(), buttonText->color(), buttonHeader->color(),
                         textFont, headerFont, boxTableComments->isChecked());
    // 2D plots page: options tab
    app->titleOn = boxTitle->isChecked();
    app->allAxesOn = boxAllAxes->isChecked();
    app->canvasFrameOn = boxFrame->isChecked();
    app->canvasFrameWidth = boxFrameWidth->value();
    app->drawBackbones = boxBackbones->isChecked();
    app->axesLineWidth = boxLineWidth->value();
    app->defaultPlotMargin = boxMargin->value();
    app->setGraphDefaultSettings(boxAutoscaling->isChecked(), boxScaleFonts->isChecked(),
                                 boxResize->isChecked(), boxAntialiasing->isChecked());
    // 2D plots page: curves tab
    app->defaultCurveStyle = curveStyle();
    app->defaultCurveLineWidth = boxCurveLineWidth->value();
    app->defaultSymbolSize = 2 * boxSymbolSize->value() + 1;
    // 2D plots page: ticks tab
    app->majTicksLength = boxMajTicksLength->value();
    app->minTicksLength = boxMinTicksLength->value();
    app->majTicksStyle = boxMajTicks->currentIndex();
    app->minTicksStyle = boxMinTicks->currentIndex();
    // 2D plots page: fonts tab
    app->plotAxesFont = axesFont;
    app->plotNumbersFont = numbersFont;
    app->plotLegendFont = legendFont;
    app->plotTitleFont = titleFont;
    // 2D plots page: print tab
    app->d_print_cropmarks = boxPrintCropmarks->isChecked();
    app->d_scale_plots_on_print = boxScaleLayersOnPrint->isChecked();
    QList<MyWidget *> windows = app->windowsList();
    for (MyWidget *w : windows) {
        if (w->inherits("MultiLayer")) {
            (dynamic_cast<MultiLayer *>(w))
                    ->setScaleLayersOnPrint(boxScaleLayersOnPrint->isChecked());
            (dynamic_cast<MultiLayer *>(w))->printCropmarks(boxPrintCropmarks->isChecked());
        }
    }
    // general page: application tab
    app->changeAppFont(appFont);
    setFont(appFont);
    app->changeAppStyle(boxStyle->currentText());
#ifdef SEARCH_FOR_UPDATES
    app->autoSearchUpdates = boxSearchUpdates->isChecked();
#endif
    app->setSaveSettings(boxSave->isChecked(), boxMinutes->value());
    app->defaultScriptingLang = boxScriptingLanguage->currentText();

    app->undoLimit = boxUndoLimit->value(); // FIXME: can apply only after restart

    // general page: numeric format tab
    app->d_decimal_digits = boxAppPrecision->value();
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex()) {
    case 0:
        locale = QLocale::system();
        break;
    case 1:
        locale = QLocale::c();
        break;
    case 2:
        locale = QLocale(QLocale::German);
        break;
    case 3:
        locale = QLocale(QLocale::French);
        break;
    }

    int currentBoxIndex = boxDefaultNumericFormat->currentIndex();
    if (currentBoxIndex > -1) {
        app->d_default_numeric_format =
                boxDefaultNumericFormat->itemData(currentBoxIndex).toChar().toLatin1();
    }

    if (boxUseGroupSeparator->isChecked())
        locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
    else
        locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

    if (QLocale() != locale) {
        QLocale::setDefault(locale);
    }

    // general page: confirmations tab
    app->confirmCloseFolder = boxFolders->isChecked();
    app->updateConfirmOptions(boxTables->isChecked(), boxMatrices->isChecked(),
                              boxPlots2D->isChecked(), boxPlots3D->isChecked(),
                              boxNotes->isChecked());
    // general page: colors tab
    app->setAppColors(btnWorkspace->color(), btnPanels->color(), btnPanelsText->color());
    // 3D plots page
    app->plot3DColors = plot3DColors;
    app->showPlot3DLegend = boxShowLegend->isChecked();
    app->showPlot3DProjection = boxShowProjection->isChecked();
    app->plot3DResolution = boxResolution->value();
    app->plot3DTitleFont = plot3DTitleFont;
    app->plot3DNumbersFont = plot3DNumbersFont;
    app->plot3DAxesFont = plot3DAxesFont;
    app->orthogonal3DPlots = boxOrthogonal->isChecked();
    app->smooth3DMesh = boxSmoothMesh->isChecked();
    app->autoscale3DPlots = boxAutoscale3DPlots->isChecked();
    app->setPlot3DOptions();

    // fitting page
    app->fit_output_precision = boxPrecision->value();
    app->pasteFitResultsToPlot = plotLabelBox->isChecked();
    app->writeFitResultsToLog = logBox->isChecked();
    app->fitPoints = generatePointsBox->value();
    app->generateUniformFitPoints = generatePointsBtn->isChecked();
    app->generatePeakCurves = groupBoxMultiPeak->isChecked();
    app->peakCurvesColor = boxPeaksColor->color();
    app->fit_scale_errors = scaleErrorsBox->isChecked();
    app->d_2_linear_fit_points = linearFit2PointsBox->isChecked();
    app->saveSettings();

    // calculate a sensible width for the items list
    // (default QListWidget size is 256 which looks too big)
    QFontMetrics fm(itemsList->font());
    int width = 32, i = 0;
    for (i = 0; i < itemsList->count(); i++) {
        auto newWidth = fm.horizontalAdvance(itemsList->item(i)->text());
        if (newWidth > width)
            width = newWidth;
    }
    itemsList->setMaximumWidth(itemsList->iconSize().width() + width + 50);
    // resize the list to the maximum width
    itemsList->resize(itemsList->maximumWidth(), itemsList->height());

    auto &settings = ApplicationWindow::getSettings();
    settings.beginGroup("[Table]");
    settings.setValue("DefaultRowHeight", boxTableRowHeight->value());
    settings.endGroup();
    settings.setValue("/General/UseForeignSeparator", boxUseForeignSeparator->isChecked());
    settings.setValue("/General/SetColumnTypeToTextOnInvalidInput",
                      boxConvertToTextColumn->isChecked());
}

int ConfigDialog::curveStyle()
{
    int style = 0;
    switch (boxCurveStyle->currentIndex()) {
    case 0:
        style = Graph::Line;
        break;
    case 1:
        style = Graph::Scatter;
        break;
    case 2:
        style = Graph::LineSymbols;
        break;
    case 3:
        style = Graph::VerticalDropLines;
        break;
    case 4:
        style = Graph::Spline;
        break;
    case 5:
        style = Graph::VerticalSteps;
        break;
    case 6:
        style = Graph::HorizontalSteps;
        break;
    case 7:
        style = Graph::Area;
        break;
    case 8:
        style = Graph::VerticalBars;
        break;
    case 9:
        style = Graph::HorizontalBars;
        break;
    }
    return style;
}

void ConfigDialog::pickBgColor()
{
    QColor c = QColorDialog::getColor(buttonBackground->color(), this);
    if (!c.isValid() || c == buttonBackground->color())
        return;

    buttonBackground->setColor(c);
}

void ConfigDialog::pickTextColor()
{
    QColor c = QColorDialog::getColor(buttonText->color(), this);
    if (!c.isValid() || c == buttonText->color())
        return;

    buttonText->setColor(c);
}

void ConfigDialog::pickHeaderColor()
{
    QColor c = QColorDialog::getColor(buttonHeader->color(), this);
    if (!c.isValid() || c == buttonHeader->color())
        return;

    buttonHeader->setColor(c);
}

void ConfigDialog::pickTextFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, textFont, this);
    if (ok) {
        textFont = font;
    } else {
        return;
    }
}

void ConfigDialog::pickHeaderFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, headerFont, this);
    if (ok) {
        headerFont = font;
    } else {
        return;
    }
}

void ConfigDialog::pickLegendFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, legendFont, this);
    if (ok) {
        legendFont = font;
    } else {
        return;
    }
}

void ConfigDialog::pickAxesFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, axesFont, this);
    if (ok) {
        axesFont = font;
    } else {
        return;
    }
}

void ConfigDialog::pickNumbersFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, numbersFont, this);
    if (ok) {
        numbersFont = font;
    } else {
        return;
    }
}

void ConfigDialog::pickTitleFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, titleFont, this);
    if (ok)
        titleFont = font;
    else
        return;
}

void ConfigDialog::pickApplicationFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, appFont, this);
    if (ok)
        appFont = font;
    else
        return;
    fontsBtn->setFont(appFont);
}

void ConfigDialog::pickPanelsTextColor()
{
    QColor c = QColorDialog::getColor(btnPanelsText->color(), this);
    if (!c.isValid() || c == btnPanelsText->color())
        return;

    btnPanelsText->setColor(c);
}

void ConfigDialog::pickPanelsColor()
{
    QColor c = QColorDialog::getColor(btnPanels->color(), this);
    if (!c.isValid() || c == btnPanels->color())
        return;

    btnPanels->setColor(c);
}

void ConfigDialog::pickWorkspaceColor()
{
    QColor c = QColorDialog::getColor(btnWorkspace->color(), this);
    if (!c.isValid() || c == btnWorkspace->color())
        return;

    btnWorkspace->setColor(c);
}

void ConfigDialog::pickDataMaxColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[0])), this);
    if (!c.isValid())
        return;

    plot3DColors[0] = c.name();
}

void ConfigDialog::pickDataMinColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[4])), this);
    if (!c.isValid())
        return;

    plot3DColors[4] = c.name();
}

void ConfigDialog::pick3DBackgroundColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[7])), this);
    if (!c.isValid())
        return;

    plot3DColors[7] = c.name();
}

void ConfigDialog::pickMeshColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[2])), this);
    if (!c.isValid())
        return;

    plot3DColors[2] = c.name();
}

void ConfigDialog::pickGridColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[3])), this);
    if (!c.isValid())
        return;

    plot3DColors[3] = c.name();
}

void ConfigDialog::pick3DAxesColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[6])), this);
    if (!c.isValid())
        return;

    plot3DColors[6] = c.name();
}

void ConfigDialog::pick3DNumbersColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[5])), this);
    if (!c.isValid())
        return;

    plot3DColors[5] = c.name();
}

void ConfigDialog::pick3DLabelsColor()
{
    QColor c = QColorDialog::getColor(QColor(COLORVALUE(plot3DColors[1])), this);
    if (!c.isValid())
        return;

    plot3DColors[1] = c.name();
}

void ConfigDialog::pick3DTitleFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, plot3DTitleFont, this);
    if (ok)
        plot3DTitleFont = font;
    else
        return;
}

void ConfigDialog::pick3DNumbersFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, plot3DNumbersFont, this);
    if (ok)
        plot3DNumbersFont = font;
    else
        return;
}

void ConfigDialog::pick3DAxesFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, plot3DAxesFont, this);
    if (ok)
        plot3DAxesFont = font;
    else
        return;
}

void ConfigDialog::setColumnSeparator(const QString &sep)
{
    if (sep == "\t")
        boxSeparator->setCurrentIndex(0);
    else if (sep == " ")
        boxSeparator->setCurrentIndex(1);
    else if (sep == ";\t")
        boxSeparator->setCurrentIndex(2);
    else if (sep == ",\t")
        boxSeparator->setCurrentIndex(3);
    else if (sep == "; ")
        boxSeparator->setCurrentIndex(4);
    else if (sep == ", ")
        boxSeparator->setCurrentIndex(5);
    else if (sep == ";")
        boxSeparator->setCurrentIndex(6);
    else if (sep == ",")
        boxSeparator->setCurrentIndex(7);
    else {
        QString separator = sep;
        boxSeparator->setEditText(separator.replace(" ", "\\s").replace("\t", "\\t"));
    }
}

void ConfigDialog::switchToLanguage(int param)
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    app->switchToLanguage(param);
    languageChange();
}

void ConfigDialog::insertLanguagesList()
{
    auto *app = dynamic_cast<ApplicationWindow *>(parentWidget());
    QStringList locales = app->locales;
    QStringList languages;
    int lang = 0;
    for (int i = 0; i < locales.size(); i++) {
        if (locales[i] == "en")
            languages.push_back("English");
        else {
            QLocale loc = QLocale(locales[i]);
            QTranslator translator;
            if (translator.load(loc, "makhber", "_", app->qmPath)) {

                QString language = loc.nativeLanguageName();
                if (locales[i].size() > 2)
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
                    language.append(" (" + loc.nativeTerritoryName() + ")");
#else
                    language.append(" (" + loc.nativeCountryName() + ")");
#endif
                languages.push_back(language);
            }
        }

        if (locales[i] == app->appLanguage)
            lang = i;
    }
    boxLanguage->addItems(languages);
    boxLanguage->setCurrentIndex(lang);
}

void ConfigDialog::showPointsBox(bool)
{
    if (generatePointsBtn->isChecked()) {
        lblPoints->show();
        generatePointsBox->show();
        linearFit2PointsBox->show();
    } else {
        lblPoints->hide();
        generatePointsBox->hide();
        linearFit2PointsBox->hide();
    }
}

void ConfigDialog::updateDecSepPreview()
{
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex()) {
    case 0:
        locale = QLocale::system();
        break;
    case 1:
        locale = QLocale::c();
        break;
    case 2:
        locale = QLocale(QLocale::German);
        break;
    case 3:
        locale = QLocale(QLocale::French);
        break;
    }

    if (boxUseGroupSeparator->isChecked())
        locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
    else
        locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

    boxSeparatorPreview->setText(
            tr("Preview:", "preview of the decimal separator") + " "
            + locale.toString(1000.1234567890123456, 'f', boxAppPrecision->value()));
}
