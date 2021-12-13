/***************************************************************************
    File                 : AxesDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : General plot options dialog

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
#include "AxesDialog.h"

#include "plot2D/dialogs/TextDialog.h"
#include "plot2D/Grid.h"
#include "plot2D/Plot.h"
#include "scripting/MyParser.h"
#include "core/ColorButton.h"
#include "core/ApplicationWindow.h"
#include "core/TextFormatButtons.h"

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>

#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QLayout>
#include <QMessageBox>
#include <QFontDialog>
#include <QDate>
#include <QList>
#include <QListWidget>
#include <QVector>
#include <QGroupBox>
#include <QRadioButton>

#include <array>
#include <cmath>

AxesDialog::AxesDialog()
{
    setWindowTitle(tr("General Plot Options"));

    initScalesPage();
    initGridPage();
    initAxesPage();
    initFramePage();

    auto bottomButtons = new QHBoxLayout();
    bottomButtons->addStretch();

    buttonApply = new QPushButton();
    buttonApply->setText(tr("&Apply"));
    bottomButtons->addWidget(buttonApply);

    buttonOk = new QPushButton();
    buttonOk->setText(tr("&OK"));
    buttonOk->setDefault(true);
    bottomButtons->addWidget(buttonOk);

    buttonCancel = new QPushButton();
    buttonCancel->setText(tr("&Cancel"));
    bottomButtons->addWidget(buttonCancel);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(&generalDialog);
    mainLayout->addLayout(bottomButtons);

    lastPage = scalesPage;

    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(updatePlot()));
    connect(&generalDialog, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
}

void AxesDialog::initScalesPage()
{
    scalesPage = new QWidget();

    auto *middleBox = new QGroupBox(QString());
    auto *middleLayout = new QGridLayout(middleBox);

    middleLayout->addWidget(new QLabel(tr("From")), 0, 0);
    boxStart = new QLineEdit();
    middleLayout->addWidget(boxStart, 0, 1);

    middleLayout->addWidget(new QLabel(tr("To")), 1, 0);
    boxEnd = new QLineEdit();
    middleLayout->addWidget(boxEnd, 1, 1);

    boxScaleTypeLabel = new QLabel(tr("Type"));
    boxScaleType = new QComboBox();
    boxScaleType->addItem(tr("linear"));
    boxScaleType->addItem(tr("logarithmic"));
    middleLayout->addWidget(boxScaleTypeLabel, 2, 0);
    middleLayout->addWidget(boxScaleType, 2, 1);

    btnInvert = new QCheckBox();
    btnInvert->setText(tr("Inverted"));
    btnInvert->setChecked(false);
    middleLayout->addWidget(btnInvert, 3, 1);

    middleLayout->setRowStretch(4, 1);

    auto *rightBox = new QGroupBox(QString());
    auto *rightLayout = new QGridLayout(rightBox);
    auto *stepWidget = new QWidget();
    auto *stepWidgetLayout = new QVBoxLayout(stepWidget);

    btnStep = new QRadioButton(rightBox);
    btnStep->setText(tr("Step"));
    btnStep->setChecked(true);
    rightLayout->addWidget(btnStep, 0, 0);

    boxStep = new QLineEdit();
    stepWidgetLayout->addWidget(boxStep);
    boxUnit = new QComboBox();
    boxUnit->hide();
    stepWidgetLayout->addWidget(boxUnit);

    rightLayout->addWidget(stepWidget, 0, 1);

    btnMajor = new QRadioButton(rightBox);
    btnMajor->setText(tr("Major Ticks"));
    rightLayout->addWidget(btnMajor, 1, 0);

    boxMajorValue = new QSpinBox();
    boxMajorValue->setDisabled(true);
    rightLayout->addWidget(boxMajorValue, 1, 1);

    minorBoxLabel = new QLabel(tr("Minor Ticks"));
    rightLayout->addWidget(minorBoxLabel, 2, 0);

    boxMinorValue = new QComboBox();
    boxMinorValue->setEditable(true);
    boxMinorValue->addItems(QStringList() << "0"
                                          << "1"
                                          << "4"
                                          << "9"
                                          << "14"
                                          << "19");
    rightLayout->addWidget(boxMinorValue, 2, 1);

    rightLayout->setRowStretch(3, 1);

    QPixmap image0(":/bottom_scl.xpm");
    QPixmap image1(":/left_scl.xpm");
    QPixmap image2(":/top_scl.xpm");
    QPixmap image3(":/right_scl.xpm");

    axesList = new QListWidget();
    axesList->addItem(new QListWidgetItem(image0, tr("Bottom")));
    axesList->addItem(new QListWidgetItem(image1, tr("Left")));
    axesList->addItem(new QListWidgetItem(image2, tr("Top")));
    axesList->addItem(new QListWidgetItem(image3, tr("Right")));
    axesList->setIconSize(image0.size());
    axesList->setCurrentRow(-1);

    // calculate a sensible width for the items list
    // (default QListWidget size is 256 which looks too big)
    QFontMetrics fm(axesList->font());
    int width = 32, i = 0;
    for (i = 0; i < axesList->count(); i++) {
        auto newWidth = fm.horizontalAdvance(axesList->item(i)->text());
        if (newWidth > width)
            width = newWidth;
    }

    axesList->setMaximumWidth(axesList->iconSize().width() + width + 50);
    // resize the list to the maximum width
    axesList->resize(axesList->maximumWidth(), axesList->height());

    auto *mainLayout = new QHBoxLayout(scalesPage);
    mainLayout->addWidget(axesList);
    mainLayout->addWidget(middleBox);
    mainLayout->addWidget(rightBox);

    generalDialog.addTab(scalesPage, tr("Scale"));

    connect(btnInvert, SIGNAL(clicked()), this, SLOT(updatePlot()));
    connect(axesList, SIGNAL(currentRowChanged(int)), this, SLOT(updateScale()));
    connect(boxScaleType, SIGNAL(activated(int)), this, SLOT(updateMinorTicksList(int)));

    connect(btnStep, SIGNAL(toggled(bool)), boxStep, SLOT(setEnabled(bool)));
    connect(btnStep, SIGNAL(toggled(bool)), boxUnit, SLOT(setEnabled(bool)));
    connect(btnMajor, SIGNAL(toggled(bool)), boxMajorValue, SLOT(setEnabled(bool)));
}

void AxesDialog::initGridPage()
{
    gridPage = new QWidget();

    auto *rightBox = new QGroupBox(QString());
    auto *rightLayout = new QGridLayout(rightBox);

    boxMajorGrid = new QCheckBox();
    boxMajorGrid->setText(tr("Major Grids"));
    boxMajorGrid->setChecked(true);
    rightLayout->addWidget(boxMajorGrid, 0, 1);

    boxMinorGrid = new QCheckBox();
    boxMinorGrid->setText(tr("Minor Grids"));
    boxMinorGrid->setChecked(false);
    rightLayout->addWidget(boxMinorGrid, 0, 2);

    rightLayout->addWidget(new QLabel(tr("Line Color")), 1, 0);

    boxColorMajor = new ColorButton();
    rightLayout->addWidget(boxColorMajor, 1, 1);

    boxColorMinor = new ColorButton();
    boxColorMinor->setDisabled(true);
    rightLayout->addWidget(boxColorMinor, 1, 2);

    rightLayout->addWidget(new QLabel(tr("Line Type")), 2, 0);

    boxTypeMajor = new QComboBox();
    boxTypeMajor->addItem("_____");
    boxTypeMajor->addItem("- - -");
    boxTypeMajor->addItem(".....");
    boxTypeMajor->addItem("_._._");
    boxTypeMajor->addItem("_.._..");
    rightLayout->addWidget(boxTypeMajor, 2, 1);

    boxTypeMinor = new QComboBox();
    boxTypeMinor->addItem("_____");
    boxTypeMinor->addItem("- - -");
    boxTypeMinor->addItem(".....");
    boxTypeMinor->addItem("_._._");
    boxTypeMinor->addItem("_.._..");
    boxTypeMinor->setDisabled(true);
    rightLayout->addWidget(boxTypeMinor, 2, 2);

    rightLayout->addWidget(new QLabel(tr("Thickness")), 3, 0);

    boxWidthMajor = new QSpinBox();
    boxWidthMajor->setRange(1, 20);
    boxWidthMajor->setValue(1);
    rightLayout->addWidget(boxWidthMajor, 3, 1);

    boxWidthMinor = new QSpinBox();
    boxWidthMinor->setRange(1, 20);
    boxWidthMinor->setValue(1);
    boxWidthMinor->setDisabled(true);
    rightLayout->addWidget(boxWidthMinor, 3, 2);

    rightLayout->addWidget(new QLabel(tr("Axes")), 4, 0);

    boxGridXAxis = new QComboBox();
    boxGridXAxis->addItem(tr("Bottom"));
    boxGridXAxis->addItem(tr("Top"));
    rightLayout->addWidget(boxGridXAxis, 4, 1);

    boxGridYAxis = new QComboBox();
    boxGridYAxis->addItem(tr("Left"));
    boxGridYAxis->addItem(tr("Right"));
    rightLayout->addWidget(boxGridYAxis, 4, 2);

    rightLayout->addWidget(new QLabel(tr("Additional lines")), 5, 0);

    boxXLine = new QCheckBox();
    boxXLine->setText(tr("X=0"));
    boxXLine->setDisabled(true);
    rightLayout->addWidget(boxXLine, 5, 1);

    boxYLine = new QCheckBox();
    boxYLine->setText(tr("Y=0"));
    rightLayout->addWidget(boxYLine, 5, 2);

    rightLayout->setRowStretch(6, 1);
    rightLayout->setColumnStretch(4, 1);

    QPixmap image2(":/image2.xpm");
    QPixmap image3(":/image3.xpm");

    axesGridList = new QListWidget();
    axesGridList->addItem(new QListWidgetItem(image3, tr("Horizontal")));
    axesGridList->addItem(new QListWidgetItem(image2, tr("Vertical")));
    axesGridList->setIconSize(image3.size());
    axesGridList->setCurrentRow(-1);

    // calculate a sensible width for the items list
    // (default QListWidget size is 256 which looks too big)
    QFontMetrics fm(axesGridList->font());
    int width = 32, i = 0;
    for (i = 0; i < axesGridList->count(); i++) {
        auto newWidth = fm.horizontalAdvance(axesGridList->item(i)->text());
        if (newWidth > width)
            width = newWidth;
    }
    axesGridList->setMaximumWidth(axesGridList->iconSize().width() + width + 50);
    // resize the list to the maximum width
    axesGridList->resize(axesGridList->maximumWidth(), axesGridList->height());

    auto *mainLayout2 = new QHBoxLayout(gridPage);
    mainLayout2->addWidget(axesGridList);
    mainLayout2->addWidget(rightBox);

    generalDialog.addTab(gridPage, tr("Grid"));

    // grid page slot connections
    connect(axesGridList, SIGNAL(currentRowChanged(int)), this, SLOT(showGridOptions(int)));

    connect(boxMajorGrid, SIGNAL(toggled(bool)), this, SLOT(majorGridEnabled(bool)));
    connect(boxMinorGrid, SIGNAL(toggled(bool)), this, SLOT(minorGridEnabled(bool)));
    connect(boxColorMajor, SIGNAL(changed(QColor)), this, SLOT(updateGrid()));
    connect(boxColorMinor, SIGNAL(changed(QColor)), this, SLOT(updateGrid()));
    connect(boxTypeMajor, SIGNAL(activated(int)), this, SLOT(updateGrid()));
    connect(boxTypeMinor, SIGNAL(activated(int)), this, SLOT(updateGrid()));
    connect(boxWidthMajor, SIGNAL(valueChanged(int)), this, SLOT(updateGrid()));
    connect(boxWidthMinor, SIGNAL(valueChanged(int)), this, SLOT(updateGrid()));
    connect(boxXLine, SIGNAL(clicked()), this, SLOT(updatePlot()));
    connect(boxYLine, SIGNAL(clicked()), this, SLOT(updatePlot()));
}

void AxesDialog::initAxesPage()
{
    // axes page
    QPixmap image4(":/image4.xpm");
    QPixmap image5(":/image5.xpm");
    QPixmap image6(":/image6.xpm");
    QPixmap image7(":/image7.xpm");

    axesPage = new QWidget();

    boxAxisType = new QComboBox();
    boxAxisType->addItem(tr("Numeric"), (int)Graph::AxisType::Numeric);
    boxAxisType->addItem(tr("Text from table"), (int)Graph::AxisType::Txt);
    boxAxisType->addItem(tr("Day of the week"), (int)Graph::AxisType::Day);
    boxAxisType->addItem(tr("Month"), (int)Graph::AxisType::Month);
    boxAxisType->addItem(tr("Time"), (int)Graph::AxisType::Time);
    boxAxisType->addItem(tr("Date"), (int)Graph::AxisType::Date);
    boxAxisType->addItem(tr("Date & Time"), (int)Graph::AxisType::DateTime);
    boxAxisType->addItem(tr("Column Headings"), (int)Graph::AxisType::ColHeader);

    axesTitlesList = new QListWidget();
    axesTitlesList->addItem(new QListWidgetItem(image4, tr("Bottom")));
    axesTitlesList->addItem(new QListWidgetItem(image5, tr("Left")));
    axesTitlesList->addItem(new QListWidgetItem(image6, tr("Top")));
    axesTitlesList->addItem(new QListWidgetItem(image7, tr("Right")));
    axesTitlesList->setIconSize(image6.size());
    axesTitlesList->setMaximumWidth((int)(image6.width() * 1.5));
    axesTitlesList->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    axesTitlesList->setCurrentRow(-1);

    // calculate a sensible width for the items list
    // (default QListWidget size is 256 which looks too big)
    QFontMetrics fm(axesTitlesList->font());
    int width = 32, i = 0;
    for (i = 0; i < axesTitlesList->count(); i++) {
        auto newWidth = fm.horizontalAdvance(axesTitlesList->item(i)->text());
        if (newWidth > width)
            width = newWidth;
    }
    axesTitlesList->setMaximumWidth(axesTitlesList->iconSize().width() + width + 50);
    // resize the list to the maximum width
    axesTitlesList->resize(axesTitlesList->maximumWidth(), axesTitlesList->height());

    auto *topLayout = new QHBoxLayout();

    boxShowAxis = new QCheckBox(tr("Show"));
    boxShowAxis->setChecked(true);
    topLayout->addWidget(boxShowAxis);

    labelBox = new QGroupBox(tr("Title"));
    topLayout->addWidget(labelBox);

    auto *labelBoxLayout = new QVBoxLayout(labelBox);
    labelBoxLayout->setSpacing(2);

    boxTitle = new QTextEdit();
    boxTitle->setAcceptRichText(false);
    QFontMetrics metrics(this->font());
    boxTitle->setMaximumHeight(3 * metrics.height());
    labelBoxLayout->addWidget(boxTitle);

    auto *hl = new QHBoxLayout();
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(2);
    buttonLabelFont = new QPushButton(tr("&Font"));
    hl->addWidget(buttonLabelFont);

    formatButtons = new TextFormatButtons(boxTitle);
    formatButtons->toggleCurveButton(false);
    hl->addWidget(formatButtons);
    hl->addStretch();

    boxTitle->setMaximumWidth(buttonLabelFont->width() + formatButtons->width());
    labelBoxLayout->addLayout(hl);

    auto *bottomLayout = new QHBoxLayout();

    auto *leftBox = new QGroupBox(QString());
    bottomLayout->addWidget(leftBox);
    auto *leftBoxLayout = new QGridLayout(leftBox);

    leftBoxLayout->addWidget(new QLabel(tr("Type")), 0, 0);

    leftBoxLayout->addWidget(boxAxisType, 0, 1);

    leftBoxLayout->addWidget(new QLabel(tr("Font")), 1, 0);

    btnAxesFont = new QPushButton();
    btnAxesFont->setText(tr("Axis &Font"));
    leftBoxLayout->addWidget(btnAxesFont, 1, 1);

    leftBoxLayout->addWidget(new QLabel(tr("Color")), 2, 0);
    boxAxisColor = new ColorButton();
    leftBoxLayout->addWidget(boxAxisColor, 2, 1);

    leftBoxLayout->addWidget(new QLabel(tr("Major Ticks")), 3, 0);

    boxMajorTicksType = new QComboBox();
    boxMajorTicksType->addItem(tr("None"));
    boxMajorTicksType->addItem(tr("Out"));
    boxMajorTicksType->addItem(tr("In & Out"));
    boxMajorTicksType->addItem(tr("In"));
    leftBoxLayout->addWidget(boxMajorTicksType, 3, 1);

    leftBoxLayout->addWidget(new QLabel(tr("Minor Ticks")), 4, 0);

    boxMinorTicksType = new QComboBox();
    boxMinorTicksType->addItem(tr("None"));
    boxMinorTicksType->addItem(tr("Out"));
    boxMinorTicksType->addItem(tr("In & Out"));
    boxMinorTicksType->addItem(tr("In"));
    leftBoxLayout->addWidget(boxMinorTicksType, 4, 1);

    leftBoxLayout->addWidget(new QLabel(tr("Stand-off")), 5, 0);
    boxBaseline = new QSpinBox();
    boxBaseline->setRange(0, 1000);
    leftBoxLayout->addWidget(boxBaseline);

    boxShowLabels = new QGroupBox(tr("Show Labels"));
    boxShowLabels->setCheckable(true);
    boxShowLabels->setChecked(true);

    bottomLayout->addWidget(boxShowLabels);
    auto *rightBoxLayout = new QGridLayout(boxShowLabels);

    label1 = new QLabel(tr("Column"));
    rightBoxLayout->addWidget(label1, 0, 0);

    boxColName = new QComboBox();
    rightBoxLayout->addWidget(boxColName, 0, 1);

    labelTable = new QLabel(tr("Table"));
    rightBoxLayout->addWidget(labelTable, 1, 0);

    boxTableName = new QComboBox();
    rightBoxLayout->addWidget(boxTableName, 1, 1);

    label2 = new QLabel(tr("Format"));
    rightBoxLayout->addWidget(label2, 2, 0);

    boxFormat = new QComboBox();
    boxFormat->setDuplicatesEnabled(false);
    rightBoxLayout->addWidget(boxFormat, 2, 1);

    label3 = new QLabel(tr("Precision"));
    rightBoxLayout->addWidget(label3, 3, 0);
    boxPrecision = new QSpinBox();
    boxPrecision->setRange(0, 10);
    rightBoxLayout->addWidget(boxPrecision, 3, 1);

    rightBoxLayout->addWidget(new QLabel(tr("Angle")), 4, 0);

    boxAngle = new QSpinBox();
    boxAngle->setRange(-90, 90);
    boxAngle->setSingleStep(5);
    rightBoxLayout->addWidget(boxAngle, 4, 1);

    rightBoxLayout->addWidget(new QLabel(tr("Color")), 5, 0);
    boxAxisNumColor = new ColorButton();
    rightBoxLayout->addWidget(boxAxisNumColor, 5, 1);

    boxShowFormula = new QCheckBox(tr("For&mula"));
    rightBoxLayout->addWidget(boxShowFormula, 6, 0);

    boxFormula = new QTextEdit();
    boxFormula->setAcceptRichText(false);
    boxFormula->setMaximumHeight(3 * metrics.height());
    boxFormula->hide();
    rightBoxLayout->addWidget(boxFormula, 6, 1);
    rightBoxLayout->setRowStretch(7, 1);

    auto *rightLayout = new QVBoxLayout();
    rightLayout->addLayout(topLayout);
    rightLayout->addLayout(bottomLayout);
    rightLayout->addStretch(1);

    auto *mainLayout3 = new QHBoxLayout(axesPage);
    mainLayout3->addWidget(axesTitlesList);
    mainLayout3->addLayout(rightLayout);

    generalDialog.addTab(axesPage, tr("Axis"));

    connect(buttonLabelFont, SIGNAL(clicked()), this, SLOT(customAxisLabelFont()));

    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(updateShowBox(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(updateAxisColor(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(updateTitleBox(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(setTicksType(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(setAxisType(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(setBaselineDist(int)));
    connect(axesTitlesList, SIGNAL(currentRowChanged(int)), this, SLOT(updateLabelsFormat(int)));

    connect(boxShowLabels, SIGNAL(clicked(bool)), this, SLOT(updateTickLabelsList(bool)));

    connect(boxAxisColor, SIGNAL(clicked()), this, SLOT(pickAxisColor()));
    connect(boxAxisNumColor, SIGNAL(clicked()), this, SLOT(pickAxisNumColor()));
    connect(boxShowFormula, SIGNAL(clicked()), this, SLOT(showFormulaBox()));

    connect(boxMajorTicksType, SIGNAL(activated(int)), this, SLOT(updateMajTicksType(int)));
    connect(boxMinorTicksType, SIGNAL(activated(int)), this, SLOT(updateMinTicksType(int)));

    connect(boxShowAxis, SIGNAL(clicked()), this, SLOT(showAxis()));
    connect(boxFormat, SIGNAL(activated(int)), this, SLOT(setLabelsNumericFormat(int)));

    connect(btnAxesFont, SIGNAL(clicked()), this, SLOT(customAxisFont()));
    connect(boxBaseline, SIGNAL(valueChanged(int)), this, SLOT(changeBaselineDist(int)));
    connect(boxAxisType, SIGNAL(activated(int)), this, SLOT(showAxisFormatOptions(int)));
    connect(boxPrecision, SIGNAL(valueChanged(int)), this, SLOT(setLabelsNumericFormat(int)));
}

void AxesDialog::initFramePage()
{
    frame = new QWidget();

    boxFramed = new QGroupBox(tr("Canvas frame"));
    boxFramed->setCheckable(true);

    auto *boxFramedLayout = new QGridLayout(boxFramed);
    boxFramedLayout->addWidget(new QLabel(tr("Color")), 0, 0);
    boxFrameColor = new ColorButton(boxFramed);
    boxFramedLayout->addWidget(boxFrameColor, 0, 1);

    boxFramedLayout->addWidget(new QLabel(tr("Width")), 1, 0);
    boxFrameWidth = new QSpinBox();
    boxFrameWidth->setMinimum(1);
    boxFramedLayout->addWidget(boxFrameWidth, 1, 1);

    boxFramedLayout->setRowStretch(2, 1);

    auto *boxAxes = new QGroupBox(tr("Axes"));
    auto *boxAxesLayout = new QGridLayout(boxAxes);
    boxBackbones = new QCheckBox();
    boxBackbones->setText(tr("Draw backbones"));
    boxAxesLayout->addWidget(boxBackbones, 0, 0);

    boxAxesLayout->addWidget(new QLabel(tr("Line Width")), 1, 0);
    boxAxesLinewidth = new QSpinBox();
    boxAxesLinewidth->setRange(1, 100);
    boxAxesLayout->addWidget(boxAxesLinewidth, 1, 1);

    boxAxesLayout->addWidget(new QLabel(tr("Major ticks length")), 2, 0);
    boxMajorTicksLength = new QSpinBox();
    boxMajorTicksLength->setRange(0, 1000);
    boxAxesLayout->addWidget(boxMajorTicksLength, 2, 1);

    boxAxesLayout->addWidget(new QLabel(tr("Minor ticks length")), 3, 0);
    boxMinorTicksLength = new QSpinBox();
    boxMinorTicksLength->setRange(0, 1000);
    boxAxesLayout->addWidget(boxMinorTicksLength, 3, 1);

    boxAxesLayout->setRowStretch(4, 1);

    auto *mainLayout = new QHBoxLayout(frame);
    mainLayout->addWidget(boxFramed);
    mainLayout->addWidget(boxAxes);

    generalDialog.addTab(frame, tr("General"));

    connect(boxFrameColor, SIGNAL(clicked()), this, SLOT(pickCanvasFrameColor()));
    connect(boxBackbones, SIGNAL(toggled(bool)), this, SLOT(drawAxesBackbones(bool)));
    connect(boxFramed, SIGNAL(toggled(bool)), this, SLOT(drawFrame(bool)));
    connect(boxFrameWidth, SIGNAL(valueChanged(int)), this, SLOT(updateFrame(int)));
    connect(boxAxesLinewidth, SIGNAL(valueChanged(int)), this, SLOT(changeAxesLinewidth(int)));
    connect(boxMajorTicksLength, SIGNAL(valueChanged(int)), this,
            SLOT(changeMajorTicksLength(int)));
    connect(boxMinorTicksLength, SIGNAL(valueChanged(int)), this,
            SLOT(changeMinorTicksLength(int)));
}

void AxesDialog::changeMinorTicksLength(int minLength)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->changeTicksLength(minLength, boxMajorTicksLength->value());
    boxMajorTicksLength->setMinimum(minLength);
}

void AxesDialog::changeMajorTicksLength(int majLength)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->changeTicksLength(boxMinorTicksLength->value(), majLength);
    boxMinorTicksLength->setMaximum(majLength);
}

void AxesDialog::drawAxesBackbones(bool draw)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->drawAxesBackbones(draw);
}

void AxesDialog::changeAxesLinewidth(int width)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->setAxesLinewidth(width);
}

void AxesDialog::drawFrame(bool framed)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->drawCanvasFrame(framed, boxFrameWidth->value(), boxFrameColor->color());
}

void AxesDialog::updateFrame(int width)
{
    if (generalDialog.currentWidget() != frame)
        return;

    d_graph->drawCanvasFrame(boxFramed->isChecked(), width, boxFrameColor->color());
}

void AxesDialog::pickCanvasFrameColor()
{
    QColor c = QColorDialog::getColor(boxFrameColor->color(), this);
    if (!c.isValid() || c == boxFrameColor->color())
        return;

    boxFrameColor->setColor(c);
    d_graph->drawCanvasFrame(boxFramed->isChecked(), boxFrameWidth->value(), c);
}

void AxesDialog::showAxisFormatOptions(int format)
{
    int axis = mapToQwtAxisId();

    boxFormat->clear();
    boxFormat->setEditable(false);
    boxFormat->hide();
    boxPrecision->hide();
    boxColName->hide();
    label1->hide();
    label2->hide();
    label3->hide();
    boxShowFormula->hide();
    boxFormula->hide();
    boxTableName->hide();
    labelTable->hide();

    switch (static_cast<Graph::AxisType>(boxAxisType->itemData(format).toInt())) {
    case Graph::AxisType::Numeric:
        label2->show();
        boxFormat->show();
        boxFormat->addItem(tr("Automatic"));
        boxFormat->addItem(tr("Decimal: 100.0"));
        boxFormat->addItem(tr("Scientific: 1e2"));
        boxFormat->addItem(tr("Scientific: 10^2"));
        boxFormat->setCurrentIndex(d_graph->plotWidget()->axisLabelFormat(axis));

        label3->show();
        boxPrecision->show();
        boxShowFormula->show();

        showAxisFormula(mapToQwtAxisId());
        break;

    case Graph::AxisType::Txt:
        label1->show();
        boxColName->show();
        break;

    case Graph::AxisType::Day: {
        int day = (QDate::currentDate()).dayOfWeek();
        label2->show();
        boxFormat->show();
        boxFormat->addItem(QLocale().dayName(day, QLocale::ShortFormat));
        boxFormat->addItem(QLocale().dayName(day, QLocale::LongFormat));
        boxFormat->addItem((QLocale().dayName(day, QLocale::ShortFormat)).left(1));
        boxFormat->setCurrentIndex(formatInfo[axis].toInt());
    } break;

    case Graph::AxisType::Month: {
        int month = (QDate::currentDate()).month();
        label2->show();
        boxFormat->show();
        boxFormat->addItem(QLocale().monthName(month, QLocale::ShortFormat));
        boxFormat->addItem(QLocale().monthName(month, QLocale::LongFormat));
        boxFormat->addItem((QLocale().monthName(month, QLocale::ShortFormat)).left(1));
        boxFormat->setCurrentIndex(formatInfo[axis].toInt());
    } break;

    case Graph::AxisType::Time: {
        label2->show();
        boxFormat->show();
        boxFormat->setEditable(true);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = formatInfo[axis].split(";", Qt::KeepEmptyParts);
#else
        QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
#endif
        if (lst.count() == 2) {
            boxFormat->addItem(lst[1]);
            boxFormat->setItemText(boxFormat->currentIndex(), lst[1]);
        }

        boxFormat->addItem("h");
        boxFormat->addItem("h ap");
        boxFormat->addItem("h AP");
        boxFormat->addItem("h:mm");
        boxFormat->addItem("h:mm ap");
        boxFormat->addItem("hh:mm");
        boxFormat->addItem("h:mm:ss");
        boxFormat->addItem("h:mm:ss.zzz");
        boxFormat->addItem("mm:ss");
        boxFormat->addItem("mm:ss.zzz");
        boxFormat->addItem("hmm");
        boxFormat->addItem("hmmss");
        boxFormat->addItem("hhmmss");
    } break;

    case Graph::AxisType::Date: {
        label2->show();
        boxFormat->show();
        boxFormat->setEditable(true);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = formatInfo[axis].split(";", Qt::KeepEmptyParts);
#else
        QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
#endif
        if (lst.count() == 2) {
            boxFormat->addItem(lst[1]);
            boxFormat->setItemText(boxFormat->currentIndex(), lst[1]);
        }
        boxFormat->addItem("yyyy-MM-dd");
        boxFormat->addItem("dd.MM.yyyy");
        boxFormat->addItem("ddd MMMM d yy");
        boxFormat->addItem("dd/MM/yyyy");
    } break;

    case Graph::AxisType::DateTime: {
        label2->show();
        boxFormat->show();
        boxFormat->setEditable(true);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = formatInfo[axis].split(";", Qt::KeepEmptyParts);
#else
        QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
#endif
        if (lst.count() == 2) {
            boxFormat->addItem(lst[1]);
            boxFormat->setItemText(boxFormat->currentIndex(), lst[1]);
        }

        std::array<const char *, 9> date_strings = { "yyyy-MM-dd", "yyyy/MM/dd", "dd/MM/yyyy",
                                                     "dd/MM/yy",   "dd.MM.yyyy", "dd.MM.yy",
                                                     "MM/yyyy",    "dd.MM.",     "yyyyMMdd" };

        std::array<const char *, 9> time_strings = { "hh",           "hh ap",     "hh:mm",
                                                     "hh:mm ap",     "hh:mm:ss",  "hh:mm:ss.zzz",
                                                     "hh:mm:ss:zzz", "mm:ss.zzz", "hhmmss" };
        for (auto date_string : date_strings)
            for (auto time_string : time_strings)
                boxFormat->addItem(QString("%1 %2").arg(date_string, time_string),
                                   QVariant(QString(date_string) + " " + QString(time_string)));
    } break;

    case Graph::AxisType::ColHeader: {
        labelTable->show();
        if (tablesList.contains(formatInfo[axis]))
            boxTableName->setItemText(boxTableName->currentIndex(), formatInfo[axis]);
        boxTableName->show();
    } break;
    }
}

void AxesDialog::insertColList(const QStringList &cols)
{
    boxColName->addItems(cols);
}

void AxesDialog::showAxis()
{
    bool ok = boxShowAxis->isChecked();
    boxTitle->setEnabled(ok);
    boxAxisColor->setEnabled(ok);
    boxAxisNumColor->setEnabled(ok);
    btnAxesFont->setEnabled(ok);
    boxShowLabels->setEnabled(ok);
    boxMajorTicksType->setEnabled(ok);
    boxMinorTicksType->setEnabled(ok);
    boxAxisType->setEnabled(ok);
    boxBaseline->setEnabled(ok);
    labelBox->setEnabled(ok);

    int axis = -1;
    int a = axesTitlesList->currentRow();
    switch (a) {
    case 0: {
        axis = QwtPlot::xBottom;
        xAxisOn = ok;
        break;
    }
    case 1: {
        axis = QwtPlot::yLeft;
        yAxisOn = ok;
        break;
    }
    case 2: {
        axis = QwtPlot::xTop;
        topAxisOn = ok;
        break;
    }
    case 3: {
        axis = QwtPlot::yRight;
        rightAxisOn = ok;
        break;
    }
    }

    bool labels = false;
    if (tickLabelsOn[axis] == "1")
        labels = true;

    boxFormat->setEnabled(labels && ok);
    boxColName->setEnabled(labels && ok);
    boxShowFormula->setEnabled(labels && ok);
    boxFormula->setEnabled(labels && ok);

    if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
        boxAngle->setEnabled(labels && ok);
    else
        boxAngle->setDisabled(true);

    bool userFormat = true;
    if (boxFormat->currentIndex() == 0)
        userFormat = false;
    boxPrecision->setEnabled(labels && ok && userFormat);

    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, currentSelectedAxisType(), boxColName->currentText(), ok,
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
             boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula,
             boxAxisNumColor->color());
}

void AxesDialog::updateShowBox(int axis)
{
    switch (axis) {
    case 0: {
        boxShowAxis->setChecked(xAxisOn);
        int labelsOn = tickLabelsOn[2].toInt();
        boxShowLabels->setChecked(labelsOn);
        boxAngle->setEnabled(labelsOn && xAxisOn);
        boxFormat->setEnabled(labelsOn && xAxisOn);
        boxAngle->setValue(xBottomLabelsRotation);
        break;
    }
    case 1: {
        boxShowAxis->setChecked(yAxisOn);
        int labelsOn = tickLabelsOn[0].toInt();
        boxShowLabels->setChecked(labelsOn);
        boxFormat->setEnabled(labelsOn && yAxisOn);
        boxAngle->setEnabled(false);
        boxAngle->setValue(0);
        break;
    }
    case 2: {
        boxShowAxis->setChecked(topAxisOn);

        int labelsOn = tickLabelsOn[3].toInt();
        boxShowLabels->setChecked(labelsOn);
        boxFormat->setEnabled(labelsOn && topAxisOn);
        boxAngle->setEnabled(labelsOn && topAxisOn);
        boxAngle->setValue(xTopLabelsRotation);
        break;
    }
    case 3: {
        boxShowAxis->setChecked(rightAxisOn);
        int labelsOn = tickLabelsOn[1].toInt();
        boxShowLabels->setChecked(labelsOn);
        boxFormat->setEnabled(labelsOn && rightAxisOn);
        boxAngle->setEnabled(false);
        boxAngle->setValue(0);
        break;
    }
    }

    bool ok = boxShowAxis->isChecked();
    boxTitle->setEnabled(ok);
    boxAxisColor->setEnabled(ok);
    boxAxisNumColor->setEnabled(ok);
    btnAxesFont->setEnabled(ok);
    boxShowLabels->setEnabled(ok);
    boxMajorTicksType->setEnabled(ok);
    boxMinorTicksType->setEnabled(ok);
    boxAxisType->setEnabled(ok);
    boxBaseline->setEnabled(ok);
    labelBox->setEnabled(ok);
}

void AxesDialog::customAxisFont()
{
    bool okF = false;
    int axis = -1;
    QFont fnt;
    switch (axesTitlesList->currentRow()) {
    case 0: {
        axis = QwtPlot::xBottom;
        fnt = QFontDialog::getFont(&okF, xBottomFont, this);
        if (okF)
            xBottomFont = fnt;
        break;
    }
    case 1: {
        axis = QwtPlot::yLeft;
        fnt = QFontDialog::getFont(&okF, yLeftFont, this);
        if (okF)
            yLeftFont = fnt;
        break;
    }
    case 2: {
        axis = QwtPlot::xTop;
        fnt = QFontDialog::getFont(&okF, xTopFont, this);
        if (okF)
            xTopFont = fnt;
        break;
    }
    case 3: {
        axis = QwtPlot::yRight;
        fnt = QFontDialog::getFont(&okF, yRightFont, this);
        if (okF)
            yRightFont = fnt;
        break;
    }
    }

    d_graph->setAxisFont(axis, fnt);
}

void AxesDialog::accept()
{
    if (updatePlot())
        close();
}

void AxesDialog::majorGridEnabled(bool on)
{
    boxTypeMajor->setEnabled(on);
    boxColorMajor->setEnabled(on);
    boxWidthMajor->setEnabled(on);

    updateGrid();
}

void AxesDialog::minorGridEnabled(bool on)
{
    boxTypeMinor->setEnabled(on);
    boxColorMinor->setEnabled(on);
    boxWidthMinor->setEnabled(on);

    updateGrid();
}

void AxesDialog::updateGrid()
{
    if (generalDialog.currentWidget() != gridPage)
        return;

    Grid *grid = (Grid *)d_graph->plotWidget()->grid();
    if (!grid)
        return;

    if (axesGridList->currentRow() == 1) {
        grid->enableX(boxMajorGrid->isChecked());
        grid->enableXMin(boxMinorGrid->isChecked());

        grid->setMajPenX(QPen(boxColorMajor->color(), boxWidthMajor->value(),
                              Graph::getPenStyle(boxTypeMajor->currentIndex())));
        grid->setMinPenX(QPen(boxColorMinor->color(), boxWidthMinor->value(),
                              Graph::getPenStyle(boxTypeMinor->currentIndex())));
    } else {
        grid->enableY(boxMajorGrid->isChecked());
        grid->enableYMin(boxMinorGrid->isChecked());

        grid->setMajPenY(QPen(boxColorMajor->color(), boxWidthMajor->value(),
                              Graph::getPenStyle(boxTypeMajor->currentIndex())));
        grid->setMinPenY(QPen(boxColorMinor->color(), boxWidthMinor->value(),
                              Graph::getPenStyle(boxTypeMinor->currentIndex())));
    }

    grid->enableZeroLineX(boxXLine->isChecked());
    grid->enableZeroLineY(boxYLine->isChecked());

    grid->setAxes(boxGridXAxis->currentIndex() + 2, boxGridYAxis->currentIndex());
    d_graph->replot();
    d_graph->notifyChanges();
}

void AxesDialog::showGridOptions(int axis)
{
    Grid *grd = (Grid *)d_graph->plotWidget()->grid();
    if (!grd)
        return;

    disconnect(boxMajorGrid, SIGNAL(toggled(bool)), this, SLOT(majorGridEnabled(bool)));
    disconnect(boxMinorGrid, SIGNAL(toggled(bool)), this, SLOT(minorGridEnabled(bool)));

    if (axis == 1) {
        boxMajorGrid->setChecked(grd->xEnabled());
        boxMinorGrid->setChecked(grd->xMinEnabled());

        boxXLine->setEnabled(true);
        boxYLine->setDisabled(true);

        boxGridXAxis->setEnabled(true);
        boxGridYAxis->setDisabled(true);

        QPen majPenX = grd->majPenX();
        boxTypeMajor->setCurrentIndex(majPenX.style() - 1);
        boxColorMajor->setColor(majPenX.color());
        boxWidthMajor->setValue(majPenX.width());

        QPen minPenX = grd->minPenX();
        boxTypeMinor->setCurrentIndex(minPenX.style() - 1);
        boxColorMinor->setColor(minPenX.color());
        boxWidthMinor->setValue(minPenX.width());
    } else if (axis == 0) {
        boxMajorGrid->setChecked(grd->yEnabled());
        boxMinorGrid->setChecked(grd->yMinEnabled());

        boxXLine->setDisabled(true);
        boxYLine->setEnabled(true);

        boxGridXAxis->setDisabled(true);
        boxGridYAxis->setEnabled(true);

        QPen majPenY = grd->majPenY();
        boxTypeMajor->setCurrentIndex(majPenY.style() - 1);
        boxColorMajor->setColor(majPenY.color());
        boxWidthMajor->setValue(majPenY.width());

        QPen minPenY = grd->minPenY();
        boxTypeMinor->setCurrentIndex(minPenY.style() - 1);
        boxColorMinor->setColor(minPenY.color());
        boxWidthMinor->setValue(minPenY.width());
    }

    bool majorOn = boxMajorGrid->isChecked();
    boxTypeMajor->setEnabled(majorOn);
    boxColorMajor->setEnabled(majorOn);
    boxWidthMajor->setEnabled(majorOn);

    bool minorOn = boxMinorGrid->isChecked();
    boxTypeMinor->setEnabled(minorOn);
    boxColorMinor->setEnabled(minorOn);
    boxWidthMinor->setEnabled(minorOn);

    boxGridXAxis->setCurrentIndex(grd->xAxis() - 2);
    boxGridYAxis->setCurrentIndex(grd->yAxis());

    boxXLine->setChecked(grd->xZeroLineEnabled());
    boxYLine->setChecked(grd->yZeroLineEnabled());

    connect(boxMajorGrid, SIGNAL(toggled(bool)), this, SLOT(majorGridEnabled(bool)));
    connect(boxMinorGrid, SIGNAL(toggled(bool)), this, SLOT(minorGridEnabled(bool)));
}

void AxesDialog::stepEnabled()
{
    boxStep->setEnabled(btnStep->isChecked());
    boxUnit->setEnabled(btnStep->isChecked());
    boxMajorValue->setDisabled(btnStep->isChecked());
    btnMajor->setChecked(!btnStep->isChecked());
}

void AxesDialog::stepDisabled()
{
    boxStep->setDisabled(btnMajor->isChecked());
    boxUnit->setDisabled(btnMajor->isChecked());
    boxMajorValue->setEnabled(btnMajor->isChecked());
    btnStep->setChecked(!btnMajor->isChecked());
}

void AxesDialog::updateAxisColor(int)
{
    int a = mapToQwtAxisId();
    boxAxisColor->setColor(d_graph->axisColor(a));
    boxAxisNumColor->setColor(d_graph->axisNumbersColor(a));
}

void AxesDialog::changeBaselineDist(int baseline)
{
    int axis = mapToQwtAxisId();
    axesBaseline[axis] = baseline;

    if (d_graph->axisTitle(axis) != boxTitle->toPlainText())
        d_graph->setAxisTitle(axis, boxTitle->toPlainText());

    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();
    showAxis(axis, currentSelectedAxisType(), formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
             boxPrecision->value(), boxAngle->value(), baseline, formula, boxAxisNumColor->color());
}

bool AxesDialog::updatePlot()
{
    if (generalDialog.currentWidget() == (QWidget *)scalesPage) {
        QString from = boxStart->text().toLower();
        QString to = boxEnd->text().toLower();
        QString step = boxStep->text().toLower();
        int a = Graph::mapToQwtAxis(axesList->currentRow());
        switch (axesType[a]) {
        case Graph::AxisType::Time:
        case Graph::AxisType::Date:
        case Graph::AxisType::DateTime: {
            QDateTime qdt = QDateTime::fromString(from, "yyyy-MM-dd hh:mm:ss");
            from = QString::number(qdt.toMSecsSinceEpoch() / 86400000. + 2440587.5, 'f', 16);
            qdt = QDateTime::fromString(to, "yyyy-MM-dd hh:mm:ss");
            to = QString::number(qdt.toMSecsSinceEpoch() / 86400000. + 2440587.5, 'f', 16);
            break;
        }
        default: {
        }
        }

        double start = NAN, end = NAN, stp = 0;
        try {
            MyParser parser;
            parser.SetExpr(from);
            start = parser.Eval();
        } catch (mu::ParserError &e) {
            QMessageBox::critical(nullptr, tr("Start limit error"), QStringFromString(e.GetMsg()));
            boxStart->setFocus();
            return false;
        }
        try {
            MyParser parser;
            parser.SetExpr(to);
            end = parser.Eval();
        } catch (mu::ParserError &e) {
            QMessageBox::critical(nullptr, tr("End limit error"), QStringFromString(e.GetMsg()));
            boxEnd->setFocus();
            return false;
        }
        if (btnStep->isChecked()) {
            try {
                MyParser parser;
                parser.SetExpr(step.toUtf8().constData());
                stp = parser.Eval();
            } catch (mu::ParserError &e) {
                QMessageBox::critical(nullptr, tr("Step input error"),
                                      QStringFromString(e.GetMsg()));
                boxStep->setFocus();
                return false;
            }

            if (stp <= 0) {
                QMessageBox::critical(nullptr, tr("Step input error"),
                                      tr("Please enter a positive step value!"));
                boxStep->setFocus();
                return false;
            }

            if (axesType[a] == Graph::AxisType::Time) {
                switch (boxUnit->currentIndex()) {
                case 0:
                    break;
                case 1:
                    stp *= 1e3;
                    break;
                case 2:
                    stp *= 6e4;
                    break;
                case 3:
                    stp *= 36e5;
                    break;
                }
            } else if (axesType[a] == Graph::AxisType::Date) {
                switch (boxUnit->currentIndex()) {
                case 0:
                    break;
                case 1:
                    stp *= 7;
                    break;
                }
            } else if (axesType[a] == Graph::AxisType::DateTime) {
                switch (boxUnit->currentIndex()) {
                case 0: // milliseconds
                    stp *= 1. / 86400000.;
                    break;
                case 1:
                    stp *= 1. / 86400.;
                    break;
                case 2:
                    stp *= 1. / 1440.;
                    break;
                case 3:
                    stp *= 1. / 24.;
                    break;
                case 4:
                    break;
                case 5:
                    stp *= 7;
                    break;
                }
            }
        }

        d_graph->setScale(a, start, end, stp, boxMajorValue->value(),
                          boxMinorValue->currentText().toInt(), boxScaleType->currentIndex(),
                          btnInvert->isChecked());
        d_graph->notifyChanges();
    } else if (generalDialog.currentWidget() == gridPage) {
        updateGrid();
    } else if (generalDialog.currentWidget() == (QWidget *)axesPage) {
        int axis = mapToQwtAxisId();
        auto format = currentSelectedAxisType();
        axesType[axis] = format;

        int baseline = boxBaseline->value();
        axesBaseline[axis] = baseline;

        if (format == Graph::AxisType::Numeric) {
            if (boxShowFormula->isChecked()) {
                QString formula = boxFormula->toPlainText().toLower();
                try {
                    double value = 1.0;
                    MyParser parser;
                    if (formula.contains("x"))
                        parser.DefineVar(_T("x"), &value);
                    else if (formula.contains("y"))
                        parser.DefineVar(_T("y"), &value);
                    parser.SetExpr(formula.toUtf8().constData());
                    parser.Eval();
                } catch (mu::ParserError &e) {
                    QMessageBox::critical(nullptr, tr("Formula input error"),
                                          QStringFromString(e.GetMsg()) + "\n"
                                                  + tr("Valid variables are 'x' for Top/Bottom "
                                                       "axes and 'y' for Left/Right axes!"));
                    boxFormula->setFocus();
                    return false;
                }
            }
        } else if (format == Graph::AxisType::Time || format == Graph::AxisType::Date
                   || format == Graph::AxisType::DateTime) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList lst = formatInfo[axis].split(";", Qt::KeepEmptyParts);
#else
            QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
#endif
            if (lst.size() < 2 || lst[0].isEmpty()) {
                lst = QStringList();
                if (format == Graph::AxisType::Time)
                    lst << QTime(0, 0, 0).toString();
                else if (format == Graph::AxisType::Date)
                    lst << QDate(1900, 1, 1).toString("yyyy-MM-dd");
                else
                    lst << QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0))
                                    .toString("yyyy-MM-ddThh:mm:ss");
                lst << boxFormat->currentText();
            } else
                lst[1] = boxFormat->currentText();
            formatInfo[axis] = lst.join(";");
        } else if (format == Graph::AxisType::Day || format == Graph::AxisType::Month)
            formatInfo[axis] = QString::number(boxFormat->currentIndex());
        else if (format == Graph::AxisType::ColHeader)
            formatInfo[axis] = boxTableName->currentText();
        else
            formatInfo[axis] = boxColName->currentText();

        if (d_graph->axisTitle(axis) != boxTitle->toPlainText())
            d_graph->setAxisTitle(axis, boxTitle->toPlainText());

        if (axis == QwtPlot::xBottom)
            xBottomLabelsRotation = boxAngle->value();
        else if (axis == QwtPlot::xTop)
            xTopLabelsRotation = boxAngle->value();

        QString formula = boxFormula->toPlainText();
        if (!boxShowFormula->isChecked())
            formula = QString();
        showAxis(axis, format, formatInfo[axis], boxShowAxis->isChecked(),
                 boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
                 boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
                 boxPrecision->value(), boxAngle->value(), baseline, formula,
                 boxAxisNumColor->color());
    } else if (generalDialog.currentWidget() == (QWidget *)frame) {
        d_graph->setAxesLinewidth(boxAxesLinewidth->value());
        d_graph->changeTicksLength(boxMinorTicksLength->value(), boxMajorTicksLength->value());
        d_graph->drawCanvasFrame(boxFramed->isChecked(), boxFrameWidth->value(),
                                 boxFrameColor->color());
        d_graph->drawAxesBackbones(boxBackbones->isChecked());
    }

    return true;
}

void AxesDialog::setGraph(Graph *g)
{
    auto *app = qobject_cast<ApplicationWindow *>(parent());

    if (!app || !g)
        return;

    d_graph = g;
    Plot *p = d_graph->plotWidget();

    tablesList = app->tableWindows();
    boxTableName->addItems(tablesList);

    boxColName->addItems(app->columnsList());

    xAxisOn = p->axisEnabled(QwtPlot::xBottom);
    yAxisOn = p->axisEnabled(QwtPlot::yLeft);
    topAxisOn = p->axisEnabled(QwtPlot::xTop);
    rightAxisOn = p->axisEnabled(QwtPlot::yRight);

    xBottomFont = p->axisFont(QwtPlot::xBottom);
    yLeftFont = p->axisFont(QwtPlot::yLeft);
    xTopFont = p->axisFont(QwtPlot::xTop);
    yRightFont = p->axisFont(QwtPlot::yRight);

    majTicks = p->getMajorTicksType();
    minTicks = p->getMinorTicksType();

    formatInfo = g->axesLabelsFormatInfo();
    updateTitleBox(0);

    xBottomLabelsRotation = g->labelsRotation(QwtPlot::xBottom);
    xTopLabelsRotation = g->labelsRotation(QwtPlot::xTop);

    tickLabelsOn = g->enabledTickLabels();

    axesBaseline = g->axesBaseline();

    axesType = d_graph->axesType();

    boxAxesLinewidth->setValue(p->axesLinewidth());
    boxBackbones->setChecked(d_graph->axesBackbones());

    boxFramed->setChecked(d_graph->framed());
    boxFrameColor->setColor(d_graph->canvasFrameColor());
    boxFrameWidth->setValue(d_graph->canvasFrameWidth());

    boxMinorTicksLength->setValue(p->minorTickLength());
    boxMajorTicksLength->setValue(p->majorTickLength());

    showGridOptions(axesGridList->currentRow());
}

int AxesDialog::mapToQwtAxisId()
{
    return Graph::mapToQwtAxis(axesTitlesList->currentRow());
}

void AxesDialog::updateScale()
{
    int axis = axesList->currentRow();

    boxStart->clear();
    boxEnd->clear();
    boxStep->clear();
    boxUnit->hide();
    boxUnit->clear();

    Plot *d_plot = d_graph->plotWidget();
    int a = Graph::mapToQwtAxis(axis);
    const QwtScaleDiv scDiv = d_plot->axisScaleDiv(a);
    double astart = std::min(scDiv.lowerBound(), scDiv.upperBound());
    double aend = std::max(scDiv.lowerBound(), scDiv.upperBound());
    double astep = d_graph->axisStep(a);

    switch (axesType[a]) {
    case Graph::AxisType::Time:
    case Graph::AxisType::Date:
    case Graph::AxisType::DateTime: {
        QDateTime qdt = QDateTime::fromMSecsSinceEpoch(round((astart - 2440587.5) * 86400000.));
        boxStart->setText(qdt.toString("yyyy-MM-dd hh:mm:ss"));
        qdt = QDateTime::fromMSecsSinceEpoch(round((aend - 2440587.5) * 86400000.));
        boxEnd->setText(qdt.toString("yyyy-MM-dd hh:mm:ss"));
        break;
    }
    default: {
        boxStart->setText(QString::number(astart));
        boxEnd->setText(QString::number(aend));
    }
    }

    QList<double> lst = scDiv.ticks(QwtScaleDiv::MajorTick);
    boxStep->setText(QString::number(d_graph->axisStep(a)));
    boxMajorValue->setValue(lst.count());

    if (axesType[a] == Graph::AxisType::Time) {
        boxUnit->show();
        boxUnit->addItem(tr("millisec."));
        boxUnit->addItem(tr("sec."));
        boxUnit->addItem(tr("min."));
        boxUnit->addItem(tr("hours"));
    } else if (axesType[a] == Graph::AxisType::Date) {
        boxUnit->show();
        boxUnit->addItem(tr("days"));
        boxUnit->addItem(tr("weeks"));
    } else if (axesType[a] == Graph::AxisType::DateTime) {
        boxUnit->show();
        boxUnit->addItem(tr("millisec."));
        boxUnit->addItem(tr("sec."));
        boxUnit->addItem(tr("min."));
        boxUnit->addItem(tr("hours"));
        boxUnit->addItem(tr("days"));
        boxUnit->addItem(tr("weeks"));
        boxUnit->setCurrentIndex(4); // days
    }

    if (d_graph->axisStep(a) != 0.0) {
        btnStep->setChecked(true);
        boxStep->setEnabled(true);
        boxUnit->setEnabled(true);
        if (axesType[a] == Graph::AxisType::DateTime) {
            if (abs(astep * 24. - round(astep * 24.)) < 1e-6 * astep) {
                astep *= 24;
                boxUnit->setCurrentIndex(3); // hours
            } else if (abs(astep * 1440. - round(astep * 1440.)) < 1e-6 * astep) {
                astep *= 1440.;
                boxUnit->setCurrentIndex(2); // minutes
            } else if (abs(astep * 86400. - round(astep * 86400.)) < 1e-6 * astep) {
                astep *= 86400.;
                boxUnit->setCurrentIndex(1); // seconds
            }
            boxStep->setText(QString::number(astep));
        }

        btnMajor->setChecked(false);
        boxMajorValue->setEnabled(false);
    } else {
        btnStep->setChecked(false);
        boxStep->setEnabled(false);
        boxUnit->setEnabled(false);
        btnMajor->setChecked(true);
        boxMajorValue->setEnabled(true);
    }

    const QwtScaleEngine *sc_eng = d_plot->axisScaleEngine(a);
    btnInvert->setChecked(sc_eng->testAttribute(QwtScaleEngine::Inverted));

    // QwtTransform *tr = sc_eng->transformation();
    // boxScaleType->setCurrentIndex((int)tr->type());

    boxMinorValue->clear();
    /*if (tr->type()) // log scale
        boxMinorValue->addItems(QStringList() << "0"
                                              << "2"
                                              << "4"
                                              << "8");
    else*/
    boxMinorValue->addItems(QStringList() << "0"
                                          << "1"
                                          << "4"
                                          << "9"
                                          << "14"
                                          << "19");

    boxMinorValue->setEditText(QString::number(d_plot->axisMaxMinor(a)));
}

void AxesDialog::updateTitleBox(int axis)
{
    int axisId = Graph::mapToQwtAxis(axis);
    boxTitle->setText(d_graph->axisTitle(axisId));
}

void AxesDialog::pickAxisColor()
{
    QColor c = QColorDialog::getColor(boxAxisColor->color(), this);
    if (!c.isValid() || c == boxAxisColor->color())
        return;

    boxAxisColor->setColor(c);

    int axis = mapToQwtAxisId();
    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, currentSelectedAxisType(), formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), c, boxFormat->currentIndex(), boxPrecision->value(),
             boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::pickAxisNumColor()
{
    QColor c = QColorDialog::getColor(boxAxisNumColor->color(), this);
    if (!c.isValid() || c == boxAxisNumColor->color())
        return;

    boxAxisNumColor->setColor(c);
    int axis = mapToQwtAxisId();
    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, currentSelectedAxisType(), formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
             boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula, c);
}

void AxesDialog::setAxisType(int)
{
    int a = mapToQwtAxisId();
    auto type = d_graph->axesType()[a];

    boxAxisType->setCurrentIndex(boxAxisType->findData(static_cast<int>(type)));
    showAxisFormatOptions(boxAxisType->findData(static_cast<int>(type)));

    if (type == Graph::AxisType::Txt)
        boxColName->setItemText(boxColName->currentIndex(), formatInfo[a]);
}

void AxesDialog::setBaselineDist(int)
{
    int a = mapToQwtAxisId();
    boxBaseline->setValue(axesBaseline[a]);
}

void AxesDialog::setTicksType(int)
{
    int a = mapToQwtAxisId();
    boxMajorTicksType->setCurrentIndex(majTicks[a]);
    boxMinorTicksType->setCurrentIndex(minTicks[a]);
}

void AxesDialog::updateMajTicksType(int)
{
    int axis = mapToQwtAxisId();
    int type = boxMajorTicksType->currentIndex();
    if (majTicks[axis] == type)
        return;

    majTicks[axis] = type;
    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, currentSelectedAxisType(), formatInfo[axis], boxShowAxis->isChecked(), type,
             boxMinorTicksType->currentIndex(), boxShowLabels->isChecked(), boxAxisColor->color(),
             boxFormat->currentIndex(), boxPrecision->value(), boxAngle->value(),
             boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::updateMinTicksType(int)
{
    int axis = mapToQwtAxisId();
    int type = boxMinorTicksType->currentIndex();
    if (minTicks[axis] == type)
        return;

    minTicks[axis] = type;
    QString formula = boxFormula->toPlainText();

    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, currentSelectedAxisType(), formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), type, boxShowLabels->isChecked(),
             boxAxisColor->color(), boxFormat->currentIndex(), boxPrecision->value(),
             boxAngle->value(), boxBaseline->value(), formula, boxAxisNumColor->color());
}

void AxesDialog::updateTickLabelsList(bool on)
{
    int axis = mapToQwtAxisId();
    if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
        boxAngle->setEnabled(on);

    bool userFormat = true;
    if (boxFormat->currentIndex() == 0)
        userFormat = false;
    boxPrecision->setEnabled(on && userFormat);

    if (tickLabelsOn[axis] == QString::number(on))
        return;
    tickLabelsOn[axis] = QString::number(on);

    auto type = currentSelectedAxisType();
    if (type == Graph::AxisType::Day || type == Graph::AxisType::Month)
        formatInfo[axis] = QString::number(boxFormat->currentIndex());
    else if (type == Graph::AxisType::Time || type == Graph::AxisType::Date
             || type == Graph::AxisType::DateTime) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = formatInfo[axis].split(";", Qt::SkipEmptyParts);
#else
        QStringList lst = formatInfo[axis].split(";", QString::SkipEmptyParts);
#endif
        if (lst.size() < 2 || lst[0].isEmpty()) {
            lst = QStringList();
            if (type == Graph::AxisType::Time)
                lst << QTime(0, 0, 0).toString();
            else if (type == Graph::AxisType::Date)
                lst << QDate(1900, 1, 1).toString("yyyy-MM-dd");
            else
                lst << QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0)).toString("yyyy-MM-ddThh:mm:ss");
            lst << boxFormat->currentText();
        } else
            lst[1] = boxFormat->currentText();
        formatInfo[axis] = lst.join(";");
    } else
        formatInfo[axis] = boxColName->currentText();

    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, type, formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), boxAxisColor->color(), boxFormat->currentIndex(),
             boxPrecision->value(), boxAngle->value(), boxBaseline->value(), formula,
             boxAxisNumColor->color());
}

void AxesDialog::setCurrentScale(int axisPos)
{
    int axis = -1;
    switch (axisPos) {
    case QwtScaleDraw::LeftScale:
        axis = 1;
        break;
    case QwtScaleDraw::BottomScale:
        axis = 0;
        break;
    case QwtScaleDraw::RightScale:
        axis = 3;
        break;
    case QwtScaleDraw::TopScale:
        axis = 2;
        break;
    }
    if (generalDialog.currentWidget() == (QWidget *)scalesPage)
        axesList->setCurrentRow(axis);
    else if (generalDialog.currentWidget() == (QWidget *)axesPage)
        axesTitlesList->setCurrentRow(axis);
}

void AxesDialog::showAxesPage()
{
    if (generalDialog.currentWidget() != (QWidget *)axesPage)
        generalDialog.setCurrentWidget(axesPage);
}

void AxesDialog::showGridPage()
{
    if (generalDialog.currentWidget() != (QWidget *)gridPage)
        generalDialog.setCurrentWidget(gridPage);
}

void AxesDialog::setLabelsNumericFormat(int)
{
    int axis = mapToQwtAxisId();
    auto type = currentSelectedAxisType();
    int prec = boxPrecision->value();
    int format = boxFormat->currentIndex();

    Plot *plot = d_graph->plotWidget();

    if (type == Graph::AxisType::Numeric) {
        if (plot->axisLabelFormat(axis) == format && plot->axisLabelPrecision(axis) == prec)
            return;

        if (format == 0)
            boxPrecision->setEnabled(false);
        else
            boxPrecision->setEnabled(true);
    } else if (type == Graph::AxisType::Day || type == Graph::AxisType::Month)
        formatInfo[axis] = QString::number(format);
    else if (type == Graph::AxisType::Time || type == Graph::AxisType::Date
             || type == Graph::AxisType::DateTime) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList lst = formatInfo[axis].split(";", Qt::KeepEmptyParts);
#else
        QStringList lst = formatInfo[axis].split(";", QString::KeepEmptyParts);
#endif
        if (lst.size() < 2 || lst[0].isEmpty()) {
            lst = QStringList();
            if (type == Graph::AxisType::Time)
                lst << QTime(0, 0, 0).toString();
            else if (type == Graph::AxisType::Date)
                lst << QDate(1900, 1, 1).toString("yyyy-MM-dd");
            else
                lst << QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0)).toString("yyyy-MM-ddThh:mm:ss");
            lst << boxFormat->currentText();
        } else
            lst[1] = boxFormat->currentText();
        formatInfo[axis] = lst.join(";");
    } else
        formatInfo[axis] = boxColName->currentText();

    QString formula = boxFormula->toPlainText();
    if (!boxShowFormula->isChecked())
        formula = QString();

    showAxis(axis, type, formatInfo[axis], boxShowAxis->isChecked(),
             boxMajorTicksType->currentIndex(), boxMinorTicksType->currentIndex(),
             boxShowLabels->isChecked(), boxAxisColor->color(), format, prec, boxAngle->value(),
             boxBaseline->value(), formula, boxAxisNumColor->color());

    axesType[axis] = type;
}

void AxesDialog::showAxisFormula(int axis)
{
    QStringList l = d_graph->getAxesFormulas();
    QString formula = l[axis];
    if (!formula.isEmpty()) {
        boxShowFormula->setChecked(true);
        boxFormula->show();
        boxFormula->setText(formula);
    } else {
        boxShowFormula->setChecked(false);
        boxFormula->clear();
        boxFormula->hide();
    }
}

void AxesDialog::updateLabelsFormat(int)
{
    if (currentSelectedAxisType() != Graph::AxisType::Numeric)
        return;

    int a = mapToQwtAxisId();
    int format = d_graph->plotWidget()->axisLabelFormat(a);
    boxFormat->setCurrentIndex(format);
    boxPrecision->setValue(d_graph->plotWidget()->axisLabelPrecision(a));

    if (format == 0)
        boxPrecision->setEnabled(false);
    else
        boxPrecision->setEnabled(true);

    QStringList l = d_graph->getAxesFormulas();
    QString formula = l[a];
    if (!formula.isEmpty()) {
        boxShowFormula->setChecked(true);
        boxFormula->show();
        boxFormula->setText(formula);
    } else {
        boxShowFormula->setChecked(false);
        boxFormula->clear();
        boxFormula->hide();
    }
}

void AxesDialog::showGeneralPage()
{
    generalDialog.setCurrentIndex(generalDialog.indexOf(frame));
}

void AxesDialog::showFormulaBox()
{
    if (boxShowFormula->isChecked())
        boxFormula->show();
    else
        boxFormula->hide();
}

void AxesDialog::customAxisLabelFont()
{
    int axis = mapToQwtAxisId();
    bool okF = false;
    QFont oldFont = d_graph->axisTitleFont(axis);
    QFont fnt = QFontDialog::getFont(&okF, oldFont, this);
    if (okF && fnt != oldFont)
        d_graph->setAxisTitleFont(axis, fnt);
}

void AxesDialog::pageChanged(int page)
{
    QWidget *pageWidget = generalDialog.widget(page);
    if (lastPage == scalesPage && pageWidget == axesPage) {
        axesTitlesList->setCurrentRow(axesList->currentRow());
        lastPage = pageWidget;
    } else if (lastPage == axesPage && pageWidget == scalesPage) {
        axesList->setCurrentRow(axesTitlesList->currentRow());
        lastPage = pageWidget;
    }
}

int AxesDialog::exec()
{
    axesList->setCurrentRow(0);
    axesGridList->setCurrentRow(0);
    axesTitlesList->setCurrentRow(0);

    setModal(true);
    show();
    return 0;
}

void AxesDialog::updateMinorTicksList(int scaleType)
{
    updatePlot();

    boxMinorValue->clear();
    if (scaleType) // log scale
        boxMinorValue->addItems(QStringList() << "0"
                                              << "2"
                                              << "4"
                                              << "8");
    else
        boxMinorValue->addItems(QStringList() << "0"
                                              << "1"
                                              << "4"
                                              << "9"
                                              << "14"
                                              << "19");

    int a = Graph::mapToQwtAxis(axesList->currentRow());
    boxMinorValue->setEditText(QString::number(d_graph->plotWidget()->axisMaxMinor(a)));
}

void AxesDialog::showAxis(int axis, Graph::AxisType type, const QString &labelsColName, bool axisOn,
                          int majTicksType, int minTicksType, bool labelsOn, const QColor &c,
                          int format, int prec, int rotation, int baselineDist,
                          const QString &formula, const QColor &labelsColor)
{
    auto *app = qobject_cast<ApplicationWindow *>(parent());
    if (!app)
        return;

    Table *w = app->table(labelsColName);
    if ((type == Graph::AxisType::Txt || type == Graph::AxisType::ColHeader) && !w)
        return;

    if (!d_graph)
        return;
    d_graph->showAxis(axis, type, labelsColName, w, axisOn, majTicksType, minTicksType, labelsOn, c,
                      format, prec, rotation, baselineDist, formula, labelsColor);
}

Graph::AxisType AxesDialog::currentSelectedAxisType()
{
    int index = boxAxisType->currentIndex();
    if (index < 0)
        return Graph::AxisType::Numeric;
    return static_cast<Graph::AxisType>(boxAxisType->itemData(index).toInt());
}
