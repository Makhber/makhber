/***************************************************************************
        File                 : ApplicationWindow.cpp
        Project              : Makhber
        Description          : Makhber's main window
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2004-2007 by Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email address)

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

#define HOMEPAGE_URI "https://github.com/Makhber/makhber"
#define MANUAL_URI "http://sourceforge.net/projects/scidavis/files/SciDAVis%20Documentation/0.1/"
#define FORUM_URI "https://github.com/Makhber/makhber/discussions"
#define BUGREPORT_URI "https://github.com/Makhber/makhber/issues"
#define DOWNLOAD_URI "https://github.com/Makhber/makhber/releases/latest"

#include "globals.h"
#include "ApplicationWindow.h"
#include "CurvesDialog.h"
#include "PlotDialog.h"
#include "AxesDialog.h"
#include "LineDialog.h"
#include "TextDialog.h"
#include "ExportDialog.h"
#include "ErrDialog.h"
#include "Legend.h"
#include "ArrowMarker.h"
#include "ImageMarker.h"
#include "Graph.h"
#include "Plot.h"
#include "Grid.h"
#include "PlotWizard.h"
#include "PolynomFitDialog.h"
#include "ExpDecayDialog.h"
#include "FunctionDialog.h"
#include "FitDialog.h"
#include "SurfaceDialog.h"
#include "Graph3D.h"
#include "Plot3DDialog.h"
#include "ImageDialog.h"
#include "MultiLayer.h"
#include "LayerDialog.h"
#include "DataSetDialog.h"
#include "IntDialog.h"
#include "ConfigDialog.h"
#ifdef ORIGIN_IMPORT
#include "importOPJ.h"
#endif
#include "AssociationsDialog.h"
#include "RenameWindowDialog.h"
#include "QwtErrorPlotCurve.h"
#include "InterpolationDialog.h"
#include "ImportASCIIDialog.h"
#include "ImageExportDialog.h"
#include "SmoothCurveDialog.h"
#include "FilterDialog.h"
#include "FFTDialog.h"
#include "Note.h"
#include "Folder.h"
#include "FindDialog.h"
#include "ScaleDraw.h"
#include "ScriptingLangDialog.h"
#include "TableStatistics.h"
#include "Fit.h"
#include "MultiPeakFit.h"
#include "PolynomialFit.h"
#include "SigmoidalFit.h"
#include "FunctionCurve.h"
#include "QwtPieCurve.h"
#include "Spectrogram.h"
#include "Differentiation.h"
#include "SmoothFilter.h"
#include "FFTFilter.h"
#include "Convolution.h"
#include "Correlation.h"
#include "CurveRangeDialog.h"
#include "ColorButton.h"
#include "QwtHistogram.h"
#include "OpenProjectDialog.h"
#include "IconLoader.h"
#include "core/Project.h"
#include "core/column/Column.h"
#include "lib/XmlStreamReader.h"
#include "table/future_Table.h"

// TODO: move tool-specific code to an extension manager
#include "ScreenPickerTool.h"
#include "DataPickerTool.h"
#include "TranslateCurveTool.h"
#include "MultiPeakFitTool.h"
#include "LineProfileTool.h"

#include <stdio.h>
#include <stdlib.h>

#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QPrintDialog>
#include <QPixmapCache>
#include <QMenuBar>
#include <QClipboard>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTranslator>
#include <QSplitter>
#include <QApplication>
#include <QMessageBox>
#include <QPrinter>
#include <QActionGroup>
#include <QAction>
#include <QToolBar>
#include <QKeySequence>
#include <QImageReader>
#include <QImageWriter>
#include <QDateTime>
#include <QShortcut>
#include <QDockWidget>
#include <QTextStream>
#include <QVarLengthArray>
#include <QList>
#include <QUrl>
#include <QDesktopServices>
#include <QStatusBar>
#include <QToolButton>
#include <QSignalMapper>
#include <QUndoStack>
#include <QtDebug>
#include <QDialogButtonBox>
#include <QUndoView>
#include <QUndoStack>
#include <QTemporaryFile>
#include <QDebug>
#include <QTextCodec>
#include <QScrollBar>
#include <QMimeData>
#include <QElapsedTimer>
#include <QJsonDocument>

#include <zlib.h>

#include <iostream>
#include <memory>
using namespace std;

#ifdef Q_OS_WIN
#include <io.h> // for _commit()
#else
#include <unistd.h> // for fsync()
#endif

using namespace Qwt3D;

extern "C" {
void file_compress(const char *file, const char *mode);
}

ApplicationWindow::ApplicationWindow()
    : scripted(ScriptingLangManager::newEnv(this)),
      //      logWindow(new QDockWidget(this)),
      //      explorerWindow(new QDockWidget(this)),
      //      results(new QTextEdit(logWindow)),
      //#ifdef SCRIPTING_CONSOLE
      //      consoleWindow(new QDockWidget(this)),
      //      console(new QTextEdit(consoleWindow)),
      //#endif
      //      d_workspace(new QMdiArea(this)),
      //      lv(new FolderListView()),
      //      folders(new FolderListView()),

      //      hiddenWindows(new QList<MyWidget*>()),
      //      outWindows(new QList<MyWidget*>()),
      lastModified(0),
      current_folder(new Folder(tr("UNTITLED"))),
      show_windows_policy(ActiveFolder),
      appStyle(qApp->style()->objectName()),
      appFont(QFont()),
      projectname("untitled"),
      logInfo(QString()),
      savingTimerId(0),
      copiedLayer(false),
      renamedTables(QStringList()),
      copiedMarkerType(Graph::None),
#ifdef SEARCH_FOR_UPDATES
      autoSearchUpdatesRequest(false),
#endif
      lastCopiedLayer(0),
      explorerSplitter(new QSplitter(Qt::Horizontal, &explorerWindow)),
      actionNextWindow(new QAction(QIcon(QPixmap(":/next.xpm")), tr("&Next", "next window"), this)),
      actionPrevWindow(
              new QAction(QIcon(QPixmap(":/prev.xpm")), tr("&Previous", "previous window"), this))

{
    setAttribute(Qt::WA_DeleteOnClose);
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    setWindowTitle(tr("Makhber - untitled"));

    // Icons
    IconLoader::init();
    IconLoader::lumen_ = IconLoader::isLight(palette().color(QPalette::Window));

    initFonts();
    QPixmapCache::setCacheLimit(20 * QPixmapCache::cacheLimit());

    d_project = new Project();
    connect(d_project, SIGNAL(aspectAdded(const AbstractAspect *, int)), this,
            SLOT(handleAspectAdded(const AbstractAspect *, int)));
    connect(d_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *, int)), this,
            SLOT(handleAspectAboutToBeRemoved(const AbstractAspect *, int)));

    explorerWindow.setWindowTitle(tr("Project Explorer"));
    explorerWindow.setObjectName(
            "explorerWindow"); // this is needed for QMainWindow::restoreState()
    explorerWindow.setMinimumHeight(150);
    addDockWidget(Qt::BottomDockWidgetArea, &explorerWindow);

    folders.setObjectName("folders");
    folders.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    folders.setContextMenuPolicy(Qt::CustomContextMenu);

    folders.setHeaderLabels(QStringList() << tr("Folder") << QString());
    folders.setRootIsDecorated(true);
    folders.setColumnWidth(1, 0); // helps autoScroll
    folders.hideColumn(1); // helps autoScroll
#if QT_VERSION >= 0x050000
    folders.header()->setSectionsClickable(false);
    folders.header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    folders.header()->setClickable(false);
    folders.header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    folders.header()->hide();
    folders.setSelectionMode(QTreeWidget::SingleSelection);

    connect(&folders, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this,
            SLOT(folderItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(&folders, SIGNAL(itemRenamed(QTreeWidgetItem *, int, const QString &)), this,
            SLOT(renameFolder(QTreeWidgetItem *, int, const QString &)));
    connect(&folders, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(showFolderPopupMenu(const QPoint &)));
    connect(&folders, SIGNAL(dragItems(QList<QTreeWidgetItem *>)), this,
            SLOT(dragFolderItems(QList<QTreeWidgetItem *>)));
    connect(&folders, SIGNAL(dropItems(QTreeWidgetItem *)), this,
            SLOT(dropFolderItems(QTreeWidgetItem *)));
    connect(&folders, SIGNAL(renameItem(QTreeWidgetItem *, int)), this,
            SLOT(startRenameFolder(QTreeWidgetItem *, int)));
    connect(&folders, SIGNAL(addFolderItem()), this, SLOT(addFolder()));
    connect(&folders, SIGNAL(deleteSelection()), this, SLOT(deleteSelectedItems()));

    FolderListItem *fli = new FolderListItem(&folders, current_folder);
    current_folder->setFolderListItem(fli);
    folders.setCurrentItem(fli);
    fli->setExpanded(true);

    lv.setObjectName("lv");
    lv.setRootIsDecorated(false);
    lv.setContextMenuPolicy(Qt::CustomContextMenu);
    lv.setHeaderLabels(QStringList()
                       << tr("Name") << tr("Type") << tr("View") << tr("Created") << tr("Label"));
    lv.header()->setStretchLastSection(true);
    lv.setMinimumHeight(80);
    lv.setSelectionMode(QTreeWidget::ExtendedSelection);

    explorerSplitter->addWidget(&folders);
    explorerSplitter->addWidget(&lv);
    explorerWindow.setWidget(explorerSplitter);
    explorerSplitter->setSizes(QList<int>() << 50 << 50);
    explorerWindow.hide();

    logWindow.setObjectName("logWindow"); // this is needed for QMainWindow::restoreState()
    logWindow.setWindowTitle(tr("Results Log"));
    addDockWidget(Qt::TopDockWidgetArea, &logWindow);

    results->setReadOnly(true);

    logWindow.setWidget(results);
    logWindow.hide();

#ifdef SCRIPTING_CONSOLE
    consoleWindow.setObjectName("consoleWindow"); // this is needed for QMainWindow::restoreState()
    consoleWindow.setWindowTitle(tr("Scripting Console"));
    addDockWidget(Qt::TopDockWidgetArea, &consoleWindow);
    console.setReadOnly(true);
    consoleWindow.setWidget(&console);
    consoleWindow.hide();
#endif

    // Needs to be done after initialization of dock windows,
    // because we now use QDockWidget::toggleViewAction()
    createActions();
    initToolBars();
    initPlot3DToolBar();
    initMainMenu();

    d_workspace.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d_workspace.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d_workspace.setActivationOrder(QMdiArea::ActivationHistoryOrder);
    setCentralWidget(&d_workspace);
    setAcceptDrops(true);

    readSettings();
    createLanguagesList();
    insertTranslatedStrings();

    actionNextWindow->setShortcut(tr("F5", "next window shortcut"));
    connect(actionNextWindow, SIGNAL(triggered()), &d_workspace, SLOT(activateNextSubWindow()));

    actionPrevWindow->setShortcut(tr("F6", "previous window shortcut"));
    connect(actionPrevWindow, SIGNAL(triggered()), &d_workspace, SLOT(activatePreviousSubWindow()));

    connect(this, SIGNAL(modified()), this, SLOT(modifiedProject()));
    connect(&d_workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this,
            SLOT(windowActivated(QMdiSubWindow *)));
    connect(&lv, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this,
            SLOT(folderItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(&lv, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(showWindowPopupMenu(const QPoint &)));
    connect(&lv, SIGNAL(dragItems(QList<QTreeWidgetItem *>)), this,
            SLOT(dragFolderItems(QList<QTreeWidgetItem *>)));
    connect(&lv, SIGNAL(dropItems(QTreeWidgetItem *)), this,
            SLOT(dropFolderItems(QTreeWidgetItem *)));
    connect(&lv, SIGNAL(renameItem(QTreeWidgetItem *, int)), this,
            SLOT(startRenameFolder(QTreeWidgetItem *, int)));
    connect(&lv, SIGNAL(addFolderItem()), this, SLOT(addFolder()));
    connect(&lv, SIGNAL(deleteSelection()), this, SLOT(deleteSelectedItems()));
    connect(&lv, SIGNAL(itemRenamed(QTreeWidgetItem *, int, const QString &)), this,
            SLOT(renameWindow(QTreeWidgetItem *, int, const QString &)));
    connect(scriptEnv, SIGNAL(error(const QString &, const QString &, int)), this,
            SLOT(scriptError(const QString &, const QString &, int)));
    connect(scriptEnv, SIGNAL(print(const QString &)), this, SLOT(scriptPrint(const QString &)));

#ifdef SEARCH_FOR_UPDATES
    connect(&http, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(receivedVersionFile(QNetworkReply *)));
#endif

    // this has to be done after connecting scriptEnv
    scriptEnv->initialize();

    lv.setDragEnabled(true);
    lv.setAcceptDrops(true);
    lv.setDefaultDropAction(Qt::MoveAction);
    folders.setDragEnabled(true);
    folders.setAcceptDrops(true);
    folders.setDefaultDropAction(Qt::MoveAction);

    connect(d_project->undoStack(), SIGNAL(canUndoChanged(bool)), actionUndo,
            SLOT(setEnabled(bool)));
    connect(d_project->undoStack(), SIGNAL(canRedoChanged(bool)), actionRedo,
            SLOT(setEnabled(bool)));
}

void ApplicationWindow::initFonts()
{
    QString family = appFont.family();
    int pointSize = appFont.pointSize();
    tableTextFont = appFont;
    tableHeaderFont = appFont;
    plotAxesFont = QFont(family, pointSize, QFont::Bold, false);
    plotNumbersFont = QFont(family, pointSize);
    plotLegendFont = appFont;
    plotTitleFont = QFont(family, pointSize + 2, QFont::Bold, false);

    plot3DAxesFont = QFont(family, pointSize, QFont::Bold, false);
    plot3DNumbersFont = QFont(family, pointSize);
    plot3DTitleFont = QFont(family, pointSize + 2, QFont::Bold, false);
}

void ApplicationWindow::applyUserSettings()
{
    updateAppFonts();
    setScriptingLang(defaultScriptingLang);

    d_workspace.setBackground(workspaceColor);

    QPalette cg;
    cg.setColor(QPalette::Base, QColor(panelsColor));
    qApp->setPalette(cg);

    cg.setColor(QPalette::Text, QColor(panelsTextColor));
    cg.setColor(QPalette::WindowText, QColor(panelsTextColor));
    cg.setColor(QPalette::HighlightedText, QColor(panelsTextColor));
    lv.setPalette(cg);
    results->setPalette(cg);

    cg.setColor(QPalette::Text, QColor(Qt::green));
    cg.setColor(QPalette::HighlightedText, QColor(Qt::darkGreen));
    cg.setColor(QPalette::Base, QColor(Qt::black));
}

void ApplicationWindow::initToolBars()
{
    setWindowIcon(QIcon(":/appicon"));
    QPixmap openIcon, saveIcon;

    file_tools = new QToolBar(tr("File"), this);
    file_tools->setObjectName("file_tools"); // this is needed for QMainWindow::restoreState()
    file_tools->setIconSize(QSize(22, 22));
    addToolBar(Qt::TopToolBarArea, file_tools);

    file_tools->addAction(actionNewProject);

    QMenu *menu_new_aspect = new QMenu(this);
    menu_new_aspect->addAction(actionNewTable);
    menu_new_aspect->addAction(actionNewMatrix);
    menu_new_aspect->addAction(actionNewNote);
    menu_new_aspect->addAction(actionNewGraph);
    menu_new_aspect->addAction(actionNewFunctionPlot);
    menu_new_aspect->addAction(actionNewSurfacePlot);
    QToolButton *btn_new_aspect = new QToolButton(this);
    btn_new_aspect->setMenu(menu_new_aspect);
    btn_new_aspect->setPopupMode(QToolButton::InstantPopup);
    btn_new_aspect->setIcon(QPixmap(":/new_aspect.xpm"));
    btn_new_aspect->setToolTip(tr("New Aspect"));
    file_tools->addWidget(btn_new_aspect);

    file_tools->addAction(actionOpen);
    file_tools->addAction(actionOpenTemplate);
    file_tools->addAction(actionLoad);
    file_tools->addAction(actionSaveProject);
    file_tools->addAction(actionSaveTemplate);

    file_tools->addSeparator();

    file_tools->addAction(actionPrint);
    file_tools->addAction(actionExportPDF);

    file_tools->addSeparator();

    file_tools->addAction(actionShowExplorer);
    file_tools->addAction(actionShowLog);
    file_tools->addAction(locktoolbar);

    edit_tools = new QToolBar(tr("Edit"), this);
    edit_tools->setObjectName("edit_tools"); // this is needed for QMainWindow::restoreState()
    edit_tools->setIconSize(QSize(22, 22));
    addToolBar(edit_tools);

    edit_tools->addAction(actionUndo);
    edit_tools->addAction(actionRedo);
    edit_tools->addAction(actionCutSelection);
    edit_tools->addAction(actionCopySelection);
    edit_tools->addAction(actionPasteSelection);
    edit_tools->addAction(actionClearSelection);

    graph_tools = new QToolBar(tr("Graph"), this);
    graph_tools->setObjectName("graph_tools"); // this is needed for QMainWindow::restoreState()
    graph_tools->setIconSize(QSize(22, 22));
    addToolBar(graph_tools);

    dataTools = new QActionGroup(this);
    dataTools->setExclusive(true);

    btnPointer = new QAction(tr("Disable &Tools"), this);
    btnPointer->setActionGroup(dataTools);
    btnPointer->setCheckable(true);
    btnPointer->setIcon(QIcon(QPixmap(":/pointer.xpm")));
    btnPointer->setChecked(true);
    graph_tools->addAction(btnPointer);

    graph_tools->addSeparator();

    QMenu *menu_layers = new QMenu(this);
    QToolButton *btn_layers = new QToolButton(this);
    btn_layers->setMenu(menu_layers);
    btn_layers->setPopupMode(QToolButton::InstantPopup);
    btn_layers->setIcon(QPixmap(":/arrangeLayers.xpm"));
    btn_layers->setToolTip(tr("Manage layers"));
    graph_tools->addWidget(btn_layers);

    menu_layers->addAction(actionAutomaticLayout);
    menu_layers->addAction(actionAddLayer);
    menu_layers->addAction(actionDeleteLayer);
    menu_layers->addAction(actionShowLayerDialog);

    QMenu *menu_curves = new QMenu(this);
    QToolButton *btn_curves = new QToolButton(this);
    btn_curves->setMenu(menu_curves);
    btn_curves->setPopupMode(QToolButton::InstantPopup);
    btn_curves->setIcon(QPixmap(":/curves.xpm"));
    btn_curves->setToolTip(tr("Add curves / error bars"));
    graph_tools->addWidget(btn_curves);

    menu_curves->addAction(actionShowCurvesDialog);
    menu_curves->addAction(actionAddErrorBars);
    menu_curves->addAction(actionAddFunctionCurve);

    QMenu *menu_plot_enrichments = new QMenu(this);
    QToolButton *btn_plot_enrichments = new QToolButton(this);
    btn_plot_enrichments->setMenu(menu_plot_enrichments);
    btn_plot_enrichments->setPopupMode(QToolButton::InstantPopup);
    btn_plot_enrichments->setIcon(QPixmap(":/text.xpm"));
    btn_plot_enrichments->setToolTip(tr("Enrichments"));
    graph_tools->addWidget(btn_plot_enrichments);

    actionAddText = new QAction(tr("Add &Text"), this);
    actionAddText->setShortcut(tr("ALT+T"));
    actionAddText->setIcon(QIcon(QPixmap(":/text.xpm")));
    actionAddText->setCheckable(true);
    connect(actionAddText, SIGNAL(triggered()), this, SLOT(addText()));
    menu_plot_enrichments->addAction(actionAddText);

    btnArrow = new QAction(tr("Draw &Arrow"), this);
    btnArrow->setShortcut(tr("CTRL+ALT+A"));
    btnArrow->setActionGroup(dataTools);
    btnArrow->setCheckable(true);
    btnArrow->setIcon(QIcon(QPixmap(":/arrow.xpm")));
    menu_plot_enrichments->addAction(btnArrow);

    btnLine = new QAction(tr("Draw &Line"), this);
    btnLine->setShortcut(tr("CTRL+ALT+L"));
    btnLine->setActionGroup(dataTools);
    btnLine->setCheckable(true);
    btnLine->setIcon(QIcon(QPixmap(":/lPlot.xpm")));
    menu_plot_enrichments->addAction(btnLine);

    menu_plot_enrichments->addAction(actionTimeStamp);
    menu_plot_enrichments->addAction(actionAddImage);
    menu_plot_enrichments->addAction(actionNewLegend);

    graph_tools->addSeparator();

    btnZoomIn = new QAction(tr("&Zoom In"), this);
    btnZoomIn->setShortcut(tr("Ctrl++"));
    btnZoomIn->setActionGroup(dataTools);
    btnZoomIn->setCheckable(true);
    btnZoomIn->setIcon(QIcon(QPixmap(":/zoom.xpm")));
    graph_tools->addAction(btnZoomIn);

    btnZoomOut = new QAction(tr("&Zoom Out"), this);
    btnZoomOut->setShortcut(tr("Ctrl+-"));
    btnZoomOut->setActionGroup(dataTools);
    btnZoomOut->setCheckable(true);
    btnZoomOut->setIcon(QIcon(QPixmap(":/zoomOut.xpm")));
    graph_tools->addAction(btnZoomOut);

    graph_tools->addAction(actionUnzoom);

    graph_tools->addSeparator();

    btnPicker = new QAction(tr("S&creen Reader"), this);
    btnPicker->setActionGroup(dataTools);
    btnPicker->setCheckable(true);
    btnPicker->setIcon(QIcon(QPixmap(":/cursor_16.xpm")));
    graph_tools->addAction(btnPicker);

    btnCursor = new QAction(tr("&Data Reader"), this);
    btnCursor->setShortcut(tr("CTRL+D"));
    btnCursor->setActionGroup(dataTools);
    btnCursor->setCheckable(true);
    btnCursor->setIcon(QIcon(QPixmap(":/select.xpm")));
    graph_tools->addAction(btnCursor);

    btnSelect = new QAction(tr("&Select Data Range"), this);
    btnSelect->setShortcut(tr("ALT+S"));
    btnSelect->setActionGroup(dataTools);
    btnSelect->setCheckable(true);
    btnSelect->setIcon(QIcon(QPixmap(":/cursors.xpm")));
    graph_tools->addAction(btnSelect);

    btnMovePoints = new QAction(tr("&Move Data Points..."), this);
    btnMovePoints->setShortcut(tr("Ctrl+ALT+M"));
    btnMovePoints->setActionGroup(dataTools);
    btnMovePoints->setCheckable(true);
    btnMovePoints->setIcon(QIcon(QPixmap(":/hand.xpm")));

    btnRemovePoints = new QAction(tr("Remove &Bad Data Points..."), this);
    btnRemovePoints->setShortcut(tr("Alt+B"));
    btnRemovePoints->setActionGroup(dataTools);
    btnRemovePoints->setCheckable(true);
    btnRemovePoints->setIcon(QIcon(QPixmap(":/gomme.xpm")));

    connect(dataTools, SIGNAL(triggered(QAction *)), this, SLOT(pickDataTool(QAction *)));

    plot_tools = new QToolBar(tr("Plot"), this);
    plot_tools->setObjectName("plot_tools"); // this is needed for QMainWindow::restoreState()
    plot_tools->setIconSize(QSize(22, 22));
    addToolBar(Qt::TopToolBarArea, plot_tools);

    QMenu *menu_plot_linespoints = new QMenu(this);
    QToolButton *btn_plot_linespoints = new QToolButton(this);
    btn_plot_linespoints->setMenu(menu_plot_linespoints);
    btn_plot_linespoints->setPopupMode(QToolButton::InstantPopup);
    btn_plot_linespoints->setIcon(QPixmap(":/lpPlot.xpm"));
    btn_plot_linespoints->setToolTip(tr("Lines and/or symbols"));
    plot_tools->addWidget(btn_plot_linespoints);
    menu_plot_linespoints->addAction(actionPlotL);
    menu_plot_linespoints->addAction(actionPlotP);
    menu_plot_linespoints->addAction(actionPlotLP);
    menu_plot_linespoints->addAction(actionPlotSpline);
    menu_plot_linespoints->addAction(actionPlotVerticalDropLines);
    menu_plot_linespoints->addAction(actionPlotHorSteps);
    menu_plot_linespoints->addAction(actionPlotVertSteps);

    QMenu *menu_plot_bars = new QMenu(this);
    QToolButton *btn_plot_bars = new QToolButton(this);
    btn_plot_bars->setMenu(menu_plot_bars);
    btn_plot_bars->setPopupMode(QToolButton::InstantPopup);
    btn_plot_bars->setIcon(QPixmap(":/vertBars.xpm"));
    plot_tools->addWidget(btn_plot_bars);
    menu_plot_bars->addAction(actionPlotVerticalBars);
    menu_plot_bars->addAction(actionPlotHorizontalBars);

    plot_tools->addAction(actionPlotArea);
    plot_tools->addAction(actionPlotPie);
    plot_tools->addAction(actionPlotHistogram);
    plot_tools->addAction(actionBoxPlot);

    QMenu *menu_plot_vect = new QMenu(this);
    QToolButton *btn_plot_vect = new QToolButton(this);
    btn_plot_vect->setMenu(menu_plot_vect);
    btn_plot_vect->setPopupMode(QToolButton::InstantPopup);
    btn_plot_vect->setIcon(QPixmap(":/vectXYXY.xpm"));
    plot_tools->addWidget(btn_plot_vect);
    menu_plot_vect->addAction(actionPlotVectXYXY);
    menu_plot_vect->addAction(actionPlotVectXYAM);

    plot_tools->addSeparator();

    plot_tools->addAction(actionPlot3DRibbon);
    plot_tools->addAction(actionPlot3DBars);
    plot_tools->addAction(actionPlot3DScatter);
    plot_tools->addAction(actionPlot3DTrajectory);

    table_tools = new QToolBar(tr("Table"), this);
    table_tools->setObjectName("table_tools"); // this is needed for QMainWindow::restoreState()
    table_tools->setIconSize(QSize(22, 22));
    addToolBar(Qt::TopToolBarArea, table_tools);

    graph_tools->setEnabled(false);
    table_tools->setEnabled(false);
    plot_tools->setEnabled(false);

    d_status_info = new QLabel(this);
    d_status_info->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    d_status_info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d_status_info->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d_status_info, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(showStatusBarContextMenu(const QPoint &)));

    statusBar()->addWidget(d_status_info, 1);

    matrix_plot_tools = new QToolBar(tr("Matrix Plot"), this);
    matrix_plot_tools->setObjectName("matrix_plot_tools");
    addToolBar(Qt::BottomToolBarArea, matrix_plot_tools);

    matrix_plot_tools->addAction(actionPlot3DWireFrame);
    matrix_plot_tools->addAction(actionPlot3DHiddenLine);

    matrix_plot_tools->addAction(actionPlot3DPolygons);
    matrix_plot_tools->addAction(actionPlot3DWireSurface);

    matrix_plot_tools->addSeparator();

    matrix_plot_tools->addAction(actionPlot3DBars);
    matrix_plot_tools->addAction(actionPlot3DScatter);

    matrix_plot_tools->addSeparator();
    matrix_plot_tools->addAction(actionColorMap);
    matrix_plot_tools->addAction(actionContourMap);
    matrix_plot_tools->addAction(actionGrayMap);

    matrix_plot_tools->setEnabled(false);
}

void ApplicationWindow::lockToolbar(const bool status)
{
    if (status) {
        file_tools->setMovable(false);
        edit_tools->setMovable(false);
        graph_tools->setMovable(false);
        graph_3D_tools->setMovable(false);
        plot_tools->setMovable(false);
        table_tools->setMovable(false);
        matrix_plot_tools->setMovable(false);
        locktoolbar->setIcon(QIcon(QPixmap(":/lock.xpm")));
    } else {
        file_tools->setMovable(true);
        edit_tools->setMovable(true);
        graph_tools->setMovable(true);
        graph_3D_tools->setMovable(true);
        plot_tools->setMovable(true);
        table_tools->setMovable(true);
        matrix_plot_tools->setMovable(true);
        locktoolbar->setIcon(QIcon(QPixmap(":/unlock.xpm")));
    }
}

void ApplicationWindow::insertTranslatedStrings()
{
    if (projectname == "untitled")
        setWindowTitle(tr("Makhber - untitled"));

    lv.headerItem()->setText(0, tr("Name"));
    lv.headerItem()->setText(1, tr("Type"));
    lv.headerItem()->setText(2, tr("View"));
    lv.headerItem()->setText(3, tr("Created"));
    lv.headerItem()->setText(4, tr("Label"));

    explorerWindow.setWindowTitle(tr("Project Explorer"));
    logWindow.setWindowTitle(tr("Results Log"));
#ifdef SCRIPTING_CONSOLE
    consoleWindow.setWindowTitle(tr("Scripting Console"));
#endif
    table_tools->setWindowTitle(tr("Table"));
    plot_tools->setWindowTitle(tr("Plot"));
    graph_tools->setWindowTitle(tr("Graph"));
    file_tools->setWindowTitle(tr("File"));
    edit_tools->setWindowTitle(tr("Edit"));
    matrix_plot_tools->setWindowTitle(tr("Matrix Plot"));
    graph_3D_tools->setWindowTitle(tr("3D Surface"));

    file->setTitle(tr("&File"));
    edit->setTitle(tr("&Edit"));
    view->setTitle(tr("&View"));
    scriptingMenu->setTitle(tr("Scripting"));
    graph->setTitle(tr("&Graph"));
    plot3DMenu->setTitle(tr("3D &Plot"));
    matrixMenu->setTitle(tr("&Matrix"));
    tableMenu->setTitle(tr("&Table"));
    plot2D->setTitle(tr("&Plot"));
    dataMenu->setTitle(tr("&Analysis"));
    plotDataMenu->setTitle(tr("&Tools"));
    calcul->setTitle(tr("&Analysis"));
    d_quick_fit_menu->setTitle(tr("&Quick Fit"));
    format->setTitle(tr("For&mat"));
    windowsMenu->setTitle(tr("&Windows"));
    help->setTitle(tr("&Help"));

    type->setTitle(tr("&New"));
    recent->setTitle(tr("&Recent Projects"));
    exportPlot->setTitle(tr("&Export Graph"));

    specialPlot->setTitle(tr("Special Line/Symb&ol"));
    stat->setTitle(tr("Statistical &Graphs"));
    panels->setTitle(tr("Pa&nel"));
    plot3D->setTitle(tr("3&D Plot"));

    translateMenu->setTitle(tr("&Translate"));
    smooth->setTitle(tr("&Smooth"));
    filter->setTitle(tr("&FFT Filter"));
    decay->setTitle(tr("Fit E&xponential Decay"));
    multiPeakMenu->setTitle(tr("Fit &Multi-Peak"));

    translateActionsStrings();
    for (auto w : windowsList())
        customMenu(w);
}

void ApplicationWindow::initMainMenu()
{
    file = new QMenu(this);
    file->setTitle(tr("&File"));
    file->setFont(appFont);

    type = file->addMenu(tr("&New"));
    type->setFont(appFont);
    type->addAction(actionNewProject);
    type->addAction(actionNewTable);
    type->addAction(actionNewMatrix);
    type->addAction(actionNewNote);
    type->addAction(actionNewGraph);
    type->addAction(actionNewFunctionPlot);
    type->addAction(actionNewSurfacePlot);

    file->addAction(actionOpen);

    recent = file->addMenu(tr("&Recent Projects"));
    recent->setFont(appFont);

    file->addSeparator();

    file->addAction(actionLoadImage);
    file->addAction(actionImportImage);

    file->addSeparator();

    file->addAction(actionSaveProject);
    file->addAction(actionSaveProjectAs);

    file->addSeparator();
    file->addAction(actionOpenTemplate);
    file->addAction(actionSaveTemplate);
    file->addSeparator();

    exportPlot = file->addMenu(tr("&Export Graph"));
    exportPlot->addAction(actionExportGraph);
    exportPlot->addAction(actionExportAllGraphs);

    file->addAction(actionPrint);
    file->addAction(actionPrintAllPlots);

    file->addSeparator();

    file->addAction(actionShowExportASCIIDialog);
    file->addAction(actionLoad);

    file->addSeparator();

    file->addAction(actionCloseAllWindows);

    edit = new QMenu(this);
    edit->setFont(appFont);
    edit->setTitle(tr("&Edit"));
    edit->addAction(actionUndo);
    edit->addAction(actionRedo);

    edit->addSeparator();

    edit->addAction(actionCutSelection);
    edit->addAction(actionCopySelection);
    edit->addAction(actionPasteSelection);
    edit->addAction(actionClearSelection);

    edit->addSeparator();

    edit->addAction(actionDeleteFitTables);
    edit->addAction(actionClearLogInfo);

    edit->addSeparator();

    edit->addAction(actionShowConfigureDialog);

    view = new QMenu(this);
    view->setFont(appFont);
    view->setTitle(tr("&View"));
    toolbarsMenu = createToolbarsMenu();
    if (!toolbarsMenu)
        toolbarsMenu = new QMenu(this);
    toolbarsMenu->setTitle(tr("Toolbars"));

    view->addMenu(toolbarsMenu);
    view->addAction(locktoolbar);
    view->addSeparator();
    view->addAction(actionShowPlotWizard);
    view->addAction(actionShowExplorer);
    view->addAction(actionShowLog);
    view->addAction(actionShowHistory);
#ifdef SCRIPTING_CONSOLE
    view->addAction(actionShowConsole);
#endif

    graph = new QMenu(this);
    graph->setFont(appFont);
    graph->setTitle(tr("&Graph"));
    graph->addAction(actionShowCurvesDialog);
    graph->addAction(actionAddErrorBars);
    graph->addAction(actionAddFunctionCurve);

    graph->addSeparator();

    graph->addAction(actionAddText);
    graph->addAction(btnArrow);
    graph->addAction(btnLine);
    graph->addAction(actionTimeStamp);
    graph->addAction(actionAddImage);
    graph->addAction(actionNewLegend);

    graph->addSeparator(); // layers section
    graph->addAction(actionAutomaticLayout);
    graph->addAction(actionAddLayer);
    graph->addAction(actionDeleteLayer);
    graph->addAction(actionShowLayerDialog);

    plot3DMenu = new QMenu(this);
    plot3DMenu->setFont(appFont);
    plot3DMenu->setTitle(tr("3D &Plot"));

    plot3DMenu->addAction(actionPlot3DWireFrame);
    plot3DMenu->addAction(actionPlot3DHiddenLine);

    plot3DMenu->addAction(actionPlot3DPolygons);
    plot3DMenu->addAction(actionPlot3DWireSurface);

    plot3DMenu->addSeparator();

    plot3DMenu->addAction(actionPlot3DBars);
    plot3DMenu->addAction(actionPlot3DScatter);

    plot3DMenu->addSeparator();
    plot3DMenu->addAction(actionColorMap);
    plot3DMenu->addAction(actionContourMap);
    plot3DMenu->addAction(actionGrayMap);

    matrixMenu = new QMenu(this);
    matrixMenu->setFont(appFont);
    matrixMenu->setTitle(tr("&Matrix"));

    tableMenu = new QMenu(this);
    tableMenu->setFont(appFont);
    tableMenu->setTitle(tr("&Table"));

    initPlotMenu();
    initTableAnalysisMenu();
    initPlotDataMenu();

    calcul = new QMenu(this);
    calcul->setFont(appFont);
    calcul->setTitle(tr("&Analysis"));

    translateMenu = calcul->addMenu(tr("&Translate"));
    translateMenu->setFont(appFont);
    translateMenu->addAction(actionTranslateVert);
    translateMenu->addAction(actionTranslateHor);
    calcul->addSeparator();

    calcul->addAction(actionDifferentiate);
    calcul->addAction(actionShowIntDialog);

    calcul->addSeparator();

    smooth = calcul->addMenu(tr("&Smooth"));
    smooth->setFont(appFont);
    smooth->addAction(actionSmoothSavGol);
    smooth->addAction(actionSmoothAverage);
    smooth->addAction(actionSmoothFFT);

    filter = calcul->addMenu(tr("&FFT Filter"));
    filter->setFont(appFont);
    filter->addAction(actionLowPassFilter);
    filter->addAction(actionHighPassFilter);
    filter->addAction(actionBandPassFilter);
    filter->addAction(actionBandBlockFilter);

    calcul->addSeparator();
    calcul->addAction(actionInterpolate);
    calcul->addAction(actionFFT);
    calcul->addSeparator();

    d_quick_fit_menu = new QMenu(this);
    d_quick_fit_menu->setTitle(tr("&Quick Fit"));

    d_quick_fit_menu->addAction(actionFitLinear);
    d_quick_fit_menu->addAction(actionShowFitPolynomDialog);

    d_quick_fit_menu->addSeparator();

    decay = d_quick_fit_menu->addMenu(tr("Fit E&xponential Decay"));
    decay->setFont(appFont);
    decay->addAction(actionShowExpDecayDialog);
    decay->addAction(actionShowTwoExpDecayDialog);
    decay->addAction(actionShowExpDecay3Dialog);

    d_quick_fit_menu->addAction(actionFitExpGrowth);
    d_quick_fit_menu->addAction(actionFitSigmoidal);
    d_quick_fit_menu->addAction(actionFitGauss);
    d_quick_fit_menu->addAction(actionFitLorentz);

    multiPeakMenu = d_quick_fit_menu->addMenu(tr("Fit &Multi-peak"));
    multiPeakMenu->setFont(appFont);
    multiPeakMenu->addAction(actionMultiPeakGauss);
    multiPeakMenu->addAction(actionMultiPeakLorentz);

    d_quick_fit_menu->addSeparator();

    calcul->addMenu(d_quick_fit_menu);
    calcul->addAction(actionShowFitDialog);

    format = new QMenu(this);
    format->setFont(appFont);
    format->setTitle(tr("For&mat"));

    scriptingMenu = new QMenu(this);
    scriptingMenu->setFont(appFont);
    scriptingMenu->setTitle(tr("Scripting"));

    windowsMenu = new QMenu(this);
    windowsMenu->setFont(appFont);
    windowsMenu->setTitle(tr("&Windows"));
    connect(windowsMenu, SIGNAL(aboutToShow()), this, SLOT(windowsMenuAboutToShow()));

    help = new QMenu(this);
    help->setFont(appFont);
    help->setTitle(tr("&Help"));

    help->addAction(actionShowHelp);
#ifdef DYNAMIC_MANUAL_PATH
    help->addAction(actionChooseHelpFolder);
#endif
    help->addSeparator();
    help->addAction(actionHomePage);
#ifdef SEARCH_FOR_UPDATES
    help->addAction(actionCheckUpdates);
#endif
#ifdef DOWNLOAD_LINKS
    help->addAction(actionDownloadManual);
#endif
    help->addSeparator();
    help->addAction(actionHelpForums);
    help->addAction(actionHelpBugReports);
    help->addSeparator();
    help->addAction(actionAbout);

    disableActions();
}

void ApplicationWindow::initPlotDataMenu()
{
    plotDataMenu = new QMenu(this);
    plotDataMenu->setFont(appFont);
    plotDataMenu->setTitle(tr("&Tools"));

    plotDataMenu->addAction(btnPointer);
    plotDataMenu->addAction(btnZoomIn);
    plotDataMenu->addAction(btnZoomOut);
    plotDataMenu->addAction(actionUnzoom);
    plotDataMenu->addSeparator();

    plotDataMenu->addAction(btnPicker);
    plotDataMenu->addAction(btnCursor);
    plotDataMenu->addAction(btnSelect);

    plotDataMenu->addSeparator();

    plotDataMenu->addAction(btnMovePoints);
    plotDataMenu->addAction(btnRemovePoints);
}

void ApplicationWindow::initPlotMenu()
{
    plot2D = new QMenu(this);
    plot2D->setFont(appFont);
    plot2D->setTitle(tr("&Plot"));

    plot2D->addAction(actionPlotL);
    plot2D->addAction(actionPlotP);
    plot2D->addAction(actionPlotLP);

    specialPlot = plot2D->addMenu(tr("Special Line/Symb&ol"));
    specialPlot->setFont(appFont);
    specialPlot->addAction(actionPlotVerticalDropLines);
    specialPlot->addAction(actionPlotSpline);
    specialPlot->addAction(actionPlotVertSteps);
    specialPlot->addAction(actionPlotHorSteps);

    plot2D->addSeparator();

    plot2D->addAction(actionPlotVerticalBars);
    plot2D->addAction(actionPlotHorizontalBars);
    plot2D->addAction(actionPlotArea);
    plot2D->addAction(actionPlotPie);
    plot2D->addAction(actionPlotVectXYXY);
    plot2D->addAction(actionPlotVectXYAM);

    plot2D->addSeparator();

    stat = plot2D->addMenu(tr("Statistical &Graphs"));
    stat->setFont(appFont);
    stat->addAction(actionBoxPlot);
    stat->addAction(actionPlotHistogram);
    stat->addAction(actionPlotStackedHistograms);

    panels = plot2D->addMenu(tr("Pa&nel"));
    panels->setFont(appFont);
    panels->addAction(actionPlot2VerticalLayers);
    panels->addAction(actionPlot2HorizontalLayers);
    panels->addAction(actionPlot4Layers);
    panels->addAction(actionPlotStackedLayers);

    plot2D->addSeparator();

    plot3D = plot2D->addMenu(tr("3&D Plot"));
    plot3D->setFont(appFont);
    plot3D->addAction(actionPlot3DRibbon);
    plot3D->addAction(actionPlot3DBars);
    plot3D->addAction(actionPlot3DScatter);
    plot3D->addAction(actionPlot3DTrajectory);
}

void ApplicationWindow::initTableAnalysisMenu()
{
    dataMenu = new QMenu(this);
    dataMenu->setFont(appFont);
    dataMenu->setTitle(tr("&Analysis"));

    dataMenu->addAction(actionShowColStatistics);
    dataMenu->addAction(actionShowRowStatistics);

    dataMenu->addSeparator();
    dataMenu->addAction(actionFFT);
    dataMenu->addSeparator();
    dataMenu->addAction(actionCorrelate);
    dataMenu->addAction(actionAutoCorrelate);
    dataMenu->addSeparator();
    dataMenu->addAction(actionConvolute);
    dataMenu->addAction(actionDeconvolute);

    dataMenu->addSeparator();
    dataMenu->addAction(actionShowFitDialog);
}

void ApplicationWindow::customMenu(MyWidget *w)
{
    menuBar()->clear();
    menuBar()->addMenu(file);
    menuBar()->addMenu(edit);
    menuBar()->addMenu(view);
    menuBar()->addMenu(scriptingMenu);

    scriptingMenu->clear();
#ifdef SCRIPTING_DIALOG
    scriptingMenu->addAction(actionScriptingLang);
#endif
    scriptingMenu->addAction(actionRestartScripting);

    // these use the same keyboard shortcut (Ctrl+Return) and should not be enabled at the same time
    actionNoteEvaluate->setEnabled(false);

    if (w) {
        actionPrintAllPlots->setEnabled(projectHas2DPlots());
        actionPrint->setEnabled(true);
        actionCutSelection->setEnabled(true);
        actionCopySelection->setEnabled(true);
        actionPasteSelection->setEnabled(true);
        actionClearSelection->setEnabled(true);
        actionSaveTemplate->setEnabled(true);

        if (w->inherits("MultiLayer")) {
            menuBar()->addMenu(graph);
            menuBar()->addMenu(plotDataMenu);
            menuBar()->addMenu(calcul);
            menuBar()->addMenu(format);

            exportPlot->setEnabled(true);
            actionShowExportASCIIDialog->setEnabled(false);
            // file->setItemEnabled (closeID,true);

            format->clear();
            format->addAction(actionShowPlotDialog);
            format->addSeparator();
            format->addAction(actionShowScaleDialog);
            format->addAction(actionShowAxisDialog);
            actionShowAxisDialog->setEnabled(true);
            format->addSeparator();
            format->addAction(actionShowGridDialog);
            format->addAction(actionShowTitleDialog);
        } else if (w->inherits("Graph3D")) {
            disableActions();

            menuBar()->addMenu(format);

            actionPrint->setEnabled(true);
            actionSaveTemplate->setEnabled(true);
            exportPlot->setEnabled(true);
            // file->setItemEnabled (closeID,true);

            format->clear();
            format->addAction(actionShowPlotDialog);
            format->addAction(actionShowScaleDialog);
            format->addAction(actionShowAxisDialog);
            format->addAction(actionShowTitleDialog);
            if (((Graph3D *)w)->coordStyle() == Qwt3D::NOCOORD)
                actionShowAxisDialog->setEnabled(false);
        } else if (w->inherits("Table")) {
            menuBar()->addMenu(plot2D);
            menuBar()->addMenu(dataMenu);

            actionShowExportASCIIDialog->setEnabled(true);
            exportPlot->setEnabled(false);
            // file->setItemEnabled (closeID,true);

            tableMenu->clear();
            static_cast<Table *>(w)->d_future_table->fillProjectMenu(tableMenu);
            tableMenu->addSeparator();
            tableMenu->addAction(actionShowExportASCIIDialog);
            tableMenu->addSeparator();
            tableMenu->addAction(actionConvertTable);
            menuBar()->addMenu(tableMenu);
        } else if (w->inherits("Matrix")) {
            menuBar()->addMenu(plot3DMenu);

            matrixMenu->clear();
            static_cast<Matrix *>(w)->d_future_matrix->fillProjectMenu(matrixMenu);
            matrixMenu->addSeparator();
            matrixMenu->addAction(actionInvertMatrix);
            matrixMenu->addAction(actionMatrixDeterminant);
            matrixMenu->addSeparator();
            matrixMenu->addAction(actionConvertMatrix);
            menuBar()->addMenu(matrixMenu);
        } else if (w->inherits("Note")) {
            actionSaveTemplate->setEnabled(false);
            actionNoteEvaluate->setEnabled(true);
            scriptingMenu->addSeparator();
            scriptingMenu->addAction(actionNoteExecute);
            scriptingMenu->addAction(actionNoteExecuteAll);
            scriptingMenu->addAction(actionNoteEvaluate);

            actionNoteExecute->disconnect(SIGNAL(triggered()));
            actionNoteExecuteAll->disconnect(SIGNAL(triggered()));
            actionNoteEvaluate->disconnect(SIGNAL(triggered()));
            connect(actionNoteExecute, SIGNAL(triggered()), w, SLOT(execute()));
            connect(actionNoteExecuteAll, SIGNAL(triggered()), w, SLOT(executeAll()));
            connect(actionNoteEvaluate, SIGNAL(triggered()), w, SLOT(evaluate()));
        } else
            disableActions();

        menuBar()->addMenu(windowsMenu);
    } else
        disableActions();

    menuBar()->addMenu(help);
}

void ApplicationWindow::disableActions()
{
    actionSaveTemplate->setEnabled(false);
    actionPrintAllPlots->setEnabled(false);
    actionPrint->setEnabled(false);
    actionShowExportASCIIDialog->setEnabled(false);
    exportPlot->setEnabled(false);
    // file->setItemEnabled (closeID,false);

    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);

    actionCutSelection->setEnabled(false);
    actionCopySelection->setEnabled(false);
    actionPasteSelection->setEnabled(false);
    actionClearSelection->setEnabled(false);
}

void ApplicationWindow::customToolBars(MyWidget *w)
{
    if (w) {
        if (!projectHas3DPlots())
            graph_3D_tools->setEnabled(false);
        if (!projectHas2DPlots())
            graph_tools->setEnabled(false);
        if (!projectHasMatrices())
            matrix_plot_tools->setEnabled(false);
        if (tableWindows().count() <= 0) {
            table_tools->setEnabled(false);
            plot_tools->setEnabled(false);
        }

        if (w->inherits("MultiLayer")) {
            graph_tools->setEnabled(true);
            graph_3D_tools->setEnabled(false);
            table_tools->setEnabled(false);
            matrix_plot_tools->setEnabled(false);

            Graph *g = static_cast<MultiLayer *>(w)->activeGraph();
            if (g) {
                dataTools->blockSignals(true);
                if (g->rangeSelectorsEnabled())
                    btnSelect->setChecked(true);
                else if (g->zoomOn())
                    btnZoomIn->setChecked(true);
                else if (g->drawArrow())
                    btnArrow->setChecked(true);
                else if (g->drawLineActive())
                    btnLine->setChecked(true);
                else if (g->activeTool() == 0)
                    btnPointer->setChecked(true);
                else
                    switch (g->activeTool()->rtti()) {
                    case PlotToolInterface::DataPicker:
                        switch (static_cast<DataPickerTool *>(g->activeTool())->mode()) {
                        case DataPickerTool::Display:
                            btnCursor->setChecked(true);
                            break;
                        case DataPickerTool::Move:
                            btnMovePoints->setChecked(true);
                            break;
                        case DataPickerTool::Remove:
                            btnRemovePoints->setChecked(true);
                            break;
                        }
                        break;
                    case PlotToolInterface::ScreenPicker:
                        btnPicker->setChecked(true);
                        break;
                    default:
                        btnPointer->setChecked(true);
                        break;
                    }
                dataTools->blockSignals(false);
            }
            if (g && g->curves() > 0) {
                plot_tools->setEnabled(true);
                QwtPlotCurve *c = g->curve(g->curves() - 1);
                // plot tools managed by d_plot_mapper
                for (int i = 0; i <= (int)Graph::VerticalSteps; i++) {
                    QAction *a = static_cast<QAction *>(d_plot_mapper->mapping(i));
                    if (a)
                        a->setEnabled(Graph::canConvertTo(c, (Graph::CurveType)i));
                }
                // others
                actionPlotPie->setEnabled(Graph::canConvertTo(c, Graph::Pie));
                actionPlotVectXYAM->setEnabled(Graph::canConvertTo(c, Graph::VectXYAM));
                actionPlotVectXYXY->setEnabled(Graph::canConvertTo(c, Graph::VectXYXY));
                actionBoxPlot->setEnabled(Graph::canConvertTo(c, Graph::Box));
                // 3D plots
                actionPlot3DRibbon->setEnabled(false);
                actionPlot3DScatter->setEnabled(false);
                actionPlot3DTrajectory->setEnabled(false);
                actionPlot3DBars->setEnabled(false);
            } else
                plot_tools->setEnabled(false);
        } else if (w->inherits("Table")) {
            table_tools->clear();
            static_cast<Table *>(w)->d_future_table->fillProjectToolBar(table_tools);
            table_tools->setEnabled(true);

            graph_tools->setEnabled(false);
            graph_3D_tools->setEnabled(false);
            matrix_plot_tools->setEnabled(false);

            plot_tools->setEnabled(true);
            // plot tools managed by d_plot_mapper
            for (int i = 0; i <= (int)Graph::VerticalSteps; i++) {
                QAction *a = static_cast<QAction *>(d_plot_mapper->mapping(i));
                if (a)
                    a->setEnabled(true);
            }
            // others
            actionPlotPie->setEnabled(true);
            actionPlotVectXYAM->setEnabled(true);
            actionPlotVectXYXY->setEnabled(true);
            actionBoxPlot->setEnabled(true);
            // 3D plots
            actionPlot3DRibbon->setEnabled(true);
            actionPlot3DScatter->setEnabled(true);
            actionPlot3DTrajectory->setEnabled(true);
            actionPlot3DBars->setEnabled(true);
        } else if (w->inherits("Matrix")) {
            graph_tools->setEnabled(false);
            graph_3D_tools->setEnabled(false);
            table_tools->setEnabled(false);
            plot_tools->setEnabled(false);
            matrix_plot_tools->setEnabled(true);
        } else if (w->inherits("Graph3D")) {
            graph_tools->setEnabled(false);
            table_tools->setEnabled(false);
            plot_tools->setEnabled(false);
            matrix_plot_tools->setEnabled(false);

            Graph3D *plot = (Graph3D *)w;
            if (plot->plotStyle() == Qwt3D::NOPLOT)
                graph_3D_tools->setEnabled(false);
            else
                graph_3D_tools->setEnabled(true);

            custom3DActions(w);
        } else if (w->inherits("Note")) {
            graph_tools->setEnabled(false);
            graph_3D_tools->setEnabled(false);
            table_tools->setEnabled(false);
            plot_tools->setEnabled(false);
            matrix_plot_tools->setEnabled(false);
        }

    } else {
        graph_tools->setEnabled(false);
        table_tools->setEnabled(false);
        plot_tools->setEnabled(false);
        graph_3D_tools->setEnabled(false);
        matrix_plot_tools->setEnabled(false);
    }
}

void ApplicationWindow::plot3DRibbon()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *table = static_cast<Table *>(d_workspace.activeSubWindow());
    if (table->selectedColumns().count() == 1) {
        if (!validFor3DPlot(table))
            return;
        dataPlot3D(table, table->colName(table->firstSelectedColumn()));
    } else
        QMessageBox::warning(this, tr("Plot error"),
                             tr("You must select exactly one column for plotting!"));
}

void ApplicationWindow::plot3DWireframe()
{
    plot3DMatrix(Qwt3D::WIREFRAME);
}

void ApplicationWindow::plot3DHiddenLine()
{
    plot3DMatrix(Qwt3D::HIDDENLINE);
}

void ApplicationWindow::plot3DPolygons()
{
    plot3DMatrix(Qwt3D::FILLED);
}

void ApplicationWindow::plot3DWireSurface()
{
    plot3DMatrix(Qwt3D::FILLEDMESH);
}

void ApplicationWindow::plot3DBars()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("Table")) {
        Table *table = static_cast<Table *>(w);

        if (table->selectedColumns().count() == 1) {
            if (!validFor3DPlot(table))
                return;
            dataPlotXYZ(table, table->colName(table->firstSelectedColumn()), Graph3D::Bars);
        } else
            QMessageBox::warning(this, tr("Plot error"),
                                 tr("You must select exactly one column for plotting!"));
    } else if (w->inherits("Matrix"))
        plot3DMatrix(Qwt3D::USER);
}

void ApplicationWindow::plot3DScatter()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("Table")) {
        Table *table = static_cast<Table *>(w);

        if (table->selectedColumns().count() == 1) {
            if (!validFor3DPlot(table))
                return;
            dataPlotXYZ(table, table->colName(table->firstSelectedColumn()), Graph3D::Scatter);
        } else
            QMessageBox::warning(this, tr("Plot error"),
                                 tr("You must select exactly one column for plotting!"));
    } else if (w->inherits("Matrix"))
        plot3DMatrix(Qwt3D::POINTS);
}

void ApplicationWindow::plot3DTrajectory()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("Table")) {
        Table *table = static_cast<Table *>(w);

        if (table->selectedColumns().count() == 1) {
            if (!validFor3DPlot(table))
                return;
            dataPlotXYZ(table, table->colName(table->firstSelectedColumn()), Graph3D::Trajectory);
        } else
            QMessageBox::warning(this, tr("Plot error"),
                                 tr("You must select exactly one column for plotting!"));
    }
}

void ApplicationWindow::plotPie()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *table = static_cast<Table *>(d_workspace.activeSubWindow());

    if (table->selectedColumns().count() != 1) {
        QMessageBox::warning(this, tr("Plot error"),
                             tr("You must select exactly one column for plotting!"));
        return;
    }
    if (table->noXColumn()) {
        QMessageBox::critical(0, tr("Error"),
                              tr("Please set a default X column for this table, first!"));
        return;
    }

    QStringList s = table->selectedColumns();
    if (s.count() > 0) {
        multilayerPlot(table, s, Graph::Pie, table->firstSelectedRow(), table->lastSelectedRow());
    } else
        QMessageBox::warning(this, tr("Error"), tr("Please select a column to plot!"));
}

void ApplicationWindow::plotVectXYXY()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *table = static_cast<Table *>(d_workspace.activeSubWindow());

    if (!validFor2DPlot(table, Graph::VectXYXY))
        return;

    QStringList s = table->selectedColumns();
    if (s.count() == 4) {
        multilayerPlot(table, s, Graph::VectXYXY, table->firstSelectedRow(),
                       table->lastSelectedRow());
    } else
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select four columns for this operation!"));
}

void ApplicationWindow::plotVectXYAM()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *table = static_cast<Table *>(d_workspace.activeSubWindow());

    if (!validFor2DPlot(table, Graph::VectXYAM))
        return;

    QStringList s = table->selectedColumns();
    if (s.count() == 4) {
        multilayerPlot(table, s, Graph::VectXYAM, table->firstSelectedRow(),
                       table->lastSelectedRow());
    } else
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select four columns for this operation!"));
}

void ApplicationWindow::renameListViewItem(const QString &oldName, const QString &newName)
{
    QTreeWidgetItem *it = lv.findItems(oldName, Qt::MatchExactly | Qt::MatchCaseSensitive).value(0);
    if (it)
        it->setText(0, newName);
}

void ApplicationWindow::setListViewLabel(const QString &caption, const QString &label)
{
    QTreeWidgetItem *it = lv.findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive).value(0);
    if (it)
        it->setText(4, label);
}

void ApplicationWindow::setListViewDate(const QString &caption, const QString &date)
{
    QTreeWidgetItem *it = lv.findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive).value(0);
    if (it)
        it->setText(3, date);
}

void ApplicationWindow::setListView(const QString &caption, const QString &view)
{
    QTreeWidgetItem *it = lv.findItems(caption, Qt::MatchExactly | Qt::MatchCaseSensitive).value(0);
    if (it)
        it->setText(2, view);
}

void ApplicationWindow::updateTableNames(const QString &oldName, const QString &newName)
{
    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList gr_lst = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, gr_lst)
                ((Graph *)widget)->updateCurveNames(oldName, newName);
        } else if (w->inherits("Graph3D")) {
            QString name = ((Graph3D *)w)->formula();
            if (name.contains(oldName, Qt::CaseSensitive)) {
                name.replace(oldName, newName);
                ((Graph3D *)w)->setPlotAssociation(name);
            }
        }
    }
}

void ApplicationWindow::updateColNames(const QString &oldName, const QString &newName)
{
    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList gr_lst = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, gr_lst)
                ((Graph *)widget)->updateCurveNames(oldName, newName, false);
        } else if (w->inherits("Graph3D")) {
            QString name = ((Graph3D *)w)->formula();
            if (name.contains(oldName)) {
                name.replace(oldName, newName);
                ((Graph3D *)w)->setPlotAssociation(name);
            }
        }
    }
}

void ApplicationWindow::changeMatrixName(const QString &oldName, const QString &newName)
{
    QList<MyWidget *> lst = windowsList();
    foreach (MyWidget *w, lst) {
        if (w->inherits("Graph3D")) {
            QString s = ((Graph3D *)w)->formula();
            if (s.contains(oldName)) {
                s.replace(oldName, newName);
                ((Graph3D *)w)->setPlotAssociation(s);
            }
        } else if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *gr_widget, graphsList) {
                Graph *g = (Graph *)gr_widget;
                for (int i = 0; i < g->curves(); i++) {
                    QwtPlotItem *sp = (QwtPlotItem *)g->plotItem(i);
                    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram
                        && sp->title().text() == oldName)
                        sp->setTitle(newName);
                }
            }
        }
    }
}

void ApplicationWindow::remove3DMatrixPlots(Matrix *m)
{
    if (!m)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("Graph3D") && ((Graph3D *)w)->matrix() == m)
            ((Graph3D *)w)->clearData();
        else if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            for (int j = 0; j < (int)graphsList.count(); j++) {
                Graph *g = (Graph *)graphsList.at(j);
                for (int i = 0; i < g->curves(); i++) {
                    Spectrogram *sp = (Spectrogram *)g->plotItem(i);
                    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && sp->matrix() == m)
                        g->removeCurve(i);
                }
            }
        }
    }
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::updateMatrixPlots(MyWidget *window)
{
    Matrix *m = (Matrix *)window;
    if (!m)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("Graph3D") && ((Graph3D *)w)->matrix() == m)
            ((Graph3D *)w)->updateMatrixData(m);
        else if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            for (int j = 0; j < (int)graphsList.count(); j++) {
                Graph *g = (Graph *)graphsList.at(j);
                for (int i = 0; i < g->curves(); i++) {
                    Spectrogram *sp = (Spectrogram *)g->plotItem(i);
                    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && sp->matrix() == m)
                        sp->updateData(m);
                }
            }
        }
    }

    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::add3DData()
{
    if (tableWindows().count() <= 0) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no tables available in this project.</h4>"
                                "<p><h4>Please create a table and try again!</h4>"));
        return;
    }

    // TODO: string list -> Column * list
    QStringList zColumns = columnsList(Makhber::Z);
    if ((int)zColumns.count() <= 0) {
        QMessageBox::critical(this, tr("Warning"),
                              tr("There are no available columns with plot designation set to Z!"));
        return;
    }

    DataSetDialog *ad = new DataSetDialog(tr("Column") + " : ", this);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    connect(ad, SIGNAL(options(const QString &)), this, SLOT(insertNew3DData(const QString &)));
    ad->setWindowTitle(tr("Choose data set"));
    ad->setCurveNames(zColumns);
    ad->exec();
}

void ApplicationWindow::change3DData()
{
    DataSetDialog *ad = new DataSetDialog(tr("Column") + " : ", this);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    connect(ad, SIGNAL(options(const QString &)), this, SLOT(change3DData(const QString &)));

    ad->setWindowTitle(tr("Choose data set"));
    // TODO: string list -> Column * list
    ad->setCurveNames(columnsList(Makhber::Z));
    ad->exec();
}

void ApplicationWindow::change3DMatrix()
{
    DataSetDialog *ad = new DataSetDialog(tr("Matrix") + " : ", this);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    connect(ad, SIGNAL(options(const QString &)), this, SLOT(change3DMatrix(const QString &)));

    ad->setWindowTitle(tr("Choose matrix to plot"));
    ad->setCurveNames(matrixNames());

    Graph3D *g = (Graph3D *)d_workspace.activeSubWindow();
    if (g && g->matrix())
        ad->setCurentDataSet(g->matrix()->name());
    ad->exec();
}

void ApplicationWindow::change3DMatrix(const QString &matrix_name)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        Graph3D *g = (Graph3D *)d_workspace.activeSubWindow();
        Matrix *m = matrix(matrix_name);
        if (m && g)
            g->changeMatrix(m);

        emit modified();
    }
}

void ApplicationWindow::add3DMatrixPlot()
{
    QStringList matrices = matrixNames();
    if ((int)matrices.count() <= 0) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no matrices available in this project.</h4>"
                                "<p><h4>Please create a matrix and try again!</h4>"));
        return;
    }

    DataSetDialog *ad = new DataSetDialog(tr("Matrix") + " :", this);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    connect(ad, SIGNAL(options(const QString &)), this, SLOT(insert3DMatrixPlot(const QString &)));

    ad->setWindowTitle(tr("Choose matrix to plot"));
    ad->setCurveNames(matrices);
    ad->exec();
}

void ApplicationWindow::insert3DMatrixPlot(const QString &matrix_name)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->addMatrixData(matrix(matrix_name));
        emit modified();
    }
}

void ApplicationWindow::insertNew3DData(const QString &colName)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->insertNewData(table(colName), colName);
        emit modified();
    }
}

void ApplicationWindow::change3DData(const QString &colName)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->changeDataColumn(table(colName), colName);
        emit modified();
    }
}

void ApplicationWindow::editSurfacePlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        Graph3D *g = (Graph3D *)d_workspace.activeSubWindow();

        SurfaceDialog *sd = new SurfaceDialog(this);
        sd->setAttribute(Qt::WA_DeleteOnClose);
        connect(sd,
                SIGNAL(options(const QString &, double, double, double, double, double, double)), g,
                SLOT(insertFunction(const QString &, double, double, double, double, double,
                                    double)));
        connect(sd, SIGNAL(clearFunctionsList()), this, SLOT(clearSurfaceFunctionsList()));

        sd->insertFunctionsList(surfaceFunc);
        if (g->hasData()) {
            sd->setFunction(g->formula());
            sd->setLimits(g->xStart(), g->xStop(), g->yStart(), g->yStop(), g->zStart(),
                          g->zStop());
        }
        sd->exec();
    }
}

void ApplicationWindow::newSurfacePlot()
{
    SurfaceDialog *sd = new SurfaceDialog(this);
    sd->setAttribute(Qt::WA_DeleteOnClose);
    connect(sd, SIGNAL(options(const QString &, double, double, double, double, double, double)),
            this, SLOT(newPlot3D(const QString &, double, double, double, double, double, double)));
    connect(sd, SIGNAL(clearFunctionsList()), this, SLOT(clearSurfaceFunctionsList()));

    sd->insertFunctionsList(surfaceFunc);
    sd->exec();
}

Graph3D *ApplicationWindow::newPlot3D(const QString &formula, double xl, double xr, double yl,
                                      double yr, double zl, double zr)
{
    QString label = generateUniqueName(tr("Graph"));

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addFunction(formula, xl, xr, yl, yr, zl, zr);
    plot->resize(500, 400);
    plot->setWindowTitle(label);
    plot->setName(label);
    customPlot3D(plot);
    plot->update();

    initPlot3D(plot);

    emit modified();
    return plot;
}

void ApplicationWindow::updateSurfaceFuncList(const QString &s)
{
    surfaceFunc.removeAll(s);
    surfaceFunc.push_front(s);
    while ((int)surfaceFunc.size() > 10)
        surfaceFunc.pop_back();
}

Graph3D *ApplicationWindow::newPlot3D(const QString &caption, const QString &formula, double xl,
                                      double xr, double yl, double yr, double zl, double zr)
{
    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addFunction(formula, xl, xr, yl, yr, zl, zr);
    plot->update();

    QString label = caption;
    while (alreadyUsedName(label))
        label = generateUniqueName(tr("Graph"));

    plot->setWindowTitle(label);
    plot->setName(label);
    initPlot3D(plot);
    return plot;
}

Graph3D *ApplicationWindow::dataPlot3D(Table *table, const QString &colName)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString label = generateUniqueName(tr("Graph"));
    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addData(table, colName);
    plot->resize(500, 400);
    plot->setWindowTitle(label);
    plot->setName(label);

    customPlot3D(plot);
    plot->update();
    initPlot3D(plot);

    emit modified();
    QApplication::restoreOverrideCursor();
    return plot;
}

Graph3D *ApplicationWindow::dataPlot3D(const QString &caption, const QString &formula, double xl,
                                       double xr, double yl, double yr, double zl, double zr)
{
    int pos = formula.indexOf("_", 0);
    QString wCaption = formula.left(pos);

    Table *w = table(wCaption);
    if (!w)
        return 0;

    int posX = formula.indexOf("(", pos);
    QString xCol = formula.mid(pos + 1, posX - pos - 1);

    pos = formula.indexOf(",", posX);
    posX = formula.indexOf("(", pos);
    QString yCol = formula.mid(pos + 1, posX - pos - 1);

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addData(w, xCol, yCol, xl, xr, yl, yr, zl, zr);
    plot->update();

    QString label = caption;
    while (alreadyUsedName(label))
        label = generateUniqueName(tr("Graph"));

    plot->setWindowTitle(label);
    plot->setName(label);
    initPlot3D(plot);

    return plot;
}

Graph3D *ApplicationWindow::newPlot3D()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString label = generateUniqueName(tr("Graph"));

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->resize(500, 400);
    plot->setWindowTitle(label);
    plot->setName(label);

    customPlot3D(plot);
    initPlot3D(plot);

    emit modified();
    QApplication::restoreOverrideCursor();
    return plot;
}

Graph3D *ApplicationWindow::dataPlotXYZ(Table *table, const QString &zColName, int type)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString label = generateUniqueName(tr("Graph"));
    int zCol = table->colIndex(zColName);
    int yCol = table->colY(zCol);
    int xCol = table->colX(zCol);

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addData(table, xCol, yCol, zCol, type);
    plot->resize(500, 400);
    plot->setWindowTitle(label);
    plot->setName(label);

    customPlot3D(plot);
    plot->update();
    initPlot3D(plot);

    emit modified();
    QApplication::restoreOverrideCursor();
    return plot;
}

Graph3D *ApplicationWindow::dataPlotXYZ(const QString &caption, const QString &formula, double xl,
                                        double xr, double yl, double yr, double zl, double zr)
{
    int pos = formula.indexOf("_", 0);
    QString wCaption = formula.left(pos);

    Table *w = table(wCaption);
    if (!w)
        return 0;

    int posX = formula.indexOf("(X)", pos);
    QString xColName = formula.mid(pos + 1, posX - pos - 1);

    pos = formula.indexOf(",", posX);

    posX = formula.indexOf("(Y)", pos);
    QString yColName = formula.mid(pos + 1, posX - pos - 1);

    pos = formula.indexOf(",", posX);
    posX = formula.indexOf("(Z)", pos);
    QString zColName = formula.mid(pos + 1, posX - pos - 1);

    int xCol = w->colIndex(xColName);
    int yCol = w->colIndex(yColName);
    int zCol = w->colIndex(zColName);

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addData(w, xCol, yCol, zCol, xl, xr, yl, yr, zl, zr);
    plot->update();

    QString label = caption;
    if (alreadyUsedName(label))
        label = generateUniqueName(tr("Graph"));

    plot->setWindowTitle(label);
    plot->setName(label);
    initPlot3D(plot);
    return plot;
}

void ApplicationWindow::customPlot3D(Graph3D *plot)
{
    plot->setDataColors(QColor(COLORVALUE(plot3DColors[4])), QColor(COLORVALUE(plot3DColors[0])));
    plot->updateColors(QColor(COLORVALUE(plot3DColors[2])), QColor(COLORVALUE(plot3DColors[6])),
                       QColor(COLORVALUE(plot3DColors[5])), QColor(COLORVALUE(plot3DColors[1])),
                       QColor(COLORVALUE(plot3DColors[7])), QColor(COLORVALUE(plot3DColors[3])));

    plot->setResolution(plot3DResolution);
    plot->showColorLegend(showPlot3DLegend);
    plot->setSmoothMesh(smooth3DMesh);
    plot->setOrtho(orthogonal3DPlots);
    if (showPlot3DProjection)
        plot->setFloorData();

    plot->setNumbersFont(plot3DNumbersFont);
    plot->setXAxisLabelFont(plot3DAxesFont);
    plot->setYAxisLabelFont(plot3DAxesFont);
    plot->setZAxisLabelFont(plot3DAxesFont);
    plot->setTitleFont(plot3DTitleFont);
}

void ApplicationWindow::initPlot3D(Graph3D *plot)
{
    d_workspace.addSubWindow(plot);
    current_folder->addWindow(plot);
    plot->setFolder(current_folder);
    connectSurfacePlot(plot);

    plot->setWindowIcon(QPixmap(":/trajectory.xpm"));
    plot->show();
    plot->setFocus();

    addListViewItem(plot);

    if (!graph_3D_tools->isEnabled())
        graph_3D_tools->setEnabled(true);

    customMenu(plot);
    customToolBars(plot);
}

Matrix *ApplicationWindow::importImage()
{
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    QString filter = tr("Images") + " (", aux1, aux2;
    for (int i = 0; i < (int)list.count(); i++) {
        aux1 = " *." + list[i] + " ";
        aux2 += " *." + list[i] + ";;";
        filter += aux1;
    }
    filter += ");;" + aux2;

    QString fn =
            QFileDialog::getOpenFileName(this, tr("Import image from file"), imagesDirPath, filter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        imagesDirPath = fi.absolutePath();
        return importImage(fn);
    } else
        return 0;
}

void ApplicationWindow::loadImage()
{
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    QString filter = tr("Images") + " (", aux1, aux2;
    for (int i = 0; i < (int)list.count(); i++) {
        aux1 = " *." + list[i] + " ";
        aux2 += " *." + list[i] + ";;";
        filter += aux1;
    }
    filter += ");;" + aux2;

    QString fn =
            QFileDialog::getOpenFileName(this, tr("Load image from file"), imagesDirPath, filter);
    if (!fn.isEmpty()) {
        loadImage(fn);
        QFileInfo fi(fn);
        imagesDirPath = fi.absolutePath();
    }
}

void ApplicationWindow::loadImage(const QString &fn)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MultiLayer *plot = multilayerPlot(generateUniqueName(tr("Graph")));
    plot->setWindowLabel(fn);
    plot->setCaptionPolicy(MyWidget::Both);
    setListViewLabel(plot->name(), fn);

    plot->showNormal();
    Graph *g = plot->addLayer(0, 0, plot->width(), plot->height());

    g->setTitle("");
    QVector<bool> axesOn(4);
    for (int j = 0; j < 4; j++)
        axesOn[j] = false;
    g->enableAxes(axesOn);
    g->removeLegend();
    g->setIgnoreResizeEvents(false);
    g->addImage(fn);
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::polishGraph(Graph *g, int style)
{
    if (style == Graph::VerticalBars || style == Graph::HorizontalBars
        || style == Graph::Histogram) {
        QList<int> ticksList;
        int ticksStyle = ScaleDraw::Out;
        ticksList << ticksStyle << ticksStyle << ticksStyle << ticksStyle;
        g->setMajorTicksType(ticksList);
        g->setMinorTicksType(ticksList);
    }
    if (style == Graph::HorizontalBars) {
        g->setAxisTitle(0, tr("Y Axis Title"));
        g->setAxisTitle(1, tr("X Axis Title"));
    }
}

MultiLayer *ApplicationWindow::multilayerPlot(const QString &caption)
{
    MultiLayer *ml = new MultiLayer("", &d_workspace, 0);
    ml->setAttribute(Qt::WA_DeleteOnClose);
    QString label = caption;
    initMultilayerPlot(ml, label.replace(QRegExp("_"), "-"));
    return ml;
}

MultiLayer *ApplicationWindow::newGraph(const QString &caption)
{
    MultiLayer *ml = multilayerPlot(generateUniqueName(caption));
    if (ml) {
        Graph *g = ml->addLayer();
        setPreferences(g);
        g->newLegend();
        g->setAutoscaleFonts(false);
        g->setIgnoreResizeEvents(false);
        ml->arrangeLayers(false, false);
        g->setAutoscaleFonts(autoScaleFonts); // restore user defined fonts behaviour
        g->setIgnoreResizeEvents(!autoResizeLayers);
        customMenu(ml);
    }
    return ml;
}

MultiLayer *ApplicationWindow::multilayerPlot(Table *w, const QStringList &colList, int style,
                                              int startRow, int endRow)
{ // used when plotting selected columns
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MultiLayer *g = new MultiLayer("", &d_workspace, 0);
    g->setAttribute(Qt::WA_DeleteOnClose);

    initMultilayerPlot(g, generateUniqueName(tr("Graph")));

    Graph *ag = g->addLayer();
    if (!ag)
        return 0;

    setPreferences(ag);
    ag->insertCurvesList(w, colList, style, defaultCurveLineWidth, defaultSymbolSize, startRow,
                         endRow);

    polishGraph(ag, style);
    ag->newLegend();
    g->arrangeLayers(false, false);
    customMenu(g);

    emit modified();
    QApplication::restoreOverrideCursor();
    return g;
}

MultiLayer *ApplicationWindow::multilayerPlot(int c, int r, int style)
{ // used when plotting from the panel menu
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return 0;

    Table *w = (Table *)d_workspace.activeSubWindow();
    if (!validFor2DPlot(w, style))
        return 0;

    QStringList list;
    switch (style) {
    case Graph::Histogram:
    case Graph::Pie:
    case Graph::Box:
        list = w->selectedColumns();
        break;
    default:
        list = w->selectedYColumns();
        break;
    }

    int curves = (int)list.count();
    if (r < 0)
        r = curves;

    MultiLayer *g = new MultiLayer("", &d_workspace, 0);
    g->setAttribute(Qt::WA_DeleteOnClose);

    initMultilayerPlot(g, generateUniqueName(tr("Graph")));

    int layers = c * r;
    if (curves < layers) {
        for (int i = 0; i < curves; i++) {
            Graph *ag = g->addLayer();
            if (ag) {
                setPreferences(ag);
                ag->insertCurvesList(w, QStringList(list[i]), style, defaultCurveLineWidth,
                                     defaultSymbolSize);
                ag->newLegend();
                ag->setAutoscaleFonts(false); // in order to avoid to small fonts
                ag->setIgnoreResizeEvents(false);
                polishGraph(ag, style);
            }
        }
    } else {
        for (int i = 0; i < layers; i++) {
            Graph *ag = g->addLayer();
            if (ag) {
                QStringList lst;
                lst << list[i];
                setPreferences(ag);
                ag->insertCurvesList(w, lst, style, defaultCurveLineWidth, defaultSymbolSize);
                ag->newLegend();
                ag->setAutoscaleFonts(false); // in order to avoid to small fonts
                ag->setIgnoreResizeEvents(false);
                polishGraph(ag, style);
            }
        }
    }
    g->setRows(r);
    g->setCols(c);
    g->arrangeLayers(false, false);
    QWidgetList lst = g->graphPtrs();
    foreach (QWidget *widget, lst) {
        Graph *ag = (Graph *)widget;
        ag->setAutoscaleFonts(autoScaleFonts); // restore user defined fonts behaviour
        ag->setIgnoreResizeEvents(!autoResizeLayers);
    }
    customMenu(g);
    emit modified();
    return g;
}

MultiLayer *ApplicationWindow::multilayerPlot(const QStringList &colList)
{ // used when plotting from wizard
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    MultiLayer *g = new MultiLayer("", &d_workspace, 0);
    g->setAttribute(Qt::WA_DeleteOnClose);

    initMultilayerPlot(g, generateUniqueName(tr("Graph")));

    Graph *ag = g->addLayer();
    setPreferences(ag);
    polishGraph(ag, defaultCurveStyle);
    int curves = (int)colList.count();
    int errorBars = 0;
    for (int i = 0; i < curves; i++) {
        if (colList[i].contains("(yErr)") || colList[i].contains("(xErr)"))
            errorBars++;
    }

    for (int i = 0; i < curves; i++) {
        QString s = colList[i];
        int pos = s.indexOf(":", 0);
        QString caption = s.left(pos) + "_";
        Table *w = (Table *)table(caption);

        int posX = s.indexOf("(X)", pos);
        QString xColName = caption + s.mid(pos + 2, posX - pos - 2);
        int xCol = w->colIndex(xColName);

        posX = s.indexOf(",", posX);
        int posY = s.indexOf("(Y)", posX);
        QString yColName = caption + s.mid(posX + 2, posY - posX - 2);

        if (s.contains("(yErr)") || s.contains("(xErr)")) {
            posY = s.indexOf(",", posY);
            int posErr, errType;
            if (s.contains("(yErr)")) {
                errType = QwtErrorPlotCurve::Vertical;
                posErr = s.indexOf("(yErr)", posY);
            } else {
                errType = QwtErrorPlotCurve::Horizontal;
                posErr = s.indexOf("(xErr)", posY);
            }

            QString errColName = caption + s.mid(posY + 2, posErr - posY - 2);
            ag->addErrorBars(xColName, yColName, w, errColName, errType);
        } else
            ag->insertCurve(w, xCol, yColName, defaultCurveStyle);

        CurveLayout cl = ag->initCurveLayout(defaultCurveStyle, curves - errorBars);
        cl.lWidth = defaultCurveLineWidth;
        cl.sSize = defaultSymbolSize;
        ag->updateCurveLayout(i, &cl);
    }
    ag->newLegend();
    ag->updatePlot();
    g->arrangeLayers(true, false);
    customMenu(g);
    emit modified();
    QApplication::restoreOverrideCursor();
    return g;
}

void ApplicationWindow::initBareMultilayerPlot(MultiLayer *g, const QString &name)
{ // FIXME: workaround, init without unnecessary g->show()
    QString label = name;
    while (alreadyUsedName(label))
        label = generateUniqueName(tr("Graph"));

    connectMultilayerPlot(g);

    g->setWindowTitle(label);
    g->setName(label);
    g->setWindowIcon(QPixmap(":/graph.xpm"));
    g->setScaleLayersOnPrint(d_scale_plots_on_print);
    g->printCropmarks(d_print_cropmarks);

    d_workspace.addSubWindow(g);
    current_folder->addWindow(g);
    g->setFolder(current_folder);
    addListViewItem(g);
}

void ApplicationWindow::initMultilayerPlot(MultiLayer *g, const QString &name)
{
    initBareMultilayerPlot(g, name);
    g->show(); // FIXME: bad idea do it here
    g->setFocus();
}

void ApplicationWindow::customizeTables(const QColor &bgColor, const QColor &textColor,
                                        const QColor &headerColor, const QFont &textFont,
                                        const QFont &headerFont, bool showComments)
{
    tableBkgdColor = bgColor;
    tableTextColor = textColor;
    tableHeaderColor = headerColor;
    tableTextFont = textFont;
    tableHeaderFont = headerFont;
    d_show_table_comments = showComments;

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("Table"))
            customTable((Table *)w);
    }
}

void ApplicationWindow::customTable(Table *w)
{
    QPalette cg;
    cg.setColor(QPalette::Base, QColor(tableBkgdColor));
    cg.setColor(QPalette::Text, QColor(tableTextColor));
    w->setPalette(cg);

    w->setHeaderColor(tableHeaderColor);
    w->setTextFont(tableTextFont);
    w->setHeaderFont(tableHeaderFont);
    w->showComments(d_show_table_comments);
}

void ApplicationWindow::setPreferences(Graph *g)
{
    if (!g->isPiePlot()) {
        if (allAxesOn) {
            QVector<bool> axesOn(QwtPlot::axisCnt);
            axesOn.fill(true);
            g->enableAxes(axesOn);
            g->updateSecondaryAxis(QwtPlot::xTop);
            g->updateSecondaryAxis(QwtPlot::yRight);
        }

        QList<int> ticksList;
        ticksList << majTicksStyle << majTicksStyle << majTicksStyle << majTicksStyle;
        g->setMajorTicksType(ticksList);
        ticksList.clear();
        ticksList << minTicksStyle << minTicksStyle << minTicksStyle << minTicksStyle;
        g->setMinorTicksType(ticksList);

        g->setTicksLength(minTicksLength, majTicksLength);
        g->setAxesLinewidth(axesLineWidth);
        g->drawAxesBackbones(drawBackbones);
    }

    g->initFonts(plotAxesFont, plotNumbersFont);
    g->setTextMarkerDefaults(legendFrameStyle, plotLegendFont, legendTextColor, legendBackground);
    g->setArrowDefaults(defaultArrowLineWidth, defaultArrowColor, defaultArrowLineStyle,
                        defaultArrowHeadLength, defaultArrowHeadAngle, defaultArrowHeadFill);
    g->initTitle(titleOn, plotTitleFont);
    g->drawCanvasFrame(canvasFrameOn, canvasFrameWidth);
    g->plotWidget()->setMargin(defaultPlotMargin);
    g->enableAutoscaling(autoscale2DPlots);
    g->setAutoscaleFonts(autoScaleFonts);
    g->setIgnoreResizeEvents(!autoResizeLayers);
    g->setAntialiasing(antialiasing2DPlots);
}

void ApplicationWindow::newWrksheetPlot(const QString &name, const QString &label,
                                        QList<Column *> columns)
{
    Table *w = newTable(name, label, columns);
    MultiLayer *plot = multilayerPlot(w, QStringList(QString(w->name()) + "_intensity"), 0);
    Graph *g = (Graph *)plot->activeGraph();
    if (g) {
        g->setTitle("");
        g->setXAxisTitle(tr("pixels"));
        g->setYAxisTitle(tr("pixel intensity (a.u.)"));
    }
}

/*
 *used when importing an ASCII file
 */
Table *ApplicationWindow::newTable(const QString &fname, const QString &sep, int lines,
                                   bool renameCols, bool stripSpaces, bool simplifySpaces,
                                   bool convertToNumeric, QLocale numericLocale)
{
    Table *w = new Table(scriptEnv, fname, sep, lines, renameCols, stripSpaces, simplifySpaces,
                         convertToNumeric, numericLocale, fname, &d_workspace);
    if (w) {
        w->setName(generateUniqueName(tr("Table")));
        initTable(w);
        d_project->addChild(w->d_future_table);
    }
    return w;
}

/*
 *creates a new empty table
 */
Table *ApplicationWindow::newTable()
{
    Table *w = new Table(scriptEnv, 30, 2, "", &d_workspace, 0);
    w->setName(generateUniqueName(tr("Table")));
    initTable(w);
    d_project->addChild(w->d_future_table);
    return w;
}

/*
 *used when opening a project file
 */
Table *ApplicationWindow::newTable(const QString &caption, int r, int c)
{
    Table *w = new Table(scriptEnv, r, c, "", &d_workspace, 0);
    w->setName(caption);
    initTable(w);
    d_project->addChild(w->d_future_table);
    if (w->name() != caption) // the table was renamed
    {
        renamedTables << caption << w->name();

        QMessageBox::warning(this, tr("Renamed Window"),
                             tr("The table '%1' already exists. It has been renamed '%2'.")
                                     .arg(caption)
                                     .arg(w->name()));
    }
    return w;
}

Table *ApplicationWindow::newTable(int r, int c, const QString &name, const QString &legend)
{
    Table *w = new Table(scriptEnv, r, c, legend, &d_workspace, 0);
    w->setName(name);
    initTable(w);
    d_project->addChild(w->d_future_table);
    return w;
}

Table *ApplicationWindow::newTable(const QString &name, const QString &legend,
                                   QList<Column *> columns)
{
    Table *w = new Table(scriptEnv, 0, 0, legend, &d_workspace, 0);
    w->d_future_table->appendColumns(columns);
    w->setName(name);
    initTable(w);
    d_project->addChild(w->d_future_table);
    return w;
}

Table *ApplicationWindow::newHiddenTable(const QString &name, const QString &label,
                                         QList<Column *> columns)
{
    auto w = newTable(name, label, columns);
    hideWindow(w);
    return w;
}

void ApplicationWindow::initTable(Table *w)
{
    d_workspace.addSubWindow(w);
    w->setWindowIcon(QPixmap(":/worksheet.xpm"));
    current_folder->addWindow(w);
    w->setFolder(current_folder);
    addListViewItem(w);
    w->showNormal();

    connectTable(w);
    customTable(w);

    w->d_future_table->setPlotMenu(plot2D);

    emit modified();
}

/*
 * !creates a new table with type statistics on target columns/rows of table base
 */
TableStatistics *ApplicationWindow::newTableStatistics(Table *base, int type, QList<int> target,
                                                       const QString &caption)
{
    TableStatistics *s =
            new TableStatistics(scriptEnv, &d_workspace, base, (TableStatistics::Type)type, target);
    if (!caption.isEmpty())
        s->setName(caption);

    d_project->addChild(s->d_future_table);
    connect(base, SIGNAL(modifiedData(Table *, const QString &)), s,
            SLOT(update(Table *, const QString &)));
    connect(base, SIGNAL(changedColHeader(const QString &, const QString &)), s,
            SLOT(renameCol(const QString &, const QString &)));
    connect(base, SIGNAL(removedCol(const QString &)), s, SLOT(removeCol(const QString &)));
    connect(base->d_future_table, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)), this,
            SLOT(removeDependentTableStatistics(const AbstractAspect *)));
    return s;
}

void ApplicationWindow::removeDependentTableStatistics(const AbstractAspect *aspect)
{
    ::future::Table *future_table =
            qobject_cast<::future::Table *>(const_cast<AbstractAspect *>(aspect));
    if (!future_table)
        return;
    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *win, windows) {
        TableStatistics *table_stat = qobject_cast<TableStatistics *>(win);
        if (!table_stat)
            continue;
        Table *table = qobject_cast<Table *>(future_table->view());
        if (!table)
            continue;
        if (table_stat->base() == table)
            d_project->removeChild(table_stat->d_future_table);
    }
}

/*
 *creates a new empty note window
 */
Note *ApplicationWindow::newNote(const QString &caption)
{
    Note *m = new Note(scriptEnv, "", &d_workspace);
    if (caption.isEmpty())
        initNote(m, generateUniqueName(tr("Notes")));
    else
        initNote(m, caption);
    m->showNormal();
    return m;
}

void ApplicationWindow::initNote(Note *m, const QString &caption)
{
    QString name = caption;
    while (name.isEmpty() || alreadyUsedName(name))
        name = generateUniqueName(tr("Notes"));

    d_workspace.addSubWindow(m);
    m->setWindowTitle(name);
    m->setName(name);
    m->setWindowIcon(QPixmap(":/note.xpm"));
    m->askOnCloseEvent(confirmCloseNotes);
    m->setFolder(current_folder);

    current_folder->addWindow(m);
    addListViewItem(m);

    connect(m, SIGNAL(modifiedWindow(MyWidget *)), this, SLOT(modifiedProject(MyWidget *)));
    connect(m, SIGNAL(closedWindow(MyWidget *)), this, SLOT(closeWindow(MyWidget *)));
    connect(m, SIGNAL(hiddenWindow(MyWidget *)), this, SLOT(hideWindow(MyWidget *)));
    connect(m, SIGNAL(statusChanged(MyWidget *)), this, SLOT(updateWindowStatus(MyWidget *)));
    connect(m, SIGNAL(showTitleBarMenu()), this, SLOT(showWindowTitleBarMenu()));

    emit modified();
}

Matrix *ApplicationWindow::newMatrix(int rows, int columns)
{
    Matrix *m = new Matrix(scriptEnv, rows, columns, "", 0, 0);
    QString caption = generateUniqueName(tr("Matrix"));
    while (alreadyUsedName(caption)) {
        caption = generateUniqueName(tr("Matrix"));
    }
    m->setName(caption);
    initMatrix(m);
    d_project->addChild(m->d_future_matrix);
    return m;
}

Matrix *ApplicationWindow::newMatrix(const QString &caption, int r, int c)
{
    Matrix *w = new Matrix(scriptEnv, r, c, "", 0, 0);
    QString name = caption;
    while (alreadyUsedName(name)) {
        name = generateUniqueName(caption);
    }
    w->setName(name);
    initMatrix(w);
    d_project->addChild(w->d_future_matrix);
    if (w->name() != caption) // the matrix was renamed
        renamedTables << caption << w->name();

    return w;
}

void ApplicationWindow::matrixDeterminant()
{
    Matrix *m = (Matrix *)d_workspace.activeSubWindow();
    if (!m)
        return;

    QDateTime dt = QDateTime::currentDateTime();
    QString info = QLocale().toString(dt);
    info += "\n" + tr("Determinant of ") + QString(m->name()) + ":\t";
    info += "det = " + QString::number(m->determinant()) + "\n";
    info += "-------------------------------------------------------------\n";

    logInfo += info;

    showResults(true);
}

void ApplicationWindow::invertMatrix()
{
    Matrix *m = (Matrix *)d_workspace.activeSubWindow();
    if (!m)
        return;

    m->invert();
}

Table *ApplicationWindow::convertMatrixToTable()
{
    Matrix *m = (Matrix *)d_workspace.activeSubWindow();
    if (!m)
        return 0;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int rows = m->numRows();
    int cols = m->numCols();

    Table *w = new Table(scriptEnv, rows, cols, "", &d_workspace, 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            w->setCell(i, j, m->cell(i, j));
    }

    w->setName(generateUniqueName(tr("Table")));
    initTable(w);
    d_project->addChild(w->d_future_table);
    w->setWindowLabel(m->windowLabel());
    w->setCaptionPolicy(m->captionPolicy());
    w->resize(m->size());
    w->showNormal();

    QApplication::restoreOverrideCursor();

    return w;
}

void ApplicationWindow::initMatrix(Matrix *m)
{
    d_workspace.addSubWindow(m);
    m->setWindowIcon(QPixmap(":/matrix.xpm"));
    m->askOnCloseEvent(confirmCloseMatrix);
    m->setNumericFormat(d_default_numeric_format, d_decimal_digits);
    m->setFolder(current_folder);

    current_folder->addWindow(m);
    m->setFolder(current_folder);
    addListViewItem(m);
    m->showNormal();

    connect(m, SIGNAL(showTitleBarMenu()), this, SLOT(showWindowTitleBarMenu()));
    connect(m, SIGNAL(modifiedWindow(MyWidget *)), this, SLOT(modifiedProject(MyWidget *)));
    connect(m, SIGNAL(modifiedWindow(MyWidget *)), this, SLOT(updateMatrixPlots(MyWidget *)));
    connect(m, SIGNAL(hiddenWindow(MyWidget *)), this, SLOT(hideWindow(MyWidget *)));
    connect(m, SIGNAL(statusChanged(MyWidget *)), this, SLOT(updateWindowStatus(MyWidget *)));
    connect(m, SIGNAL(showContextMenu()), this, SLOT(showWindowContextMenu()));
    emit modified();
}

Matrix *ApplicationWindow::convertTableToMatrix()
{
    Table *m = (Table *)d_workspace.activeSubWindow();
    if (!m)
        return 0;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int rows = m->numRows();
    int cols = m->numCols();

    Matrix *w = new Matrix(scriptEnv, rows, cols, "", 0, 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            w->setText(i, j, m->text(i, j));
    }

    QString caption = generateUniqueName(m->name());
    w->setName(caption);
    initMatrix(w);
    d_project->addChild(w->d_future_matrix);

    w->setCaptionPolicy(m->captionPolicy());
    w->resize(m->size());
    w->showNormal();

    QApplication::restoreOverrideCursor();
    return w;
}

MyWidget *ApplicationWindow::window(const QString &name)
{
    MyWidget *widget = 0;
    QList<MyWidget *> windows = windowsList();
    for (int i = 0; i < windows.count(); i++) {
        widget = windows.at(i);
        if (widget && widget->name() == name)
            return widget;
    }
    return widget;
}

Table *ApplicationWindow::table(const QString &name)
{
    int pos = name.indexOf("_", 0);
    QString caption = name.left(pos);

    QList<MyWidget *> lst = windowsList();
    foreach (MyWidget *w, lst) {
        if (w->inherits("Table") && static_cast<Table *>(w)->name() == caption) {
            return (Table *)w;
        }
    }
    return 0;
}

Matrix *ApplicationWindow::matrix(const QString &name)
{
    QString caption = name;
    if (!renamedTables.isEmpty() && renamedTables.contains(caption)) {
        int index = renamedTables.indexOf(caption);
        caption = renamedTables[index + 1];
    }

    QList<MyWidget *> lst = windowsList();
    foreach (MyWidget *w, lst) {
        if (w->inherits("Matrix") && static_cast<Matrix *>(w)->name() == caption) {
            return (Matrix *)w;
        }
    }
    return 0;
}

void ApplicationWindow::windowActivated(QMdiSubWindow *w)
{
    if (!w || !w->inherits("MyWidget"))
        return;

    customToolBars((MyWidget *)w);
    customMenu((MyWidget *)w);

    Folder *f = ((MyWidget *)w)->folder();
    if (f)
        f->setActiveWindow((MyWidget *)w);
}

void ApplicationWindow::addErrorBars()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g)
        return;

    if (!g->curves()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("There are no curves available on this plot!"));
        return;
    }

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));
        return;
    }

    ErrDialog *ed = new ErrDialog(this);
    ed->setAttribute(Qt::WA_DeleteOnClose);
    connect(ed, SIGNAL(options(const QString &, int, const QString &, int)), this,
            SLOT(defineErrorBars(const QString &, int, const QString &, int)));
    connect(ed, SIGNAL(options(const QString &, const QString &, int)), this,
            SLOT(defineErrorBars(const QString &, const QString &, int)));

    ed->setCurveNames(g->analysableCurvesList());
    ed->setSrcTables(tableList());
    ed->exec();
}

void ApplicationWindow::defineErrorBars(const QString &name, int type, const QString &percent,
                                        int direction)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    Table *w = table(name);
    if (!w) { // user defined function
        QMessageBox::critical(
                this, tr("Error bars error"),
                tr("This feature is not available for user defined function curves!"));
        return;
    }

    DataCurve *master_curve = (DataCurve *)g->curve(name);
    QString xColName = master_curve->xColumnName();
    if (xColName.isEmpty())
        return;

    Column *errors = new Column("1", Makhber::ColumnMode::Numeric);
    Column *data;
    if (direction == QwtErrorPlotCurve::Horizontal) {
        errors->setPlotDesignation(Makhber::xErr);
        data = w->d_future_table->column(xColName);
    } else {
        errors->setPlotDesignation(Makhber::yErr);
        data = w->d_future_table->column(name);
    }
    if (!data)
        return;

    int rows = data->rowCount();
    if (type == 0) {
        double fraction = percent.toDouble() / 100.0;
        for (int i = 0; i < rows; i++)
            errors->setValueAt(i, data->valueAt(i) * fraction);
    } else if (type == 1) {
        double average = 0.0;
        double dev = 0.0;
        for (int i = 0; i < rows; i++)
            average += data->valueAt(i);
        average /= rows;
        for (int i = 0; i < rows; i++)
            dev += pow(data->valueAt(i) - average, 2);
        dev = sqrt(dev / rows);
        for (int i = 0; i < rows; i++)
            errors->setValueAt(i, dev);
    }
    w->d_future_table->addChild(errors);
    g->addErrorBars(xColName, name, w, errors->name(), direction);
}

void ApplicationWindow::defineErrorBars(const QString &curveName, const QString &errColumnName,
                                        int direction)
{
    Table *w = table(curveName);
    if (!w) { // user defined function --> no worksheet available
        QMessageBox::critical(
                this, tr("Error"),
                tr("This feature is not available for user defined function curves!"));
        return;
    }

    Table *errTable = table(errColumnName);
    if (w->numRows() != errTable->numRows()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("The selected columns have different numbers of rows!"));

        addErrorBars();
        return;
    }

    int errCol = errTable->colIndex(errColumnName);
    if (errTable->d_future_table->column(errCol)->dataType() != Makhber::TypeDouble) {
        QMessageBox::critical(this, tr("Error"),
                              tr("You can only define error bars for numeric columns."));
        addErrorBars();
        return;
    }

    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    g->addErrorBars(curveName, errTable, errColumnName, direction);
    emit modified();
}

void ApplicationWindow::removeCurves(const QString &name)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList lst = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, lst)
                ((Graph *)widget)->removeCurves(name);
        } else if (w->inherits("Graph3D")) {
            if ((((Graph3D *)w)->formula()).contains(name))
                ((Graph3D *)w)->clearData();
        }
    }
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::updateCurves(Table *t, const QString &name)
{
    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            for (int k = 0; k < (int)graphsList.count(); k++) {
                Graph *g = (Graph *)graphsList.at(k);
                if (g)
                    g->updateCurvesData(t, name);
            }
        } else if (w->inherits("Graph3D")) {
            Graph3D *g = (Graph3D *)w;
            if ((g->formula()).contains(name))
                g->updateData(t);
        }
    }
}

void ApplicationWindow::showPreferencesDialog()
{
    ConfigDialog *cd = new ConfigDialog(this);
    cd->setAttribute(Qt::WA_DeleteOnClose);
    cd->setColumnSeparator(columnSeparator);
    cd->exec();
}

void ApplicationWindow::setSaveSettings(bool autoSaving, int min)
{
    if (autoSave == autoSaving && autoSaveTime == min)
        return;

    autoSave = autoSaving;
    autoSaveTime = min;

    killTimer(savingTimerId);

    if (autoSave)
        savingTimerId = startTimer(autoSaveTime * 60000);
    else
        savingTimerId = 0;
}

void ApplicationWindow::changeAppStyle(const QString &s)
{
    // style keys are case insensitive
    if (appStyle.toLower() == s.toLower())
        return;

    qApp->setStyle(s);
    appStyle = qApp->style()->objectName();

    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Active, QPalette::Base, QColor(panelsColor));
    qApp->setPalette(pal);
}

void ApplicationWindow::changeAppFont(const QFont &f)
{
    if (appFont == f)
        return;

    appFont = f;
    updateAppFonts();
}

void ApplicationWindow::updateAppFonts()
{
    qApp->setFont(appFont);
    this->setFont(appFont);
    scriptingMenu->setFont(appFont);
    windowsMenu->setFont(appFont);
    view->setFont(appFont);
    graph->setFont(appFont);
    file->setFont(appFont);
    format->setFont(appFont);
    calcul->setFont(appFont);
    edit->setFont(appFont);
    dataMenu->setFont(appFont);
    recent->setFont(appFont);
    help->setFont(appFont);
    type->setFont(appFont);
    plot2D->setFont(appFont);
    plot3D->setFont(appFont);
    plot3DMenu->setFont(appFont);
    matrixMenu->setFont(appFont);
    specialPlot->setFont(appFont);
    panels->setFont(appFont);
    stat->setFont(appFont);
    smooth->setFont(appFont);
    filter->setFont(appFont);
    decay->setFont(appFont);
    plotDataMenu->setFont(appFont);
    tableMenu->setFont(appFont);
    exportPlot->setFont(appFont);
    translateMenu->setFont(appFont);
    multiPeakMenu->setFont(appFont);
}

void ApplicationWindow::updateConfirmOptions(bool askTables, bool askMatrices, bool askPlots2D,
                                             bool askPlots3D, bool askNotes)
{
    QList<MyWidget *> windows = windowsList();
    if (confirmCloseTable != askTables) {
        confirmCloseTable = askTables;
        for (int i = 0; i < int(windows.count()); i++) {
            if (windows.at(i)->inherits("Table"))
                ((MyWidget *)windows.at(i))->askOnCloseEvent(confirmCloseTable);
        }
    }

    if (confirmCloseMatrix != askMatrices) {
        confirmCloseMatrix = askMatrices;
        for (int i = 0; i < int(windows.count()); i++) {
            if (windows.at(i)->inherits("Matrix"))
                ((MyWidget *)windows.at(i))->askOnCloseEvent(confirmCloseMatrix);
        }
    }

    if (confirmClosePlot2D != askPlots2D) {
        confirmClosePlot2D = askPlots2D;
        for (int i = 0; i < int(windows.count()); i++) {
            if (windows.at(i)->inherits("MultiLayer"))
                ((MyWidget *)windows.at(i))->askOnCloseEvent(confirmClosePlot2D);
        }
    }

    if (confirmClosePlot3D != askPlots3D) {
        confirmClosePlot3D = askPlots3D;
        for (int i = 0; i < int(windows.count()); i++) {
            if (windows.at(i)->inherits("Graph3D"))
                ((MyWidget *)windows.at(i))->askOnCloseEvent(confirmClosePlot3D);
        }
    }

    if (confirmCloseNotes != askNotes) {
        confirmCloseNotes = askNotes;
        for (int i = 0; i < int(windows.count()); i++) {
            if (windows.at(i)->inherits("Note"))
                ((MyWidget *)windows.at(i))->askOnCloseEvent(confirmCloseNotes);
        }
    }
}

void ApplicationWindow::setGraphDefaultSettings(bool autoscale, bool scaleFonts, bool resizeLayers,
                                                bool antialiasing)
{
    if (autoscale2DPlots == autoscale && autoScaleFonts == scaleFonts
        && autoResizeLayers != resizeLayers && antialiasing2DPlots == antialiasing)
        return;

    autoscale2DPlots = autoscale;
    autoScaleFonts = scaleFonts;
    autoResizeLayers = !resizeLayers;
    antialiasing2DPlots = antialiasing;

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList lst = ((MultiLayer *)w)->graphPtrs();
            Graph *g;
            foreach (QWidget *widget, lst) {
                g = (Graph *)widget;
                g->enableAutoscaling(autoscale2DPlots);
                g->updateScale();
                g->setIgnoreResizeEvents(!autoResizeLayers);
                g->setAutoscaleFonts(autoScaleFonts);
                g->setAntialiasing(antialiasing2DPlots);
            }
        }
    }
}

void ApplicationWindow::setLegendDefaultSettings(int frame, const QFont &font,
                                                 const QColor &textCol, const QColor &backgroundCol)
{
    if (legendFrameStyle == frame && legendTextColor == textCol && legendBackground == backgroundCol
        && plotLegendFont == font)
        return;

    legendFrameStyle = frame;
    legendTextColor = textCol;
    legendBackground = backgroundCol;
    plotLegendFont = font;

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, graphsList)
                ((Graph *)widget)->setTextMarkerDefaults(frame, font, textCol, backgroundCol);
        }
    }
    saveSettings();
}

void ApplicationWindow::setArrowDefaultSettings(const QPen &pen, int headLength, int headAngle,
                                                bool fillHead)
{
    if (defaultArrowLineWidth == pen.width() && defaultArrowColor == pen.color()
        && defaultArrowLineStyle == pen.style() && defaultArrowHeadLength == headLength
        && defaultArrowHeadAngle == headAngle && defaultArrowHeadFill == fillHead)
        return;

    defaultArrowLineWidth = pen.width();
    defaultArrowColor = pen.color();
    defaultArrowLineStyle = pen.style();
    defaultArrowHeadLength = headLength;
    defaultArrowHeadAngle = headAngle;
    defaultArrowHeadFill = fillHead;

    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            QWidgetList graphsList = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, graphsList)
                ((Graph *)widget)
                        ->setArrowDefaults(defaultArrowLineWidth, defaultArrowColor,

                                           defaultArrowLineStyle, defaultArrowHeadLength,
                                           defaultArrowHeadAngle, defaultArrowHeadFill);
        }
    }
    saveSettings();
}

ApplicationWindow *ApplicationWindow::plotFile(const QString &fn)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ApplicationWindow *app = new ApplicationWindow();
    app->applyUserSettings();
    app->showMaximized();

    Table *t = app->newTable(fn, app->columnSeparator, 0, true, app->strip_spaces,
                             app->simplify_spaces, app->d_convert_to_numeric,
                             app->d_ASCII_import_locale);
    t->setCaptionPolicy(MyWidget::Both);
    app->multilayerPlot(t, t->YColumns(), Graph::LineSymbols);
    QApplication::restoreOverrideCursor();
    return 0;
}

void ApplicationWindow::importASCII()
{
    ImportASCIIDialog *import_dialog = new ImportASCIIDialog(
            d_workspace.currentSubWindow() && d_workspace.currentSubWindow()->inherits("Table"),
            this, d_extended_import_ASCII_dialog);
    import_dialog->setDirectory(asciiDirPath);
    import_dialog->selectNameFilter(d_ASCII_file_filter);
    if (import_dialog->exec() != QDialog::Accepted)
        return;

    asciiDirPath = import_dialog->directory().path();
    if (import_dialog->rememberOptions()) {
        columnSeparator = import_dialog->columnSeparator();
        ignoredLines = import_dialog->ignoredLines();
        renameColumns = import_dialog->renameColumns();
        strip_spaces = import_dialog->stripSpaces();
        simplify_spaces = import_dialog->simplifySpaces();
        d_ASCII_import_locale = import_dialog->decimalSeparators();
        d_convert_to_numeric = import_dialog->convertToNumeric();
        saveSettings();
    }

    QLocale save_locale = QLocale();
    QLocale::setDefault(import_dialog->decimalSeparators());
    importASCII(import_dialog->selectedFiles(), import_dialog->importMode(),
                import_dialog->columnSeparator(), import_dialog->ignoredLines(),
                import_dialog->renameColumns(), import_dialog->stripSpaces(),
                import_dialog->simplifySpaces(), import_dialog->convertToNumeric(),
                import_dialog->decimalSeparators());
    QLocale::setDefault(save_locale);
}

void ApplicationWindow::importASCII(const QStringList &files, int import_mode,
                                    const QString &local_column_separator, int local_ignored_lines,
                                    bool local_rename_columns, bool local_strip_spaces,
                                    bool local_simplify_spaces, bool local_convert_to_numeric,
                                    QLocale local_numeric_locale)
{
    if (files.isEmpty())
        return;

    // this is very much a special case, and thus is handled completely in its own block
    if (import_mode == ImportASCIIDialog::NewTables) {
        int dx = 0, dy = 0;
        QStringList sorted_files = files;
        sorted_files.sort();
        for (int i = 0; i < sorted_files.size(); i++) {
            Table *w = newTable(sorted_files[i], local_column_separator, local_ignored_lines,
                                local_rename_columns, local_strip_spaces, local_simplify_spaces,
                                local_convert_to_numeric, local_numeric_locale);
            if (!w)
                continue;
            initTable(w);
            w->setCaptionPolicy(MyWidget::Both);
            setListViewLabel(w->name(), sorted_files[i]);
            if (i == 0) {
                dx = w->verticalHeaderWidth();
                dy = w->frameGeometry().height() - w->height();
                w->move(QPoint(0, 0));
            } else
                w->move(QPoint(i * dx, i * dy));
        }
        modifiedProject();
        return;
    }

    Table *table = qobject_cast<Table *>(d_workspace.currentSubWindow());
    if (!table)
        return;

    foreach (QString file, files) {
        Table *temp = new Table(scriptEnv, file, local_column_separator, local_ignored_lines,
                                local_rename_columns, local_strip_spaces, local_simplify_spaces,
                                local_convert_to_numeric, local_numeric_locale, "temp");
        if (!temp)
            continue;

        // need to check data types of columns for append/overwrite
        if (import_mode == ImportASCIIDialog::NewRows
            || import_mode == ImportASCIIDialog::Overwrite) {
            if (local_convert_to_numeric) {
                for (int col = 0; col < qMin(temp->columnCount(), table->columnCount()); col++)
                    if (table->column(col)->columnMode() != Makhber::ColumnMode::Numeric) {
                        QMessageBox::critical(this, tr("ASCII Import Failed"),
                                              tr("Numeric data cannot be imported into non-numeric "
                                                 "column \"%1\".")
                                                      .arg(table->column(col)->name()));
                        delete temp;
                        return;
                    }
            } else {
                for (int col = 0; col < qMin(temp->columnCount(), table->columnCount()); col++)
                    if (table->column(col)->columnMode() != Makhber::ColumnMode::Text) {
                        QMessageBox::critical(this, tr("ASCII Import Failed"),
                                              tr("Non-numeric data cannot be imported into "
                                                 "non-text column \"%1\".")
                                                      .arg(table->column(col)->name()));
                        delete temp;
                        return;
                    }
            }
        }

        // copy or move data from temp to table
        switch (import_mode) {
        case ImportASCIIDialog::NewColumns:
            while (temp->d_future_table->childCount() > 0)
                temp->d_future_table->reparentChild(table->d_future_table,
                                                    temp->d_future_table->child(0));
            break;
        case ImportASCIIDialog::NewRows: {
            int missing_columns = temp->columnCount() - table->columnCount();
            for (int col = 0; col < missing_columns; col++) {
                Column *new_col = new Column(tr("new_by_import") + QString::number(col + 1),
                                             local_convert_to_numeric ? Makhber::ColumnMode::Numeric
                                                                      : Makhber::ColumnMode::Text);
                new_col->setPlotDesignation(Makhber::Y);
                table->d_future_table->addChild(new_col);
            }
            Q_ASSERT(table->columnCount() >= temp->columnCount());
            int start_row = table->rowCount();
            table->d_future_table->setRowCount(table->rowCount() + temp->rowCount());
            for (int col = 0; col < temp->columnCount(); col++) {
                Column *src_col = temp->column(col);
                Column *dst_col = table->column(col);
                Q_ASSERT(src_col->dataType() == dst_col->dataType());
                dst_col->copy(src_col, 0, start_row, src_col->rowCount());
                if (local_rename_columns)
                    dst_col->setName(src_col->name());
            }
            break;
        }
        case ImportASCIIDialog::Overwrite: {
            if (table->rowCount() < temp->rowCount())
                table->d_future_table->setRowCount(temp->rowCount());
            for (int col = 0; col < table->columnCount() && col < temp->columnCount(); col++) {
                Column *src_col = temp->column(col);
                Column *dst_col = table->column(col);
                Q_ASSERT(src_col->dataType() == dst_col->dataType());
                dst_col->copy(src_col, 0, 0, temp->rowCount());
                if (local_rename_columns)
                    dst_col->setName(src_col->name());
            }
            if (temp->columnCount() > table->columnCount()) {
                temp->d_future_table->removeColumns(0, table->columnCount());
                while (temp->d_future_table->childCount() > 0)
                    temp->d_future_table->reparentChild(table->d_future_table,
                                                        temp->d_future_table->child(0));
            }
            break;
        }
        }
        delete temp;
    }

    table->setWindowLabel(files.join("; "));
    table->notifyChanges();
    modifiedProject(table);
    modifiedProject();
}

void ApplicationWindow::open()
{
    OpenProjectDialog *open_dialog = new OpenProjectDialog(this, d_extended_open_dialog);
    open_dialog->setDirectory(workingDir);
    auto &settings = getSettings();
    open_dialog->setCodec(settings.value("/General/Dialogs/LastUsedOriginLocale", "").toString());
    if (open_dialog->exec() != QDialog::Accepted || open_dialog->selectedFiles().isEmpty())
        return;
    workingDir = open_dialog->directory().path();
    settings.setValue("/General/Dialogs/LastUsedOriginLocale", open_dialog->codec());

    switch (open_dialog->openMode()) {
    case OpenProjectDialog::NewProject: {
        QString fn = open_dialog->selectedFiles()[0];
        QFileInfo fi(fn);

        if (projectname != "untitled") {
            QFileInfo fi(projectname);
            QString pn = fi.absolutePath();
            if (fn == pn) {
                QMessageBox::warning(this, tr("File opening error"),
                                     tr("The file: <b>%1</b> is the current file!").arg(fn));
                return;
            }
        }

        if (fn.endsWith(".sciprj", Qt::CaseInsensitive)
            || fn.endsWith(".sciprj~", Qt::CaseInsensitive)
            || fn.endsWith(".qti", Qt::CaseInsensitive) || fn.endsWith(".qti~", Qt::CaseInsensitive)
            || fn.endsWith(".sciprj.gz", Qt::CaseInsensitive)
            || fn.endsWith(".qti.gz", Qt::CaseInsensitive)
            || fn.endsWith(".opj", Qt::CaseInsensitive) || fn.endsWith(".ogm", Qt::CaseInsensitive)
            || fn.endsWith(".ogw", Qt::CaseInsensitive) || fn.endsWith(".ogg", Qt::CaseInsensitive)
            || fn.endsWith(".org", Qt::CaseInsensitive)) {
            if (!fi.exists()) {
                QMessageBox::critical(this, tr("File opening error"),
                                      tr("The file: <b>%1</b> doesn't exist!").arg(fn));
                return;
            }

            saveSettings(); // the recent projects must be saved

            ApplicationWindow *a = open(fn);
            if (a) {
                a->workingDir = workingDir;
                if (fn.endsWith(".sciprj", Qt::CaseInsensitive)
                    || fn.endsWith(".sciprj~", Qt::CaseInsensitive)
                    || fn.endsWith(".sciprj.gz", Qt::CaseInsensitive)
                    || fn.endsWith(".qti.gz", Qt::CaseInsensitive)
                    || fn.endsWith(".qti", Qt::CaseInsensitive)
                    || fn.endsWith(".qti~", Qt::CaseInsensitive)
                    || fn.endsWith(".opj", Qt::CaseInsensitive)
                    || fn.endsWith(".ogg", Qt::CaseInsensitive)
                    || fn.endsWith(".org", Qt::CaseInsensitive))
                    this->close();
            }
        } else {
            QMessageBox::critical(this, tr("File opening error"),
                                  tr("The file <b>%1</b> is not a valid project file.").arg(fn));
            return;
        }
        break;
    }
    case OpenProjectDialog::NewFolder:
        appendProject(open_dialog->selectedFiles()[0]);
        break;
    }
}

ApplicationWindow *ApplicationWindow::open(const QString &fn, const QStringList &scriptArgs)
{
    if (fn.endsWith(".opj", Qt::CaseInsensitive) || fn.endsWith(".ogm", Qt::CaseInsensitive)
        || fn.endsWith(".ogw", Qt::CaseInsensitive) || fn.endsWith(".ogg", Qt::CaseInsensitive)
        || fn.endsWith(".org", Qt::CaseInsensitive))
#ifdef ORIGIN_IMPORT
        return importOPJ(fn);
#else
    {
        QMessageBox::critical(
                this, tr("File opening error"),
                tr("Makhber currently does not support Origin import. If you are interested in "
                   "reviving and maintaining an Origin import filter, contact the developers.")
                        .arg(fn));
        return 0;
    }
#endif
    else if (fn.endsWith(".py", Qt::CaseInsensitive))
        return loadScript(fn, scriptArgs);
    else if (fn.endsWith(".sciprj", Qt::CaseInsensitive)
             || fn.endsWith(".sciprj.gz", Qt::CaseInsensitive)
             || fn.endsWith(".qti", Qt::CaseInsensitive)
             || fn.endsWith(".qti.gz", Qt::CaseInsensitive)
             || fn.endsWith(".sciprj~", Qt::CaseInsensitive)
             || fn.endsWith(".sciprj.gz~", Qt::CaseInsensitive)
             || fn.endsWith(".qti~", Qt::CaseInsensitive)
             || fn.endsWith(".qti.gz~", Qt::CaseInsensitive))
        return openProject(fn);
    else
        return plotFile(fn);
}

void ApplicationWindow::openRecentProject()
{
    QAction *trigger = qobject_cast<QAction *>(sender());
    if (!trigger)
        return;
    QString fn = trigger->text();
    int pos = fn.indexOf(" ", 0);
    fn = fn.right(fn.length() - pos - 1);

    QFile f(fn);
    if (!f.exists()) {
        QMessageBox::critical(this, tr("File Open Error"),
                              tr("The file: <b> %1 </b> <p>does not exist anymore!"
                                 "<p>It will be removed from the list.")
                                      .arg(fn));

        recentProjects.removeAll(fn);
        updateRecentProjectsList();
        return;
    }

    if (projectname != "untitled") {
        QFileInfo fi(projectname);
        QString pn = fi.absolutePath();
        if (fn == pn) {
            QMessageBox::warning(this, tr("File opening error"),
                                 tr("The file: <p><b> %1 </b><p> is the current file!").arg(fn));
            return;
        }
    }

    if (!fn.isEmpty()) {
        saveSettings(); // the recent projects must be saved
        ApplicationWindow *a = open(fn);
        if (a
            && (fn.endsWith(".sciprj", Qt::CaseInsensitive)
                || fn.endsWith(".sciprj~", Qt::CaseInsensitive)
                || fn.endsWith(".sciprj.gz", Qt::CaseInsensitive)
                || fn.endsWith(".qti.gz", Qt::CaseInsensitive)
                || fn.endsWith(".qti", Qt::CaseInsensitive)
                || fn.endsWith(".qti~", Qt::CaseInsensitive)
                || fn.endsWith(".opj", Qt::CaseInsensitive)
                || fn.endsWith(".ogg", Qt::CaseInsensitive)
                || fn.endsWith(".org", Qt::CaseInsensitive)))
            this->close();
    }
}

QFile *ApplicationWindow::openCompressedFile(const QString &fn)
{
    QTemporaryFile *file;
    char buf[16384];
    int len, err;

    gzFile in = gzopen(QFile::encodeName(fn).constData(), "rb");
    if (!in) {
        QMessageBox::critical(this, tr("File opening error"), tr("zlib can't open %1.").arg(fn));
        return 0;
    }
    file = new QTemporaryFile();
    if (!file || !file->open()) {
        gzclose(in);
        QMessageBox::critical(
                this, tr("File opening error"),
                tr("Can't create temporary file for writing uncompressed copy of %1.").arg(fn));
        return 0;
    }

    forever {
        len = gzread(in, buf, sizeof(buf));
        if (len == 0)
            break;
        if (len < 0) {
            QMessageBox::critical(this, tr("File opening error"), gzerror(in, &err));
            gzclose(in);
            file->close();
            delete file;
            return 0;
        }
        if (file->write(buf, len) != len) {
            QMessageBox::critical(
                    this, tr("File opening error"),
                    tr("Error writing to temporary file: %1").arg(file->errorString()));
            gzclose(in);
            file->close();
            delete file;
            return 0;
        }
    }

    gzclose(in);
    file->reset();
    return file;
}

bool ApplicationWindow::loadProject(const QString &fn)
{
    unique_ptr<QFile> file;

    if (fn.endsWith(".gz", Qt::CaseInsensitive) || fn.endsWith(".gz~", Qt::CaseInsensitive)) {
        file.reset(openCompressedFile(fn));
        if (!file)
            return false;
    } else {
        file.reset(new QFile(fn));
        if (!file->open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("File opening error"), file->errorString());
            return false;
        }
    }

    QTextStream t(file.get());
    t.setCodec(QTextCodec::codecForName("UTF-8"));
    QString s;
    QStringList list;

    s = t.readLine();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    list = s.split(QRegExp("\\s"), Qt::SkipEmptyParts);
#else
    list = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
#endif
    if (list.count() < 2
        || (list[0] != "Makhber" && list[0] != "SciDAVis" && list[0] != "QtiPlot")) {
        if (QFile::exists(fn + "~")) {
            int choice = QMessageBox::question(
                    this, tr("File opening error"),
                    tr("The file <b>%1</b> is corrupted, but there exists a backup copy.<br>Do you "
                       "want to open the backup instead?")
                            .arg(fn),
                    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);
            if (choice == QMessageBox::Yes) {
                QMessageBox::information(
                        this, tr("Opening backup copy"),
                        tr("The original (corrupt) file is being left untouched, in case you want "
                           "to "
                           "try rescuing data manually. If you want to continue working with the "
                           "automatically restored backup copy, you have to explicitly overwrite "
                           "the "
                           "original file."));
                return loadProject(fn + "~");
            }
        }
        QMessageBox::critical(this, tr("File opening error"),
                              tr("The file <b>%1</b> is not a valid project file.").arg(fn));
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList vl = list[1].split(".", Qt::SkipEmptyParts);
#else
    QStringList vl = list[1].split(".", QString::SkipEmptyParts);
#endif
    if (fn.endsWith(".qti", Qt::CaseInsensitive) || fn.endsWith(".qti.gz", Qt::CaseInsensitive)
        || fn.endsWith(".qti~", Qt::CaseInsensitive)
        || fn.endsWith(".qti.gz~", Qt::CaseInsensitive)) {
        d_file_version = 100 * (vl[0]).toInt() + 10 * (vl[1]).toInt() + (vl[2]).toInt();
        if (d_file_version > 90) {
            QMessageBox::critical(this, tr("File opening error"),
                                  tr("Makhber does not support QtiPlot project files from "
                                     "versions later than 0.9.0.")
                                          .arg(fn));
            return false;
        }
    } else
        d_file_version = ((vl[0]).toInt() << 16) + ((vl[1]).toInt() << 8) + (vl[2]).toInt();

    projectname = fn;
    setWindowTitle(tr("Makhber") + " - " + fn);

    QFileInfo fi(fn);
    QString baseName = fi.fileName();

    if (d_file_version < 73)
        t.readLine();

    s = t.readLine();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    list = s.split("\t", Qt::SkipEmptyParts);
#else
    list = s.split("\t", QString::SkipEmptyParts);
#endif
    if (list[0] == "<scripting-lang>") {
        if (!setScriptingLang(list[1], true))
            QMessageBox::warning(
                    this, tr("File opening error"),
                    tr("The file \"%1\" was created using \"%2\" as scripting language.\n\n"
                       "Initializing support for this language FAILED; I'm using \"%3\" instead.\n"
                       "Various parts of this file may not be displayed as expected.")
                            .arg(fn)
                            .arg(list[1])
                            .arg(scriptEnv->objectName()));

        s = t.readLine();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        list = s.split("\t", Qt::SkipEmptyParts);
#else
        list = s.split("\t", QString::SkipEmptyParts);
#endif
    }
    int aux = 0, widgets = list[1].toInt();

    QString titleBase = tr("Window") + ": ";
    QString title = titleBase + "1/" + QString::number(widgets) + "  ";

    QProgressDialog progress; /*(this);*/
    progress.setWindowModality(Qt::WindowModal);
    progress.setRange(0, widgets);
    progress.setMinimumWidth(width() / 2);
    progress.setWindowTitle(tr("Opening file") + ": " + baseName);
    progress.setLabelText(title);
    progress.activateWindow();

    Folder *cf = projectFolder();
    folders.blockSignals(true);
    blockSignals(true);
    // rename project folder item
    FolderListItem *item = (FolderListItem *)folders.topLevelItem(0);
    item->setText(0, fi.baseName());
    item->folder()->setName(fi.baseName());

    // process tables and matrix information
    while (!t.atEnd() && !progress.wasCanceled()) {
        s = t.readLine(4096); // workaround for safely reading very big lines
        list.clear();
        if (s.left(8) == "<folder>") {
            list = s.split("\t");
            Folder &f = current_folder->addChild<Folder>(list[1]);
            f.setBirthDate(list[2]);
            f.setModificationDate(list[3]);
            if (list.count() > 4)
                if (list[4] == "current")
                    cf = &f;

            FolderListItem *fli = new FolderListItem(current_folder->folderListItem(), &f);
            fli->setText(0, list[1]);
            f.setFolderListItem(fli);

            current_folder = &f;
        } else if (s == "<table>") {
            title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
            progress.setLabelText(title);

            openTable(this, t);
            progress.setValue(aux);
        } else if (s.left(17) == "<TableStatistics>") {
            QStringList lst;
            while (s != "</TableStatistics>") {
                s = t.readLine();
                lst << s;
            }
            lst.pop_back();
            openTableStatistics(lst);
        } else if (s == "<matrix>") {
            title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
            progress.setLabelText(title);
            QStringList lst;
            while (s != "</matrix>") {
                s = t.readLine();
                lst << s;
            }
            lst.pop_back();
            openMatrix(this, lst);
            progress.setValue(aux);
        } else if (s == "<note>") {
            title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
            progress.setLabelText(title);
            for (int i = 0; i < 3; i++) {
                s = t.readLine();
                list << s;
            }
            Note *m = openNote(this, list);
            QStringList cont;
            while (s != "</note>") {
                s = t.readLine();
                cont << s;
            }
            cont.pop_back();
            m->restore(cont);
            progress.setValue(aux);
        } else if (s == "</folder>") {
            Folder *parent = (Folder *)current_folder->parent();
            if (!parent)
                current_folder = projectFolder();
            else
                current_folder = parent;
        }
    }

    if (progress.wasCanceled()) {
        saved = true;
        close();
        return false;
    }

    // process the rest
    t.seek(0);

    MultiLayer *plot = 0;
    while (!t.atEnd() && !progress.wasCanceled()) {
        s = t.readLine(4096); // workaround for safely reading very big lines
        if (s.left(8) == "<folder>") {
            list = s.split("\t");
            current_folder = current_folder->findSubfolder(list[1]);
        } else if (s == "<multiLayer>") { // process multilayers information
            title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
            progress.setLabelText(title);

            s = t.readLine();
            QStringList graph = s.split("\t");
            QString caption = graph[0];
            plot = multilayerPlot(caption);
            plot->setCols(graph[1].toInt());
            plot->setRows(graph[2].toInt());

            setListViewDate(caption, graph[3]);
            plot->setBirthDate(graph[3]);

            restoreWindowGeometry(this, plot, t.readLine());
            plot->blockSignals(true);

            if (d_file_version > 71) {
                QStringList lst = t.readLine().split("\t");
                plot->setWindowLabel(lst[1]);
                setListViewLabel(plot->name(), lst[1]);
                if (lst.length() > 2) {
                    plot->setCaptionPolicy((MyWidget::CaptionPolicy)lst[2].toInt());
                } else {
                    QMessageBox::warning(this, tr("File opening error"),
                                         tr("Invalid WindowLabel line:\n'%1'\nin file %2.")
                                                 .arg(lst.join(" "))
                                                 .arg(fn));
                    plot->setCaptionPolicy(MyWidget::CaptionPolicy::Name);
                    // Partial fix for sf #403
                    t.readLine();
                }
            }
            if (d_file_version > 83) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                QStringList lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                plot->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
                lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                plot->setSpacing(lst[1].toInt(), lst[2].toInt());
                lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                plot->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                plot->setAlignement(lst[1].toInt(), lst[2].toInt());
#else
                QStringList lst = t.readLine().split("\t", QString::SkipEmptyParts);
                plot->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
                lst = t.readLine().split("\t", QString::SkipEmptyParts);
                plot->setSpacing(lst[1].toInt(), lst[2].toInt());
                lst = t.readLine().split("\t", QString::SkipEmptyParts);
                plot->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                lst = t.readLine().split("\t", QString::SkipEmptyParts);
                plot->setAlignement(lst[1].toInt(), lst[2].toInt());
#endif
            }

            while (s != "</multiLayer>") { // open layers
                s = t.readLine();
                if (s.left(7) == "<graph>") {
                    list.clear();
                    while (s != "</graph>") {
                        s = t.readLine();
                        list << s;
                    }
                    openGraph(this, plot, list);
                }
            }
            plot->blockSignals(false);
            activateSubWindow(plot);
            progress.setValue(aux);
        } else if (s == "<SurfacePlot>") { // process 3D plots information
            list.clear();
            title = titleBase + QString::number(++aux) + "/" + QString::number(widgets);
            progress.setLabelText(title);
            while (s != "</SurfacePlot>") {
                s = t.readLine();
                list << s;
            }
            openSurfacePlot(this, list);
            progress.setValue(aux);
        } else if (s == "</folder>") {
            Folder *parent = (Folder *)current_folder->parent();
            if (!parent)
                current_folder = projectFolder();
            else
                current_folder = parent;
        } else if (s.left(5) == "<log>") { // process analysis information
            s = t.readLine();
            while (s != "</log>") {
                logInfo += s + "\n";
                s = t.readLine();
            }
            results->setText(logInfo);
        }
    }

    if (progress.wasCanceled()) {
        saved = true;
        close();
        return false;
    }

    logInfo = logInfo.remove("</log>\n", Qt::CaseInsensitive);

    folders.setCurrentItem(cf->folderListItem());
    folders.blockSignals(false);
    // change folder to user defined current folder
    changeFolder(cf, true);

    blockSignals(false);
    renamedTables.clear();

    show();
    executeNotes();
    savedProject();

    recentProjects.removeAll(fn);
    recentProjects.push_front(fn);
    updateRecentProjectsList();

    return true;
}

ApplicationWindow *ApplicationWindow::openProject(const QString &fn)
{
    unique_ptr<ApplicationWindow> app(new ApplicationWindow);
    app->applyUserSettings();
    return app->loadProject(fn) ? app.release() : nullptr;
}

void ApplicationWindow::executeNotes()
{
    QList<MyWidget *> lst = projectFolder()->windowsList();
    foreach (MyWidget *widget, lst)
        if (widget->inherits("Note") && ((Note *)widget)->autoexec())
            ((Note *)widget)->executeAll();
}

void ApplicationWindow::scriptError(const QString &message, const QString &scriptName,
                                    int lineNumber)
{
    Q_UNUSED(scriptName)
    Q_UNUSED(lineNumber)
    QMessageBox::critical(this, tr("Makhber") + " - " + tr("Script Error"), message);
}

void ApplicationWindow::scriptPrint(const QString &text)
{
#ifdef SCRIPTING_CONSOLE
    if (!text.isEmpty())
        console.insertPlainText(text);
#else
    printf(text.toUtf8().constData());
#endif
}

bool ApplicationWindow::setScriptingLang(const QString &lang, bool force, bool batch)
{
    if (!force && lang == scriptEnv->objectName())
        return true;
    if (lang.isEmpty())
        return false;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    ScriptingEnv *newEnv = ScriptingLangManager::newEnv(lang.toStdString(), this, batch);
    if (!newEnv) {
        QApplication::restoreOverrideCursor();
        return false;
    }

    connect(newEnv, SIGNAL(error(const QString &, const QString &, int)), this,
            SLOT(scriptError(const QString &, const QString &, int)));
    connect(newEnv, SIGNAL(print(const QString &)), this, SLOT(scriptPrint(const QString &)));
    if (!newEnv->initialize()) {
        QApplication::restoreOverrideCursor();
        return false;
    }

    // notify everyone who might be interested
    ScriptingChangeEvent sce(newEnv);
    QApplication::sendEvent(this, &sce);

    foreach (QObject *i, findChildren<QWidget *>())
        QApplication::postEvent(i, new ScriptingChangeEvent(newEnv));

    QApplication::restoreOverrideCursor();

    return true;
}

void ApplicationWindow::showScriptingLangDialog()
{
    ScriptingLangDialog *d = new ScriptingLangDialog(scriptEnv, this);
    d->showNormal();
    d->activateWindow();
}

void ApplicationWindow::restartScriptingEnv()
{
    if (setScriptingLang(scriptEnv->objectName(), true))
        executeNotes();
    else
        QMessageBox::critical(
                this, tr("Scripting Error"),
                tr("Scripting language \"%1\" failed to initialize.").arg(scriptEnv->objectName()));
}

// TODO: rewrite the template system
void ApplicationWindow::openTemplate()
{
    QString filter = "Makhber/QtiPlot 2D Graph Template (*.qpt);;";
    filter += "Makhber/QtiPlot 3D Surface Template (*.qst);;";
    filter += "Makhber/QtiPlot Table Template (*.qtt);;";
    filter += "Makhber/QtiPlot Matrix Template (*.qmt)";

    QString fn = QFileDialog::getOpenFileName(this, tr("Open Template File"), templatesDir, filter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        templatesDir = fi.absolutePath();
        if (fn.contains(".qmt", Qt::CaseSensitive) || fn.contains(".qpt", Qt::CaseSensitive)
            || fn.contains(".qtt", Qt::CaseSensitive) || fn.contains(".qst", Qt::CaseSensitive)) {
            if (!fi.exists()) {
                QMessageBox::critical(this, tr("File opening error"),
                                      tr("The file: <b>%1</b> doesn't exist!").arg(fn));
                return;
            }
            QFile f(fn);
            QTextStream t(&f);
            t.setCodec(QTextCodec::codecForName("UTF-8"));
            f.open(QIODevice::ReadOnly);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList l = t.readLine().split(QRegExp("\\s"), Qt::SkipEmptyParts);
#else
            QStringList l = t.readLine().split(QRegExp("\\s"), QString::SkipEmptyParts);
#endif
            QString fileType = l[0];
            if ((fileType != "Makhber") && (fileType != "QtiPlot")) {
                QMessageBox::critical(
                        this, tr("File opening error"),
                        tr("The file: <b> %1 </b> was not created using Makhber!").arg(fn));
                return;
            }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList vl = l[1].split(".", Qt::SkipEmptyParts);
#else
            QStringList vl = l[1].split(".", QString::SkipEmptyParts);
#endif
            if (fileType == "QtiPlot") {
                d_file_version = 100 * (vl[0]).toInt() + 10 * (vl[1]).toInt() + (vl[2]).toInt();
                if (d_file_version > 90) {
                    QMessageBox::critical(this, tr("File opening error"),
                                          tr("Makhber does not support QtiPlot template files "
                                             "from versions later than 0.9.0.")
                                                  .arg(fn));
                    return;
                }
            } else
                d_file_version = ((vl[0]).toInt() << 16) + ((vl[1]).toInt() << 8) + (vl[2]).toInt();

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            MyWidget *w = 0;
            QString templateType;
            t >> templateType;

            if (templateType == "<SurfacePlot>") {
                t.skipWhiteSpace();
                QStringList lst;
                while (!t.atEnd())
                    lst << t.readLine();
                w = openSurfacePlot(this, lst);
                if (w)
                    ((Graph3D *)w)->clearData();
            } else {
                int rows, cols;
                t >> rows;
                t >> cols;
                t.skipWhiteSpace();
                QString geometry = t.readLine();

                if (templateType == "<multiLayer>") { // FIXME: workarounds for template
                    w = new MultiLayer("", &d_workspace, 0);
                    w->setAttribute(Qt::WA_DeleteOnClose);
                    QString label = generateUniqueName(tr("Graph"));
                    initBareMultilayerPlot((MultiLayer *)w, label.replace(QRegExp("_"), "-"));
                    if (w) {
                        ((MultiLayer *)w)->setCols(cols);
                        ((MultiLayer *)w)->setRows(rows);
                        restoreWindowGeometry(this, w, geometry);
                        if (d_file_version > 83) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                            QStringList lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                            ((MultiLayer *)w)
                                    ->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(),
                                                 lst[4].toInt());
                            lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                            ((MultiLayer *)w)->setSpacing(lst[1].toInt(), lst[2].toInt());
                            lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                            ((MultiLayer *)w)->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                            lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                            ((MultiLayer *)w)->setAlignement(lst[1].toInt(), lst[2].toInt());
#else
                            QStringList lst = t.readLine().split("\t", QString::SkipEmptyParts);
                            ((MultiLayer *)w)
                                    ->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(),
                                                 lst[4].toInt());
                            lst = t.readLine().split("\t", QString::SkipEmptyParts);
                            ((MultiLayer *)w)->setSpacing(lst[1].toInt(), lst[2].toInt());
                            lst = t.readLine().split("\t", QString::SkipEmptyParts);
                            ((MultiLayer *)w)->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                            lst = t.readLine().split("\t", QString::SkipEmptyParts);
                            ((MultiLayer *)w)->setAlignement(lst[1].toInt(), lst[2].toInt());
#endif
                        }
                        while (!t.atEnd()) { // open layers
                            QString s = t.readLine();
                            if (s.left(7) == "<graph>") {
                                QStringList lst;
                                while (s != "</graph>") {
                                    s = t.readLine();
                                    lst << s;
                                }
                                openGraph(this, (MultiLayer *)w, lst);
                            }
                        }
                    }
                } else {
                    if (templateType == "<table>")
                        w = newTable(tr("Table1"), rows, cols);
                    else if (templateType == "<matrix>")
                        w = newMatrix(rows, cols);
                    if (w) {
                        QStringList lst;
                        while (!t.atEnd())
                            lst << t.readLine();
                        w->restore(lst);
                        restoreWindowGeometry(this, w, geometry);
                    }
                }
            }

            f.close();
            if (w) {
                switch (w->status()) {
                case MyWidget::Maximized:
                    w->setMaximized();
                    break;
                case MyWidget::Minimized:
                    w->setMinimized();
                    break;
                case MyWidget::Hidden:
                    w->setHidden();
                    break;
                case MyWidget::Normal:
                    w->setNormal();
                    break;
                }
                customMenu((MyWidget *)w);
                customToolBars((MyWidget *)w);
            }
            QApplication::restoreOverrideCursor();
        } else {
            QMessageBox::critical(
                    this, tr("File opening error"),
                    tr("The file: <b>%1</b> is not a Makhber template file!").arg(fn));
            return;
        }
    }
}

void ApplicationWindow::readSettings()
{
    auto &settings = getSettings();
    /* ---------------- group General --------------- */
    settings.beginGroup("/General");
#ifdef SEARCH_FOR_UPDATES
    autoSearchUpdates = settings.value("/AutoSearchUpdates", false).toBool();
#endif
    appLanguage =
            settings.value("/Language", QLocale::system().name().section('_', 0, 0)).toString();
    show_windows_policy =
            (ShowWindowsPolicy)settings.value("/ShowWindowsPolicy", ApplicationWindow::ActiveFolder)
                    .toInt();

    recentProjects = settings.value("/RecentProjects").toStringList();
    // Follows an ugly hack added by Ion in order to fix Qt4 porting issues
    //(only needed on Windows due to a Qt bug?)
#ifdef Q_OS_WIN
    if (!recentProjects.isEmpty() && recentProjects[0].contains("^e"))
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        recentProjects = recentProjects[0].split("^e", Qt::SkipEmptyParts);
#else
        recentProjects = recentProjects[0].split("^e", QString::SkipEmptyParts);
#endif
    else if (recentProjects.count() == 1) {
        QString s = recentProjects[0];
        if (s.remove(QRegExp("\\s")).isEmpty())
            recentProjects = QStringList();
    }
#endif

    updateRecentProjectsList();

    changeAppStyle(settings.value("/Style", appStyle).toString());
    undoLimit = settings.value("/UndoLimit", 10).toInt();
    d_project->undoStack()->setUndoLimit(undoLimit);
    autoSave = settings.value("/AutoSave", true).toBool();
    autoSaveTime = settings.value("/AutoSaveTime", 15).toInt();
    defaultScriptingLang = settings.value("/ScriptingLang", "muParser").toString();

    QLocale temp_locale = QLocale(settings.value("/Locale", QLocale::system().name()).toString());
    bool usegl = settings.value("/LocaleUseGroupSeparator", true).toBool();
    if (usegl)
        temp_locale.setNumberOptions(temp_locale.numberOptions() & ~QLocale::OmitGroupSeparator);
    else
        temp_locale.setNumberOptions(temp_locale.numberOptions() | QLocale::OmitGroupSeparator);
    QLocale::setDefault(temp_locale);

    d_decimal_digits = settings.value("/DecimalDigits", 6).toInt();
    d_default_numeric_format = settings.value("/DefaultNumericFormat", 'g').toChar().toLatin1();

    // restore geometry of main window
    restoreGeometry(settings.value("/ProjectWindow/Geometry").toByteArray());

    // restore dock windows and tool bars
    restoreState(settings.value("/DockWindows").toByteArray());
    explorerSplitter->restoreState(settings.value("/ExplorerSplitter").toByteArray());

    QStringList applicationFont = settings.value("/Font").toStringList();
    if (applicationFont.size() == 4)
        appFont = QFont(applicationFont[0], applicationFont[1].toInt(), applicationFont[2].toInt(),
                        applicationFont[3].toInt());

    settings.beginGroup("/Dialogs");
    d_extended_open_dialog = settings.value("/ExtendedOpenDialog", true).toBool();
    d_extended_export_dialog = settings.value("/ExtendedExportDialog", true).toBool();
    d_extended_import_ASCII_dialog = settings.value("/ExtendedImportAsciiDialog", true).toBool();
    d_extended_plot_dialog =
            settings.value("/ExtendedPlotDialog", true).toBool(); // used by PlotDialog

    settings.beginGroup("/AddRemoveCurves");
    d_add_curves_dialog_size =
            QSize(settings.value("/Width", 700).toInt(), settings.value("/Height", 400).toInt());
    d_show_current_folder = settings.value("/ShowCurrentFolder", false).toBool();
    settings.endGroup(); // AddRemoveCurves Dialog
    settings.endGroup(); // Dialogs

    settings.beginGroup("/Colors");
    workspaceColor = QColor(COLORVALUE(settings.value("/Workspace", "darkGray").toString()));
    // see http://doc.trolltech.com/4.2/qvariant.html for instructions on qcolor <-> qvariant
    // conversion
    panelsColor = QColor(COLORVALUE(settings.value("/Panels", "#ffffffff").toString()));
    panelsTextColor = QColor(COLORVALUE(settings.value("/PanelsText", "#ff000000").toString()));
    settings.endGroup(); // Colors

    settings.beginGroup("/Paths");
    workingDir = settings.value("/WorkingDir", qApp->applicationDirPath()).toString();
    helpFilePath = settings.value("/HelpFile", "").toString();
#ifdef PLUGIN_PATH
    QString defaultFitPluginsPath = qApp->applicationDirPath() + PLUGIN_PATH;
#else // defined PLUGIN_PATH
#ifdef Q_OS_WIN
    QString defaultFitPluginsPath = qApp->applicationDirPath() + "fitPlugins";
#else // defined Q_OS_WIN
    QString defaultFitPluginsPath = "/usr/lib/makhber/plugins";
#endif
#endif // defined PLUGIN_PATH
#ifdef DYNAMIC_PLUGIN_PATH
    fitPluginsPath = settings.value("/FitPlugins", defaultFitPluginsPath).toString();
#else // defined DYNAMIC_PLUGIN_PATH
    fitPluginsPath = defaultFitPluginsPath;
#endif

#ifdef Q_OS_WIN
    templatesDir = settings.value("/TemplatesDir", qApp->applicationDirPath()).toString();
    asciiDirPath = settings.value("/ASCII", qApp->applicationDirPath()).toString();
    imagesDirPath = settings.value("/Images", qApp->applicationDirPath()).toString();
#else
    templatesDir = settings.value("/TemplatesDir", QDir::homePath()).toString();
    asciiDirPath = settings.value("/ASCII", QDir::homePath()).toString();
    imagesDirPath = settings.value("/Images", QDir::homePath()).toString();
#endif
    locktoolbar->setChecked(settings.value("LockToolbars", false).toBool());
    settings.endGroup(); // Paths
    settings.endGroup();
    /* ------------- end group General ------------------- */

    settings.beginGroup("/UserFunctions");
    fitFunctions = settings.value("/FitFunctions").toStringList();
    surfaceFunc = settings.value("/SurfaceFunctions").toStringList();
    xFunctions = settings.value("/xFunctions").toStringList();
    yFunctions = settings.value("/yFunctions").toStringList();
    rFunctions = settings.value("/rFunctions").toStringList();
    thetaFunctions = settings.value("/thetaFunctions").toStringList();
    settings.endGroup(); // UserFunctions

    settings.beginGroup("/Confirmations");
    confirmCloseFolder = settings.value("/Folder", true).toBool();
    confirmCloseTable = settings.value("/Table", true).toBool();
    confirmCloseMatrix = settings.value("/Matrix", true).toBool();
    confirmClosePlot2D = settings.value("/Plot2D", true).toBool();
    confirmClosePlot3D = settings.value("/Plot3D", true).toBool();
    confirmCloseNotes = settings.value("/Note", true).toBool();
    settings.endGroup(); // Confirmations

    /* ---------------- group Tables --------------- */
    settings.beginGroup("/Tables");
    d_show_table_comments = settings.value("/DisplayComments", false).toBool();
    QStringList tableFonts = settings.value("/Fonts").toStringList();
    if (tableFonts.size() == 8) {
        tableTextFont = QFont(tableFonts[0], tableFonts[1].toInt(), tableFonts[2].toInt(),
                              tableFonts[3].toInt());
        tableHeaderFont = QFont(tableFonts[4], tableFonts[5].toInt(), tableFonts[6].toInt(),
                                tableFonts[7].toInt());
    }

    settings.beginGroup("/Colors");
    tableBkgdColor = QColor(COLORVALUE(settings.value("/Background", "#ffffffff").toString()));
    tableTextColor = QColor(COLORVALUE(settings.value("/Text", "#ff000000").toString()));
    tableHeaderColor = QColor(COLORVALUE(settings.value("/Header", "#ff000000").toString()));
    settings.endGroup(); // Colors
    settings.endGroup();
    /* --------------- end group Tables ------------------------ */

    /* --------------- group 2D Plots ----------------------------- */
    settings.beginGroup("/2DPlots");
    settings.beginGroup("/General");
    titleOn = settings.value("/Title", true).toBool();
    allAxesOn = settings.value("/AllAxes", false).toBool();
    canvasFrameOn = settings.value("/CanvasFrame", false).toBool();
    canvasFrameWidth = settings.value("/CanvasFrameWidth", 0).toInt();
    defaultPlotMargin = settings.value("/Margin", 0).toInt();
    drawBackbones = settings.value("/AxesBackbones", true).toBool();
    axesLineWidth = settings.value("/AxesLineWidth", 1).toInt();
    autoscale2DPlots = settings.value("/Autoscale", true).toBool();
    autoScaleFonts = settings.value("/AutoScaleFonts", true).toBool();
    autoResizeLayers = settings.value("/AutoResizeLayers", true).toBool();
    antialiasing2DPlots = settings.value("/Antialiasing", true).toBool();
    d_scale_plots_on_print = settings.value("/ScaleLayersOnPrint", false).toBool();
    d_print_cropmarks = settings.value("/PrintCropmarks", false).toBool();

    QStringList graphFonts = settings.value("/Fonts").toStringList();
    if (graphFonts.size() == 16) {
        plotAxesFont = QFont(graphFonts[0], graphFonts[1].toInt(), graphFonts[2].toInt(),
                             graphFonts[3].toInt());
        plotNumbersFont = QFont(graphFonts[4], graphFonts[5].toInt(), graphFonts[6].toInt(),
                                graphFonts[7].toInt());
        plotLegendFont = QFont(graphFonts[8], graphFonts[9].toInt(), graphFonts[10].toInt(),
                               graphFonts[11].toInt());
        plotTitleFont = QFont(graphFonts[12], graphFonts[13].toInt(), graphFonts[14].toInt(),
                              graphFonts[15].toInt());
    }
    settings.endGroup(); // General

    settings.beginGroup("/Curves");
    defaultCurveStyle = settings.value("/Style", Graph::LineSymbols).toInt();
    defaultCurveLineWidth = settings.value("/LineWidth", 1).toInt();
    defaultSymbolSize = settings.value("/SymbolSize", 7).toInt();
    settings.endGroup(); // Curves

    settings.beginGroup("/Ticks");
    majTicksStyle = settings.value("/MajTicksStyle", ScaleDraw::Out).toInt();
    minTicksStyle = settings.value("/MinTicksStyle", ScaleDraw::Out).toInt();
    minTicksLength = settings.value("/MinTicksLength", 5).toInt();
    majTicksLength = settings.value("/MajTicksLength", 9).toInt();
    settings.endGroup(); // Ticks

    settings.beginGroup("/Legend");
    legendFrameStyle = settings.value("/FrameStyle", Legend::Line).toInt();
    legendTextColor = QColor(COLORVALUE(
            settings.value("/TextColor", "#ff000000").toString())); // default color Qt::black
    legendBackground = QColor(COLORVALUE(
            settings.value("/BackgroundColor", "#ffffffff").toString())); // default color Qt::white
    legendBackground.setAlpha(
            settings.value("/Transparency", 0).toInt()); // transparent by default;
    settings.endGroup(); // Legend

    settings.beginGroup("/Arrows");
    defaultArrowLineWidth = settings.value("/Width", 1).toInt();
    defaultArrowColor = QColor(COLORVALUE(
            settings.value("/Color", "#ff000000").toString())); // default color Qt::black
    defaultArrowHeadLength = settings.value("/HeadLength", 4).toInt();
    defaultArrowHeadAngle = settings.value("/HeadAngle", 45).toInt();
    defaultArrowHeadFill = settings.value("/HeadFill", true).toBool();
    defaultArrowLineStyle =
            Graph::getPenStyle(settings.value("/LineStyle", "SolidLine").toString());
    settings.endGroup(); // Arrows
    settings.endGroup();
    /* ----------------- end group 2D Plots --------------------------- */

    /* ----------------- group 3D Plots --------------------------- */
    settings.beginGroup("/3DPlots");
    showPlot3DLegend = settings.value("/Legend", true).toBool();
    showPlot3DProjection = settings.value("/Projection", false).toBool();
    smooth3DMesh = settings.value("/Antialiasing", true).toBool();
    plot3DResolution = settings.value("/Resolution", 1).toInt();
    orthogonal3DPlots = settings.value("/Orthogonal", false).toBool();
    autoscale3DPlots = settings.value("/Autoscale", true).toBool();

    QStringList plot3DFonts = settings.value("/Fonts").toStringList();
    if (plot3DFonts.size() == 12) {
        plot3DTitleFont = QFont(plot3DFonts[0], plot3DFonts[1].toInt(), plot3DFonts[2].toInt(),
                                plot3DFonts[3].toInt());
        plot3DNumbersFont = QFont(plot3DFonts[4], plot3DFonts[5].toInt(), plot3DFonts[6].toInt(),
                                  plot3DFonts[7].toInt());
        plot3DAxesFont = QFont(plot3DFonts[8], plot3DFonts[9].toInt(), plot3DFonts[10].toInt(),
                               plot3DFonts[11].toInt());
    }

    settings.beginGroup("/Colors");
    plot3DColors << settings.value("/MaxData", "blue").toString();
    plot3DColors << settings.value("/Labels", "#000000").toString();
    plot3DColors << settings.value("/Mesh", "#000000").toString();
    plot3DColors << settings.value("/Grid", "#000000").toString();
    plot3DColors << settings.value("/MinData", "red").toString();
    plot3DColors << settings.value("/Numbers", "#000000").toString();
    plot3DColors << settings.value("/Axes", "#000000").toString();
    plot3DColors << settings.value("/Background", "#ffffff").toString();
    settings.endGroup(); // Colors
    settings.endGroup();
    /* ----------------- end group 3D Plots --------------------------- */

    settings.beginGroup("/Fitting");
    fit_output_precision = settings.value("/OutputPrecision", 15).toInt();
    pasteFitResultsToPlot = settings.value("/PasteResultsToPlot", false).toBool();
    writeFitResultsToLog = settings.value("/WriteResultsToLog", true).toBool();
    generateUniformFitPoints = settings.value("/GenerateFunction", true).toBool();
    fitPoints = settings.value("/Points", 100).toInt();
    generatePeakCurves = settings.value("/GeneratePeakCurves", true).toBool();
    peakCurvesColor = QColor(
            COLORVALUE(settings.value("/PeaksColor", "#ff00ff00").toString())); // green color
    fit_scale_errors = settings.value("/ScaleErrors", false).toBool();
    d_2_linear_fit_points = settings.value("/TwoPointsLinearFit", true).toBool();
    settings.endGroup(); // Fitting

    settings.beginGroup("/ImportASCII");
    columnSeparator = settings.value("/ColumnSeparator", "\\t").toString();
    columnSeparator.replace("\\t", "\t").replace("\\s", " ");
    ignoredLines = settings.value("/IgnoreLines", 0).toInt();
    renameColumns = settings.value("/RenameColumns", true).toBool();
    strip_spaces = settings.value("/StripSpaces", false).toBool();
    simplify_spaces = settings.value("/SimplifySpaces", false).toBool();
    d_ASCII_file_filter = settings.value("/AsciiFileTypeFilter", "*").toString();
    d_ASCII_import_locale = settings.value("/AsciiImportLocale", "C").toString();
    d_convert_to_numeric = settings.value("/ConvertToNumeric", true).toBool();
    settings.endGroup(); // Import ASCII

    settings.beginGroup("/ExportImage");
    d_image_export_filter = settings.value("/ImageFileTypeFilter", ".png").toString();
    d_export_transparency = settings.value("/ExportTransparency", false).toBool();
    d_export_quality = settings.value("/ImageQuality", 100).toInt();
    d_export_resolution = settings.value("/Resolution", 72).toInt();
    d_export_color = settings.value("/ExportColor", true).toBool();
    d_export_vector_size = settings.value("/ExportPageSize", QPrinter::Custom).toInt();
    d_keep_plot_aspect = settings.value("/KeepAspect", true).toBool();
    d_export_orientation = settings.value("/Orientation", QPrinter::Landscape).toInt();
    settings.endGroup(); // ExportImage
}

void ApplicationWindow::saveSettings()
{
    auto &settings = getSettings();
    /* ---------------- group General --------------- */
    settings.beginGroup("/General");
#ifdef SEARCH_FOR_UPDATES
    settings.setValue("/AutoSearchUpdates", autoSearchUpdates);
#endif
    settings.setValue("/Language", appLanguage);
    settings.setValue("/ShowWindowsPolicy", show_windows_policy);
    settings.setValue("/RecentProjects", recentProjects);
    settings.setValue("/Style", appStyle);
    settings.setValue("/AutoSave", autoSave);
    settings.setValue("/AutoSaveTime", autoSaveTime);
    settings.setValue("/UndoLimit", undoLimit);
    settings.setValue("/ScriptingLang", defaultScriptingLang);
    settings.setValue("/Locale", QLocale().name());
    settings.setValue("/LocaleUseGroupSeparator",
                      bool(!(QLocale().numberOptions() & QLocale::OmitGroupSeparator)));
    settings.setValue("/DecimalDigits", d_decimal_digits);
    settings.setValue("/DefaultNumericFormat", QChar(d_default_numeric_format));

    settings.setValue("/ProjectWindow/Geometry", saveGeometry());
    settings.setValue("/DockWindows", saveState());
    settings.setValue("/ExplorerSplitter", explorerSplitter->saveState());

    QStringList applicationFont;
    applicationFont << appFont.family();
    applicationFont << QString::number(appFont.pointSize());
    applicationFont << QString::number(appFont.weight());
    applicationFont << QString::number(appFont.italic());
    settings.setValue("/Font", applicationFont);

    settings.beginGroup("/Dialogs");
    settings.setValue("/ExtendedOpenDialog", d_extended_open_dialog);
    settings.setValue("/ExtendedExportDialog", d_extended_export_dialog);
    settings.setValue("/ExtendedImportAsciiDialog", d_extended_import_ASCII_dialog);
    settings.setValue("/ExtendedPlotDialog", d_extended_plot_dialog);
    settings.beginGroup("/AddRemoveCurves");
    settings.setValue("/Width", d_add_curves_dialog_size.width());
    settings.setValue("/Height", d_add_curves_dialog_size.height());
    settings.setValue("/ShowCurrentFolder", d_show_current_folder);
    settings.endGroup(); // AddRemoveCurves Dialog
    settings.endGroup(); // Dialogs

    settings.beginGroup("/Colors");
    settings.setValue("/Workspace", COLORNAME(workspaceColor));
    settings.setValue("/Panels", COLORNAME(panelsColor));
    settings.setValue("/PanelsText", COLORNAME(panelsTextColor));
    settings.endGroup(); // Colors

    settings.beginGroup("/Paths");
    settings.setValue("/WorkingDir", workingDir);
    settings.setValue("/TemplatesDir", templatesDir);
    settings.setValue("/HelpFile", helpFilePath);
    settings.setValue("/FitPlugins", fitPluginsPath);
    settings.setValue("/ASCII", asciiDirPath);
    settings.setValue("/Images", imagesDirPath);

    settings.setValue("LockToolbars", locktoolbar->isChecked());

    settings.endGroup(); // Paths
    settings.endGroup();
    /* ---------------- end group General --------------- */

    settings.beginGroup("/UserFunctions");
    settings.setValue("/FitFunctions", fitFunctions);
    settings.setValue("/SurfaceFunctions", surfaceFunc);
    settings.setValue("/xFunctions", xFunctions);
    settings.setValue("/yFunctions", yFunctions);
    settings.setValue("/rFunctions", rFunctions);
    settings.setValue("/thetaFunctions", thetaFunctions);
    settings.endGroup(); // UserFunctions

    settings.beginGroup("/Confirmations");
    settings.setValue("/Folder", confirmCloseFolder);
    settings.setValue("/Table", confirmCloseTable);
    settings.setValue("/Matrix", confirmCloseMatrix);
    settings.setValue("/Plot2D", confirmClosePlot2D);
    settings.setValue("/Plot3D", confirmClosePlot3D);
    settings.setValue("/Note", confirmCloseNotes);
    settings.endGroup(); // Confirmations

    /* ----------------- group Tables -------------- */
    settings.beginGroup("/Tables");
    settings.setValue("/DisplayComments", d_show_table_comments);
    QStringList tableFonts;
    tableFonts << tableTextFont.family();
    tableFonts << QString::number(tableTextFont.pointSize());
    tableFonts << QString::number(tableTextFont.weight());
    tableFonts << QString::number(tableTextFont.italic());
    tableFonts << tableHeaderFont.family();
    tableFonts << QString::number(tableHeaderFont.pointSize());
    tableFonts << QString::number(tableHeaderFont.weight());
    tableFonts << QString::number(tableHeaderFont.italic());
    settings.setValue("/Fonts", tableFonts);

    settings.beginGroup("/Colors");
    settings.setValue("/Background", COLORNAME(tableBkgdColor));
    settings.setValue("/Text", COLORNAME(tableTextColor));
    settings.setValue("/Header", COLORNAME(tableHeaderColor));
    settings.endGroup(); // Colors
    settings.endGroup();
    /* ----------------- end group Tables ---------- */

    /* ----------------- group 2D Plots ------------ */
    settings.beginGroup("/2DPlots");
    settings.beginGroup("/General");
    settings.setValue("/Title", titleOn);
    settings.setValue("/AllAxes", allAxesOn);
    settings.setValue("/CanvasFrame", canvasFrameOn);
    settings.setValue("/CanvasFrameWidth", canvasFrameWidth);
    settings.setValue("/Margin", defaultPlotMargin);
    settings.setValue("/AxesBackbones", drawBackbones);
    settings.setValue("/AxesLineWidth", axesLineWidth);
    settings.setValue("/Autoscale", autoscale2DPlots);
    settings.setValue("/AutoScaleFonts", autoScaleFonts);
    settings.setValue("/AutoResizeLayers", autoResizeLayers);
    settings.setValue("/Antialiasing", antialiasing2DPlots);
    settings.setValue("/ScaleLayersOnPrint", d_scale_plots_on_print);
    settings.setValue("/PrintCropmarks", d_print_cropmarks);

    QStringList graphFonts;
    graphFonts << plotAxesFont.family();
    graphFonts << QString::number(plotAxesFont.pointSize());
    graphFonts << QString::number(plotAxesFont.weight());
    graphFonts << QString::number(plotAxesFont.italic());
    graphFonts << plotNumbersFont.family();
    graphFonts << QString::number(plotNumbersFont.pointSize());
    graphFonts << QString::number(plotNumbersFont.weight());
    graphFonts << QString::number(plotNumbersFont.italic());
    graphFonts << plotLegendFont.family();
    graphFonts << QString::number(plotLegendFont.pointSize());
    graphFonts << QString::number(plotLegendFont.weight());
    graphFonts << QString::number(plotLegendFont.italic());
    graphFonts << plotTitleFont.family();
    graphFonts << QString::number(plotTitleFont.pointSize());
    graphFonts << QString::number(plotTitleFont.weight());
    graphFonts << QString::number(plotTitleFont.italic());
    settings.setValue("/Fonts", graphFonts);
    settings.endGroup(); // General

    settings.beginGroup("/Curves");
    settings.setValue("/Style", defaultCurveStyle);
    settings.setValue("/LineWidth", defaultCurveLineWidth);
    settings.setValue("/SymbolSize", defaultSymbolSize);
    settings.endGroup(); // Curves

    settings.beginGroup("/Ticks");
    settings.setValue("/MajTicksStyle", majTicksStyle);
    settings.setValue("/MinTicksStyle", minTicksStyle);
    settings.setValue("/MinTicksLength", minTicksLength);
    settings.setValue("/MajTicksLength", majTicksLength);
    settings.endGroup(); // Ticks

    settings.beginGroup("/Legend");
    settings.setValue("/FrameStyle", legendFrameStyle);
    settings.setValue("/TextColor", COLORNAME(legendTextColor));
    settings.setValue("/BackgroundColor", COLORNAME(legendBackground));
    settings.setValue("/Transparency", legendBackground.alpha());
    settings.endGroup(); // Legend

    settings.beginGroup("/Arrows");
    settings.setValue("/Width", defaultArrowLineWidth);
    settings.setValue("/Color", COLORNAME(defaultArrowColor));
    settings.setValue("/HeadLength", defaultArrowHeadLength);
    settings.setValue("/HeadAngle", defaultArrowHeadAngle);
    settings.setValue("/HeadFill", defaultArrowHeadFill);
    settings.setValue("/LineStyle", Graph::penStyleName(defaultArrowLineStyle));
    settings.endGroup(); // Arrows
    settings.endGroup();
    /* ----------------- end group 2D Plots -------- */

    /* ----------------- group 3D Plots ------------ */
    settings.beginGroup("/3DPlots");
    settings.setValue("/Legend", showPlot3DLegend);
    settings.setValue("/Projection", showPlot3DProjection);
    settings.setValue("/Antialiasing", smooth3DMesh);
    settings.setValue("/Resolution", plot3DResolution);
    settings.setValue("/Orthogonal", orthogonal3DPlots);
    settings.setValue("/Autoscale", autoscale3DPlots);

    QStringList plot3DFonts;
    plot3DFonts << plot3DTitleFont.family();
    plot3DFonts << QString::number(plot3DTitleFont.pointSize());
    plot3DFonts << QString::number(plot3DTitleFont.weight());
    plot3DFonts << QString::number(plot3DTitleFont.italic());
    plot3DFonts << plot3DNumbersFont.family();
    plot3DFonts << QString::number(plot3DNumbersFont.pointSize());
    plot3DFonts << QString::number(plot3DNumbersFont.weight());
    plot3DFonts << QString::number(plot3DNumbersFont.italic());
    plot3DFonts << plot3DAxesFont.family();
    plot3DFonts << QString::number(plot3DAxesFont.pointSize());
    plot3DFonts << QString::number(plot3DAxesFont.weight());
    plot3DFonts << QString::number(plot3DAxesFont.italic());
    settings.setValue("/Fonts", plot3DFonts);

    settings.beginGroup("/Colors");
    settings.setValue("/MaxData", plot3DColors[0]);
    settings.setValue("/Labels", plot3DColors[1]);
    settings.setValue("/Mesh", plot3DColors[2]);
    settings.setValue("/Grid", plot3DColors[3]);
    settings.setValue("/MinData", plot3DColors[4]);
    settings.setValue("/Numbers", plot3DColors[5]);
    settings.setValue("/Axes", plot3DColors[6]);
    settings.setValue("/Background", plot3DColors[7]);
    settings.endGroup(); // Colors
    settings.endGroup();
    /* ----------------- end group 2D Plots -------- */

    settings.beginGroup("/Fitting");
    settings.setValue("/OutputPrecision", fit_output_precision);
    settings.setValue("/PasteResultsToPlot", pasteFitResultsToPlot);
    settings.setValue("/WriteResultsToLog", writeFitResultsToLog);
    settings.setValue("/GenerateFunction", generateUniformFitPoints);
    settings.setValue("/Points", fitPoints);
    settings.setValue("/GeneratePeakCurves", generatePeakCurves);
    settings.setValue("/PeaksColor", COLORNAME(peakCurvesColor));
    settings.setValue("/ScaleErrors", fit_scale_errors);
    settings.setValue("/TwoPointsLinearFit", d_2_linear_fit_points);
    settings.endGroup(); // Fitting

    settings.beginGroup("/ImportASCII");
    QString sep = columnSeparator;
    settings.setValue("/ColumnSeparator", sep.replace("\t", "\\t").replace(" ", "\\s"));
    settings.setValue("/IgnoreLines", ignoredLines);
    settings.setValue("/RenameColumns", renameColumns);
    settings.setValue("/StripSpaces", strip_spaces);
    settings.setValue("/SimplifySpaces", simplify_spaces);
    settings.setValue("/AsciiFileTypeFilter", d_ASCII_file_filter);
    settings.setValue("/AsciiImportLocale", d_ASCII_import_locale.name());
    settings.setValue("/ConvertToNumeric", d_convert_to_numeric);
    settings.endGroup(); // ImportASCII

    settings.beginGroup("/ExportImage");
    settings.setValue("/ImageFileTypeFilter", d_image_export_filter);
    settings.setValue("/ExportTransparency", d_export_transparency);
    settings.setValue("/ImageQuality", d_export_quality);
    settings.setValue("/Resolution", d_export_resolution);
    settings.setValue("/ExportColor", d_export_color);
    settings.setValue("/ExportPageSize", d_export_vector_size);
    settings.setValue("/KeepAspect", d_keep_plot_aspect);
    settings.setValue("/Orientation", d_export_orientation);
    settings.endGroup(); // ExportImage
}

void ApplicationWindow::exportGraph()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    MultiLayer *plot2D = 0;
    Graph3D *plot3D = 0;
    if (w->inherits("MultiLayer")) {
        plot2D = (MultiLayer *)w;
        if (plot2D->isEmpty()) {
            QMessageBox::critical(
                    this, tr("Export Error"),
                    tr("<h4>There are no plot layers available in this window!</h4>"));
            return;
        }
    } else if (w->inherits("Graph3D"))
        plot3D = (Graph3D *)w;
    else
        return;

    ImageExportDialog *ied = new ImageExportDialog(this, plot2D != NULL, d_extended_export_dialog);
    ied->setDirectory(workingDir);
    ied->selectFilter(d_image_export_filter);
    if (ied->exec() != QDialog::Accepted)
        return;
    workingDir = ied->directory().path();
    if (ied->selectedFiles().isEmpty())
        return;

    QString selected_filter = ied->selectedNameFilter();
    QString file_name = ied->selectedFiles()[0];
    QFileInfo file_info(file_name);
    if (!file_info.fileName().contains("."))
        file_name.append(selected_filter.remove("*"));

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Export Error"),
                              tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that "
                                 "you have the right to write to this location!")
                                      .arg(file_name));
        return;
    }

    if (selected_filter.contains(".eps") || selected_filter.contains(".pdf")
        || selected_filter.contains(".ps")) {
        if (plot3D)
            plot3D->exportVector(file_name, selected_filter.remove(QRegularExpression("\\.")));
        else if (plot2D)
            plot2D->exportVector(file_name, ied->resolution(), ied->color(), ied->keepAspect(),
                                 ied->pageSize(), ied->pageOrientation());
    } else if (selected_filter.contains(".svg")) {
        if (plot2D)
            plot2D->exportSVG(file_name);
        else
            plot3D->exportVector(file_name, "svg");
    } else {
        QList<QByteArray> list = QImageWriter::supportedImageFormats();
        for (int i = 0; i < (int)list.count(); i++) {
            if (selected_filter.contains("." + (list[i]).toLower())) {
                if (plot2D)
                    plot2D->exportImage(file_name, ied->quality());
                else if (plot3D)
                    plot3D->exportImage(file_name, ied->quality());
            }
        }
    }
}

void ApplicationWindow::exportLayer()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)w)->activeGraph();
    if (!g)
        return;

    ImageExportDialog *ied = new ImageExportDialog(this, g != NULL, d_extended_export_dialog);
    ied->setDirectory(workingDir);
    ied->selectFilter(d_image_export_filter);
    if (ied->exec() != QDialog::Accepted)
        return;
    workingDir = ied->directory().path();
    if (ied->selectedFiles().isEmpty())
        return;

    QString selected_filter = ied->selectedNameFilter();
    QString file_name = ied->selectedFiles()[0];
    QFileInfo file_info(file_name);
    if (!file_info.fileName().contains("."))
        file_name.append(selected_filter.remove("*"));

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Export Error"),
                              tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that "
                                 "you have the right to write to this location!")
                                      .arg(file_name));
        return;
    }

    if (selected_filter.contains(".eps") || selected_filter.contains(".pdf")
        || selected_filter.contains(".ps"))
        g->exportVector(file_name, ied->resolution(), ied->color(), ied->keepAspect(),
                        ied->pageSize(), ied->pageOrientation());
    else if (selected_filter.contains(".svg"))
        g->exportSVG(file_name);
    else {
        QList<QByteArray> list = QImageWriter::supportedImageFormats();
        for (int i = 0; i < (int)list.count(); i++)
            if (selected_filter.contains("." + (list[i]).toLower()))
                g->exportImage(file_name, ied->quality());
    }
}

void ApplicationWindow::exportAllGraphs()
{
    ImageExportDialog *ied = new ImageExportDialog(this, true, d_extended_export_dialog);
    ied->setWindowTitle(tr("Choose a directory to export the graphs to"));
    QStringList tmp = ied->nameFilters();
    ied->setFileMode(QFileDialog::Directory);
    ied->setNameFilters(tmp);
    ied->setLabelText(QFileDialog::FileType, tr("Output format:"));
    ied->setLabelText(QFileDialog::FileName, tr("Directory:"));

    ied->setDirectory(workingDir);
    ied->selectFilter(d_image_export_filter);

    if (ied->exec() != QDialog::Accepted)
        return;
    workingDir = ied->directory().path();
    if (ied->selectedFiles().isEmpty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString output_dir = ied->selectedFiles()[0];
    QString file_suffix = ied->selectedNameFilter();
    file_suffix = file_suffix.toLower();
    file_suffix.remove("*");

    QList<MyWidget *> windows = windowsList();
    bool confirm_overwrite = true;
    MultiLayer *plot2D;
    Graph3D *plot3D;

    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            plot3D = 0;
            plot2D = (MultiLayer *)w;
            if (plot2D->isEmpty()) {
                QApplication::restoreOverrideCursor();
                QMessageBox::warning(
                        this, tr("Warning"),
                        tr("There are no plot layers available in window <b>%1</b>.<br>"
                           "Graph window not exported!")
                                .arg(plot2D->name()));
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                continue;
            }
        } else if (w->inherits("Graph3D")) {
            plot2D = 0;
            plot3D = (Graph3D *)w;
        } else
            continue;

        QString file_name = output_dir + "/" + w->objectName() + file_suffix;
        QFile f(file_name);
        if (f.exists() && confirm_overwrite) {
            QApplication::restoreOverrideCursor();
            switch (QMessageBox::question(this, tr("Overwrite file?"),
                                          tr("A file called: <p><b>%1</b><p>already exists. "
                                             "Do you want to overwrite it?")
                                                  .arg(file_name),
                                          tr("&Yes"), tr("&All"), tr("&Cancel"), 0, 1)) {
            case 1:
                confirm_overwrite = false;
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                break;
            case 0:
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                break;
            case 2:
                return;
            }
        }
        if (!f.open(QIODevice::WriteOnly)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(
                    this, tr("Export Error"),
                    tr("Could not write to file: <br><h4>%1</h4><p>"
                       "Please verify that you have the right to write to this location!")
                            .arg(file_name));
            return;
        }
        if (file_suffix.contains(".eps") || file_suffix.contains(".pdf")
            || file_suffix.contains(".ps")) {
            if (plot3D)
                plot3D->exportVector(file_name, file_suffix.remove("."));
            else if (plot2D)
                plot2D->exportVector(file_name, ied->resolution(), ied->color());
        } else if (file_suffix.contains(".svg")) {
            if (plot2D)
                plot2D->exportSVG(file_name);
        } else {
            QList<QByteArray> list = QImageWriter::supportedImageFormats();
            for (int i = 0; i < (int)list.count(); i++) {
                if (file_suffix.contains("." + (list[i]).toLower())) {
                    if (plot2D)
                        plot2D->exportImage(file_name, ied->quality());
                    else if (plot3D)
                        plot3D->exportImage(file_name, ied->quality());
                }
            }
        }
    }

    QApplication::restoreOverrideCursor();
}

QString ApplicationWindow::windowGeometryInfo(MyWidget *w)
{
    QString s = "geometry\t";
    if (w->status() == MyWidget::Maximized) {
        if (w == w->folder()->activeWindow())
            return s + "maximized\tactive\n";
        else
            return s + "maximized\n";
    }

    if (!w->parent())
        s += "0\t0\t500\t400\t";
    else {
        QPoint p = w->pos(); // store position
        s += QString::number(p.x()) + "\t";
        s += QString::number(p.y()) + "\t";
        s += QString::number(w->frameGeometry().width()) + "\t";
        s += QString::number(w->frameGeometry().height()) + "\t";
    }

    if (w->status() == MyWidget::Minimized)
        s += "minimized\t";

    bool hide = hidden(w);
    if (w == w->folder()->activeWindow() && !hide)
        s += "active\n";
    else if (hide)
        s += "hidden\n";
    else
        s += "\n";
    return s;
}

void ApplicationWindow::restoreWindowGeometry(ApplicationWindow *app, MyWidget *w, const QString s)
{
    w->blockSignals(true);
    QString caption = w->name();
    if (s.contains("minimized")) {
        QStringList lst = s.split("\t");
        if (lst.count() > 4)
            w->setGeometry(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
        w->setStatus(MyWidget::Minimized);
        app->setListView(caption, tr("Minimized"));
    } else if (s.contains("maximized")) {
        w->setStatus(MyWidget::Maximized);
        app->setListView(caption, tr("Maximized"));
    } else {
        QStringList lst = s.split("\t");
        w->setGeometry(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(), lst[4].toInt());
        w->setStatus(MyWidget::Normal);

        if (lst.count() > 5) {
            if (lst[5] == "hidden")
                app->hideWindow(w);
        }
    }

    if (s.contains("active")) {
        Folder *f = w->folder();
        if (f)
            f->setActiveWindow(w);
    }

    w->blockSignals(false);
}

Folder *ApplicationWindow::projectFolder()
{
    return ((FolderListItem *)folders.topLevelItem(0))->folder();
}

bool ApplicationWindow::saveProject()
{
    if (projectname == "untitled" || projectname.endsWith(".opj", Qt::CaseInsensitive)
        || projectname.endsWith(".ogm", Qt::CaseInsensitive)
        || projectname.endsWith(".ogw", Qt::CaseInsensitive)
        || projectname.endsWith(".ogg", Qt::CaseInsensitive)
        || projectname.endsWith(".org", Qt::CaseInsensitive)) {
        saveProjectAs();
        return false;
    }

    bool compress = false;
    QString fn = projectname;
    if (fn.endsWith(".gz")) {
        fn = fn.left(fn.length() - 3);
        compress = true;
    }

    saveFolder(projectFolder(), fn);

    if (compress)
        file_compress(QFile::encodeName(fn).constData(), "wb9");

    setWindowTitle("Makhber - " + projectname);
    savedProject();
    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);

    if (autoSave) {
        if (savingTimerId)
            killTimer(savingTimerId);
        savingTimerId = startTimer(autoSaveTime * 60000);
    } else
        savingTimerId = 0;

    QApplication::restoreOverrideCursor();
    return true;
}

void ApplicationWindow::saveProjectAs()
{
    QString filter = tr("Makhber project") + " (*.sciprj);;";
    filter += tr("Compressed Makhber project") + " (*.sciprj.gz)";

    QString selectedFilter;
    QString fn = QFileDialog::getSaveFileName(this, tr("Save Project As"), workingDir, filter,
                                              &selectedFilter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        workingDir = fi.absolutePath();
        QString baseName = fi.fileName();
        if (!baseName.endsWith(".sciprj") && !baseName.endsWith(".sciprj.gz")) {
            fn.append(".sciprj");
            if (selectedFilter.contains(".gz"))
                fn.append(".gz");
        }
        projectname = fn;

        if (saveProject()) {
            recentProjects.removeAll(fn);
            recentProjects.push_front(fn);
            updateRecentProjectsList();

            QFileInfo fi(fn);
            QString baseName = fi.baseName();
            FolderListItem *item = (FolderListItem *)folders.topLevelItem(0);
            item->setText(0, baseName);
            item->folder()->setName(baseName);
        }
    }
}

void ApplicationWindow::saveNoteAs()
{
    Note *w = (Note *)d_workspace.activeSubWindow();
    if (!w || !w->inherits("Note"))
        return;
    w->exportASCII();
}

void ApplicationWindow::saveAsTemplate()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    QString filter;
    if (w->inherits("Matrix"))
        filter = tr("Makhber/QtiPlot Matrix Template") + " (*.qmt)";
    else if (w->inherits("MultiLayer"))
        filter = tr("Makhber/QtiPlot 2D Graph Template") + " (*.qpt)";
    else if (w->inherits("Table"))
        filter = tr("Makhber/QtiPlot Table Template") + " (*.qtt)";
    else if (w->inherits("Graph3D"))
        filter = tr("Makhber/QtiPlot 3D Surface Template") + " (*.qst)";

    QString selectedFilter;
    QString fn =
            QFileDialog::getSaveFileName(this, tr("Save Window As Template"),
                                         templatesDir + "/" + w->name(), filter, &selectedFilter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        workingDir = fi.absolutePath();
        QString baseName = fi.fileName();
        if (!baseName.contains(".")) {
            selectedFilter = selectedFilter.right(5).left(4);
            fn.append(selectedFilter);
        }

        QFile f(fn);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Export Error"),
                                  tr("Could not write to file: <br><h4> %1 </h4><p>Please verify "
                                     "that you have the right to write to this location!")
                                          .arg(fn));
            return;
        }
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QString text = Makhber::schemaVersion() + " template file\n";
        text += w->saveAsTemplate(windowGeometryInfo(w));
        QTextStream t(&f);
        t.setCodec(QTextCodec::codecForName("UTF-8"));
        t << text;
        f.close();
        QApplication::restoreOverrideCursor();
    }
}

void ApplicationWindow::renameActiveWindow()
{
    MyWidget *m = (MyWidget *)d_workspace.activeSubWindow();
    if (!m)
        return;

    RenameWindowDialog *rwd = new RenameWindowDialog(this);
    rwd->setAttribute(Qt::WA_DeleteOnClose);
    rwd->setWidget(m);
    rwd->exec();
}

void ApplicationWindow::renameWindow(QTreeWidgetItem *item, int, const QString &text)
{
    if (auto wli = dynamic_cast<WindowListItem *>(item))
        if (auto w = wli->window())
            if (text != w->name())
                renameWindow(w, text);
}

bool ApplicationWindow::renameWindow(MyWidget *w, const QString &text)
{
    if (!w)
        return false;

    QString name = w->name();

    QString newName = text;
    newName.replace("-", "_");
    if (newName.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid name!"));
        return false;
    } else if (newName.contains(QRegExp("\\W"))) {
        QMessageBox::critical(
                this, tr("Error"),
                tr("The name you chose is not valid: only letters and digits are allowed!") + "<p>"
                        + tr("Please choose another name!"));
        return false;
    }

    newName.replace("_", "-");

    while (alreadyUsedName(newName)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Name <b>%1</b> already exists!").arg(newName) + "<p>"
                                      + tr("Please choose another name!") + "<p>"
                                      + tr("Warning: for internal consistency reasons the "
                                           "underscore character is replaced with a minus sign."));
        return false;
    }

    if (w->inherits("Table")) {
        QStringList labels = ((Table *)w)->colNames();
        if (labels.contains(newName)) {
            QMessageBox::critical(
                    this, tr("Error"),
                    tr("The table name must be different from the names of its columns!") + "<p>"
                            + tr("Please choose another name!"));
            return false;
        }

        updateTableNames(name, newName);
    } else if (w->inherits("Matrix"))
        changeMatrixName(name, newName);

    w->setName(newName);
    w->setCaptionPolicy(w->captionPolicy());
    renameListViewItem(name, newName);
    return true;
}

// TODO: string list -> Column * list
QStringList ApplicationWindow::columnsList(Makhber::PlotDesignation plotType)
{
    QList<MyWidget *> windows = windowsList();
    QStringList list;
    foreach (MyWidget *w, windows) {
        if (!w->inherits("Table"))
            continue;

        Table *t = (Table *)w;
        for (int i = 0; i < t->numCols(); i++) {
            if (t->colPlotDesignation(i) == plotType)
                list << QString(t->name()) + "_" + t->colLabel(i);
        }
    }

    return list;
}

// TODO: string list -> Column * list
QStringList ApplicationWindow::columnsList()
{
    QList<MyWidget *> windows = windowsList();
    QStringList list;
    foreach (MyWidget *w, windows) {
        if (!w->inherits("Table"))
            continue;

        Table *t = (Table *)w;
        for (int i = 0; i < t->numCols(); i++) {
            list << QString(t->name()) + "_" + t->colLabel(i);
        }
    }

    return list;
}

void ApplicationWindow::showCurvesDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    if (((MultiLayer *)d_workspace.activeSubWindow())->isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("This functionality is not available for pie plots!"));
    } else {
        CurvesDialog *crvDialog = new CurvesDialog(this);
        crvDialog->setAttribute(Qt::WA_DeleteOnClose);
        crvDialog->setGraph(g);
        crvDialog->resize(d_add_curves_dialog_size);
        crvDialog->show();
    }
}

QList<MyWidget *> *ApplicationWindow::tableList()
{
    QList<MyWidget *> *lst = new QList<MyWidget *>();
    foreach (MyWidget *w, windowsList()) {
        if (w->inherits("Table"))
            lst->append(w);
    }
    return lst;
}

void ApplicationWindow::showPlotAssociations(int curve)
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)w)->activeGraph();
    if (!g)
        return;

    AssociationsDialog *ad = new AssociationsDialog(this, Qt::WindowStaysOnTopHint);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    ad->setGraph(g);
    ad->initTablesList(tableList(), curve);
    ad->exec();
}

void ApplicationWindow::showTitleDialog()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("MultiLayer")) {
        Graph *g = ((MultiLayer *)w)->activeGraph();
        if (g) {
            TextDialog *td = new TextDialog(TextDialog::AxisTitle, this, Qt::Widget);
            td->setAttribute(Qt::WA_DeleteOnClose);
            connect(td, SIGNAL(changeFont(const QFont &)), g, SLOT(setTitleFont(const QFont &)));
            connect(td, SIGNAL(changeText(const QString &)), g, SLOT(setTitle(const QString &)));
            connect(td, SIGNAL(changeColor(const QColor &)), g,
                    SLOT(setTitleColor(const QColor &)));
            connect(td, SIGNAL(changeAlignment(int)), g, SLOT(setTitleAlignment(int)));

            QwtText t = g->plotWidget()->title();
            td->setText(t.text());
            td->setFont(t.font());
            td->setTextColor(t.color());
            td->setAlignment(t.renderFlags());
            td->exec();
        }
    } else if (w->inherits("Graph3D")) {
        Plot3DDialog *pd = (Plot3DDialog *)showPlot3dDialog();
        if (pd)
            pd->showTitleTab();
        delete pd;
    }
}

void ApplicationWindow::showXAxisTitleDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        TextDialog *td = new TextDialog(TextDialog::AxisTitle, this, Qt::Widget);
        td->setAttribute(Qt::WA_DeleteOnClose);
        connect(td, SIGNAL(changeFont(const QFont &)), g, SLOT(setXAxisTitleFont(const QFont &)));
        connect(td, SIGNAL(changeText(const QString &)), g, SLOT(setXAxisTitle(const QString &)));
        connect(td, SIGNAL(changeColor(const QColor &)), g,
                SLOT(setXAxisTitleColor(const QColor &)));
        connect(td, SIGNAL(changeAlignment(int)), g, SLOT(setXAxisTitleAlignment(int)));

        QStringList t = g->scalesTitles();
        td->setText(t[0]);
        td->setFont(g->axisTitleFont(2));
        td->setTextColor(g->axisTitleColor(2));
        td->setAlignment(g->axisTitleAlignment(2));
        td->setWindowTitle(tr("X Axis Title"));
        td->exec();
    }
}

void ApplicationWindow::showYAxisTitleDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        TextDialog *td = new TextDialog(TextDialog::AxisTitle, this, Qt::Widget);
        td->setAttribute(Qt::WA_DeleteOnClose);
        connect(td, SIGNAL(changeFont(const QFont &)), g, SLOT(setYAxisTitleFont(const QFont &)));
        connect(td, SIGNAL(changeText(const QString &)), g, SLOT(setYAxisTitle(const QString &)));
        connect(td, SIGNAL(changeColor(const QColor &)), g,
                SLOT(setYAxisTitleColor(const QColor &)));
        connect(td, SIGNAL(changeAlignment(int)), g, SLOT(setYAxisTitleAlignment(int)));

        QStringList t = g->scalesTitles();
        td->setText(t[1]);
        td->setFont(g->axisTitleFont(0));
        td->setTextColor(g->axisTitleColor(0));
        td->setAlignment(g->axisTitleAlignment(0));
        td->setWindowTitle(tr("Y Axis Title"));
        td->exec();
    }
}

void ApplicationWindow::showRightAxisTitleDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        TextDialog *td = new TextDialog(TextDialog::AxisTitle, this, Qt::Widget);
        td->setAttribute(Qt::WA_DeleteOnClose);
        connect(td, SIGNAL(changeFont(const QFont &)), g,
                SLOT(setRightAxisTitleFont(const QFont &)));
        connect(td, SIGNAL(changeText(const QString &)), g,
                SLOT(setRightAxisTitle(const QString &)));
        connect(td, SIGNAL(changeColor(const QColor &)), g,
                SLOT(setRightAxisTitleColor(const QColor &)));
        connect(td, SIGNAL(changeAlignment(int)), g, SLOT(setRightAxisTitleAlignment(int)));

        QStringList t = g->scalesTitles();
        td->setText(t[3]);
        td->setFont(g->axisTitleFont(1));
        td->setTextColor(g->axisTitleColor(1));
        td->setAlignment(g->axisTitleAlignment(1));
        td->setWindowTitle(tr("Right Axis Title"));
        td->exec();
    }
}

void ApplicationWindow::showTopAxisTitleDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        TextDialog *td = new TextDialog(TextDialog::AxisTitle, this, Qt::Widget);
        td->setAttribute(Qt::WA_DeleteOnClose);
        connect(td, SIGNAL(changeFont(const QFont &)), g, SLOT(setTopAxisTitleFont(const QFont &)));
        connect(td, SIGNAL(changeText(const QString &)), g, SLOT(setTopAxisTitle(const QString &)));
        connect(td, SIGNAL(changeColor(const QColor &)), g,
                SLOT(setTopAxisTitleColor(const QColor &)));
        connect(td, SIGNAL(changeAlignment(int)), g, SLOT(setTopAxisTitleAlignment(int)));

        QStringList t = g->scalesTitles();
        td->setText(t[2]);
        td->setFont(g->axisTitleFont(3));
        td->setTextColor(g->axisTitleColor(3));
        td->setAlignment(g->axisTitleAlignment(3));
        td->setWindowTitle(tr("Top Axis Title"));
        td->exec();
    }
}

void ApplicationWindow::showExportASCIIDialog()
{
    Table *table = qobject_cast<Table *>(d_workspace.activeSubWindow());
    if (table) {
        ExportDialog *ed = new ExportDialog(this, Qt::WindowContextHelpButtonHint);
        ed->setAttribute(Qt::WA_DeleteOnClose);
        connect(ed, SIGNAL(exportTable(const QString &, const QString &, bool, bool)), this,
                SLOT(exportASCII(const QString &, const QString &, bool, bool)));
        connect(ed, SIGNAL(exportAllTables(const QString &, bool, bool)), this,
                SLOT(exportAllTables(const QString &, bool, bool)));

        ed->setTableNames(tableWindows());
        ed->setActiveTableName(table->name());
        ed->setColumnSeparator(columnSeparator);
        ed->exec();
    }
}

void ApplicationWindow::exportAllTables(const QString &sep, bool colNames, bool expSelection)
{
    QString dir = QFileDialog::getExistingDirectory(
            this, tr("Choose a directory to export the tables to"), workingDir,
            QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        workingDir = dir;

        bool confirmOverwrite = true;
        bool success = true;
        foreach (MyWidget *w, windowsList()) {
            if (w->inherits("Table")) {
                Table *t = (Table *)w;
                QString fileName = dir + "/" + t->name() + ".txt";
                QFile f(fileName);
                if (f.exists(fileName) && confirmOverwrite) {
                    QApplication::restoreOverrideCursor();
                    switch (QMessageBox::question(
                            this, tr("Overwrite file?"),
                            tr("A file called: <p><b>%1</b><p>already exists. "
                               "Do you want to overwrite it?")
                                    .arg(fileName),
                            tr("&Yes"), tr("&All"), tr("&Cancel"), 0, 1)) {
                    case 0:
                        success = t->exportASCII(fileName, sep, colNames, expSelection);
                        break;

                    case 1:
                        confirmOverwrite = false;
                        success = t->exportASCII(fileName, sep, colNames, expSelection);
                        break;

                    case 2:
                        return;
                        break;
                    }
                } else
                    success = t->exportASCII(fileName, sep, colNames, expSelection);

                if (!success)
                    break;
            }
        }
        QApplication::restoreOverrideCursor();
    }
}

void ApplicationWindow::exportASCII(const QString &tableName, const QString &sep, bool colNames,
                                    bool expSelection)
{
    Table *t = table(tableName);
    if (!t)
        return;

    QString selectedFilter;
    QString fname =
            QFileDialog::getSaveFileName(this, tr("Choose a filename to save under"), asciiDirPath,
                                         "*.txt;;*.csv;;*.dat;;*.DAT", &selectedFilter);
    if (!fname.isEmpty()) {
        QFileInfo fi(fname);
        QString baseName = fi.fileName();
        if (baseName.contains(".") == 0)
            fname.append(selectedFilter.remove("*"));

        asciiDirPath = fi.absolutePath();

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        t->exportASCII(fname, sep, colNames, expSelection);
        QApplication::restoreOverrideCursor();
    }
}

void ApplicationWindow::correlate()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *t = (Table *)d_workspace.activeSubWindow();
    QStringList s = t->selectedColumns();
    if ((int)s.count() != 2) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select two columns for this operation!"));
        return;
    }

    Correlation *cor = new Correlation(this, t, s[0], s[1]);
    cor->run();
    delete cor;
}

void ApplicationWindow::autoCorrelate()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *t = (Table *)d_workspace.activeSubWindow();
    QStringList s = t->selectedColumns();
    if ((int)s.count() != 1) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select exactly one columns for this operation!"));
        return;
    }

    Correlation *cor = new Correlation(this, t, s[0], s[0]);
    cor->run();
    delete cor;
}

void ApplicationWindow::convolute()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *t = (Table *)d_workspace.activeSubWindow();
    QStringList s = t->selectedColumns();
    if ((int)s.count() != 2) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select two columns for this operation:\n the first "
                                "represents the signal and the second the response function!"));
        return;
    }

    Convolution *cv = new Convolution(this, t, s[0], s[1]);
    cv->run();
    delete cv;
}

void ApplicationWindow::deconvolute()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;

    Table *t = (Table *)d_workspace.activeSubWindow();
    QStringList s = t->selectedColumns();
    if ((int)s.count() != 2) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Please select two columns for this operation:\n the first "
                                "represents the signal and the second the response function!"));
        return;
    }

    Deconvolution *dcv = new Deconvolution(this, t, s[0], s[1]);
    dcv->run();
    delete dcv;
}

void ApplicationWindow::showColStatistics()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;
    Table *t = (Table *)d_workspace.activeSubWindow();

    if (int(t->selectedColumns().count()) > 0) {
        QList<int> targets;
        for (int i = 0; i < t->numCols(); i++)
            if (t->isColumnSelected(i, false))
                targets << i;
        newTableStatistics(t, TableStatistics::StatColumn, targets)->showNormal();
    } else
        QMessageBox::warning(this, tr("Column selection error"),
                             tr("Please select a column first!"));
}

void ApplicationWindow::showRowStatistics()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Table"))
        return;
    Table *t = (Table *)d_workspace.activeSubWindow();

    if (t->numSelectedRows() > 0) {
        QList<int> targets;
        for (int i = 0; i < t->numRows(); i++)
            if (t->isRowSelected(i, false))
                targets << i;
        newTableStatistics(t, TableStatistics::StatRow, targets)->showNormal();
    } else
        QMessageBox::warning(this, tr("Row selection error"), tr("Please select a row first!"));
}

void ApplicationWindow::plot2VerticalLayers()
{
    multilayerPlot(1, 2, defaultCurveStyle);
}

void ApplicationWindow::plot2HorizontalLayers()
{
    multilayerPlot(2, 1, defaultCurveStyle);
}

void ApplicationWindow::plot4Layers()
{
    multilayerPlot(2, 2, defaultCurveStyle);
}

void ApplicationWindow::plotStackedLayers()
{
    multilayerPlot(1, -1, defaultCurveStyle);
}

void ApplicationWindow::plotStackedHistograms()
{
    multilayerPlot(1, -1, Graph::Histogram);
}

void ApplicationWindow::showGeneralPlotDialog()
{
    MyWidget *plot = (MyWidget *)d_workspace.activeSubWindow();
    if (!plot)
        return;

    if (plot->inherits("MultiLayer") && ((MultiLayer *)plot)->layers())
        showPlotDialog();
    else if (plot->inherits("Graph3D")) {
        if (auto gd = dynamic_cast<Plot3DDialog *>(showScaleDialog()))
            gd->showGeneralTab();
    }
}

void ApplicationWindow::showAxisDialog()
{
    QWidget *plot = (QWidget *)d_workspace.activeSubWindow();
    if (!plot)
        return;

    if (plot->inherits("MultiLayer") && ((MultiLayer *)plot)->layers()) {
        if (auto gd = dynamic_cast<AxesDialog *>(showScaleDialog()))
            gd->showAxesPage();
    } else if (plot->inherits("Graph3D"))
        if (auto gd = dynamic_cast<Plot3DDialog *>(showScaleDialog()))
            gd->showAxisTab();
}

void ApplicationWindow::showGridDialog()
{
    if (auto gd = dynamic_cast<AxesDialog *>(showScaleDialog()))
        gd->showGridPage();
}

QDialog *ApplicationWindow::showScaleDialog()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return 0;

    if (w->inherits("MultiLayer")) {
        if (((MultiLayer *)w)->isEmpty())
            return 0;

        Graph *g = ((MultiLayer *)w)->activeGraph();
        auto &ad = addChild<AxesDialog>();
        ad.setGraph(g);
        ad.exec();
        return &ad;
    } else if (w->inherits("Graph3D"))
        return showPlot3dDialog();

    return 0;
}

AxesDialog *ApplicationWindow::showScalePageFromAxisDialog(int axisPos)
{
    AxesDialog *gd = (AxesDialog *)showScaleDialog();
    if (gd)
        gd->setCurrentScale(axisPos);

    return gd;
}

AxesDialog *ApplicationWindow::showAxisPageFromAxisDialog(int axisPos)
{
    AxesDialog *gd = (AxesDialog *)showScaleDialog();
    if (gd) {
        gd->showAxesPage();
        gd->setCurrentScale(axisPos);
    }
    return gd;
}

QDialog *ApplicationWindow::showPlot3dDialog()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        Graph3D *g = (Graph3D *)d_workspace.activeSubWindow();
        if (!g->hasData()) {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Not available for empty 3D surface plots!"));
            return 0;
        }

        Plot3DDialog *pd = new Plot3DDialog(this);
        pd->setPlot(g);

        connect(pd,
                SIGNAL(updateColors(const QColor &, const QColor &, const QColor &, const QColor &,
                                    const QColor &, const QColor &)),
                g,
                SLOT(updateColors(const QColor &, const QColor &, const QColor &, const QColor &,
                                  const QColor &, const QColor &)));
        connect(pd, SIGNAL(setDataColorMap(const QString &)), g,
                SLOT(setDataColorMap(const QString &)));
        connect(pd, SIGNAL(updateDataColors(const QColor &, const QColor &)), g,
                SLOT(setDataColors(const QColor &, const QColor &)));
        connect(pd, SIGNAL(updateTitle(const QString &, const QColor &, const QFont &)), g,
                SLOT(updateTitle(const QString &, const QColor &, const QFont &)));
        connect(pd, SIGNAL(updateResolution(int)), g, SLOT(setResolution(int)));
        connect(pd, SIGNAL(showColorLegend(bool)), g, SLOT(showColorLegend(bool)));
        connect(pd, SIGNAL(setOrtho(bool)), g, SLOT(setOrtho(bool)));
        connect(pd, SIGNAL(updateLabel(int, const QString &, const QFont &)), g,
                SLOT(updateLabel(int, const QString &, const QFont &)));
        connect(pd, SIGNAL(updateScale(int, const QStringList &)), g,
                SLOT(updateScale(int, const QStringList &)));
        connect(pd, SIGNAL(adjustLabels(int)), g, SLOT(adjustLabels(int)));
        connect(pd, SIGNAL(updateTickLength(int, double, double)), g,
                SLOT(updateTickLength(int, double, double)));
        connect(pd, SIGNAL(setNumbersFont(const QFont &)), g, SLOT(setNumbersFont(const QFont &)));
        connect(pd, SIGNAL(updateMeshLineWidth(int)), g, SLOT(setMeshLineWidth(int)));
        connect(pd, SIGNAL(updateBars(double)), g, SLOT(updateBars(double)));
        connect(pd, SIGNAL(updatePoints(double, bool)), g, SLOT(updatePoints(double, bool)));
        connect(pd, SIGNAL(showWorksheet()), g, SLOT(showWorksheet()));
        connect(pd, SIGNAL(updateZoom(double)), g, SLOT(updateZoom(double)));
        connect(pd, SIGNAL(updateScaling(double, double, double)), g,
                SLOT(updateScaling(double, double, double)));
        connect(pd, SIGNAL(updateCones(double, int)), g, SLOT(updateCones(double, int)));
        connect(pd, SIGNAL(updateCross(double, double, bool, bool)), g,
                SLOT(updateCross(double, double, bool, bool)));

        pd->setMeshLineWidth(g->meshLineWidth());
        pd->setDataColors(g->minDataColor(), g->maxDataColor());
        pd->setColors(g->titleColor(), g->meshColor(), g->axesColor(), g->numColor(),
                      g->labelColor(), g->bgColor(), g->gridColor());

        pd->setTitle(g->plotTitle());
        pd->setTitleFont(g->titleFont());

        pd->setZoom(g->zoom());
        pd->setScaling(g->xScale(), g->yScale(), g->zScale());
        pd->setResolution(g->resolution());
        pd->showLegend(g->isLegendOn());
        pd->setOrthogonal(g->isOrthogonal());
        pd->setAxesLabels(g->axesLabels());
        pd->setAxesTickLengths(g->axisTickLengths());
        pd->setAxesFonts(g->xAxisLabelFont(), g->yAxisLabelFont(), g->zAxisLabelFont());
        pd->setScales(g->scaleLimits());
        pd->setLabelsDistance(g->labelsDistance());
        pd->setNumbersFonts(g->numbersFont());

        if (g->coordStyle() == Qwt3D::NOCOORD)
            pd->disableAxesOptions();

        Qwt3D::PLOTSTYLE style = g->plotStyle();
        Graph3D::PointStyle pt = g->pointType();

        if (style == Qwt3D::USER) {
            switch (pt) {
            case Graph3D::None:
                break;

            case Graph3D::Dots:
                pd->disableMeshOptions();
                pd->initPointsOptionsStack();
                pd->showPointsTab(g->pointsSize(), g->smoothPoints());
                break;

            case Graph3D::VerticalBars:
                pd->showBarsTab(g->barsRadius());
                break;

            case Graph3D::HairCross:
                pd->disableMeshOptions();
                pd->initPointsOptionsStack();
                pd->showCrossHairTab(g->crossHairRadius(), g->crossHairLinewidth(),
                                     g->smoothCrossHair(), g->boxedCrossHair());
                break;

            case Graph3D::Cones:
                pd->disableMeshOptions();
                pd->initPointsOptionsStack();
                pd->showConesTab(g->coneRadius(), g->coneQuality());
                break;
            }
        } else if (style == Qwt3D::FILLED)
            pd->disableMeshOptions();
        else if (style == Qwt3D::HIDDENLINE || style == Qwt3D::WIREFRAME)
            pd->disableLegend();

        if (g->grids() == 0)
            pd->disableGridOptions();

        if (g->userFunction())
            pd->customWorksheetBtn(QString());
        else if (g->getTable())
            pd->customWorksheetBtn(tr("&Worksheet"));
        else if (g->matrix())
            pd->customWorksheetBtn(tr("&Matrix"));

        pd->exec();
        return pd;
    } else
        return 0;
}

void ApplicationWindow::showPlotDialog(int curveKey)
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("MultiLayer")) {
        PlotDialog *pd = new PlotDialog(d_extended_plot_dialog, this);
        pd->setAttribute(Qt::WA_DeleteOnClose);
        pd->setMultiLayer((MultiLayer *)w);
        pd->insertColumnsList(columnsList());
        if (curveKey >= 0) {
            Graph *g = ((MultiLayer *)w)->activeGraph();
            if (g)
                pd->selectCurve(g->curveIndex(curveKey));
        }
        pd->initFonts(plotTitleFont, plotAxesFont, plotNumbersFont, plotLegendFont);
        pd->showAll(d_extended_plot_dialog);
        pd->show();
    }
}

void ApplicationWindow::showCurvePlotDialog()
{
    showPlotDialog(actionShowCurvePlotDialog->data().toInt());
}

QMenu *ApplicationWindow::showCurveContextMenuImpl(int curveKey)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return nullptr;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    DataCurve *c = (DataCurve *)g->curve(g->curveIndex(curveKey));
    if (!c || !c->isVisible())
        return nullptr;

    auto curveMenu = new QMenu(this);
    curveMenu->addAction(c->title().text(), this, SLOT(showCurvePlotDialog()));
    curveMenu->addSeparator();

    curveMenu->addAction(actionHideCurve);
    actionHideCurve->setData(curveKey);

    if (g->visibleCurves() > 1 && c->type() == Graph::Function) {
        curveMenu->addAction(actionHideOtherCurves);
        actionHideOtherCurves->setData(curveKey);
    } else if (c->type() != Graph::Function) {
        if ((g->visibleCurves() - c->errorBarsList().count()) > 1) {
            curveMenu->addAction(actionHideOtherCurves);
            actionHideOtherCurves->setData(curveKey);
        }
    }

    if (g->visibleCurves() != g->curves())
        curveMenu->addAction(actionShowAllCurves);
    curveMenu->addSeparator();

    if (c->type() == Graph::Function) {
        curveMenu->addAction(actionEditFunction);
        actionEditFunction->setData(curveKey);
    } else if (c->type() != Graph::ErrorBars) {
        curveMenu->addAction(actionEditCurveRange);
        actionEditCurveRange->setData(curveKey);

        curveMenu->addAction(actionCurveFullRange);
        if (c->isFullRange())
            actionCurveFullRange->setDisabled(true);
        else
            actionCurveFullRange->setEnabled(true);
        actionCurveFullRange->setData(curveKey);

        curveMenu->addSeparator();
    }

    curveMenu->addAction(actionShowCurveWorksheet);
    actionShowCurveWorksheet->setData(curveKey);

    curveMenu->addAction(actionShowCurvePlotDialog);
    actionShowCurvePlotDialog->setData(curveKey);

    curveMenu->addSeparator();

    curveMenu->addAction(actionRemoveCurve);
    actionRemoveCurve->setData(curveKey);
    return curveMenu;
}

void ApplicationWindow::showAllCurves()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    for (int i = 0; i < g->curves(); i++)
        g->showCurve(i);
    g->replot();
}

void ApplicationWindow::hideOtherCurves()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionHideOtherCurves->data().toInt();
    for (int i = 0; i < g->curves(); i++)
        g->showCurve(i, false);

    g->showCurve(g->curveIndex(curveKey));
    g->replot();
}

void ApplicationWindow::hideCurve()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionHideCurve->data().toInt();
    g->showCurve(g->curveIndex(curveKey), false);
}

void ApplicationWindow::removeCurve()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionRemoveCurve->data().toInt();
    g->removeCurve(g->curveIndex(curveKey));
    g->updatePlot();
}

void ApplicationWindow::showCurveWorksheet(Graph *g, int curveIndex)
{
    if (!g)
        return;

    QwtPlotItem *it = g->plotItem(curveIndex);
    if (!it)
        return;

    if (auto sp = dynamic_cast<Spectrogram *>(it)) {
        if (sp->matrix())
            sp->matrix()->showMaximized();
    } else if (((PlotCurve *)it)->type() == Graph::Function)
        g->createTable((PlotCurve *)it);
    else
        showTable(it->title().text());
}

void ApplicationWindow::showCurveWorksheet()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionShowCurveWorksheet->data().toInt();
    showCurveWorksheet(g, g->curveIndex(curveKey));
}

void ApplicationWindow::zoomIn()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    if ((Graph *)plot->activeGraph()->isPiePlot()) {
        if (btnZoomIn->isChecked())
            QMessageBox::warning(this, tr("Warning"),
                                 tr("This functionality is not available for pie plots!"));
        btnPointer->setChecked(true);
        return;
    }

    QWidgetList graphsList = plot->graphPtrs();
    foreach (QWidget *widget, graphsList) {
        Graph *g = (Graph *)widget;
        if (!g->isPiePlot())
            g->zoom(true);
    }
}

void ApplicationWindow::zoomOut()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty() || (Graph *)plot->activeGraph()->isPiePlot())
        return;

    ((Graph *)plot->activeGraph())->zoomOut();
    btnPointer->setChecked(true);
}

void ApplicationWindow::setAutoScale()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"));
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (g) {
        g->setAutoScale();
        emit modified();
    }
}

void ApplicationWindow::removePoints()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g || !g->validCurvesDataSize()) {
        btnPointer->setChecked(true);
        return;
    }

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));
        btnPointer->setChecked(true);
        return;
    } else {
        switch (QMessageBox::warning(this, tr("Makhber"),
                                     tr("This will modify the data in the worksheets!\nAre you "
                                        "sure you want to continue?"),
                                     tr("Continue"), tr("Cancel"), 0, 1)) {
        case 0:
            g->setActiveTool(new DataPickerTool(g, this, DataPickerTool::Remove, d_status_info,
                                                SLOT(setText(const QString &))));
            break;

        case 1:
            btnPointer->setChecked(true);
            break;
        }
    }
}

void ApplicationWindow::movePoints()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g || !g->validCurvesDataSize()) {
        btnPointer->setChecked(true);
        return;
    }

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));

        btnPointer->setChecked(true);
        return;
    } else {
        switch (QMessageBox::warning(this, tr("Makhber"),
                                     tr("This will modify the data in the worksheets!\nAre you "
                                        "sure you want to continue?"),
                                     tr("Continue"), tr("Cancel"), 0, 1)) {
        case 0:
            if (g) {
                g->setActiveTool(new DataPickerTool(g, this, DataPickerTool::Move, d_status_info,
                                                    SLOT(setText(const QString &))));
            }
            break;

        case 1:
            btnPointer->setChecked(true);
            break;
        }
    }
}

void ApplicationWindow::exportPDF()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("MultiLayer") && ((MultiLayer *)w)->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"));
        return;
    }

    QString fname = QFileDialog::getSaveFileName(this, tr("Choose a filename to save under"),
                                                 workingDir, "*.pdf");
    if (!fname.isEmpty()) {
        QFileInfo fi(fname);
        QString baseName = fi.fileName();
        if (!baseName.contains("."))
            fname.append(".pdf");

        workingDir = fi.absolutePath();

        QFile f(fname);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Export Error"),
                                  tr("Could not write to file: <h4>%1</h4><p>Please verify that "
                                     "you have the right to write to this location or that the "
                                     "file is not being used by another application!")
                                          .arg(fname));
            return;
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        ((MyWidget *)w)->exportPDF(fname);

        QApplication::restoreOverrideCursor();
    }
}

void ApplicationWindow::print(MyWidget *w)
{
    if (w->inherits("MultiLayer") && ((MultiLayer *)w)->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"));
        return;
    }

    w->print();
}

// print active window
void ApplicationWindow::print()
{
    MyWidget *w = (MyWidget *)(d_workspace.activeSubWindow());
    if (!w)
        return;

    print(w);
}

// print window from project explorer
void ApplicationWindow::printWindow()
{
    WindowListItem *it = (WindowListItem *)lv.currentItem();
    MyWidget *w = it->window();
    if (!w)
        return;

    print(w);
}

void ApplicationWindow::printAllPlots()
{
    QPrinter printer;
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(true);

    QList<MyWidget *> windows = windowsList();

    int plots = 0;
    for (auto w : windows) {
        if (w->inherits("MultiLayer"))
            plots++;
    }

    QPrintDialog dialog(&printer, this);
    dialog.setMinMax(0, plots);
    if (dialog.exec()) {
        QPainter paint(&printer);

        printer.setFromTo(0, plots);

        for (auto w : windows) {
            if (w->inherits("MultiLayer") && printer.newPage())
                ((MultiLayer *)w)->printAllLayers(&paint);
        }
        paint.end();
    }
}

void ApplicationWindow::showExpGrowthDialog()
{
    showExpDecayDialog(-1);
}

void ApplicationWindow::showExpDecayDialog()
{
    showExpDecayDialog(1);
}

void ApplicationWindow::showExpDecayDialog(int type)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    ExpDecayDialog *edd = new ExpDecayDialog(type, this);
    edd->setAttribute(Qt::WA_DeleteOnClose);
    connect(g, SIGNAL(destroyed()), edd, SLOT(close()));

    edd->setGraph(g);
    edd->show();
}

void ApplicationWindow::showTwoExpDecayDialog()
{
    showExpDecayDialog(2);
}

void ApplicationWindow::showExpDecay3Dialog()
{
    showExpDecayDialog(3);
}

void ApplicationWindow::showFitDialog()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    MultiLayer *plot = 0;
    if (w->inherits("MultiLayer"))
        plot = (MultiLayer *)w;
    else if (w->inherits("Table"))
        plot = multilayerPlot((Table *)w, ((Table *)w)->drawableColumnSelection(),
                              Graph::LineSymbols);

    if (!plot)
        return;

    Graph *g = (Graph *)plot->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    FitDialog *fd = new FitDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose);
    connect(fd, SIGNAL(clearFunctionsList()), this, SLOT(clearFitFunctionsList()));
    connect(fd, SIGNAL(saveFunctionsList(const QStringList &)), this,
            SLOT(saveFitFunctionsList(const QStringList &)));
    connect(plot, SIGNAL(destroyed()), fd, SLOT(close()));

    fd->addUserFunctions(fitFunctions);
    fd->setGraph(g);
    fd->setSrcTables(tableList());
    fd->exec();
}

void ApplicationWindow::showFilterDialog(int filter)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g && g->validCurvesDataSize()) {
        FilterDialog *fd = new FilterDialog(filter, this);
        fd->setAttribute(Qt::WA_DeleteOnClose);
        fd->setGraph(g);
        fd->exec();
    }
}

void ApplicationWindow::lowPassFilterDialog()
{
    showFilterDialog(FFTFilter::LowPass);
}

void ApplicationWindow::highPassFilterDialog()
{
    showFilterDialog(FFTFilter::HighPass);
}

void ApplicationWindow::bandPassFilterDialog()
{
    showFilterDialog(FFTFilter::BandPass);
}

void ApplicationWindow::bandBlockFilterDialog()
{
    showFilterDialog(FFTFilter::BandBlock);
}

void ApplicationWindow::showFFTDialog()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w)
        return;

    FFTDialog *sd = 0;
    if (w->inherits("MultiLayer")) {
        Graph *g = ((MultiLayer *)w)->activeGraph();
        if (g && g->validCurvesDataSize()) {
            sd = new FFTDialog(FFTDialog::onGraph, this);
            sd->setAttribute(Qt::WA_DeleteOnClose);
            sd->setGraph(g);
        }
    } else if (w->inherits("Table")) {
        sd = new FFTDialog(FFTDialog::onTable, this);
        sd->setAttribute(Qt::WA_DeleteOnClose);
        sd->setTable((Table *)w);
    }

    if (sd)
        sd->exec();
}

void ApplicationWindow::showSmoothDialog(int m)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    SmoothCurveDialog *sd = new SmoothCurveDialog(m, this);
    sd->setAttribute(Qt::WA_DeleteOnClose);
    sd->setGraph(g);
    sd->exec();
}

void ApplicationWindow::showSmoothSavGolDialog()
{
    showSmoothDialog(SmoothFilter::SavitzkyGolay);
}

void ApplicationWindow::showSmoothFFTDialog()
{
    showSmoothDialog(SmoothFilter::FFT);
}

void ApplicationWindow::showSmoothAverageDialog()
{
    showSmoothDialog(SmoothFilter::Average);
}

void ApplicationWindow::showInterpolationDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    InterpolationDialog *id = new InterpolationDialog(this);
    id->setAttribute(Qt::WA_DeleteOnClose);
    connect(g, SIGNAL(destroyed()), id, SLOT(close()));
    id->setGraph(g);
    id->show();
}

void ApplicationWindow::showFitPolynomDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    PolynomFitDialog *pfd = new PolynomFitDialog(this);
    pfd->setAttribute(Qt::WA_DeleteOnClose);
    connect(g, SIGNAL(destroyed()), pfd, SLOT(close()));
    pfd->setGraph(g);
    pfd->show();
}

void ApplicationWindow::fitLinear()
{
    analysis("fitLinear");
}

void ApplicationWindow::updateLog(const QString &result)
{
    if (!result.isEmpty()) {
        logInfo += result;
        showResults(true);
        emit modified();
    }
}

void ApplicationWindow::showIntegrationDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    IntDialog *id = new IntDialog(this);
    id->setAttribute(Qt::WA_DeleteOnClose);
    connect(g, SIGNAL(destroyed()), id, SLOT(close()));
    id->setGraph(g);
    id->show();
}

void ApplicationWindow::fitSigmoidal()
{
    analysis("fitSigmoidal");
}

void ApplicationWindow::fitGauss()
{
    analysis("fitGauss");
}

void ApplicationWindow::fitLorentz()

{
    analysis("fitLorentz");
}

void ApplicationWindow::differentiate()
{
    analysis("differentiate");
}

void ApplicationWindow::showResults(bool ok)
{
    if (ok) {
        if (!logInfo.isEmpty())
            results->setText(logInfo);
        else
            results->setText(tr("Sorry, there are no results to display!"));

        logWindow.show();
        QTextCursor cur = results->textCursor();
        cur.movePosition(QTextCursor::End);
        results->setTextCursor(cur);
    } else
        logWindow.hide();
}

void ApplicationWindow::showResults(const QString &s, bool ok)
{
    logInfo += s;
    if (!logInfo.isEmpty())
        results->setText(logInfo);
    showResults(ok);
}

void ApplicationWindow::showScreenReader()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    QWidgetList graphsList = plot->graphPtrs();
    foreach (QWidget *w, graphsList)
        ((Graph *)w)
                ->setActiveTool(new ScreenPickerTool((Graph *)w, d_status_info,
                                                     SLOT(setText(const QString &))));
}

void ApplicationWindow::showRangeSelectors()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("There are no plot layers available in this window!"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g)
        return;

    if (!g->curves()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("There are no curves available on this plot!"));
        btnPointer->setChecked(true);
        return;
    } else if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));
        btnPointer->setChecked(true);
        return;
    }

    g->enableRangeSelectors(d_status_info, SLOT(setText(const QString &)));
}

void ApplicationWindow::showCursor()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    if ((Graph *)plot->activeGraph()->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));

        btnPointer->setChecked(true);
        return;
    }

    QWidgetList graphsList = plot->graphPtrs();
    foreach (QWidget *w, graphsList)
        if (!((Graph *)w)->isPiePlot() && ((Graph *)w)->validCurvesDataSize())
            ((Graph *)w)
                    ->setActiveTool(new DataPickerTool((Graph *)w, this, DataPickerTool::Display,
                                                       d_status_info,
                                                       SLOT(setText(const QString &))));
}

void ApplicationWindow::newLegend()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (g)
        g->newLegend();
}

void ApplicationWindow::addTimeStamp()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (g)
        g->addTimeStamp();
}

void ApplicationWindow::disableAddText()
{
    actionAddText->setChecked(false);
    showTextDialog();
}

void ApplicationWindow::addText()
{
    if (!btnPointer->isChecked())
        btnPointer->setChecked(true);

    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();

    switch (QMessageBox::information(
            this, tr("Add new layer?"),
            tr("Do you want to add the text on a new layer or on the active layer?"),
            tr("On &New Layer"), tr("On &Active Layer"), tr("&Cancel"), 0, 2)) {
    case 0:
        plot->addTextLayer(legendFrameStyle, plotLegendFont, legendTextColor, legendBackground);
        break;

    case 1: {
        if (plot->isEmpty()) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("<h4>There are no plot layers available in this window.</h4>"
                                    "<p><h4>Please add a layer and try again!</h4>"));

            actionAddText->setChecked(false);
            return;
        }

        Graph *g = (Graph *)plot->activeGraph();
        if (g)
            g->drawText(true);
    } break;

    case 2:
        actionAddText->setChecked(false);
        return;
        break;
    }
}

void ApplicationWindow::addImage()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g)
        return;

    QList<QByteArray> list = QImageReader::supportedImageFormats();
    QString filter = tr("Images") + " (", aux1, aux2;
    for (int i = 0; i < (int)list.count(); i++) {
        aux1 = " *." + list[i] + " ";
        aux2 += " *." + list[i] + ";;";
        filter += aux1;
    }
    filter += ");;" + aux2;

    QString fn =
            QFileDialog::getOpenFileName(this, tr("Insert image from file"), imagesDirPath, filter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        imagesDirPath = fi.absolutePath();

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        g->addImage(fn);
        QApplication::restoreOverrideCursor();
    }
}

void ApplicationWindow::drawLine()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));

        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (g) {
        g->drawLine(true);
        emit modified();
    }
}

void ApplicationWindow::drawArrow()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));

        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (g) {
        g->drawLine(true, 1);
        emit modified();
    }
}

void ApplicationWindow::showImageDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        ImageMarker *im = (ImageMarker *)g->selectedMarkerPtr();
        if (!im)
            return;

        ImageDialog *id = new ImageDialog(this);
        id->setAttribute(Qt::WA_DeleteOnClose);
        connect(id, SIGNAL(setGeometry(int, int, int, int)), g,
                SLOT(updateImageMarker(int, int, int, int)));
        id->setWindowIcon(QPixmap(":/appicon"));
        id->setOrigin(im->origin());
        id->setSize(im->size());
        id->exec();
    }
}

void ApplicationWindow::showLayerDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("There are no plot layers available in this window."));
        return;
    }

    LayerDialog *id = new LayerDialog(this);
    id->setAttribute(Qt::WA_DeleteOnClose);
    id->setMultiLayer(plot);
    id->exec();
}

void ApplicationWindow::showPlotGeometryDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    Graph *g = plot->activeGraph();
    if (g) {
        ImageDialog *id = new ImageDialog(this);
        id->setAttribute(Qt::WA_DeleteOnClose);
        connect(id, SIGNAL(setGeometry(int, int, int, int)), plot,
                SLOT(setGraphGeometry(int, int, int, int)));
        id->setWindowIcon(QPixmap(":/appicon"));
        id->setWindowTitle(tr("Layer Geometry"));
        id->setOrigin(g->pos());
        id->setSize(g->plotWidget()->size());
        id->exec();
    }
}

void ApplicationWindow::showTextDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        Legend *m = (Legend *)g->selectedMarkerPtr();
        if (!m)
            return;

        TextDialog *td = new TextDialog(TextDialog::TextMarker, this, Qt::Widget);
        td->setAttribute(Qt::WA_DeleteOnClose);
        connect(td,
                SIGNAL(values(const QString &, int, int, const QFont &, const QColor &,
                              const QColor &)),
                g,
                SLOT(updateTextMarker(const QString &, int, int, const QFont &, const QColor &,
                                      const QColor &)));

        td->setWindowIcon(QPixmap(":/appicon"));
        td->setText(m->text());
        td->setFont(m->font());
        td->setTextColor(m->textColor());
        td->setBackgroundColor(m->backgroundColor());
        td->setBackgroundType(m->frameStyle());
        td->setAngle(m->angle());
        td->exec();
    }
}

void ApplicationWindow::showLineDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g) {
        ArrowMarker *lm = (ArrowMarker *)g->selectedMarkerPtr();
        if (!lm)
            return;

        LineDialog *ld = new LineDialog(lm, this);
        ld->setAttribute(Qt::WA_DeleteOnClose);
        ld->exec();
    }
}

void ApplicationWindow::addColToTable()
{
    Table *m = (Table *)d_workspace.activeSubWindow();
    if (m)
        m->addCol();
}

void ApplicationWindow::clearSelection()
{
    if (lv.hasFocus()) {
        deleteSelectedItems();
        return;
    }

    QWidget *m = (QWidget *)d_workspace.activeSubWindow();
    if (!m)
        return;

    if (m->inherits("Table"))
        ((Table *)m)->clearSelection();
    else if (m->inherits("Matrix"))
        ((Matrix *)m)->clearSelection();
    else if (m->inherits("MultiLayer")) {
        Graph *g = ((MultiLayer *)m)->activeGraph();
        if (!g)
            return;

        if (g->titleSelected())
            g->removeTitle();
        else if (g->markerSelected())
            g->removeMarker();
    } else if (m->inherits("Note"))
        ((Note *)m)->textWidget()->clear();
    emit modified();
}

void ApplicationWindow::copySelection()
{
    if (results->hasFocus()) {
        results->copy();
        return;
    }

    QWidget *m = (QWidget *)d_workspace.activeSubWindow();
    if (!m)
        return;

    if (m->inherits("Table"))
        ((Table *)m)->copySelection();
    else if (m->inherits("Matrix"))
        ((Matrix *)m)->copySelection();
    else if (m->inherits("MultiLayer")) {
        MultiLayer *plot = (MultiLayer *)m;
        if (!plot || plot->layers() == 0)
            return;

        Graph *g = (Graph *)plot->activeGraph();
        if (!g)
            return;
        if (g->markerSelected())
            copyMarker();
        else
            copyActiveLayer();
    } else if (m->inherits("Note"))
        ((Note *)m)->textWidget()->copy();
}

void ApplicationWindow::cutSelection()
{
    QWidget *m = (QWidget *)d_workspace.activeSubWindow();
    if (!m)
        return;

    if (m->inherits("Table"))
        ((Table *)m)->cutSelection();
    else if (m->inherits("Matrix"))
        ((Matrix *)m)->cutSelection();
    else if (m->inherits("MultiLayer")) {
        MultiLayer *plot = (MultiLayer *)m;
        if (!plot || plot->layers() == 0)
            return;

        Graph *g = (Graph *)plot->activeGraph();
        if (!g)
            return;
        if (g->markerSelected()) {
            copyMarker();
            g->removeMarker();
        } else {
            copyActiveLayer();
            plot->removeLayer();
        }
    } else if (m->inherits("Note"))
        ((Note *)m)->textWidget()->cut();

    emit modified();
}

void ApplicationWindow::copyMarker()
{
    QWidget *m = (QWidget *)d_workspace.activeSubWindow();
    MultiLayer *plot = (MultiLayer *)m;
    Graph *g = (Graph *)plot->activeGraph();
    if (g && g->markerSelected()) {
        g->copyMarker();
        copiedMarkerType = g->copiedMarkerType();
        QRect rect = g->copiedMarkerRect();
        auxMrkStart = rect.topLeft();
        auxMrkEnd = rect.bottomRight();

        if (copiedMarkerType == Graph::Text) {
            Legend *m = (Legend *)g->selectedMarkerPtr();
            auxMrkText = m->text();
            auxMrkColor = m->textColor();
            auxMrkFont = m->font();
            auxMrkBkg = m->frameStyle();
            auxMrkBkgColor = m->backgroundColor();
        } else if (copiedMarkerType == Graph::Arrow) {
            ArrowMarker *m = (ArrowMarker *)g->selectedMarkerPtr();
            auxMrkWidth = m->width();
            auxMrkColor = m->color();
            auxMrkStyle = m->style();
            startArrowOn = m->hasStartArrow();
            endArrowOn = m->hasEndArrow();
            arrowHeadLength = m->headLength();
            arrowHeadAngle = m->headAngle();
            fillArrowHead = m->filledArrowHead();
        } else if (copiedMarkerType == Graph::Image) {
            ImageMarker *im = (ImageMarker *)g->selectedMarkerPtr();
            if (im)
                auxMrkFileName = im->fileName();
        }
    }
    copiedLayer = false;
}

void ApplicationWindow::pasteSelection()
{
    QWidget *m = (QWidget *)d_workspace.activeSubWindow();
    if (!m)
        return;

    if (m->inherits("Table"))
        ((Table *)m)->pasteSelection();
    else if (m->inherits("Matrix"))
        ((Matrix *)m)->pasteSelection();
    else if (m->inherits("Note"))
        ((Note *)m)->textWidget()->paste();
    else if (m->inherits("MultiLayer")) {
        MultiLayer *plot = (MultiLayer *)m;
        if (!plot)
            return;
        if (copiedLayer) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            Graph *g = plot->addLayer();
            setPreferences(g);
            g->copy(this, lastCopiedLayer);
            QPoint pos = plot->mapFromGlobal(QCursor::pos());
            plot->setGraphGeometry(pos.x(), pos.y() - 20, lastCopiedLayer->width(),
                                   lastCopiedLayer->height());

            QApplication::restoreOverrideCursor();
        } else {
            if (plot->layers() == 0)
                return;

            Graph *g = (Graph *)plot->activeGraph();
            if (!g)
                return;

            g->setCopiedMarkerType(copiedMarkerType);
            g->setCopiedMarkerEnds(auxMrkStart, auxMrkEnd);

            if (copiedMarkerType == Graph::Text)
                g->setCopiedTextOptions(auxMrkBkg, auxMrkText, auxMrkFont, auxMrkColor,
                                        auxMrkBkgColor);
            if (copiedMarkerType == Graph::Arrow)
                g->setCopiedArrowOptions(auxMrkWidth, auxMrkStyle, auxMrkColor, startArrowOn,
                                         endArrowOn, arrowHeadLength, arrowHeadAngle,
                                         fillArrowHead);
            if (copiedMarkerType == Graph::Image)
                g->setCopiedImageName(auxMrkFileName);
            g->pasteMarker();
        }
    }
    emit modified();
}

MyWidget *ApplicationWindow::clone()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w) {
        QMessageBox::critical(this, tr("Duplicate window error"),
                              tr("There are no windows available in this project!"));
        return 0;
    }

    return clone(w);
}

MyWidget *ApplicationWindow::clone(MyWidget *w)
{
    if (!w)
        return 0;

    MyWidget *nw = 0;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (w->inherits("MultiLayer")) {
        nw = multilayerPlot(generateUniqueName(tr("Graph")));
        ((MultiLayer *)nw)->copy(this, (MultiLayer *)w);
    } else if (w->inherits("Table")) {
        Table *t = (Table *)w;
        QString caption = generateUniqueName(tr("Table"));
        nw = newTable(caption, t->numRows(), t->numCols());
        ((Table *)nw)->copy(t);
    } else if (w->inherits("Graph3D")) {
        Graph3D *g = (Graph3D *)w;
        if (!g->hasData()) {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this, tr("Duplicate error"),
                                 tr("Empty 3D surface plots cannot be duplicated!"));
            return 0;
        }

        QString caption = generateUniqueName(tr("Graph"));
        QString s = g->formula();
        if (g->userFunction())
            nw = newPlot3D(caption, s, g->xStart(), g->xStop(), g->yStart(), g->yStop(),
                           g->zStart(), g->zStop());
        else if (s.endsWith("(Z)"))
            nw = dataPlotXYZ(caption, s, g->xStart(), g->xStop(), g->yStart(), g->yStop(),
                             g->zStart(), g->zStop());
        else if (s.endsWith("(Y)")) // Ribbon plot
            nw = dataPlot3D(caption, s, g->xStart(), g->xStop(), g->yStart(), g->yStop(),
                            g->zStart(), g->zStop());
        else
            nw = openMatrixPlot3D(caption, s, g->xStart(), g->xStop(), g->yStart(), g->yStop(),
                                  g->zStart(), g->zStop());

        if (!nw)
            return 0;

        ((Graph3D *)nw)->copy(g);
        customToolBars((MyWidget *)nw);
    } else if (w->inherits("Matrix")) {
        nw = newMatrix(((Matrix *)w)->numRows(), ((Matrix *)w)->numCols());
        ((Matrix *)nw)->copy((Matrix *)w);
    } else if (w->inherits("Note")) {
        nw = newNote();
        if (nw)
            ((Note *)nw)->setText(((Note *)w)->text());
    }

    if (nw) {
        if (w->inherits("MultiLayer")) {
            if (w->status() == MyWidget::Maximized)
                nw->showMaximized();
        } else if (w->inherits("Graph3D")) {
            ((Graph3D *)nw)->setIgnoreFonts(true);
            if (w->status() == MyWidget::Maximized) {
                w->showNormal();
                w->resize(500, 400);
                nw->resize(w->size());
                nw->showMaximized();
            } else
                nw->resize(w->size());
            ((Graph3D *)nw)->setIgnoreFonts(false);
        } else {
            nw->resize(w->size());
            nw->showNormal();
        }

        nw->setWindowLabel(w->windowLabel());
        nw->setCaptionPolicy(w->captionPolicy());
        setListViewLabel(nw->name(), w->windowLabel());
    }
    QApplication::restoreOverrideCursor();
    return nw;
}

void ApplicationWindow::undo()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    d_project->undoStack()->undo();
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::redo()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    d_project->undoStack()->redo();
    QApplication::restoreOverrideCursor();
}

bool ApplicationWindow::hidden(MyWidget *window)
{
    if (hiddenWindows.contains(window) || outWindows.contains(window))
        return true;

    return false;
}

void ApplicationWindow::updateWindowStatus(MyWidget *w)
{
    setListView(w->name(), w->aspect());
    if (w->status() == MyWidget::Maximized) {
        QList<MyWidget *> windows = current_folder->windowsList();
        foreach (MyWidget *oldMaxWindow, windows) {
            if (oldMaxWindow != w && oldMaxWindow->status() == MyWidget::Maximized)
                oldMaxWindow->setStatus(MyWidget::Normal);
        }
    }
}

void ApplicationWindow::hideActiveWindow()
{
    MyWidget *w = (MyWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    hideWindow(w);
}

void ApplicationWindow::hideWindow(MyWidget *w)
{
    hiddenWindows.append(w);
    w->setHidden();
    emit modified();
}

void ApplicationWindow::resizeActiveWindow()
{
    MyWidget *w = qobject_cast<MyWidget *>(d_workspace.activeSubWindow());
    if (!w)
        return;

    ImageDialog *id = new ImageDialog(this);
    id->setAttribute(Qt::WA_DeleteOnClose);
    connect(id, SIGNAL(setGeometry(int, int, int, int)), this,
            SLOT(setWindowGeometry(int, int, int, int)));

    id->setWindowTitle(tr("Window Geometry"));
    id->setOrigin(w->pos());
    id->setSize(w->size());
    id->exec();
}

void ApplicationWindow::resizeWindow()
{
    MyWidget *w = qobject_cast<MyWidget *>(d_workspace.activeSubWindow());
    if (!w)
        return;

    d_workspace.setActiveSubWindow(w);

    ImageDialog *id = new ImageDialog(this);
    id->setAttribute(Qt::WA_DeleteOnClose);
    connect(id, SIGNAL(setGeometry(int, int, int, int)), this,
            SLOT(setWindowGeometry(int, int, int, int)));

    id->setWindowTitle(tr("Window Geometry"));
    id->setOrigin(w->pos());
    id->setSize(w->size());
    id->exec();
}

void ApplicationWindow::setWindowGeometry(int x, int y, int w, int h)
{
    d_workspace.activeSubWindow()->setGeometry(x, y, w, h);
}

void ApplicationWindow::activateSubWindow()
{
    setWindowState(Qt::WindowActive);
    raise();
    show();
    WindowListItem *it = (WindowListItem *)lv.currentItem();
    activateSubWindow(it->window());
}

void ApplicationWindow::activateSubWindow(MyWidget *w)
{
    if (!w)
        return;

    w->setNormal();
    d_workspace.setActiveSubWindow(w);

    updateWindowLists(w);
    emit modified();
}

void ApplicationWindow::maximizeWindow()
{
    MyWidget *w = qobject_cast<MyWidget *>(d_workspace.activeSubWindow());
    if (!w)
        return;

    updateWindowLists(w);
    w->setMaximized();
    emit modified();
}

void ApplicationWindow::minimizeWindow()
{
    MyWidget *w = qobject_cast<MyWidget *>(d_workspace.activeSubWindow());
    if (!w)
        return;

    updateWindowLists(w);
    w->setMinimized();
    emit modified();
}

void ApplicationWindow::updateWindowLists(MyWidget *w)
{
    if (!w)
        return;

    if (hiddenWindows.contains(w))
        hiddenWindows.takeAt(hiddenWindows.indexOf(w));
    else if (outWindows.contains(w)) {
        outWindows.takeAt(outWindows.indexOf(w));
        d_workspace.addSubWindow(w);
        w->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void ApplicationWindow::closeActiveWindow()
{
    QWidget *w = (QWidget *)d_workspace.activeSubWindow();
    if (w)
        w->close();
}

void ApplicationWindow::removeWindowFromLists(MyWidget *w)
{
    if (!w)
        return;

    QString caption = w->name();
    if (w->inherits("Table")) {
        Table *m = (Table *)w;
        for (int i = 0; i < m->numCols(); i++) {
            QString name = m->colName(i);
            removeCurves(name);
        }
        if (w == lastModified) {
            actionUndo->setEnabled(false);
            actionRedo->setEnabled(false);
        }
    } else if (w->inherits("MultiLayer")) {
        MultiLayer *ml = (MultiLayer *)w;
        Graph *g = ml->activeGraph();
        if (g)
            btnPointer->setChecked(true);
    } else if (w->inherits("Matrix"))
        remove3DMatrixPlots((Matrix *)w);

    if (hiddenWindows.contains(w))
        hiddenWindows.takeAt(hiddenWindows.indexOf(w));
    else if (outWindows.contains(w))
        outWindows.takeAt(outWindows.indexOf(w));
}

void ApplicationWindow::closeWindow(MyWidget *window)
{
    if (!window)
        return;

    removeWindowFromLists(window);
    window->folder()->removeWindow(window);

    // update list view in project explorer
    QTreeWidgetItem *it =
            lv.findItems(window->name(), Qt::MatchExactly | Qt::MatchCaseSensitive).at(0);
    if (it)
        lv.takeTopLevelItem(lv.indexOfTopLevelItem(it));

    if (window->inherits("Matrix"))
        window->setParent(0);
    else if (window->inherits("Table"))
        window->setParent(0);
    else
        window->deleteLater();
    emit modified();
}

void ApplicationWindow::about()
{
    Makhber::about();
}

void ApplicationWindow::windowsMenuAboutToShow()
{
    QList<QMdiSubWindow *> windows = d_workspace.subWindowList();
    int n = int(windows.count());
    if (!n)
        return;

    windowsMenu->clear();
    windowsMenu->addAction(tr("&Cascade"), this, SLOT(cascade()));
    windowsMenu->addAction(tr("&Tile"), &d_workspace, SLOT(tileSubWindows()));
    windowsMenu->addSeparator();
    windowsMenu->addAction(actionNextWindow);
    windowsMenu->addAction(actionPrevWindow);
    windowsMenu->addSeparator();
    windowsMenu->addAction(actionRename);
    windowsMenu->addAction(actionCopyWindow);
    windowsMenu->addSeparator();
    windowsMenu->addAction(actionResizeActiveWindow);
    windowsMenu->addAction(tr("&Hide Window"), this, SLOT(hideActiveWindow()));
    windowsMenu->addAction(QPixmap(":/close.xpm"), tr("Close &Window"), this,
                           SLOT(closeActiveWindow()), Qt::CTRL + Qt::Key_W);

    if (n > 0 && n < 10) {
        windowsMenu->addSeparator();
        for (int i = 0; i < n; ++i) {
            MyWidget *widget = qobject_cast<MyWidget *>(windows.at(i));
            if (!widget)
                continue;
            QAction *actId =
                    windowsMenu->addAction(widget->name(), this, SLOT(windowsMenuActivated(bool)));
            actId->setData(i);
            actId->setCheckable(true);
            actId->setChecked(d_workspace.activeSubWindow() == windows.at(i));
        }
    } else if (n >= 10) {
        windowsMenu->addSeparator();
        for (int i = 0; i < 9; ++i) {
            MyWidget *widget = qobject_cast<MyWidget *>(windows.at(i));
            if (!widget)
                continue;
            QAction *actId =
                    windowsMenu->addAction(widget->name(), this, SLOT(windowsMenuActivated(bool)));
            actId->setData(i);
            actId->setCheckable(true);
            actId->setChecked(d_workspace.activeSubWindow() == windows.at(i));
        }
        windowsMenu->addSeparator();
        windowsMenu->addAction(tr("More windows..."), this, SLOT(showMoreWindows()));
    }
}

QMenu *ApplicationWindow::showMarkerPopupMenuImpl()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return nullptr;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    auto markerMenu = new QMenu(this);

    if (g->imageMarkerSelected()) {
        markerMenu->addAction(QPixmap(":/pixelProfile.xpm"), tr("&View Pixel Line profile"), this,
                              SLOT(pixelLineProfile()));
        markerMenu->addAction(tr("&Intensity Matrix"), this, SLOT(intensityTable()));
        markerMenu->addSeparator();
    }

    markerMenu->addAction(IconLoader::load("edit-cut"), tr("&Cut"), this, SLOT(cutSelection()));
    markerMenu->addAction(IconLoader::load("edit-copy"), tr("&Copy"), this, SLOT(copySelection()));
    markerMenu->addAction(QPixmap(":/erase.xpm"), tr("&Delete"), this, SLOT(clearSelection()));
    markerMenu->addSeparator();
    if (g->arrowMarkerSelected())
        markerMenu->addAction(tr("&Properties..."), this, SLOT(showLineDialog()));
    else if (g->imageMarkerSelected())
        markerMenu->addAction(tr("&Properties..."), this, SLOT(showImageDialog()));
    else
        markerMenu->addAction(tr("&Properties..."), this, SLOT(showTextDialog()));

    return markerMenu;
}

void ApplicationWindow::showMoreWindows()
{
    if (explorerWindow.isVisible())
        QMessageBox::information(this, "Makhber",
                                 tr("Please use the project explorer to select a window!"));
    else
        explorerWindow.show();
}

void ApplicationWindow::windowsMenuActivated(bool checked)
{
    Q_UNUSED(checked)

    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;
    int id = act->data().toInt();

    QList<QMdiSubWindow *> windows = d_workspace.subWindowList();
    MyWidget *w = qobject_cast<MyWidget *>(windows.at(id));
    if (w) {
        w->showNormal();
        w->setFocus();
        if (hidden(w)) {
            hiddenWindows.takeAt(hiddenWindows.indexOf(w));
            setListView(w->name(), tr("Normal"));
        }
    }
}

void ApplicationWindow::newProject()
{
    saveSettings(); // the recent projects must be saved

    ApplicationWindow *ed = new ApplicationWindow();
    ed->applyUserSettings();
    ed->newTable();

    if (this->isMaximized())
        ed->showMaximized();
    else
        ed->show();

    ed->savedProject();

    this->close();
}

void ApplicationWindow::savedProject()
{
    actionSaveProject->setEnabled(false);
    saved = true;
    d_project->undoStack()->clear();
}

void ApplicationWindow::modifiedProject()
{
    actionSaveProject->setEnabled(true);
    saved = false;
}

void ApplicationWindow::modifiedProject(MyWidget *w)
{
    modifiedProject();

    actionUndo->setEnabled(true);
    lastModified = w;
}

void ApplicationWindow::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == savingTimerId)
        saveProject();
    else
        QWidget::timerEvent(e);
}

void ApplicationWindow::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QStringList asciiFiles;
        QList<QUrl> urls = e->mimeData()->urls();

        foreach (QUrl url, urls) {
            QString fileName = url.toLocalFile();
            QFileInfo fileInfo(fileName);
            QString ext = fileInfo.completeSuffix().toLower();

            if (ext == "sciprj" || ext == "sciprj~" || ext == "sciprj.gz" || ext == "sciprj.gz~"
                || ext == "opj" || ext == "qti" || ext == "qti.gz" || ext == "ogm" || ext == "ogw"
                || ext == "ogg" || ext == "org") {
                open(fileName);
            } else if (ext == "csv" || ext == "dat" || ext == "txt" || ext == "tsv") {
                asciiFiles << fileName;
            } else if (ext == "bmp" || ext == "bw" || ext == "eps" || ext == "epsf" || ext == "epsi"
                       || ext == "exr" || ext == "kra" || ext == "ora" || ext == "pcx"
                       || ext == "psd" || ext == "ras" || ext == "rgb" || ext == "rgba"
                       || ext == "sgi" || ext == "tga" || ext == "xcf" || ext == "dds"
                       || ext == "gif" || ext == "ico" || ext == "jp2" || ext == "jpeg"
                       || ext == "jpg" || ext == "mng" || ext == "pbm" || ext == "pgm"
                       || ext == "pic" || ext == "png" || ext == "ppm" || ext == "svg"
                       || ext == "svgz" || ext == "tif" || ext == "tiff" || ext == "webp"
                       || ext == "xbm" || ext == "xpm" || ext == "xv") {
                loadImage(fileName);
            }
        }
        if (!asciiFiles.isEmpty()) {
            importASCII(asciiFiles, ImportASCIIDialog::NewTables, columnSeparator, ignoredLines,
                        renameColumns, strip_spaces, simplify_spaces, d_convert_to_numeric,
                        d_ASCII_import_locale);
        }
    }
}

void ApplicationWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->source()) {
        e->ignore();
        return;
    }
    (e->mimeData()->hasUrls()) ? e->acceptProposedAction() : e->ignore();
}

void ApplicationWindow::closeEvent(QCloseEvent *ce)
{
    if (!saved) {
        QString s = tr("Save changes to project: <p><b> %1 </b> ?").arg(projectname);
        switch (QMessageBox::information(this, tr("Makhber"), s, tr("Yes"), tr("No"), tr("Cancel"),
                                         0, 2)) {
        case 0:
            if (!saveProject()) {
                ce->ignore();
                break;
            }
            saveSettings(); // the recent projects must be saved
            ce->accept();
            break;

        case 1:
        default:
            saveSettings(); // the recent projects must be saved
            ce->accept();
            break;

        case 2:
            ce->ignore();
            break;
        }
    } else {
        saveSettings(); // the recent projects must be saved
        ce->accept();
    }
}

void ApplicationWindow::customEvent(QEvent *e)
{
    if (e->type() == SCRIPTING_CHANGE_EVENT) {
        scriptingChangeEvent((ScriptingChangeEvent *)e);
        // If the event is triggered by setScriptingLang(), the connections are already made
        // (for messages emitted during initialization). However, it's good programming practice not
        // to assume a particular call path for an event; which means that we don't know for sure
        // at this point whether scriptEnv is connected or not.
        scriptEnv->disconnect(this);
        connect(scriptEnv, SIGNAL(error(const QString &, const QString &, int)), this,
                SLOT(scriptError(const QString &, const QString &, int)));
        connect(scriptEnv, SIGNAL(print(const QString &)), this,
                SLOT(scriptPrint(const QString &)));
    }
}

void ApplicationWindow::deleteSelectedItems()
{
    if (folders.hasFocus()
        && folders.currentItem()
                != folders.topLevelItem(
                        0)) { // we never allow the user to delete the project folder item
        deleteFolder();
        return;
    }

    QList<QTreeWidgetItem *> lst;
    for (QTreeWidgetItemIterator it(&lv); *it; it++) {
        if ((*it)->isSelected())
            lst.append((*it));
    }

    folders.blockSignals(true);
    for (auto item : lst) {
        if (item->type() == FolderListItem::FolderType) {
            Folder *f = ((FolderListItem *)item)->folder();
            if (deleteFolder(f))
                delete item;
        } else
            ((WindowListItem *)item)->window()->close();
    }
    folders.blockSignals(false);
}

QMenu *ApplicationWindow::showListViewSelectionMenuImpl()
{
    QMenu *cm = new QMenu(this);
    cm->addAction(tr("&Delete Selection"), this, SLOT(deleteSelectedItems()), Qt::Key_F8);
    return cm;
}

QMenu *ApplicationWindow::showListViewPopupMenuImpl()
{
    QMenu *cm = new QMenu(this);
    QMenu *window = cm->addMenu(tr("New &Window"));

    window->addAction(actionNewTable);
    window->addAction(actionNewMatrix);
    window->addAction(actionNewNote);
    window->addAction(actionNewGraph);
    window->addAction(actionNewFunctionPlot);
    window->addAction(actionNewSurfacePlot);
    cm->addAction(QPixmap(":/newfolder.xpm"), tr("New F&older"), this, SLOT(addFolder()),
                  Qt::Key_F7);
    cm->addSeparator();
    cm->addAction(tr("Auto &Column Width"), &lv, SLOT(adjustColumns()));
    return cm;
}

void ApplicationWindow::showWindowPopupMenu(const QPoint &p)
{
    if (auto m = showWindowPopupMenuImpl(lv.itemAt(p)))
        m->exec(lv.mapToGlobal(p) + QPoint(0, lv.header()->height()));
}

QMenu *ApplicationWindow::showWindowPopupMenuImpl(QTreeWidgetItem *it)
{
    if (folders.isRenaming())
        return nullptr;

    if (!it)
        return showListViewPopupMenuImpl();

    int selected = 0;
    QTreeWidgetItemIterator itv(&lv);
    while (*itv) {
        if ((*itv)->isSelected())
            selected++;

        if (selected > 1)
            return showListViewSelectionMenuImpl();
        itv++;
    }

    if (auto fl = dynamic_cast<FolderListItem *>(it)) {
        current_folder = fl->folder();
        return showFolderPopupMenuImpl(fl, false);
    }

    if (auto wli = dynamic_cast<WindowListItem *>(it))
        if (auto w = wli->window())
            return showWindowMenuImpl(w);
    return nullptr;
}

void ApplicationWindow::showTable(const QString &curve)
{
    Table *w = table(curve);
    if (!w)
        return;

    updateWindowLists(w);
    int colIndex = w->colIndex(curve);
    w->deselectAll();
    w->setCellsSelected(0, colIndex, w->d_future_table->rowCount() - 1, colIndex);
    w->showMaximized();
    QTreeWidgetItem *it = lv.findItems(w->name(), Qt::MatchExactly | Qt::MatchCaseSensitive).at(0);
    if (it)
        it->setText(2, tr("Maximized"));
    emit modified();
}

QStringList ApplicationWindow::depending3DPlots(Matrix *m)
{
    QList<MyWidget *> windows = windowsList();
    QStringList plots;
    for (int i = 0; i < (int)windows.count(); i++) {
        MyWidget *w = windows.at(i);
        if (w && w->inherits("Graph3D") && ((Graph3D *)w)->matrix() == m)
            plots << w->name();
    }
    return plots;
}

// TODO: Implement this in an elegant way
QStringList ApplicationWindow::dependingPlots(const QString &name)
{
    QList<MyWidget *> windows = windowsList();
    QStringList onPlot, plots;

    for (int i = 0; i < windows.count(); i++) {
        MyWidget *w = windows.at(i);
        if (!w)
            continue;
        if (w->inherits("MultiLayer")) {
            QWidgetList lst = ((MultiLayer *)w)->graphPtrs();
            foreach (QWidget *widget, lst) {
                Graph *g = (Graph *)widget;
                onPlot = g->curvesList();
                onPlot = onPlot.filter(QRegExp("^" + name + "_.*"));
                if (onPlot.count() > 0 && !plots.contains(w->name()))
                    plots << w->name();
            }
        } else if (w->inherits("Graph3D")) {
            if ((((Graph3D *)w)->formula()).contains(name, Qt::CaseSensitive)
                && !plots.contains(w->name()))
                plots << w->name();
        }
    }
    return plots;
}

QStringList ApplicationWindow::multilayerDependencies(MyWidget *w)
{
    QStringList tables;
    MultiLayer *g = (MultiLayer *)w;
    QWidgetList graphsList = g->graphPtrs();
    for (int i = 0; i < graphsList.count(); i++) {
        Graph *ag = (Graph *)graphsList.at(i);
        QStringList onPlot = ag->curvesList();
        for (int j = 0; j < onPlot.count(); j++) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList tl = onPlot[j].split("_", Qt::SkipEmptyParts);
#else
            QStringList tl = onPlot[j].split("_", QString::SkipEmptyParts);
#endif
            if (!tables.contains(tl[0]))
                tables << tl[0];
        }
    }
    return tables;
}

void ApplicationWindow::showGraphContextMenu()
{
    QWidget *w = (QWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("MultiLayer")) {
        MultiLayer *plot = (MultiLayer *)w;
        QMenu cm(this);
        QMenu *calcul = cm.addMenu(tr("Anal&yze"));

        Graph *ag = (Graph *)plot->activeGraph();
        if (ag->isPiePlot())
            cm.addAction(tr("Re&move Pie Curve"), ag, SLOT(removePie()));
        else {
            if (ag->visibleCurves() != ag->curves()) {
                cm.addAction(actionShowAllCurves);
                cm.addSeparator();
            }
            cm.addAction(actionShowCurvesDialog);
            cm.addAction(actionAddFunctionCurve);

            QMenu *translate = calcul->addMenu(tr("&Translate"));
            translate->addAction(actionTranslateVert);
            translate->addAction(actionTranslateHor);
            calcul->addSeparator();

            calcul->addAction(actionDifferentiate);
            calcul->addAction(actionShowIntDialog);
            calcul->addSeparator();

            QMenu *smooth = calcul->addMenu(tr("&Smooth"));
            smooth->addAction(actionSmoothSavGol);
            smooth->addAction(actionSmoothFFT);
            smooth->addAction(actionSmoothAverage);

            QMenu *filter = calcul->addMenu(tr("&FFT Filter"));
            filter->addAction(actionLowPassFilter);
            filter->addAction(actionHighPassFilter);
            filter->addAction(actionBandPassFilter);
            filter->addAction(actionBandBlockFilter);

            calcul->addSeparator();
            calcul->addAction(actionInterpolate);
            calcul->addAction(actionFFT);
            calcul->addSeparator();
            calcul->addAction(actionFitLinear);
            calcul->addAction(actionShowFitPolynomDialog);
            calcul->addSeparator();

            QMenu *decay = calcul->addMenu(tr("Fit E&xponential Decay"));
            decay->addAction(actionShowExpDecayDialog);
            decay->addAction(actionShowTwoExpDecayDialog);
            decay->addAction(actionShowExpDecay3Dialog);

            calcul->addAction(actionFitExpGrowth);
            calcul->addAction(actionFitSigmoidal);
            calcul->addAction(actionFitGauss);
            calcul->addAction(actionFitLorentz);

            QMenu *multiPeakMenu = calcul->addMenu(tr("Fit &Multi-Peak"));
            multiPeakMenu->addAction(actionMultiPeakGauss);
            multiPeakMenu->addAction(actionMultiPeakLorentz);

            calcul->addSeparator();
            calcul->addAction(actionShowFitDialog);
        }

        if (copiedLayer) {
            cm.addSeparator();
            cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Layer"), this,
                         SLOT(pasteSelection()));
        } else if (copiedMarkerType >= 0) {
            cm.addSeparator();
            if (copiedMarkerType == Graph::Text)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Text"), plot,
                             SIGNAL(pasteMarker()));
            else if (copiedMarkerType == Graph::Arrow)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Line/Arrow"), plot,
                             SIGNAL(pasteMarker()));
            else if (copiedMarkerType == Graph::Image)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Image"), plot,
                             SIGNAL(pasteMarker()));
        }
        cm.addSeparator();
        QMenu *copy = cm.addMenu(tr("&Copy"));
        copy->setIcon(IconLoader::load("edit-copy"));
        copy->addAction(tr("&Layer"), this, SLOT(copyActiveLayer()));
        copy->addAction(tr("&Window"), plot, SLOT(copyAllLayers()));

        QMenu *exports = cm.addMenu(tr("E&xport"));
        exports->addAction(tr("&Layer"), this, SLOT(exportLayer()));
        exports->addAction(tr("&Window"), this, SLOT(exportGraph()));

        QMenu *prints = cm.addMenu(tr("&Print"));
        prints->setIcon(QPixmap(":/fileprint.xpm"));
        prints->addAction(tr("&Layer"), plot, SLOT(printActiveLayer()));
        prints->addAction(tr("&Window"), plot, SLOT(print()));

        cm.addSeparator();
        cm.addAction(QPixmap(":/resize.xpm"), tr("&Geometry..."), plot,
                     SIGNAL(showGeometryDialog()));
        cm.addAction(tr("P&roperties..."), this, SLOT(showGeneralPlotDialog()));
        cm.addSeparator();
        cm.addAction(QPixmap(":/close.xpm"), tr("&Delete Layer"), plot, SLOT(confirmRemoveLayer()));
        cm.exec(QCursor::pos());
    }
}

void ApplicationWindow::showLayerButtonContextMenu()
{
    QWidget *w = (QWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    if (w->inherits("MultiLayer")) {
        MultiLayer *plot = (MultiLayer *)w;
        QMenu cm(this);

        Graph *ag = (Graph *)plot->activeGraph();

        cm.addAction(actionAddLayer);
        cm.addAction(actionDeleteLayer);
        cm.addSeparator();

        if (ag->isPiePlot())
            cm.addAction(tr("Re&move Pie Curve"), ag, SLOT(removePie()));
        else {
            if (ag->visibleCurves() != ag->curves()) {
                cm.addAction(actionShowAllCurves);
                cm.addSeparator();
            }
            cm.addAction(actionShowCurvesDialog);
            cm.addAction(actionAddFunctionCurve);

            QMenu *calcul = cm.addMenu(tr("Anal&yze"));

            QMenu *translate = calcul->addMenu(tr("&Translate"));
            translate->addAction(actionTranslateVert);
            translate->addAction(actionTranslateHor);
            calcul->addSeparator();

            calcul->addAction(actionDifferentiate);
            calcul->addAction(actionShowIntDialog);
            calcul->addSeparator();

            QMenu *smooth = calcul->addMenu(tr("&Smooth"));
            smooth->addAction(actionSmoothSavGol);
            smooth->addAction(actionSmoothFFT);
            smooth->addAction(actionSmoothAverage);

            QMenu *filter = calcul->addMenu(tr("&FFT Filter"));
            filter->addAction(actionLowPassFilter);
            filter->addAction(actionHighPassFilter);
            filter->addAction(actionBandPassFilter);
            filter->addAction(actionBandBlockFilter);

            calcul->addSeparator();
            calcul->addAction(actionInterpolate);
            calcul->addAction(actionFFT);
            calcul->addSeparator();
            calcul->addAction(actionFitLinear);
            calcul->addAction(actionShowFitPolynomDialog);
            calcul->addSeparator();

            QMenu *decay = calcul->addMenu(tr("Fit E&xponential Decay"));
            decay->addAction(actionShowExpDecayDialog);
            decay->addAction(actionShowTwoExpDecayDialog);
            decay->addAction(actionShowExpDecay3Dialog);

            calcul->addAction(actionFitExpGrowth);
            calcul->addAction(actionFitSigmoidal);
            calcul->addAction(actionFitGauss);
            calcul->addAction(actionFitLorentz);

            QMenu *multiPeakMenu = calcul->addMenu(tr("Fit &Multi-Peak"));
            multiPeakMenu->addAction(actionMultiPeakGauss);
            multiPeakMenu->addAction(actionMultiPeakLorentz);

            calcul->addSeparator();
            calcul->addAction(actionShowFitDialog);
        }

        if (copiedLayer) {
            cm.addSeparator();
            cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Layer"), this,
                         SLOT(pasteSelection()));
        } else if (copiedMarkerType >= 0) {
            cm.addSeparator();
            if (copiedMarkerType == Graph::Text)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Text"), plot,
                             SIGNAL(pasteMarker()));
            else if (copiedMarkerType == Graph::Arrow)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Line/Arrow"), plot,
                             SIGNAL(pasteMarker()));
            else if (copiedMarkerType == Graph::Image)
                cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Image"), plot,
                             SIGNAL(pasteMarker()));
        }
        cm.addSeparator();

        QMenu *copy = cm.addMenu(tr("&Copy"));
        copy->setIcon(IconLoader::load("edit-copy"));
        copy->addAction(tr("&Layer"), this, SLOT(copyActiveLayer()));
        copy->addAction(tr("&Window"), plot, SLOT(copyAllLayers()));

        QMenu *exports = cm.addMenu(tr("E&xport"));
        exports->addAction(tr("&Layer"), this, SLOT(exportLayer()));
        exports->addAction(tr("&Window"), this, SLOT(exportGraph()));

        QMenu *prints = cm.addMenu(tr("&Print"));
        prints->setIcon(QPixmap(":/fileprint.xpm"));
        prints->addAction(tr("&Layer"), plot, SLOT(printActiveLayer()));
        prints->addAction(tr("&Window"), plot, SLOT(print()));

        cm.addSeparator();
        cm.addAction(QPixmap(":/resize.xpm"), tr("&Geometry..."), plot,
                     SIGNAL(showGeometryDialog()));
        cm.addAction(tr("P&roperties..."), this, SLOT(showGeneralPlotDialog()));
        cm.addSeparator();
        cm.addAction(QPixmap(":/close.xpm"), tr("&Delete Layer"), plot, SLOT(confirmRemoveLayer()));
        cm.exec(QCursor::pos());
    }
}

void ApplicationWindow::showWindowContextMenu()
{
    QWidget *w = (QWidget *)d_workspace.activeSubWindow();
    if (!w)
        return;

    QMenu cm(this);
    if (w->inherits("MultiLayer")) {
        MultiLayer *g = (MultiLayer *)w;
        if (copiedLayer) {
            cm.addAction(IconLoader::load("edit-paste"), tr("&Paste Layer"), this,
                         SLOT(pasteSelection()));
            cm.addSeparator();
        }

        cm.addAction(actionAddLayer);
        if (g->layers() != 0) {
            cm.addAction(actionDeleteLayer);
            cm.addSeparator();
            cm.addAction(actionShowPlotGeometryDialog);
            cm.addAction(actionShowLayerDialog);
            cm.addSeparator();
        } else
            cm.addSeparator();
        cm.addAction(actionRename);
        cm.addAction(actionCopyWindow);
        cm.addSeparator();
        cm.addAction(IconLoader::load("edit-copy"), tr("&Copy Page"), g, SLOT(copyAllLayers()));
        cm.addAction(tr("E&xport Page"), this, SLOT(exportGraph()));
        cm.addAction(actionPrint);
        cm.addSeparator();
        cm.addAction(actionCloseWindow);
    } else if (w->inherits("Graph3D")) {
        Graph3D *g = (Graph3D *)w;
        if (!g->hasData()) {
            QMenu *plot3D = cm.addMenu(tr("3D &Plot"));
            plot3D->addAction(actionAdd3DData);
            plot3D->addAction(tr("&Matrix..."), this, SLOT(add3DMatrixPlot()));
            plot3D->addAction(actionEditSurfacePlot);
        } else {
            if (g->getTable())
                cm.addAction(tr("Choose &Data Set..."), this, SLOT(change3DData()));
            else if (g->matrix())
                cm.addAction(tr("Choose &Matrix..."), this, SLOT(change3DMatrix()));
            else if (g->userFunction())
                cm.addAction(actionEditSurfacePlot);
            cm.addAction(QPixmap(":/erase.xpm"), tr("C&lear"), g, SLOT(clearData()));
        }

        cm.addSeparator();
        cm.addAction(actionRename);
        cm.addAction(actionCopyWindow);
        cm.addSeparator();
        cm.addAction(tr("&Copy Graph"), g, SLOT(copyImage()));
        cm.addAction(tr("&Export"), this, SLOT(exportGraph()));
        cm.addAction(actionPrint);
        cm.addSeparator();
        cm.addAction(actionCloseWindow);
    } else if (w->inherits("Matrix")) {
        Matrix *t = (Matrix *)w;
        cm.addAction(IconLoader::load("edit-cut"), tr("Cu&t"), t, SLOT(cutSelection()));
        cm.addAction(IconLoader::load("edit-copy"), tr("&Copy"), t, SLOT(copySelection()));
        cm.addAction(IconLoader::load("edit-paste"), tr("&Paste"), t, SLOT(pasteSelection()));
        cm.addSeparator();
        cm.addAction(tr("&Insert Row"), t, SLOT(insertRow()));
        cm.addAction(tr("&Insert Column"), t, SLOT(insertColumn()));
        if (t->rowsSelected()) {
            cm.addAction(QPixmap(":/close.xpm"), tr("&Delete Rows"), t, SLOT(deleteSelectedRows()));
        } else if (t->columnsSelected()) {
            cm.addAction(QPixmap(":/close.xpm"), tr("&Delete Columns"), t,
                         SLOT(deleteSelectedColumns()));
        }
        cm.addAction(QPixmap(":/erase.xpm"), tr("Clea&r"), t, SLOT(clearSelection()));
    }
    cm.exec(QCursor::pos());
}

void ApplicationWindow::showWindowTitleBarMenu()
{
    if (!qobject_cast<MyWidget *>(d_workspace.activeSubWindow()))
        return;

    showWindowMenu(qobject_cast<MyWidget *>(d_workspace.activeSubWindow()));
}

void ApplicationWindow::chooseHelpFolder()
{
// TODO: move all paths & location handling to anothor class
#if defined(Q_OS_WIN)
    const QString locateDefaultHelp =
            qApp->applicationDirPath() + QDir::toNativeSeparators("/manual/index.html");
#else
    const QString locateDefaultHelp =
            QDir::toNativeSeparators("/usr/share/doc/makhber/manual/index.html");
#endif
    if (QFile(locateDefaultHelp).exists()) {
        helpFilePath = locateDefaultHelp;
    } else {
        const QString dir = QFileDialog::getExistingDirectory(
                this, tr("Choose the location of the Makhber help folder!"),
                qApp->applicationDirPath());

        if (!dir.isEmpty()) {
            const QFile helpFile(dir + QDir::toNativeSeparators("/index.html"));
            // TODO: Probably some kind of validity check to make sure that the
            // index.html file belongs to Makhber
            if (!helpFile.exists()) {
                QMessageBox::information(
                        this, tr("index.html File Not Found!"),
                        tr("There is no file called <b>index.html</b> in this folder."
                           "<br>Please choose another folder!"));
            } else {
                helpFilePath = dir + QDir::toNativeSeparators("/index.html");
            }
        }
    }
}

void ApplicationWindow::showHelp()
{
    QFile helpFile(helpFilePath);
    if (!helpFile.exists()) {
        QMessageBox::information(
                this, tr("Help Files Not Found!"),
                tr("Please indicate the location of the help file!") + "<br>"
                        + tr("The manual can be downloaded from the following internet address:")
                        + "<p><a href = \"" MANUAL_URI "\">" MANUAL_URI "</a></p>");
        chooseHelpFolder();
#ifdef Q_OS_MAC
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Makhber", "Makhber");
#else
        QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "Makhber", "Makhber");
#endif
        settings.beginGroup("/Paths");
        settings.setValue("/HelpFile", helpFilePath);
        settings.endGroup();
    }

    if (!QDesktopServices::openUrl(QUrl(helpFilePath))) {
        QMessageBox::information(this, tr("unable to open index.html!"),
                                 tr("<b>index.html</b> file cannot be opened"));
    }
}

void ApplicationWindow::showPlotWizard()
{
    if (tableWindows().count() > 0) {
        PlotWizard *pw = new PlotWizard(this, Qt::Widget);
        pw->setAttribute(Qt::WA_DeleteOnClose);
        connect(pw, SIGNAL(plot(const QStringList &)), this,
                SLOT(multilayerPlot(const QStringList &)));

        pw->insertTablesList(tableWindows());
        // TODO: string list -> Column * list
        pw->setColumnsList(columnsList());
        pw->changeColumnsList(tableWindows()[0]);
        pw->exec();
    } else
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no tables available in this project.</h4>"
                                "<p><h4>Please create a table and try again!</h4>"));
}

void ApplicationWindow::setCurveFullRange()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionCurveFullRange->data().toInt();
    g->setCurveFullRange(g->curveIndex(curveKey));
}

void ApplicationWindow::showCurveRangeDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionEditCurveRange->data().toInt();
    showCurveRangeDialog(g, g->curveIndex(curveKey));
}

CurveRangeDialog *ApplicationWindow::showCurveRangeDialog(Graph *g, int curve)
{
    if (!g)
        return 0;

    CurveRangeDialog *crd = new CurveRangeDialog(this);
    crd->setAttribute(Qt::WA_DeleteOnClose);
    crd->setCurveToModify(g, curve);
    crd->show();
    return crd;
}

void ApplicationWindow::showFunctionDialog()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    int curveKey = actionEditFunction->data().toInt();
    showFunctionDialog(g, g->curveIndex(curveKey));
}

void ApplicationWindow::showFunctionDialog(Graph *g, int curve)
{
    if (!g)
        return;

    FunctionDialog *fd = functionDialog();
    fd->setWindowTitle(tr("Edit function"));
    fd->setCurveToModify(g, curve);
}

FunctionDialog *ApplicationWindow::functionDialog()
{
    FunctionDialog *fd = new FunctionDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose);
    connect(fd, SIGNAL(clearParamFunctionsList()), this, SLOT(clearParamFunctionsList()));
    connect(fd, SIGNAL(clearPolarFunctionsList()), this, SLOT(clearPolarFunctionsList()));

    fd->insertParamFunctionsList(xFunctions, yFunctions);
    fd->insertPolarFunctionsList(rFunctions, thetaFunctions);
    fd->show();
    fd->activateWindow();
    return fd;
}

void ApplicationWindow::addFunctionCurve()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    if (((MultiLayer *)w)->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        return;
    }

    Graph *g = ((MultiLayer *)w)->activeGraph();
    if (g) {
        FunctionDialog *fd = functionDialog();
        if (fd)
            fd->setGraph(g);
    }
}

void ApplicationWindow::updateFunctionLists(int type, QStringList &formulas)
{
    int maxListSize = 10;
    if (type == 2) {
        rFunctions.removeAll(formulas[0]);
        rFunctions.push_front(formulas[0]);

        thetaFunctions.removeAll(formulas[1]);
        thetaFunctions.push_front(formulas[1]);

        while ((int)rFunctions.size() > maxListSize)
            rFunctions.pop_back();
        while ((int)thetaFunctions.size() > maxListSize)
            thetaFunctions.pop_back();
    } else if (type == 1) {
        xFunctions.removeAll(formulas[0]);
        xFunctions.push_front(formulas[0]);

        yFunctions.removeAll(formulas[1]);
        yFunctions.push_front(formulas[1]);

        while ((int)xFunctions.size() > maxListSize)
            xFunctions.pop_back();
        while ((int)yFunctions.size() > maxListSize)
            yFunctions.pop_back();
    }
}

bool ApplicationWindow::newFunctionPlot(int type, QStringList &formulas, const QString &var,
                                        QList<double> &ranges, int points)
{
    MultiLayer *ml = newGraph();
    if (!ml)
        return false;

    if (!ml->activeGraph()->addFunctionCurve(this, type, formulas, var, ranges, points))
        return false;

    updateFunctionLists(type, formulas);
    return true;
}

void ApplicationWindow::clearLogInfo()
{
    if (!logInfo.isEmpty()) {
        logInfo = "";
        results->setText(logInfo);
        emit modified();
    }
}

void ApplicationWindow::clearParamFunctionsList()
{
    xFunctions.clear();
    yFunctions.clear();
}

void ApplicationWindow::clearPolarFunctionsList()
{
    rFunctions.clear();
    thetaFunctions.clear();
}

void ApplicationWindow::clearFitFunctionsList()
{
    fitFunctions.clear();
}

void ApplicationWindow::saveFitFunctionsList(const QStringList &l)
{
    fitFunctions = l;
}

void ApplicationWindow::clearSurfaceFunctionsList()
{
    surfaceFunc.clear();
}

void ApplicationWindow::setFramed3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->setFramed();
        actionShowAxisDialog->setEnabled(true);
    }
}

void ApplicationWindow::setBoxed3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->setBoxed();
        actionShowAxisDialog->setEnabled(true);
    }
}

void ApplicationWindow::removeAxes3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->setNoAxes();
        actionShowAxisDialog->setEnabled(false);
    }
}

void ApplicationWindow::removeGrid3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setNoGrid();
}

void ApplicationWindow::setHiddenLineGrid3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setHiddenLineGrid();
}

void ApplicationWindow::setPoints3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setPointsMesh();
}

void ApplicationWindow::setCones3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setConesMesh();
}

void ApplicationWindow::setCrosses3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setCrossMesh();
}

void ApplicationWindow::setBars3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setBarsPlot();
}

void ApplicationWindow::setLineGrid3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setLineGrid();
}

void ApplicationWindow::setFilledMesh3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setFilledMesh();
}

void ApplicationWindow::setFloorData3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setFloorData();
}

void ApplicationWindow::setFloorIso3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setFloorIsolines();
}

void ApplicationWindow::setEmptyFloor3DPlot()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setEmptyFloor();
}

void ApplicationWindow::setFrontGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setFrontGrid(on);
}

void ApplicationWindow::setBackGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setBackGrid(on);
}

void ApplicationWindow::setFloorGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setFloorGrid(on);
}

void ApplicationWindow::setCeilGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setCeilGrid(on);
}

void ApplicationWindow::setRightGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setRightGrid(on);
}

void ApplicationWindow::setLeftGrid3DPlot(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->setLeftGrid(on);
}

void ApplicationWindow::pickPlotStyle(QAction *action)
{
    if (!action)
        return;

    if (action == polygon) {
        removeGrid3DPlot();
    } else if (action == filledmesh) {
        setFilledMesh3DPlot();
    } else if (action == wireframe) {
        setLineGrid3DPlot();
    } else if (action == hiddenline) {
        setHiddenLineGrid3DPlot();
    } else if (action == pointstyle) {
        setPoints3DPlot();
    } else if (action == conestyle) {
        setCones3DPlot();
    } else if (action == crossHairStyle) {
        setCrosses3DPlot();
    } else if (action == barstyle) {
        setBars3DPlot();
    }
    emit modified();
}

void ApplicationWindow::pickCoordSystem(QAction *action)
{
    if (!action)
        return;

    if (action == Box || action == Frame) {
        if (action == Box)
            setBoxed3DPlot();
        if (action == Frame)
            setFramed3DPlot();
        grids->setEnabled(true);
    } else if (action == None) {
        removeAxes3DPlot();
        grids->setEnabled(false);
    }

    emit modified();
}

void ApplicationWindow::pickFloorStyle(QAction *action)
{
    if (!action)
        return;

    if (action == floordata) {
        setFloorData3DPlot();
    } else if (action == flooriso) {
        setFloorIso3DPlot();
    } else {
        setEmptyFloor3DPlot();
    }

    emit modified();
}

void ApplicationWindow::custom3DActions(MyWidget *w)
{
    if (w && w->inherits("Graph3D")) {
        Graph3D *plot = (Graph3D *)w;
        actionAnimate->setChecked(plot->isAnimated());
        actionPerspective->setChecked(!plot->isOrthogonal());
        switch (plot->plotStyle()) {
        case FILLEDMESH:
            wireframe->setChecked(false);
            hiddenline->setChecked(false);
            polygon->setChecked(false);
            filledmesh->setChecked(true);
            pointstyle->setChecked(false);
            barstyle->setChecked(false);
            conestyle->setChecked(false);
            crossHairStyle->setChecked(false);
            break;

        case FILLED:
            wireframe->setChecked(false);
            hiddenline->setChecked(false);
            polygon->setChecked(true);
            filledmesh->setChecked(false);
            pointstyle->setChecked(false);
            barstyle->setChecked(false);
            conestyle->setChecked(false);
            crossHairStyle->setChecked(false);
            break;

        case Qwt3D::USER:
            wireframe->setChecked(false);
            hiddenline->setChecked(false);
            polygon->setChecked(false);
            filledmesh->setChecked(false);

            if (plot->pointType() == Graph3D::VerticalBars) {
                pointstyle->setChecked(false);
                conestyle->setChecked(false);
                crossHairStyle->setChecked(false);
                barstyle->setChecked(true);
            } else if (plot->pointType() == Graph3D::Dots) {
                pointstyle->setChecked(true);
                barstyle->setChecked(false);
                conestyle->setChecked(false);
                crossHairStyle->setChecked(false);
            } else if (plot->pointType() == Graph3D::HairCross) {
                pointstyle->setChecked(false);
                barstyle->setChecked(false);
                conestyle->setChecked(false);
                crossHairStyle->setChecked(true);
            } else if (plot->pointType() == Graph3D::Cones) {
                pointstyle->setChecked(false);
                barstyle->setChecked(false);
                conestyle->setChecked(true);
                crossHairStyle->setChecked(false);
            }
            break;

        case WIREFRAME:
            wireframe->setChecked(true);
            hiddenline->setChecked(false);
            polygon->setChecked(false);
            filledmesh->setChecked(false);
            pointstyle->setChecked(false);
            barstyle->setChecked(false);
            conestyle->setChecked(false);
            crossHairStyle->setChecked(false);
            break;

        case HIDDENLINE:
            wireframe->setChecked(false);
            hiddenline->setChecked(true);
            polygon->setChecked(false);
            filledmesh->setChecked(false);
            pointstyle->setChecked(false);
            barstyle->setChecked(false);
            conestyle->setChecked(false);
            crossHairStyle->setChecked(false);
            break;

        default:
            break;
        }

        switch (plot->coordStyle()) {
        case Qwt3D::NOCOORD:
            None->setChecked(true);
            Box->setChecked(false);
            Frame->setChecked(false);
            break;

        case Qwt3D::BOX:
            None->setChecked(false);
            Box->setChecked(true);
            Frame->setChecked(false);
            break;

        case Qwt3D::FRAME:
            None->setChecked(false);
            Box->setChecked(false);
            Frame->setChecked(true);
            break;
        }

        switch (plot->floorStyle()) {
        case NOFLOOR:
            floornone->setChecked(true);
            flooriso->setChecked(false);
            floordata->setChecked(false);
            break;

        case FLOORISO:
            floornone->setChecked(false);
            flooriso->setChecked(true);
            floordata->setChecked(false);
            break;

        case FLOORDATA:
            floornone->setChecked(false);
            flooriso->setChecked(false);
            floordata->setChecked(true);
            break;
        }
        custom3DGrids(plot->grids());
    }
}

void ApplicationWindow::custom3DGrids(int grids)
{
    if (Qwt3D::BACK & grids)
        back->setChecked(true);
    else
        back->setChecked(false);

    if (Qwt3D::FRONT & grids)
        front->setChecked(true);
    else
        front->setChecked(false);

    if (Qwt3D::CEIL & grids)
        ceil->setChecked(true);
    else
        ceil->setChecked(false);

    if (Qwt3D::FLOOR & grids)
        floor->setChecked(true);
    else
        floor->setChecked(false);

    if (Qwt3D::RIGHT & grids)
        right->setChecked(true);
    else
        right->setChecked(false);

    if (Qwt3D::LEFT & grids)
        left->setChecked(true);
    else
        left->setChecked(false);
}

void ApplicationWindow::initPlot3DToolBar()
{
    graph_3D_tools = new QToolBar(tr("3D Surface"), this);
    graph_3D_tools->setObjectName(
            "graph_3D_tools"); // this is needed for QMainWindow::restoreState()
    graph_3D_tools->setIconSize(QSize(20, 20));
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, graph_3D_tools);

    coord = new QActionGroup(this);
    Box = new QAction(coord);
    Box->setIcon(QIcon(QPixmap(":/box.xpm")));
    Box->setCheckable(true);

    Frame = new QAction(coord);
    Frame->setIcon(QIcon(QPixmap(":/free_axes.xpm")));
    Frame->setCheckable(true);

    None = new QAction(coord);
    None->setIcon(QIcon(QPixmap(":/no_axes.xpm")));
    None->setCheckable(true);

    graph_3D_tools->addAction(Frame);
    graph_3D_tools->addAction(Box);
    graph_3D_tools->addAction(None);
    Box->setChecked(true);

    graph_3D_tools->addSeparator();

    // grid actions
    grids = new QActionGroup(this);
    grids->setEnabled(true);
    grids->setExclusive(false);
    front = new QAction(grids);
    front->setCheckable(true);
    front->setIcon(QIcon(QPixmap(":/frontGrid.xpm")));
    back = new QAction(grids);
    back->setCheckable(true);
    back->setIcon(QIcon(QPixmap(":/backGrid.xpm")));
    right = new QAction(grids);
    right->setCheckable(true);
    right->setIcon(QIcon(QPixmap(":/leftGrid.xpm")));
    left = new QAction(grids);
    left->setCheckable(true);
    left->setIcon(QIcon(QPixmap(":/rightGrid.xpm")));
    ceil = new QAction(grids);
    ceil->setCheckable(true);
    ceil->setIcon(QIcon(QPixmap(":/ceilGrid.xpm")));
    floor = new QAction(grids);
    floor->setCheckable(true);
    floor->setIcon(QIcon(QPixmap(":/floorGrid.xpm")));

    graph_3D_tools->addAction(front);
    graph_3D_tools->addAction(back);
    graph_3D_tools->addAction(right);
    graph_3D_tools->addAction(left);
    graph_3D_tools->addAction(ceil);
    graph_3D_tools->addAction(floor);

    graph_3D_tools->addSeparator();

    actionPerspective = new QAction(this);
    actionPerspective->setCheckable(true);
    actionPerspective->setIcon(QPixmap(":/perspective.xpm"));
    graph_3D_tools->addAction(actionPerspective);
    actionPerspective->setChecked(!orthogonal3DPlots);
    connect(actionPerspective, SIGNAL(toggled(bool)), this, SLOT(togglePerspective(bool)));

    actionResetRotation = new QAction(this);
    actionResetRotation->setCheckable(false);
    actionResetRotation->setIcon(QPixmap(":/reset_rotation.xpm"));
    graph_3D_tools->addAction(actionResetRotation);
    connect(actionResetRotation, SIGNAL(triggered()), this, SLOT(resetRotation()));

    actionFitFrame = new QAction(this);
    actionFitFrame->setCheckable(false);
    actionFitFrame->setIcon(QPixmap(":/fit_frame.xpm"));
    graph_3D_tools->addAction(actionFitFrame);
    connect(actionFitFrame, SIGNAL(triggered()), this, SLOT(fitFrameToLayer()));

    graph_3D_tools->addSeparator();

    // plot style actions
    plotstyle = new QActionGroup(this);
    wireframe = new QAction(plotstyle);
    wireframe->setCheckable(true);
    wireframe->setEnabled(true);
    wireframe->setIcon(QIcon(QPixmap(":/lineMesh.xpm")));
    hiddenline = new QAction(plotstyle);
    hiddenline->setCheckable(true);
    hiddenline->setEnabled(true);
    hiddenline->setIcon(QIcon(QPixmap(":/grid_only.xpm")));
    polygon = new QAction(plotstyle);
    polygon->setCheckable(true);
    polygon->setEnabled(true);
    polygon->setIcon(QIcon(QPixmap(":/no_grid.xpm")));
    filledmesh = new QAction(plotstyle);
    filledmesh->setCheckable(true);
    filledmesh->setIcon(QIcon(QPixmap(":/grid_poly.xpm")));
    pointstyle = new QAction(plotstyle);
    pointstyle->setCheckable(true);
    pointstyle->setIcon(QIcon(QPixmap(":/pointsMesh.xpm")));

    conestyle = new QAction(plotstyle);
    conestyle->setCheckable(true);
    conestyle->setIcon(QIcon(QPixmap(":/cones.xpm")));

    crossHairStyle = new QAction(plotstyle);
    crossHairStyle->setCheckable(true);
    crossHairStyle->setIcon(QIcon(QPixmap(":/crosses.xpm")));

    barstyle = new QAction(plotstyle);
    barstyle->setCheckable(true);
    barstyle->setIcon(QIcon(QPixmap(":/plot_bars.xpm")));

    graph_3D_tools->addAction(barstyle);
    graph_3D_tools->addAction(pointstyle);

    graph_3D_tools->addAction(conestyle);
    graph_3D_tools->addAction(crossHairStyle);
    graph_3D_tools->addSeparator();

    graph_3D_tools->addAction(wireframe);
    graph_3D_tools->addAction(hiddenline);
    graph_3D_tools->addAction(polygon);
    graph_3D_tools->addAction(filledmesh);
    filledmesh->setChecked(true);

    graph_3D_tools->addSeparator();

    // floor actions
    floorstyle = new QActionGroup(this);
    floordata = new QAction(floorstyle);
    floordata->setCheckable(true);
    floordata->setIcon(QIcon(QPixmap(":/floor.xpm")));
    flooriso = new QAction(floorstyle);
    flooriso->setCheckable(true);
    flooriso->setIcon(QIcon(QPixmap(":/isolines.xpm")));
    floornone = new QAction(floorstyle);
    floornone->setCheckable(true);
    floornone->setIcon(QIcon(QPixmap(":/no_floor.xpm")));

    graph_3D_tools->addAction(floordata);
    graph_3D_tools->addAction(flooriso);
    graph_3D_tools->addAction(floornone);
    floornone->setChecked(true);

    graph_3D_tools->addSeparator();

    actionAnimate = new QAction(this);
    actionAnimate->setCheckable(true);
    actionAnimate->setIcon(QPixmap(":/movie.xpm"));
    graph_3D_tools->addAction(actionAnimate);

    connect(actionAnimate, SIGNAL(toggled(bool)), this, SLOT(toggle3DAnimation(bool)));
    connect(coord, SIGNAL(triggered(QAction *)), this, SLOT(pickCoordSystem(QAction *)));
    connect(floorstyle, SIGNAL(triggered(QAction *)), this, SLOT(pickFloorStyle(QAction *)));
    connect(plotstyle, SIGNAL(triggered(QAction *)), this, SLOT(pickPlotStyle(QAction *)));

    connect(left, SIGNAL(triggered(bool)), this, SLOT(setLeftGrid3DPlot(bool)));
    connect(right, SIGNAL(triggered(bool)), this, SLOT(setRightGrid3DPlot(bool)));
    connect(ceil, SIGNAL(triggered(bool)), this, SLOT(setCeilGrid3DPlot(bool)));
    connect(floor, SIGNAL(triggered(bool)), this, SLOT(setFloorGrid3DPlot(bool)));
    connect(back, SIGNAL(triggered(bool)), this, SLOT(setBackGrid3DPlot(bool)));
    connect(front, SIGNAL(triggered(bool)), this, SLOT(setFrontGrid3DPlot(bool)));
}

void ApplicationWindow::pixelLineProfile()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    bool ok;
    int res = QInputDialog::getInt(this, tr("Set the number of pixels to average"),
                                   tr("Number of averaged pixels"), 1, 1, 2000, 2, &ok);
    if (!ok)
        return;

    LineProfileTool *lpt = new LineProfileTool(g, res);
    connect(lpt, SIGNAL(createTablePlot(const QString &, const QString &, QList<Column *>)), this,
            SLOT(newWrksheetPlot(const QString &, const QString &, QList<Column *>)));
    g->setActiveTool(lpt);
}

void ApplicationWindow::intensityTable()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (g)
        g->showIntensityTable();
}

Matrix *ApplicationWindow::importImage(const QString &fileName)
{
    QImage image(fileName);
    if (image.isNull())
        return NULL;

    Matrix *m = Matrix::fromImage(image, scriptEnv);
    if (!m) {
        QMessageBox::information(0, tr("Error importing image"),
                                 tr("Import of image '%1' failed").arg(fileName));
        return NULL;
    }
    QString caption = generateUniqueName(tr("Matrix"));
    m->setName(caption);
    d_project->addChild(m->d_future_matrix);
    return m;
}

void ApplicationWindow::autoArrangeLayers()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    plot->setMargins(5, 5, 5, 5);
    plot->setSpacing(5, 5);
    plot->arrangeLayers(true, false);
}

void ApplicationWindow::addLayer()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)d_workspace.activeSubWindow();
    switch (QMessageBox::information(
            this, tr("Guess best origin for the new layer?"),
            tr("Do you want Makhber to guess the best position for the new layer?\n Warning: this "
               "will rearrange existing layers!"),
            tr("&Guess"), tr("&Top-left corner"), tr("&Cancel"), 0, 2)) {
    case 0: {
        setPreferences(plot->addLayer());
        plot->arrangeLayers(true, false);
    } break;

    case 1:
        setPreferences(plot->addLayer(0, 0, plot->size().width(), plot->size().height()));
        break;

    case 2:
        return;
        break;
    }
}

void ApplicationWindow::deleteLayer()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    ((MultiLayer *)d_workspace.activeSubWindow())->confirmRemoveLayer();
}

Note *ApplicationWindow::openNote(ApplicationWindow *app, const QStringList &flist)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lst = flist[0].split("\t", Qt::SkipEmptyParts);
#else
    QStringList lst = flist[0].split("\t", QString::SkipEmptyParts);
#endif
    QString caption = lst[0];
    Note *w = app->newNote(caption);
    if (lst.count() == 2) {
        app->setListViewDate(caption, lst[1]);
        w->setBirthDate(lst[1]);
    }
    restoreWindowGeometry(app, w, flist[1]);

    lst = flist[2].split("\t");
    w->setWindowLabel(lst[1]);
    w->setCaptionPolicy((MyWidget::CaptionPolicy)lst[2].toInt());
    app->setListViewLabel(w->name(), lst[1]);
    return w;
}

// TODO: most of this code belongs into matrix
Matrix *ApplicationWindow::openMatrix(ApplicationWindow *app, const QStringList &flist)
{
    if (app->d_file_version < 0x000200) {
        QStringList::const_iterator line = flist.begin();

        QStringList list = (*line).split("\t");
        QString caption = list[0];
        int rows = list[1].toInt();
        int cols = list[2].toInt();

        Matrix *w = app->newMatrix(caption, rows, cols);
        app->setListViewDate(caption, list[3]);
        w->setBirthDate(list[3]);

        for (line++; line != flist.end(); line++) {
            QStringList fields = (*line).split("\t");
            if (fields[0] == "geometry") {
                restoreWindowGeometry(app, w, *line);
            } else if (fields[0] == "ColWidth") {
                w->setColumnsWidth(fields[1].toInt());
            } else if (fields[0] == "Formula") {
                w->setFormula(fields[1]);
            } else if (fields[0] == "<formula>") {
                QString formula;
                for (line++; line != flist.end() && *line != "</formula>"; line++)
                    formula += *line + "\n";
                formula.truncate(formula.length() - 1);
                w->setFormula(formula);
            } else if (fields[0] == "TextFormat") {
                if (fields[1] == "f")
                    w->setTextFormat('f', fields[2].toInt());
                else
                    w->setTextFormat('e', fields[2].toInt());
            } else if (fields[0] == "WindowLabel") { // d_file_version > 71
                w->setWindowLabel(fields[1]);
                w->setCaptionPolicy((MyWidget::CaptionPolicy)fields[2].toInt());
                app->setListViewLabel(w->name(), fields[1]);
            } else if (fields[0] == "Coordinates") { // d_file_version > 81
                w->setCoordinates(fields[1].toDouble(), fields[2].toDouble(), fields[3].toDouble(),
                                  fields[4].toDouble());
            } else // <data> or values
                break;
        }
        if (*line == "<data>")
            line++;

        QElapsedTimer t;
        t.start();
        // read and set table values
        for (; line != flist.end() && *line != "</data>"; line++) {
            QStringList fields = (*line).split("\t");
            int row = fields[0].toInt();
            for (int col = 0; col < cols; col++) {
                QString cell = fields[col + 1];
                if (cell.isEmpty())
                    continue;

                if (d_file_version < 90)
                    w->setCell(row, col, QLocale::c().toDouble(cell));
                else if (d_file_version >= 0x000100)
                    w->setCell(row, col, QLocale().toDouble(cell));
                else
                    w->setText(row, col, cell);
            }
            if (t.elapsed() > 1000) {
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                t.start();
            }
        }

        return w;
    } else {
        Matrix *w = app->newMatrix("matrix", 1, 1);
        int length = flist.at(0).toInt();
        int index = 1;
        QString xml(flist.at(index++));
        while (xml.length() < length && index < flist.size())
            xml += '\n' + flist.at(index++);
        XmlStreamReader reader(xml);
        reader.readNext();
        reader.readNext(); // read the start document
        if (w->d_future_matrix->load(&reader) == false) {
            QString msg_text = reader.errorString();
            QMessageBox::critical(this, tr("Error reading matrix from project file"), msg_text);
        }
        if (reader.hasWarnings()) {
            QString msg_text =
                    tr("The following problems occured when loading the project file:\n");
            QStringList warnings = reader.warningStrings();
            foreach (QString str, warnings)
                msg_text += str + "\n";
            QMessageBox::warning(this, tr("Project loading partly failed"), msg_text);
        }
        restoreWindowGeometry(app, w, flist.at(index));

        activateSubWindow(w);
        return w;
    }
}

// TODO: most of this code belongs into Table
Table *ApplicationWindow::openTable(ApplicationWindow *app, QTextStream &stream)
{
    if (app->d_file_version < 0x000200) {
        QStringList flist;
        QString s;
        while (s != "</table>") {
            s = stream.readLine();
            flist << s;
        }
        flist.pop_back();
        QStringList::const_iterator line = flist.begin();

        QStringList list = (*line).split("\t");
        QString caption = list[0];
        int rows = list[1].toInt();
        int cols = list[2].toInt();

        Table *w = app->newTable(caption, rows, cols);
        app->setListViewDate(caption, list[3]);
        w->setBirthDate(list[3]);

        for (line++; line != flist.end(); line++) {
            QStringList fields = (*line).split("\t");
            if (fields[0] == "geometry" || fields[0] == "tgeometry") {
                restoreWindowGeometry(app, w, *line);
            } else if (fields[0] == "header") {
                fields.pop_front();
                if (d_file_version >= 78)
                    w->importV0x0001XXHeader(fields);
                else {
                    w->setColPlotDesignation(list[4].toInt(), Makhber::X);
                    w->setColPlotDesignation(list[6].toInt(), Makhber::Y);
                    w->setHeader(fields);
                }
            } else if (fields[0] == "ColWidth") {
                fields.pop_front();
                w->setColWidths(fields);
            } else if (fields[0] == "com") { // legacy code
                w->setCommands(*line);
            } else if (fields[0] == "<com>") {
                for (line++; line != flist.end() && *line != "</com>"; line++) {
                    int col = (*line).mid(9, (*line).length() - 11).toInt();
                    QString formula;
                    for (line++; line != flist.end() && *line != "</col>"; line++)
                        formula += *line + "\n";
                    formula.truncate(formula.length() - 1);
                    w->setCommand(col, formula);
                }
            } else if (fields[0] == "ColType") { // d_file_version > 65
                fields.pop_front();
                w->setColumnTypes(fields);
            } else if (fields[0] == "Comments") { // d_file_version > 71
                fields.pop_front();
                w->setColComments(fields);
            } else if (fields[0] == "WindowLabel") { // d_file_version > 71
                w->setWindowLabel(fields[1]);
                w->setCaptionPolicy((MyWidget::CaptionPolicy)fields[2].toInt());
                app->setListViewLabel(w->name(), fields[1]);
            } else // <data> or values
                break;
        }

        QElapsedTimer t;
        t.start();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        for (line++; line != flist.end() && *line != "</data>";
             line++) { // read and set table values
            QStringList fields = (*line).split("\t");
            int row = fields[0].toInt();
            for (int col = 0; col < cols; col++) {
                if (fields.count() >= col + 2) {
                    QString cell = fields[col + 1];
                    if (cell.isEmpty())
                        continue;

                    if (d_file_version < 90 && w->columnType(col) == Makhber::ColumnMode::Numeric)
                        w->setCell(row, col, QLocale::c().toDouble(cell.replace(",", ".")));
                    else if (d_file_version >= 0x000100
                             && w->columnType(col) == Makhber::ColumnMode::Numeric)
                        w->setCell(row, col, QLocale().toDouble(cell));
                    else
                        w->setText(row, col, cell);
                }
            }
            if (t.elapsed() > 1000) {
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                t.start();
            }
        }
        QApplication::restoreOverrideCursor();

        return w;
    } else {
        QString s = stream.readLine();
        int length = s.toInt();

        // On Windows, loading large tables to a QString has been observed to crash
        // (apparently due to excessive memory usage).
        // => use temporary file if possible
        QTemporaryFile tmp_file;
        QString tmp_string;
        if (tmp_file.open()) {
            QTextStream tmp(&tmp_file);
            tmp.setCodec(QTextCodec::codecForName("UTF-8"));
            int read = 0;
            while (length - read >= 1024) {
                tmp << stream.read(1024);
                read += 1024;
            }
            tmp << stream.read(length - read);
            tmp.flush();
            tmp_file.seek(0);
            stream.readLine(); // skip to next newline
        } else
            while (tmp_string.length() < length)
                tmp_string += '\n' + stream.readLine();

        XmlStreamReader reader(tmp_string);
        if (tmp_file.isOpen())
            reader.setDevice(&tmp_file);

        Table *w = app->newTable("table", 1, 1);
        reader.readNext();
        reader.readNext(); // read the start document
        if (w->d_future_table->load(&reader) == false) {
            QString msg_text = reader.errorString();
            QMessageBox::critical(this, tr("Error reading table from project file"), msg_text);
        }
        if (reader.hasWarnings()) {
            QString msg_text =
                    tr("The following problems occured when loading the project file:\n");
            QStringList warnings = reader.warningStrings();
            foreach (QString str, warnings)
                msg_text += str + "\n";
            QMessageBox::warning(this, tr("Project loading partly failed"), msg_text);
        }
        w->setBirthDate(QLocale().toString(w->d_future_table->creationTime()));

        s = stream.readLine();
        restoreWindowGeometry(app, w, s);

        s = stream.readLine(); // </table>

        activateSubWindow(w);
        return w;
    }
}

TableStatistics *ApplicationWindow::openTableStatistics(const QStringList &flist)
{
    QStringList::const_iterator line = flist.begin();

    QStringList list = (*line++).split("\t");
    QString caption = list[0];

    QList<int> targets;
    for (int i = 1; i <= (*line).count('\t'); i++)
        targets << (*line).section('\t', i, i).toInt();

    TableStatistics *w = newTableStatistics(table(list[1]),
                                            list[2] == "row" ? TableStatistics::StatRow
                                                             : TableStatistics::StatColumn,
                                            targets, caption);

    setListViewDate(caption, list[3]);
    w->setBirthDate(list[3]);

    for (line++; line != flist.end(); line++) {
        QStringList fields = (*line).split("\t");
        if (fields[0] == "geometry") {
            restoreWindowGeometry(this, w, *line);
        } else if (fields[0] == "header") {
            fields.pop_front();
            if (d_file_version >= 78)
                w->importV0x0001XXHeader(fields);
            else {
                w->setColPlotDesignation(list[4].toInt(), Makhber::X);
                w->setColPlotDesignation(list[6].toInt(), Makhber::Y);
                w->setHeader(fields);
            }
        } else if (fields[0] == "ColWidth") {
            fields.pop_front();
            w->setColWidths(fields);
        } else if (fields[0] == "com") { // legacy code
            w->setCommands(*line);
        } else if (fields[0] == "<com>") {
            for (line++; line != flist.end() && *line != "</com>"; line++) {
                int col = (*line).mid(9, (*line).length() - 11).toInt();
                QString formula;
                for (line++; line != flist.end() && *line != "</col>"; line++)
                    formula += *line + "\n";
                formula.truncate(formula.length() - 1);
                w->setCommand(col, formula);
            }
        } else if (fields[0] == "ColType") { // d_file_version > 65
            fields.pop_front();
            w->setColumnTypes(fields);
        } else if (fields[0] == "Comments") { // d_file_version > 71
            fields.pop_front();
            w->setColComments(fields);
        } else if (fields[0] == "WindowLabel") { // d_file_version > 71
            w->setWindowLabel(fields[1]);
            w->setCaptionPolicy((MyWidget::CaptionPolicy)fields[2].toInt());
            setListViewLabel(w->name(), fields[1]);
        }
    }
    return w;
}

Graph *ApplicationWindow::openGraph(ApplicationWindow *app, MultiLayer *plot,
                                    const QStringList &list)
{
    Graph *ag = 0;
    int curveID = 0;
    for (int j = 0; j < (int)list.count() - 1; j++) {
        QString s = list[j];
        if (s.contains("ggeometry")) {
            QStringList fList = s.split("\t");
            ag = (Graph *)plot->addLayer(fList[1].toInt(), fList[2].toInt(), fList[3].toInt(),
                                         fList[4].toInt());
            ag->blockSignals(true);
            ag->enableAutoscaling(autoscale2DPlots);
        } else if (s.left(10) == "Background") {
            QStringList fList = s.split("\t");
            QColor c = QColor(COLORVALUE(fList[1]));
            if (fList.count() == 3)
                c.setAlpha(fList[2].toInt());
            ag->setBackgroundColor(c);
        } else if (s.startsWith("Margin")) {
            QStringList fList = s.split("\t");
            ag->plotWidget()->setMargin(fList[1].toInt());
        } else if (s.startsWith("Border")) {
            QStringList fList = s.split("\t");
            ag->setFrame(fList[1].toInt(), QColor(COLORVALUE(fList[2])));
        } else if (s.contains("EnabledAxes")) {
            QStringList fList = s.split("\t");
            ag->enableAxes(fList);
        } else if (s.contains("AxesBaseline")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.split("\t", Qt::SkipEmptyParts);
#else
            QStringList fList = s.split("\t", QString::SkipEmptyParts);
#endif
            ag->setAxesBaseline(fList);
        } else if (s.contains("EnabledTicks")) { // version < 0.8.6
            QStringList fList = s.split("\t");
            fList.pop_front();
            fList.replaceInStrings("-1", "3");
            ag->setMajorTicksType(fList);
            ag->setMinorTicksType(fList);
        } else if (s.contains("MajorTicks")) { // version >= 0.8.6
            QStringList fList = s.split("\t");
            fList.pop_front();
            ag->setMajorTicksType(fList);
        } else if (s.contains("MinorTicks")) { // version >= 0.8.6
            QStringList fList = s.split("\t");
            fList.pop_front();
            ag->setMinorTicksType(fList);
        } else if (s.contains("TicksLength")) {
            QStringList fList = s.split("\t");
            ag->setTicksLength(fList[1].toInt(), fList[2].toInt());
        } else if (s.contains("EnabledTickLabels")) {
            QStringList fList = s.split("\t");
            fList.pop_front();
            ag->setEnabledTickLabels(fList);
        } else if (s.contains("AxesColors")) {
            QStringList fList = s.split("\t");
            fList.pop_front();
            ag->setAxesColors(fList);
        } else if (s.contains("AxesNumberColors")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.split("\t", Qt::KeepEmptyParts);
#else
            QStringList fList = s.split("\t", QString::KeepEmptyParts);
#endif
            fList.pop_front();
            ag->setAxesNumColors(fList);
        } else if (s.left(5) == "grid\t") {
            ag->plotWidget()->grid()->load(s.split("\t"));
        } else if (s.startsWith("<Antialiasing>") && s.endsWith("</Antialiasing>")) {
            bool antialiasing = s.remove("<Antialiasing>").remove("</Antialiasing>").toInt();
            ag->setAntialiasing(antialiasing, false);
        } else if (s.contains("PieCurve")) {
            QStringList curve = s.split("\t");
            if (!app->renamedTables.isEmpty()) {
                QString caption = (curve[1]).left((curve[1]).indexOf("_", 0));
                if (app->renamedTables.contains(caption)) { // modify the name of the curve
                                                            // according to the new table name
                    int index = app->renamedTables.indexOf(caption);
                    QString newCaption = app->renamedTables[++index];
                    curve.replaceInStrings(caption + "_", newCaption + "_");
                }
            }
            QPen pen = QPen(QColor(COLORVALUE(curve[3])), curve[2].toInt(),
                            Graph::getPenStyle(curve[4]));

            Table *table = app->table(curve[1]);
            if (table) {
                int startRow = 0;
                int endRow = table->numRows() - 1;
                int first_color = curve[7].toInt();
                bool visible = true;
                if (d_file_version >= 90) {
                    startRow = curve[8].toInt();
                    endRow = curve[9].toInt();
                    visible = ((curve.last() == "1") ? true : false);
                }

                if (d_file_version <= 89)
                    first_color = convertOldToNewColorIndex(first_color);

                ag->plotPie(table, curve[1], pen, curve[5].toInt(), curve[6].toInt(), first_color,
                            startRow, endRow, visible);
            }
        } else if (s.left(6) == "curve\t") {
            bool curve_loaded = false; // Graph::insertCurve may fail
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList curve = s.split("\t", Qt::KeepEmptyParts);
#else
            QStringList curve = s.split("\t", QString::KeepEmptyParts);
#endif
            int s_offset = 0;
            if (curve.count() > 14) {
                if (!app->renamedTables.isEmpty()) {
                    QString caption = (curve[2]).left((curve[2]).indexOf("_", 0));

                    if (app->renamedTables.contains(caption)) { // modify the name of the curve
                                                                // according to the new table name
                        int index = app->renamedTables.indexOf(caption);
                        QString newCaption = app->renamedTables[++index];
                        curve.replaceInStrings(caption + "_", newCaption + "_");
                    }
                }

                CurveLayout cl;
                cl.connectType = curve[4].toInt();
                cl.lCol = COLORUINT(curve[5]);
                if (d_file_version <= 89)
                    cl.lCol = convertOldToNewColorIndex(cl.lCol);
                cl.lStyle = curve[6].toInt();
                cl.lWidth = curve[7].toInt();
                cl.sSize = curve[8].toInt();
                if (d_file_version <= 78)
                    cl.sType = Graph::obsoleteSymbolStyle(curve[9].toInt());
                else
                    cl.sType = curve[9].toInt();

                cl.symCol = COLORUINT(curve[10]);
                if (d_file_version <= 89)
                    cl.symCol = convertOldToNewColorIndex(cl.symCol);
                if (curve[11] == "-1")
                    cl.symbolFill = false;
                else {
                    cl.symbolFill = true;
                    cl.fillCol = COLORUINT(curve[11]);
                }
                if (d_file_version <= 89)
                    cl.fillCol = convertOldToNewColorIndex(cl.fillCol);
                cl.filledArea = curve[12].toInt();
                cl.aCol = COLORUINT(curve[13]);
                if (d_file_version <= 89)
                    cl.aCol = convertOldToNewColorIndex(cl.aCol);
                cl.aStyle = curve[14].toInt();
                if (curve.count() < 16)
                    cl.penWidth = cl.lWidth;
                else if ((d_file_version >= 79) && (curve[3].toInt() == Graph::Box)) {
                    cl.penWidth = curve[15].toInt();
                    s_offset++;
                } else if ((d_file_version >= 78) && (curve[3].toInt() <= Graph::LineSymbols)) {
                    cl.penWidth = curve[15].toInt();
                    s_offset++;
                } else
                    cl.penWidth = cl.lWidth;
                if (d_file_version >= 0x011800) // 1.24.0
                {
                    // custom dash pattern
                    cl.lCapStyle = curve[15 + s_offset].toInt();
                    cl.lJoinStyle = curve[16 + s_offset].toInt();
                    cl.lCustomDash = curve[17 + s_offset];
                    s_offset += 3;
                }

                Table *w = app->table(curve[2]);
                if (w) {
                    int plotType = curve[3].toInt();
                    if (curve.count() > (21 + s_offset)
                        && (plotType == Graph::VectXYXY || plotType == Graph::VectXYAM)) {
                        QStringList colsList;
                        colsList << curve[2];
                        colsList << curve[20 + s_offset];
                        colsList << curve[21 + s_offset];
                        if (d_file_version < 72)
                            colsList.prepend(w->colName(curve[1].toInt()));
                        else
                            colsList.prepend(curve[1]);

                        int startRow = 0;
                        int endRow = -1;
                        if (d_file_version >= 90) {
                            startRow = curve[curve.count() - 3].toInt();
                            endRow = curve[curve.count() - 2].toInt();
                        }

                        ag->plotVectorCurve(w, colsList, plotType, startRow, endRow);
                        curve_loaded = true;

                        if (d_file_version <= 77) {
                            int temp_index = convertOldToNewColorIndex(curve[15].toInt());
                            ag->updateVectorsLayout(curveID, ColorButton::color(temp_index),
                                                    curve[16].toInt(), curve[17].toInt(),
                                                    curve[18].toInt(), curve[19].toInt(), 0,
                                                    curve[20], curve[21]);
                        } else {
                            if (plotType == Graph::VectXYXY)
                                ag->updateVectorsLayout(
                                        curveID, curve[15 + s_offset], curve[16 + s_offset].toInt(),
                                        curve[17 + s_offset].toInt(), curve[18 + s_offset].toInt(),
                                        curve[19 + s_offset].toInt(), 0);
                            else if (curve.count() > 22 + s_offset)
                                ag->updateVectorsLayout(
                                        curveID, curve[15 + s_offset], curve[16 + s_offset].toInt(),
                                        curve[17 + s_offset].toInt(), curve[18 + s_offset].toInt(),
                                        curve[19 + s_offset].toInt(), curve[22 + s_offset].toInt());
                        }
                    } else if (plotType == Graph::Box) {
                        ag->openBoxDiagram(w, curve, d_file_version);
                        curve_loaded = true;
                    } else if (plotType == Graph::Histogram && curve.count() > 19) {
                        if (d_file_version < 90)
                            curve_loaded = ag->plotHistogram(w, QStringList() << curve[2]);
                        else
                            curve_loaded = ag->plotHistogram(w, QStringList() << curve[2],
                                                             curve[curve.count() - 3].toInt(),
                                                             curve[curve.count() - 2].toInt());
                        if (curve_loaded) {
                            QwtHistogram *h = (QwtHistogram *)ag->curve(curveID);
                            if (d_file_version <= 76)
                                h->setBinning(curve[16].toInt(), curve[17].toDouble(),
                                              curve[18].toDouble(), curve[19].toDouble());
                            else if (curve.count() > 20 + s_offset)
                                h->setBinning(curve[17 + s_offset].toInt(),
                                              curve[18 + s_offset].toDouble(),
                                              curve[19 + s_offset].toDouble(),
                                              curve[20 + s_offset].toDouble());
                            h->loadData();
                        }
                    } else {
                        if (d_file_version < 72)
                            curve_loaded = ag->insertCurve(w, curve[1].toInt(), curve[2], plotType);
                        else if (d_file_version < 90)
                            curve_loaded = ag->insertCurve(w, curve[1], curve[2], plotType);
                        else {
                            int startRow = curve[curve.count() - 3].toInt();
                            int endRow = curve[curve.count() - 2].toInt();
                            curve_loaded = ag->insertCurve(w, curve[1], curve[2], plotType,
                                                           startRow, endRow);
                        }
                    }

                    if (curve_loaded
                        && (plotType == Graph::VerticalBars || plotType == Graph::HorizontalBars
                            || plotType == Graph::Histogram)) {
                        if (d_file_version <= 76 && curve.count() > 15)
                            ag->setBarsGap(curveID, curve[15].toInt(), 0);
                        else if (curve.count() > (16 + s_offset))
                            ag->setBarsGap(curveID, curve[15 + s_offset].toInt(),
                                           curve[16 + s_offset].toInt());
                    }
                    if (curve_loaded)
                        ag->updateCurveLayout(curveID, &cl);
                    if (d_file_version >= 88) {
                        QwtPlotCurve *c = ag->curve(curveID);
                        if (c && c->rtti() == QwtPlotItem::Rtti_PlotCurve) {
                            if (d_file_version < 90)
                                c->setAxis(curve[curve.count() - 2].toInt(),
                                           curve[curve.count() - 1].toInt());
                            else {
                                c->setAxis(curve[curve.count() - 5].toInt(),
                                           curve[curve.count() - 4].toInt());
                                c->setVisible(curve.last().toInt());
                            }
                        }
                    }
                }
                if (curve_loaded)
                    curveID++;
            }
        } else if (s.contains("FunctionCurve")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList curve = s.split("\t", Qt::KeepEmptyParts);
#else
            QStringList curve = s.split("\t", QString::KeepEmptyParts);
#endif
            CurveLayout cl;
            cl.connectType = curve[6].toInt();
            cl.lCol = COLORUINT(curve[7]);
            cl.lStyle = curve[8].toInt();
            cl.lWidth = curve[9].toInt();
            cl.sSize = curve[10].toInt();
            cl.sType = curve[11].toInt();
            cl.symCol = COLORUINT(curve[12]);
            if (curve[13] == "-1")
                cl.symbolFill = false;
            else {
                cl.symbolFill = true;
                cl.fillCol = COLORUINT(curve[13]);
            }
            cl.filledArea = curve[14].toInt();
            cl.aCol = COLORUINT(curve[15]);
            cl.aStyle = curve[16].toInt();
            int current_index = 17;
            if (curve.count() < 16)
                cl.penWidth = cl.lWidth;
            else if ((d_file_version >= 79) && (curve[5].toInt() == Graph::Box)) {
                cl.penWidth = curve[17].toInt();
                current_index++;
            } else if ((d_file_version >= 78) && (curve[5].toInt() <= Graph::LineSymbols)) {
                cl.penWidth = curve[17].toInt();
                current_index++;
            } else
                cl.penWidth = cl.lWidth;

            if (d_file_version >= 0x011800) // 1.24.0
            {
                // skeep capStyle, joinStyle and custom dash pattern values
                current_index += 3;
            }

            QStringList func_spec;
            func_spec << curve[1];

            j++;
            while (list[j] == "<formula>") { // d_file_version >= 0x000105
                QString formula;
                for (j++; list[j] != "</formula>"; j++)
                    formula += list[j] + "\n";
                func_spec << formula;
                j++;
            }
            j--;

            if (ag->insertFunctionCurve(app, func_spec, curve[2].toInt(), d_file_version)) {
                ag->setCurveType(curveID, (Graph::CurveType)curve[5].toInt(), false);
                ag->updateCurveLayout(curveID, &cl);
                if (d_file_version >= 88) {
                    QwtPlotCurve *c = ag->curve(curveID);
                    if (c) {
                        if (current_index + 1 < curve.size())
                            c->setAxis(curve[current_index].toInt(),
                                       curve[current_index + 1].toInt());
                        if (d_file_version >= 90 && current_index + 2 < curve.size())
                            c->setVisible(curve.last().toInt());
                        else
                            c->setVisible(true);
                    }
                }
                if (ag->curve(curveID))
                    curveID++;
            }
        } else if (s.contains("ErrorBars")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList curve = s.split("\t", Qt::SkipEmptyParts);
#else
            QStringList curve = s.split("\t", QString::SkipEmptyParts);
#endif
            Table *w = app->table(curve[3]);
            Table *errTable = app->table(curve[4]);
            if (w && errTable) {
                ag->addErrorBars(curve[2], curve[3], errTable, curve[4], curve[1].toInt(),
                                 curve[5].toInt(), curve[6].toInt(), QColor(COLORVALUE(curve[7])),
                                 curve[8].toInt(), curve[10].toInt(), curve[9].toInt());
            }
            curveID++;
        } else if (s == "<spectrogram>") {
            curveID++;
            QStringList lst;
            while (s != "</spectrogram>") {
                s = list[++j];
                lst << s;
            }
            lst.pop_back();
            ag->restoreSpectrogram(app, lst);
        } else if (s.left(6) == "scale\t") {
            QStringList scl = s.split("\t");
            scl.pop_front();
            if (d_file_version < 88) {
                double step = scl[2].toDouble();
                if (scl[5] == "0")
                    step = 0.0;
                ag->setScale(QwtPlot::xBottom, scl[0].toDouble(), scl[1].toDouble(), step,
                             scl[3].toInt(), scl[4].toInt(), scl[6].toInt(), bool(scl[7].toInt()));
                ag->setScale(QwtPlot::xTop, scl[0].toDouble(), scl[1].toDouble(), step,
                             scl[3].toInt(), scl[4].toInt(), scl[6].toInt(), bool(scl[7].toInt()));

                step = scl[10].toDouble();
                if (scl[13] == "0")
                    step = 0.0;
                ag->setScale(QwtPlot::yLeft, scl[8].toDouble(), scl[9].toDouble(), step,
                             scl[11].toInt(), scl[12].toInt(), scl[14].toInt(),
                             bool(scl[15].toInt()));
                ag->setScale(QwtPlot::yRight, scl[8].toDouble(), scl[9].toDouble(), step,
                             scl[11].toInt(), scl[12].toInt(), scl[14].toInt(),
                             bool(scl[15].toInt()));
            } else
                ag->setScale(scl[0].toInt(), scl[1].toDouble(), scl[2].toDouble(),
                             scl[3].toDouble(), scl[4].toInt(), scl[5].toInt(), scl[6].toInt(),
                             bool(scl[7].toInt()));
        } else if (s.contains("PlotTitle")) {
            QStringList fList = s.split("\t");
            ag->setTitle(fList[1]);
            ag->setTitleColor(QColor(COLORVALUE(fList[2])));
            ag->setTitleAlignment(fList[3].toInt());
        } else if (s.contains("TitleFont")) {
            QStringList fList = s.split("\t");
            QFont fnt = QFont(fList[1], fList[2].toInt(), fList[3].toInt(), fList[4].toInt());
            fnt.setUnderline(fList[5].toInt());
            fnt.setStrikeOut(fList[6].toInt());
            ag->setTitleFont(fnt);
        } else if (s.contains("AxesTitles")) {
            QStringList legend = s.split("\t");
            legend.pop_front();
            for (int i = 0; i < 4; i++) {
                if (legend.count() > i)
                    ag->setAxisTitle(Graph::mapToQwtAxis(i), legend[i]);
            }
        } else if (s.contains("AxesTitleColors")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList colors = s.split("\t", Qt::SkipEmptyParts);
#else
            QStringList colors = s.split("\t", QString::SkipEmptyParts);
#endif
            ag->setAxesTitleColor(colors);
        } else if (s.contains("AxesTitleAlignment")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList align = s.split("\t", Qt::SkipEmptyParts);
#else
            QStringList align = s.split("\t", QString::SkipEmptyParts);
#endif
            ag->setAxesTitlesAlignment(align);
        } else if (s.contains("ScaleFont")) {
            QStringList fList = s.split("\t");
            QFont fnt = QFont(fList[1], fList[2].toInt(), fList[3].toInt(), fList[4].toInt());
            fnt.setUnderline(fList[5].toInt());
            fnt.setStrikeOut(fList[6].toInt());

            int axis = (fList[0].right(1)).toInt();
            ag->setAxisTitleFont(axis, fnt);
        } else if (s.contains("AxisFont")) {
            QStringList fList = s.split("\t");
            QFont fnt = QFont(fList[1], fList[2].toInt(), fList[3].toInt(), fList[4].toInt());
            fnt.setUnderline(fList[5].toInt());
            fnt.setStrikeOut(fList[6].toInt());

            int axis = (fList[0].right(1)).toInt();
            ag->setAxisFont(axis, fnt);
        } else if (s.contains("AxesFormulas")) {
            QStringList fList = s.split("\t");
            fList.removeAll(fList.first());
            ag->setAxesFormulas(fList);
        } else if (s.startsWith("<AxisFormula ")) {
            int pos = s.mid(18, s.length() - 20).toInt();
            QString formula;
            for (j++; j < (int)list.count() && list[j] != "</AxisFormula>"; j++)
                formula += list[j] + "\n";
            formula.truncate(formula.length() - 1);
            ag->setAxisFormula(pos, formula);
        } else if (s.contains("LabelsFormat")) {
            QStringList fList = s.split("\t");
            fList.pop_front();
            ag->setLabelsNumericFormat(fList);
        } else if (s.contains("LabelsRotation")) {
            QStringList fList = s.split("\t");
            ag->setAxisLabelRotation(QwtPlot::xBottom, fList[1].toInt());
            ag->setAxisLabelRotation(QwtPlot::xTop, fList[2].toInt());
        } else if (s.contains("DrawAxesBackbone")) {
            QStringList fList = s.split("\t");
            ag->loadAxesOptions(fList[1]);
        } else if (s.contains("AxesLineWidth")) {
            QStringList fList = s.split("\t");
            ag->loadAxesLinewidth(fList[1].toInt());
        } else if (s.contains("CanvasFrame")) {
            QStringList list = s.split("\t");
            ag->drawCanvasFrame(list);
        } else if (s.contains("CanvasBackground")) {
            QStringList list = s.split("\t");
            QColor c = QColor(COLORVALUE(list[1]));
            if (list.count() == 3)
                c.setAlpha(list[2].toInt());
            ag->setCanvasBackground(c);
        } else if (s.contains("Legend")) { // version <= 0.8.9
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.split("\t", Qt::KeepEmptyParts);
#else
            QStringList fList = s.split("\t", QString::KeepEmptyParts);
#endif
            ag->insertLegend(fList, d_file_version);
        } else if (s.startsWith("<legend>") && s.endsWith("</legend>")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.remove("</legend>").split("\t", Qt::KeepEmptyParts);
#else
            QStringList fList = s.remove("</legend>").split("\t", QString::KeepEmptyParts);
#endif
            ag->insertLegend(fList, d_file_version);
        } else if (s.contains("textMarker")) { // version <= 0.8.9
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.split("\t", Qt::KeepEmptyParts);
#else
            QStringList fList = s.split("\t", QString::KeepEmptyParts);
#endif
            ag->insertTextMarker(fList, d_file_version);
        } else if (s.startsWith("<text>") && s.endsWith("</text>")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QStringList fList = s.remove("</text>").split("\t", Qt::KeepEmptyParts);
#else
            QStringList fList = s.remove("</text>").split("\t", QString::KeepEmptyParts);
#endif
            ag->insertTextMarker(fList, d_file_version);
        } else if (s.contains("lineMarker")) { // version <= 0.8.9
            QStringList fList = s.split("\t");
            ag->addArrow(fList, d_file_version);
        } else if (s.startsWith("<line>") && s.endsWith("</line>")) {
            QStringList fList = s.remove("</line>").split("\t");
            ag->addArrow(fList, d_file_version);
        } else if (s.contains("ImageMarker")
                   || (s.startsWith("<image>") && s.endsWith("</image>"))) {
            QStringList fList = s.remove("</image>").split("\t");
            ag->insertImageMarker(fList, d_file_version);
        } else if (s.contains("AxisType")) {
            QStringList fList = s.split("\t");
            if (fList.size() >= 5)
                for (int i = 0; i < 4; i++) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                    QStringList lst = fList[i + 1].split(";", Qt::SkipEmptyParts);
#else
                    QStringList lst = fList[i + 1].split(";", QString::SkipEmptyParts);
#endif
                    if (lst.size() < 2)
                        continue;
                    Graph::AxisType format = static_cast<Graph::AxisType>(lst[0].toInt());
                    switch (format) {
                    case Graph::AxisType::Day:
                        ag->setLabelsDayFormat(i, lst[1].toInt());
                        break;
                    case Graph::AxisType::Month:
                        ag->setLabelsMonthFormat(i, lst[1].toInt());
                        break;
                    case Graph::AxisType::Time:
                    case Graph::AxisType::Date:
                    case Graph::AxisType::DateTime:
                        ag->setLabelsDateTimeFormat(i, format, lst[1] + ";" + lst[2]);
                        break;
                    case Graph::AxisType::Txt:
                        ag->setLabelsTextFormat(i, app->table(lst[1]), lst[1]);
                        break;
                    case Graph::AxisType::ColHeader:
                        ag->setLabelsColHeaderFormat(i, app->table(lst[1]));
                        break;
                    }
                }
        } else if (d_file_version < 69 && s.contains("AxesTickLabelsCol")) {
            QStringList fList = s.split("\t");
            if (fList.size() >= 5) {
                const QList<Graph::AxisType> &axesTypes = ag->axesType();
                for (int i = 0; i < 4; i++) {
                    switch (axesTypes[i]) {
                    case Graph::AxisType::Txt:
                        ag->setLabelsTextFormat(i, app->table(fList[i + 1]), fList[i + 1]);
                        break;
                    case Graph::AxisType::ColHeader:
                        ag->setLabelsColHeaderFormat(i, app->table(fList[i + 1]));
                        break;
                    }
                }
            }
        }
    }
    ag->replot();
    if (ag->isPiePlot()) {
        QwtPieCurve *c = (QwtPieCurve *)ag->curve(0);
        if (c)
            c->updateBoundingRect();
    }

    ag->blockSignals(false);
    ag->setIgnoreResizeEvents(!app->autoResizeLayers);
    ag->setAutoscaleFonts(app->autoScaleFonts);
    ag->setTextMarkerDefaults(app->legendFrameStyle, app->plotLegendFont, app->legendTextColor,
                              app->legendBackground);
    ag->setArrowDefaults(app->defaultArrowLineWidth, app->defaultArrowColor,
                         app->defaultArrowLineStyle, app->defaultArrowHeadLength,
                         app->defaultArrowHeadAngle, app->defaultArrowHeadFill);
    return ag;
}

Graph3D *ApplicationWindow::openSurfacePlot(ApplicationWindow *app, const QStringList &lst)
{
    QStringList fList = lst[0].split("\t");
    QString caption = fList[0];
    QString date = fList[1];
    if (date.isEmpty())
        date = QLocale().toString(QDateTime::currentDateTime());

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    fList = lst[2].split("\t", Qt::SkipEmptyParts);
#else
    fList = lst[2].split("\t", QString::SkipEmptyParts);
#endif
    Graph3D *plot = 0;

    if (fList[1].endsWith("(Y)", Qt::CaseSensitive)) // Ribbon plot
        plot = app->dataPlot3D(caption, fList[1], fList[2].toDouble(), fList[3].toDouble(),
                               fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(),
                               fList[7].toDouble());
    else if (fList[1].contains("(Z)", Qt::CaseSensitive))
        plot = app->dataPlotXYZ(caption, fList[1], fList[2].toDouble(), fList[3].toDouble(),
                                fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(),
                                fList[7].toDouble());
    else if (fList[1].startsWith("matrix<", Qt::CaseSensitive)
             && fList[1].endsWith(">", Qt::CaseInsensitive))
        plot = app->openMatrixPlot3D(caption, fList[1], fList[2].toDouble(), fList[3].toDouble(),
                                     fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(),
                                     fList[7].toDouble());
    else
        plot = app->newPlot3D(caption, fList[1], fList[2].toDouble(), fList[3].toDouble(),
                              fList[4].toDouble(), fList[5].toDouble(), fList[6].toDouble(),
                              fList[7].toDouble());

    if (!plot)
        return 0;

    app->setListViewDate(caption, date);
    plot->setBirthDate(date);
    plot->setIgnoreFonts(true);
    restoreWindowGeometry(app, plot, lst[1]);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    fList = lst[3].split("\t", Qt::SkipEmptyParts);
    plot->setStyle(fList);

    fList = lst[4].split("\t", Qt::SkipEmptyParts);
    plot->setGrid(fList[1].toInt());

    fList = lst[5].split("\t");
    plot->setTitle(fList);

    fList = lst[6].split("\t", Qt::SkipEmptyParts);
    plot->setColors(fList);

    fList = lst[7].split("\t", Qt::SkipEmptyParts);
    fList.pop_front();
    plot->setAxesLabels(fList);

    fList = lst[8].split("\t", Qt::SkipEmptyParts);
    plot->setTicks(fList);

    fList = lst[9].split("\t", Qt::SkipEmptyParts);
    plot->setTickLengths(fList);

    fList = lst[10].split("\t", Qt::SkipEmptyParts);
    plot->setOptions(fList);

    fList = lst[11].split("\t", Qt::SkipEmptyParts);
    plot->setNumbersFont(fList);

    fList = lst[12].split("\t", Qt::SkipEmptyParts);
    plot->setXAxisLabelFont(fList);

    fList = lst[13].split("\t", Qt::SkipEmptyParts);
    plot->setYAxisLabelFont(fList);

    fList = lst[14].split("\t", Qt::SkipEmptyParts);
    plot->setZAxisLabelFont(fList);

    fList = lst[15].split("\t", Qt::SkipEmptyParts);
    plot->setRotation(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[16].split("\t", Qt::SkipEmptyParts);
    plot->setZoom(fList[1].toDouble());

    fList = lst[17].split("\t", Qt::SkipEmptyParts);
    plot->setScale(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[18].split("\t", Qt::SkipEmptyParts);
    plot->setShift(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[19].split("\t", Qt::SkipEmptyParts);
    plot->setMeshLineWidth(fList[1].toInt());
#else
    fList = lst[3].split("\t", QString::SkipEmptyParts);
    plot->setStyle(fList);

    fList = lst[4].split("\t", QString::SkipEmptyParts);
    plot->setGrid(fList[1].toInt());

    fList = lst[5].split("\t");
    plot->setTitle(fList);

    fList = lst[6].split("\t", QString::SkipEmptyParts);
    plot->setColors(fList);

    fList = lst[7].split("\t", QString::SkipEmptyParts);
    fList.pop_front();
    plot->setAxesLabels(fList);

    fList = lst[8].split("\t", QString::SkipEmptyParts);
    plot->setTicks(fList);

    fList = lst[9].split("\t", QString::SkipEmptyParts);
    plot->setTickLengths(fList);

    fList = lst[10].split("\t", QString::SkipEmptyParts);
    plot->setOptions(fList);

    fList = lst[11].split("\t", QString::SkipEmptyParts);
    plot->setNumbersFont(fList);

    fList = lst[12].split("\t", QString::SkipEmptyParts);
    plot->setXAxisLabelFont(fList);

    fList = lst[13].split("\t", QString::SkipEmptyParts);
    plot->setYAxisLabelFont(fList);

    fList = lst[14].split("\t", QString::SkipEmptyParts);
    plot->setZAxisLabelFont(fList);

    fList = lst[15].split("\t", QString::SkipEmptyParts);
    plot->setRotation(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[16].split("\t", QString::SkipEmptyParts);
    plot->setZoom(fList[1].toDouble());

    fList = lst[17].split("\t", QString::SkipEmptyParts);
    plot->setScale(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[18].split("\t", QString::SkipEmptyParts);
    plot->setShift(fList[1].toDouble(), fList[2].toDouble(), fList[3].toDouble());

    fList = lst[19].split("\t", QString::SkipEmptyParts);
    plot->setMeshLineWidth(fList[1].toInt());
#endif

    if (d_file_version > 71) {
        fList = lst[20].split(
                "\t"); // using QString::SkipEmptyParts here causes a crash for empty window labels
        plot->setWindowLabel(fList[1]);
        plot->setCaptionPolicy((MyWidget::CaptionPolicy)fList[2].toInt());
        app->setListViewLabel(plot->name(), fList[1]);
    }

    if (d_file_version >= 88) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        fList = lst[21].split("\t", Qt::SkipEmptyParts);
#else
        fList = lst[21].split("\t", QString::SkipEmptyParts);
#endif
        plot->setOrtho(fList[1].toInt());
    }

    plot->update();
    plot->setIgnoreFonts(true);
    return plot;
}

void ApplicationWindow::copyActiveLayer()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    copiedLayer = true;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    delete lastCopiedLayer;
    lastCopiedLayer = new Graph(0, 0, Qt::Widget);
    lastCopiedLayer->setAttribute(Qt::WA_DeleteOnClose);
    lastCopiedLayer->setGeometry(0, 0, g->width(), g->height());
    lastCopiedLayer->copy(this, g);
    g->copyImage();
}

void ApplicationWindow::showDataSetDialog(const QString &whichFit)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g)
        return;

    DataSetDialog *ad = new DataSetDialog(tr("Curve") + ": ", this);
    ad->setAttribute(Qt::WA_DeleteOnClose);
    ad->setGraph(g);
    ad->setOperationType(whichFit);
    ad->exec();
}

void ApplicationWindow::analyzeCurve(Graph *g, const QString &whichFit, const QString &curveTitle)
{
    if (whichFit == "fitLinear" || whichFit == "fitSigmoidal" || whichFit == "fitGauss"
        || whichFit == "fitLorentz") {
        Fit *fitter = 0;
        if (whichFit == "fitLinear")
            fitter = new LinearFit(this, g);
        else if (whichFit == "fitSigmoidal")
            fitter = new SigmoidalFit(this, g);
        else if (whichFit == "fitGauss")
            fitter = new GaussFit(this, g);
        else if (whichFit == "fitLorentz")
            fitter = new LorentzFit(this, g);

        if (fitter->setDataFromCurve(curveTitle)) {
            if (whichFit != "fitLinear")
                fitter->guessInitialValues();

            fitter->scaleErrors(fit_scale_errors);
            fitter->setOutputPrecision(fit_output_precision);

            if (whichFit == "fitLinear" && d_2_linear_fit_points)
                fitter->generateFunction(generateUniformFitPoints, 2);
            else
                fitter->generateFunction(generateUniformFitPoints, fitPoints);
            fitter->fit();
            if (pasteFitResultsToPlot)
                fitter->showLegend();
            delete fitter;
        }
    } else if (whichFit == "differentiate") {
        Differentiation *diff = new Differentiation(this, g, curveTitle);
        diff->run();
        delete diff;
    }
}

void ApplicationWindow::analysis(const QString &whichFit)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("MultiLayer"))
        return;

    Graph *g = ((MultiLayer *)d_workspace.activeSubWindow())->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    QString curve_title = g->selectedCurveTitle();
    if (!curve_title.isNull()) {
        analyzeCurve(g, whichFit, curve_title);
        return;
    }

    QStringList lst = g->analysableCurvesList();
    if (lst.count() == 1) {
        const QwtPlotCurve *c = g->curve(lst[0]);
        if (c)
            analyzeCurve(g, whichFit, lst[0]);
    } else
        showDataSetDialog(whichFit);
}

void ApplicationWindow::pickPointerCursor()
{
    btnPointer->setChecked(true);
}

void ApplicationWindow::pickDataTool(QAction *action)
{
    if (!action)
        return;

    MultiLayer *m = qobject_cast<MultiLayer *>(d_workspace.activeSubWindow());
    if (!m)
        return;

    Graph *g = m->activeGraph();
    if (!g)
        return;

    g->disableTools();

    if (action == btnCursor)
        showCursor();
    else if (action == btnSelect)
        showRangeSelectors();
    else if (action == btnPicker)
        showScreenReader();
    else if (action == btnMovePoints)
        movePoints();
    else if (action == btnRemovePoints)
        removePoints();
    else if (action == btnZoomIn)
        zoomIn();
    else if (action == btnZoomOut)
        zoomOut();
    else if (action == btnArrow)
        drawArrow();
    else if (action == btnLine)
        drawLine();
}

void ApplicationWindow::connectSurfacePlot(Graph3D *plot)
{
    connect(plot, SIGNAL(showTitleBarMenu()), this, SLOT(showWindowTitleBarMenu()));
    connect(plot, SIGNAL(showContextMenu()), this, SLOT(showWindowContextMenu()));
    connect(plot, SIGNAL(showOptionsDialog()), this, SLOT(showPlot3dDialog()));
    connect(plot, SIGNAL(closedWindow(MyWidget *)), this, SLOT(closeWindow(MyWidget *)));
    connect(plot, SIGNAL(hiddenWindow(MyWidget *)), this, SLOT(hideWindow(MyWidget *)));
    connect(plot, SIGNAL(statusChanged(MyWidget *)), this, SLOT(updateWindowStatus(MyWidget *)));
    connect(plot, SIGNAL(modified()), this, SIGNAL(modified()));
    connect(plot, SIGNAL(custom3DActions(MyWidget *)), this, SLOT(custom3DActions(MyWidget *)));

    plot->askOnCloseEvent(confirmClosePlot3D);
}

void ApplicationWindow::connectMultilayerPlot(MultiLayer *g)
{
    connect(g, SIGNAL(showTitleBarMenu()), this, SLOT(showWindowTitleBarMenu()));
    connect(g, SIGNAL(showTextDialog()), this, SLOT(showTextDialog()));
    connect(g, SIGNAL(showPlotDialog(int)), this, SLOT(showPlotDialog(int)));
    connect(g, SIGNAL(showScaleDialog(int)), this, SLOT(showScalePageFromAxisDialog(int)));
    connect(g, SIGNAL(showAxisDialog(int)), this, SLOT(showAxisPageFromAxisDialog(int)));
    connect(g, SIGNAL(showCurveContextMenu(int)), this, SLOT(showCurveContextMenu(int)));
    connect(g, SIGNAL(showWindowContextMenu()), this, SLOT(showWindowContextMenu()));
    connect(g, SIGNAL(showCurvesDialog()), this, SLOT(showCurvesDialog()));
    connect(g, SIGNAL(drawLineEnded(bool)), btnPointer, SLOT(setChecked(bool)));
    connect(g, SIGNAL(drawTextOff()), this, SLOT(disableAddText()));
    connect(g, SIGNAL(showXAxisTitleDialog()), this, SLOT(showXAxisTitleDialog()));
    connect(g, SIGNAL(showYAxisTitleDialog()), this, SLOT(showYAxisTitleDialog()));
    connect(g, SIGNAL(showRightAxisTitleDialog()), this, SLOT(showRightAxisTitleDialog()));
    connect(g, SIGNAL(showTopAxisTitleDialog()), this, SLOT(showTopAxisTitleDialog()));
    connect(g, SIGNAL(showMarkerPopupMenu()), this, SLOT(showMarkerPopupMenu()));
    connect(g, SIGNAL(closedWindow(MyWidget *)), this, SLOT(closeWindow(MyWidget *)));
    connect(g, SIGNAL(hiddenWindow(MyWidget *)), this, SLOT(hideWindow(MyWidget *)));
    connect(g, SIGNAL(statusChanged(MyWidget *)), this, SLOT(updateWindowStatus(MyWidget *)));
    connect(g, SIGNAL(cursorInfo(const QString &)), d_status_info, SLOT(setText(const QString &)));
    connect(g, SIGNAL(showImageDialog()), this, SLOT(showImageDialog()));
    connect(g, SIGNAL(createTable(const QString &, const QString &, QList<Column *>)), this,
            SLOT(newTable(const QString &, const QString &, QList<Column *>)));
    connect(g, SIGNAL(viewTitleDialog()), this, SLOT(showTitleDialog()));
    connect(g, SIGNAL(modifiedWindow(MyWidget *)), this, SLOT(modifiedProject(MyWidget *)));
    connect(g, SIGNAL(modifiedPlot()), this, SLOT(modifiedProject()));
    connect(g, SIGNAL(showLineDialog()), this, SLOT(showLineDialog()));
    connect(g, SIGNAL(showGeometryDialog()), this, SLOT(showPlotGeometryDialog()));
    connect(g, SIGNAL(pasteMarker()), this, SLOT(pasteSelection()));
    connect(g, SIGNAL(showGraphContextMenu()), this, SLOT(showGraphContextMenu()));
    connect(g, SIGNAL(showLayerButtonContextMenu()), this, SLOT(showLayerButtonContextMenu()));
    connect(g, SIGNAL(createIntensityTable(const QString &)), this,
            SLOT(importImage(const QString &)));
    connect(g, SIGNAL(setPointerCursor()), this, SLOT(pickPointerCursor()));

    g->askOnCloseEvent(confirmClosePlot2D);
}

void ApplicationWindow::connectTable(Table *w)
{
    connect(w, SIGNAL(showTitleBarMenu()), this, SLOT(showWindowTitleBarMenu()));
    connect(w, SIGNAL(statusChanged(MyWidget *)), this, SLOT(updateWindowStatus(MyWidget *)));
    connect(w, SIGNAL(hiddenWindow(MyWidget *)), this, SLOT(hideWindow(MyWidget *)));
    connect(w, SIGNAL(closedWindow(MyWidget *)), this, SLOT(closeWindow(MyWidget *)));
    connect(w, SIGNAL(aboutToRemoveCol(const QString &)), this,
            SLOT(removeCurves(const QString &)));
    connect(w, SIGNAL(modifiedData(Table *, const QString &)), this,
            SLOT(updateCurves(Table *, const QString &)));
    connect(w, SIGNAL(modifiedWindow(MyWidget *)), this, SLOT(modifiedProject(MyWidget *)));
    connect(w, SIGNAL(changedColHeader(const QString &, const QString &)), this,
            SLOT(updateColNames(const QString &, const QString &)));

#ifdef LEGACY_CODE_0_2_x
    connect(w->d_future_table, SIGNAL(requestRowStatistics()), this, SLOT(showRowStatistics()));
    connect(w->d_future_table, SIGNAL(requestColumnStatistics()), this, SLOT(showColStatistics()));
#endif
    w->askOnCloseEvent(confirmCloseTable);
}

void ApplicationWindow::setAppColors(const QColor &wc, const QColor &pc, const QColor &tpc)
{
    if (workspaceColor != wc) {
        workspaceColor = wc;
        d_workspace.setBackground(wc);
    }

    if (panelsColor == pc && panelsTextColor == tpc)
        return;

    panelsColor = pc;
    panelsTextColor = tpc;

    QPalette cg;
    cg.setColor(QPalette::Base, QColor(panelsColor));
    qApp->setPalette(cg);

    cg.setColor(QPalette::Text, QColor(panelsTextColor));
    cg.setColor(QPalette::WindowText, QColor(panelsTextColor));
    cg.setColor(QPalette::HighlightedText, QColor(panelsTextColor));
    lv.setPalette(cg);
    results->setPalette(cg);
}

void ApplicationWindow::setPlot3DOptions()
{
    QList<MyWidget *> windows = windowsList();
    for (int i = 0; i < int(windows.count()); i++) {
        if (windows.at(i)->inherits("Graph3D")) {
            Graph3D *g = (Graph3D *)windows.at(i);
            g->setSmoothMesh(smooth3DMesh);
            g->setOrtho(orthogonal3DPlots);
            g->setAutoscale(autoscale3DPlots);
        }
    }
}

void ApplicationWindow::createActions()
{
    actionNewProject = new QAction(QIcon(QPixmap(":/new.xpm")), tr("New &Project"), this);
    actionNewProject->setShortcut(tr("Ctrl+N"));
    connect(actionNewProject, SIGNAL(triggered()), this, SLOT(newProject()));

    actionNewGraph = new QAction(QIcon(QPixmap(":/new_graph.xpm")), tr("New &Graph"), this);
    actionNewGraph->setShortcut(tr("Ctrl+G"));
    connect(actionNewGraph, SIGNAL(triggered()), this, SLOT(newGraph()));

    actionNewNote = new QAction(QIcon(QPixmap(":/new_note.xpm")), tr("New &Note / Script"), this);
    actionNewNote->setShortcut(tr("Ctrl+ALT+N"));
    connect(actionNewNote, SIGNAL(triggered()), this, SLOT(newNote()));

    actionNewTable = new QAction(QIcon(QPixmap(":/table.xpm")), tr("New &Table"), this);
    actionNewTable->setShortcut(tr("Ctrl+T"));
    connect(actionNewTable, SIGNAL(triggered()), this, SLOT(newTable()));

    actionNewMatrix = new QAction(QIcon(QPixmap(":/new_matrix.xpm")), tr("New &Matrix"), this);
    actionNewMatrix->setShortcut(tr("Ctrl+M"));
    connect(actionNewMatrix, SIGNAL(triggered()), this, SLOT(newMatrix()));

    actionNewFunctionPlot =
            new QAction(QIcon(QPixmap(":/newF.xpm")), tr("New &Function Plot"), this);
    actionNewFunctionPlot->setShortcut(tr("Ctrl+F"));
    connect(actionNewFunctionPlot, SIGNAL(triggered()), this, SLOT(functionDialog()));

    actionNewSurfacePlot =
            new QAction(QIcon(QPixmap(":/newFxy.xpm")), tr("New 3D &Surface Plot"), this);
    actionNewSurfacePlot->setShortcut(tr("Ctrl+ALT+Z"));
    connect(actionNewSurfacePlot, SIGNAL(triggered()), this, SLOT(newSurfacePlot()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionOpen = new QAction(QIcon(QPixmap(":/fileopen.xpm")), tr("&Open") + "...", this);
    actionOpen->setShortcut(tr("Ctrl+O"));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    actionLoadImage = new QAction(tr("Open Image &File"), this);
    actionLoadImage->setShortcut(tr("Ctrl+I"));
    connect(actionLoadImage, SIGNAL(triggered()), this, SLOT(loadImage()));

    actionImportImage = new QAction(tr("Import I&mage..."), this);
    connect(actionImportImage, SIGNAL(triggered()), this, SLOT(importImage()));

    actionSaveProject = new QAction(QIcon(QPixmap(":/filesave.xpm")), tr("&Save Project"), this);
    actionSaveProject->setShortcut(tr("Ctrl+S"));
    connect(actionSaveProject, SIGNAL(triggered()), this, SLOT(saveProject()));
    savedProject();

    actionSaveProjectAs = new QAction(tr("Save Project &As..."), this);
    connect(actionSaveProjectAs, SIGNAL(triggered()), this, SLOT(saveProjectAs()));

    actionOpenTemplate =
            new QAction(QIcon(QPixmap(":/open_template.xpm")), tr("Open Temp&late..."), this);
    connect(actionOpenTemplate, SIGNAL(triggered()), this, SLOT(openTemplate()));

    actionSaveTemplate =
            new QAction(QIcon(QPixmap(":/save_template.xpm")), tr("Save As &Template..."), this);
    connect(actionSaveTemplate, SIGNAL(triggered()), this, SLOT(saveAsTemplate()));

    actionSaveNote = new QAction(tr("Save Note As..."), this);
    connect(actionSaveNote, SIGNAL(triggered()), this, SLOT(saveNoteAs()));

    actionLoad = new QAction(QIcon(QPixmap(":/import.xpm")), tr("&Import ASCII..."), this);
    connect(actionLoad, SIGNAL(triggered()), this, SLOT(importASCII()));

    actionUndo = new QAction(IconLoader::load("edit-undo"), tr("&Undo"), this);
    actionUndo->setShortcut(tr("Ctrl+Z"));
    connect(actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
    actionUndo->setEnabled(false);

    actionRedo = new QAction(IconLoader::load("edit-redo"), tr("&Redo"), this);
    actionRedo->setShortcut(tr("Ctrl+R"));
    connect(actionRedo, SIGNAL(triggered()), this, SLOT(redo()));
    actionRedo->setEnabled(false);

    actionCopyWindow = new QAction(QIcon(QPixmap(":/duplicate.xpm")), tr("&Duplicate"), this);
    connect(actionCopyWindow, SIGNAL(triggered()), this, SLOT(clone()));

    actionCutSelection = new QAction(IconLoader::load("edit-cut"), tr("Cu&t Selection"), this);
    actionCutSelection->setShortcut(tr("Ctrl+X"));
    connect(actionCutSelection, SIGNAL(triggered()), this, SLOT(cutSelection()));

    actionCopySelection = new QAction(IconLoader::load("edit-copy"), tr("&Copy Selection"), this);
    actionCopySelection->setShortcut(tr("Ctrl+C"));
    connect(actionCopySelection, SIGNAL(triggered()), this, SLOT(copySelection()));

    actionPasteSelection =
            new QAction(IconLoader::load("edit-paste"), tr("&Paste Selection"), this);
    actionPasteSelection->setShortcut(tr("Ctrl+V"));
    connect(actionPasteSelection, SIGNAL(triggered()), this, SLOT(pasteSelection()));

    actionClearSelection =
            new QAction(QIcon(QPixmap(":/erase.xpm")), tr("&Delete Selection"), this);
    actionClearSelection->setShortcut(tr("Del", "delete key"));
    connect(actionClearSelection, SIGNAL(triggered()), this, SLOT(clearSelection()));

    locktoolbar = new QAction(QIcon(QPixmap(":/unlock.xpm")), tr("&Lock Toolbars"), this);
    locktoolbar->setCheckable(true);
    connect(locktoolbar, SIGNAL(toggled(bool)), this, SLOT(lockToolbar(bool)));

    actionShowExplorer = explorerWindow.toggleViewAction();
    actionShowExplorer->setIcon(QPixmap(":/folder.xpm"));
    actionShowExplorer->setShortcut(tr("Ctrl+E"));

    actionShowLog = logWindow.toggleViewAction();
    actionShowLog->setIcon(QPixmap(":/log.xpm"));

    actionShowHistory = new QAction(tr("Undo/Redo &History"), this);
    connect(actionShowHistory, SIGNAL(triggered(bool)), this, SLOT(showHistory()));

#ifdef SCRIPTING_CONSOLE
    actionShowConsole = consoleWindow.toggleViewAction();
#endif

    actionAddLayer = new QAction(QIcon(QPixmap(":/newLayer.xpm")), tr("Add La&yer"), this);
    actionAddLayer->setShortcut(tr("ALT+L"));
    connect(actionAddLayer, SIGNAL(triggered()), this, SLOT(addLayer()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionShowLayerDialog =
            new QAction(QIcon(QPixmap(":/arrangeLayers.xpm")), tr("Arran&ge Layers") + "...", this);
    actionShowLayerDialog->setShortcut(tr("ALT+A"));
    connect(actionShowLayerDialog, SIGNAL(triggered()), this, SLOT(showLayerDialog()));

    actionAutomaticLayout =
            new QAction(QIcon(QPixmap(":/auto_layout.xpm")), tr("Automatic Layout"), this);
    connect(actionAutomaticLayout, SIGNAL(triggered()), this, SLOT(autoArrangeLayers()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportGraph = new QAction(tr("&Current") + "...", this);
    actionExportGraph->setShortcut(tr("Alt+G"));
    connect(actionExportGraph, SIGNAL(triggered()), this, SLOT(exportGraph()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportAllGraphs = new QAction(tr("&All") + "...", this);
    actionExportAllGraphs->setShortcut(tr("Alt+X"));
    connect(actionExportAllGraphs, SIGNAL(triggered()), this, SLOT(exportAllGraphs()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportPDF = new QAction(QIcon(QPixmap(":/pdf.xpm")), tr("&Export PDF") + "...", this);
    actionExportPDF->setShortcut(tr("Ctrl+Alt+P"));
    connect(actionExportPDF, SIGNAL(triggered()), this, SLOT(exportPDF()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionPrint = new QAction(QIcon(QPixmap(":/fileprint.xpm")), tr("&Print") + "...", this);
    actionPrint->setShortcut(tr("Ctrl+P"));
    connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));

    actionPrintAllPlots = new QAction(tr("Print All Plo&ts"), this);
    connect(actionPrintAllPlots, SIGNAL(triggered()), this, SLOT(printAllPlots()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionShowExportASCIIDialog = new QAction(tr("E&xport ASCII") + "...", this);
    connect(actionShowExportASCIIDialog, SIGNAL(triggered()), this, SLOT(showExportASCIIDialog()));

    actionCloseAllWindows = new QAction(QIcon(QPixmap(":/quit.xpm")), tr("&Quit"), this);
    actionCloseAllWindows->setShortcut(tr("Ctrl+Q"));
    connect(actionCloseAllWindows, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    actionClearLogInfo = new QAction(tr("Clear &Log Information"), this);
    connect(actionClearLogInfo, SIGNAL(triggered()), this, SLOT(clearLogInfo()));

    actionDeleteFitTables =
            new QAction(QIcon(QPixmap(":/close.xpm")), tr("Delete &Fit Tables"), this);
    connect(actionDeleteFitTables, SIGNAL(triggered()), this, SLOT(deleteFitTables()));

    // FIXME: "..." should be added before translating, but this would break translations
    actionShowPlotWizard =
            new QAction(QIcon(QPixmap(":/wizard.xpm")), tr("Plot &Wizard") + "...", this);
    actionShowPlotWizard->setShortcut(tr("Ctrl+Alt+W"));
    connect(actionShowPlotWizard, SIGNAL(triggered()), this, SLOT(showPlotWizard()));

    actionShowConfigureDialog = new QAction(tr("&Preferences..."), this);
    connect(actionShowConfigureDialog, SIGNAL(triggered()), this, SLOT(showPreferencesDialog()));

    actionShowCurvesDialog =
            new QAction(QIcon(QPixmap(":/curves.xpm")), tr("Add/Remove &Curve..."), this);
    actionShowCurvesDialog->setShortcut(tr("ALT+C"));
    connect(actionShowCurvesDialog, SIGNAL(triggered()), this, SLOT(showCurvesDialog()));

    actionAddErrorBars =
            new QAction(QIcon(QPixmap(":/yerror.xpm")), tr("Add &Error Bars..."), this);
    actionAddErrorBars->setShortcut(tr("Ctrl+B"));
    connect(actionAddErrorBars, SIGNAL(triggered()), this, SLOT(addErrorBars()));

    actionAddFunctionCurve = new QAction(QIcon(QPixmap(":/fx.xpm")), tr("Add &Function..."), this);
    actionAddFunctionCurve->setShortcut(tr("Ctrl+Alt+F"));
    connect(actionAddFunctionCurve, SIGNAL(triggered()), this, SLOT(addFunctionCurve()));

    actionUnzoom = new QAction(QIcon(QPixmap(":/unzoom.xpm")), tr("&Rescale to Show All"), this);
    actionUnzoom->setShortcut(tr("Ctrl+Shift+R"));
    connect(actionUnzoom, SIGNAL(triggered()), this, SLOT(setAutoScale()));

    actionNewLegend = new QAction(QIcon(QPixmap(":/legend.xpm")), tr("New &Legend"), this);
    actionNewLegend->setShortcut(tr("Ctrl+L"));
    connect(actionNewLegend, SIGNAL(triggered()), this, SLOT(newLegend()));

    actionTimeStamp = new QAction(QIcon(QPixmap(":/clock.xpm")), tr("Add Time Stamp"), this);
    actionTimeStamp->setShortcut(tr("Ctrl+ALT+T"));
    connect(actionTimeStamp, SIGNAL(triggered()), this, SLOT(addTimeStamp()));

    actionAddImage = new QAction(QIcon(QPixmap(":/monalisa.xpm")), tr("Add &Image"), this);
    actionAddImage->setShortcut(tr("ALT+I"));
    connect(actionAddImage, SIGNAL(triggered()), this, SLOT(addImage()));

    d_plot_mapper = new QSignalMapper;
    connect(d_plot_mapper, SIGNAL(mapped(int)), this, SLOT(selectPlotType(int)));

    actionPlotL = new QAction(QIcon(QPixmap(":/lPlot.xpm")), tr("&Line"), this);
    connect(actionPlotL, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotL, Graph::Line);

    actionPlotP = new QAction(QIcon(QPixmap(":/pPlot.xpm")), tr("&Scatter"), this);
    connect(actionPlotP, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotP, Graph::Scatter);

    actionPlotLP = new QAction(QIcon(QPixmap(":/lpPlot.xpm")), tr("Line + S&ymbol"), this);
    connect(actionPlotLP, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotLP, Graph::LineSymbols);

    actionPlotVerticalDropLines =
            new QAction(QIcon(QPixmap(":/dropLines.xpm")), tr("Vertical &Drop Lines"), this);
    connect(actionPlotVerticalDropLines, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotVerticalDropLines, Graph::VerticalDropLines);

    actionPlotSpline = new QAction(QIcon(QPixmap(":/spline.xpm")), tr("&Spline"), this);
    connect(actionPlotSpline, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotSpline, Graph::Spline);

    actionPlotHorSteps = new QAction(QPixmap(":/hor_steps.xpm"), tr("&Horizontal Steps"), this);
    connect(actionPlotHorSteps, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotHorSteps, Graph::HorizontalSteps);

    actionPlotVertSteps =
            new QAction(QIcon(QPixmap(":/vert_steps.xpm")), tr("&Vertical Steps"), this);
    connect(actionPlotVertSteps, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotVertSteps, Graph::VerticalSteps);

    actionPlotVerticalBars =
            new QAction(QIcon(QPixmap(":/vertBars.xpm")), tr("&Vertical Bars"), this);
    connect(actionPlotVerticalBars, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotVerticalBars, Graph::VerticalBars);

    actionPlotHorizontalBars =
            new QAction(QIcon(QPixmap(":/hBars.xpm")), tr("&Horizontal Bars"), this);
    connect(actionPlotHorizontalBars, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotHorizontalBars, Graph::HorizontalBars);

    actionPlotArea = new QAction(QIcon(QPixmap(":/area.xpm")), tr("&Area"), this);
    connect(actionPlotArea, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotArea, Graph::Area);

    actionPlotPie = new QAction(QIcon(QPixmap(":/pie.xpm")), tr("&Pie"), this);
    connect(actionPlotPie, SIGNAL(triggered()), this, SLOT(plotPie()));

    actionPlotVectXYAM = new QAction(QIcon(QPixmap(":/vectXYAM.xpm")), tr("Vectors XY&AM"), this);
    connect(actionPlotVectXYAM, SIGNAL(triggered()), this, SLOT(plotVectXYAM()));

    actionPlotVectXYXY = new QAction(QIcon(QPixmap(":/vectXYXY.xpm")), tr("Vectors &XYXY"), this);
    connect(actionPlotVectXYXY, SIGNAL(triggered()), this, SLOT(plotVectXYXY()));

    actionPlotHistogram = new QAction(QIcon(QPixmap(":/histogram.xpm")), tr("&Histogram"), this);
    connect(actionPlotHistogram, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionPlotHistogram, Graph::Histogram);

    actionPlotStackedHistograms =
            new QAction(QIcon(QPixmap(":/stacked_hist.xpm")), tr("&Stacked Histogram"), this);
    connect(actionPlotStackedHistograms, SIGNAL(triggered()), this, SLOT(plotStackedHistograms()));

    actionPlot2VerticalLayers =
            new QAction(QIcon(QPixmap(":/panel_v2.xpm")), tr("&Vertical 2 Layers"), this);
    connect(actionPlot2VerticalLayers, SIGNAL(triggered()), this, SLOT(plot2VerticalLayers()));

    actionPlot2HorizontalLayers =
            new QAction(QIcon(QPixmap(":/panel_h2.xpm")), tr("&Horizontal 2 Layers"), this);
    connect(actionPlot2HorizontalLayers, SIGNAL(triggered()), this, SLOT(plot2HorizontalLayers()));

    actionPlot4Layers = new QAction(QIcon(QPixmap(":/panel_4.xpm")), tr("&4 Layers"), this);
    connect(actionPlot4Layers, SIGNAL(triggered()), this, SLOT(plot4Layers()));

    actionPlotStackedLayers =
            new QAction(QIcon(QPixmap(":/stacked.xpm")), tr("&Stacked Layers"), this);
    connect(actionPlotStackedLayers, SIGNAL(triggered()), this, SLOT(plotStackedLayers()));

    actionPlot3DRibbon = new QAction(QIcon(QPixmap(":/ribbon.xpm")), tr("&Ribbon"), this);
    connect(actionPlot3DRibbon, SIGNAL(triggered()), this, SLOT(plot3DRibbon()));

    actionPlot3DBars = new QAction(QIcon(QPixmap(":/bars.xpm")), tr("&Bars"), this);
    connect(actionPlot3DBars, SIGNAL(triggered()), this, SLOT(plot3DBars()));

    actionPlot3DScatter = new QAction(QIcon(QPixmap(":/scatter.xpm")), tr("&Scatter"), this);
    connect(actionPlot3DScatter, SIGNAL(triggered()), this, SLOT(plot3DScatter()));

    actionPlot3DTrajectory =
            new QAction(QIcon(QPixmap(":/trajectory.xpm")), tr("&Trajectory"), this);
    connect(actionPlot3DTrajectory, SIGNAL(triggered()), this, SLOT(plot3DTrajectory()));

    actionShowColStatistics =
            new QAction(QIcon(QPixmap(":/col_stat.xpm")), tr("Statistics on &Columns"), this);
    connect(actionShowColStatistics, SIGNAL(triggered()), this, SLOT(showColStatistics()));

    actionShowRowStatistics =
            new QAction(QIcon(QPixmap(":/stat_rows.xpm")), tr("Statistics on &Rows"), this);
    connect(actionShowRowStatistics, SIGNAL(triggered()), this, SLOT(showRowStatistics()));

    actionShowIntDialog = new QAction(tr("&Integrate ..."), this);
    connect(actionShowIntDialog, SIGNAL(triggered()), this, SLOT(showIntegrationDialog()));

    actionInterpolate = new QAction(tr("Inte&rpolate ..."), this);
    connect(actionInterpolate, SIGNAL(triggered()), this, SLOT(showInterpolationDialog()));

    actionLowPassFilter = new QAction(tr("&Low Pass..."), this);
    connect(actionLowPassFilter, SIGNAL(triggered()), this, SLOT(lowPassFilterDialog()));

    actionHighPassFilter = new QAction(tr("&High Pass..."), this);
    connect(actionHighPassFilter, SIGNAL(triggered()), this, SLOT(highPassFilterDialog()));

    actionBandPassFilter = new QAction(tr("&Band Pass..."), this);
    connect(actionBandPassFilter, SIGNAL(triggered()), this, SLOT(bandPassFilterDialog()));

    actionBandBlockFilter = new QAction(tr("&Band Block..."), this);
    connect(actionBandBlockFilter, SIGNAL(triggered()), this, SLOT(bandBlockFilterDialog()));

    actionFFT = new QAction(tr("&FFT..."), this);
    connect(actionFFT, SIGNAL(triggered()), this, SLOT(showFFTDialog()));

    actionSmoothSavGol = new QAction(tr("&Savitzky-Golay..."), this);
    connect(actionSmoothSavGol, SIGNAL(triggered()), this, SLOT(showSmoothSavGolDialog()));

    actionSmoothFFT = new QAction(tr("&FFT Filter..."), this);
    connect(actionSmoothFFT, SIGNAL(triggered()), this, SLOT(showSmoothFFTDialog()));

    actionSmoothAverage = new QAction(tr("Moving Window &Average..."), this);
    connect(actionSmoothAverage, SIGNAL(triggered()), this, SLOT(showSmoothAverageDialog()));

    actionDifferentiate = new QAction(tr("&Differentiate"), this);
    connect(actionDifferentiate, SIGNAL(triggered()), this, SLOT(differentiate()));

    actionFitLinear = new QAction(tr("Fit &Linear"), this);
    connect(actionFitLinear, SIGNAL(triggered()), this, SLOT(fitLinear()));

    actionShowFitPolynomDialog = new QAction(tr("Fit &Polynomial ..."), this);
    connect(actionShowFitPolynomDialog, SIGNAL(triggered()), this, SLOT(showFitPolynomDialog()));

    actionShowExpDecayDialog = new QAction(tr("&First Order ..."), this);
    connect(actionShowExpDecayDialog, SIGNAL(triggered()), this, SLOT(showExpDecayDialog()));

    actionShowTwoExpDecayDialog = new QAction(tr("&Second Order ..."), this);
    connect(actionShowTwoExpDecayDialog, SIGNAL(triggered()), this, SLOT(showTwoExpDecayDialog()));

    actionShowExpDecay3Dialog = new QAction(tr("&Third Order ..."), this);
    connect(actionShowExpDecay3Dialog, SIGNAL(triggered()), this, SLOT(showExpDecay3Dialog()));

    actionFitExpGrowth = new QAction(tr("Fit Exponential Gro&wth ..."), this);
    connect(actionFitExpGrowth, SIGNAL(triggered()), this, SLOT(showExpGrowthDialog()));

    actionFitSigmoidal = new QAction(tr("Fit &Boltzmann (Sigmoidal)"), this);
    connect(actionFitSigmoidal, SIGNAL(triggered()), this, SLOT(fitSigmoidal()));

    actionFitGauss = new QAction(tr("Fit &Gaussian"), this);
    connect(actionFitGauss, SIGNAL(triggered()), this, SLOT(fitGauss()));

    actionFitLorentz = new QAction(tr("Fit Lorent&zian"), this);
    connect(actionFitLorentz, SIGNAL(triggered()), this, SLOT(fitLorentz()));

    actionShowFitDialog = new QAction(tr("Fit &Wizard..."), this);
    actionShowFitDialog->setShortcut(tr("Ctrl+Y"));
    connect(actionShowFitDialog, SIGNAL(triggered()), this, SLOT(showFitDialog()));

    actionShowPlotDialog = new QAction(tr("&Plot ..."), this);
    connect(actionShowPlotDialog, SIGNAL(triggered()), this, SLOT(showGeneralPlotDialog()));

    actionShowScaleDialog = new QAction(tr("&Scales..."), this);
    connect(actionShowScaleDialog, SIGNAL(triggered()), this, SLOT(showScaleDialog()));

    actionShowAxisDialog = new QAction(tr("&Axes..."), this);
    connect(actionShowAxisDialog, SIGNAL(triggered()), this, SLOT(showAxisDialog()));

    actionShowGridDialog = new QAction(tr("&Grid ..."), this);
    connect(actionShowGridDialog, SIGNAL(triggered()), this, SLOT(showGridDialog()));

    actionShowTitleDialog = new QAction(tr("&Title ..."), this);
    connect(actionShowTitleDialog, SIGNAL(triggered()), this, SLOT(showTitleDialog()));

    actionAbout = new QAction(tr("&About Makhber"), this);
    actionAbout->setShortcut(tr("F1"));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    actionShowHelp = new QAction(tr("&Help"), this);
    actionShowHelp->setShortcut(tr("Ctrl+H"));
    connect(actionShowHelp, SIGNAL(triggered()), this, SLOT(showHelp()));

#ifdef DYNAMIC_MANUAL_PATH
    actionChooseHelpFolder = new QAction(tr("&Choose Help Folder..."), this);
    connect(actionChooseHelpFolder, SIGNAL(triggered()), this, SLOT(chooseHelpFolder()));
#endif

    actionRename = new QAction(tr("&Rename Window"), this);
    connect(actionRename, SIGNAL(triggered()), this, SLOT(renameActiveWindow()));

    actionCloseWindow = new QAction(QIcon(QPixmap(":/close.xpm")), tr("Close &Window"), this);
    actionCloseWindow->setShortcut(tr("Ctrl+W"));
    connect(actionCloseWindow, SIGNAL(triggered()), this, SLOT(closeActiveWindow()));

    actionDeleteLayer = new QAction(QIcon(QPixmap(":/erase.xpm")), tr("&Remove Layer"), this);
    actionDeleteLayer->setShortcut(tr("Alt+R"));
    connect(actionDeleteLayer, SIGNAL(triggered()), this, SLOT(deleteLayer()));

    actionResizeActiveWindow =
            new QAction(QIcon(QPixmap(":/resize.xpm")), tr("Window &Geometry..."), this);
    connect(actionResizeActiveWindow, SIGNAL(triggered()), this, SLOT(resizeActiveWindow()));

    actionHideActiveWindow = new QAction(tr("&Hide Window"), this);
    connect(actionHideActiveWindow, SIGNAL(triggered()), this, SLOT(hideActiveWindow()));

    actionShowMoreWindows = new QAction(tr("More windows..."), this);
    connect(actionShowMoreWindows, SIGNAL(triggered()), this, SLOT(showMoreWindows()));

    actionPixelLineProfile =
            new QAction(QIcon(QPixmap(":/pixelProfile.xpm")), tr("&View Pixel Line Profile"), this);
    connect(actionPixelLineProfile, SIGNAL(triggered()), this, SLOT(pixelLineProfile()));

    actionIntensityTable = new QAction(tr("&Intensity Table"), this);
    connect(actionIntensityTable, SIGNAL(triggered()), this, SLOT(intensityTable()));

    actionShowLineDialog = new QAction(tr("&Properties"), this);
    connect(actionShowLineDialog, SIGNAL(triggered()), this, SLOT(showLineDialog()));

    actionShowImageDialog = new QAction(tr("&Properties"), this);
    connect(actionShowImageDialog, SIGNAL(triggered()), this, SLOT(showImageDialog()));

    actionShowTextDialog = new QAction(tr("&Properties"), this);
    connect(actionShowTextDialog, SIGNAL(triggered()), this, SLOT(showTextDialog()));

    actionActivateWindow = new QAction(tr("&Activate Window"), this);
    connect(actionActivateWindow, SIGNAL(triggered()), this, SLOT(activateSubWindow()));

    actionMinimizeWindow = new QAction(tr("Mi&nimize Window"), this);
    connect(actionMinimizeWindow, SIGNAL(triggered()), this, SLOT(minimizeWindow()));

    actionMaximizeWindow = new QAction(tr("Ma&ximize Window"), this);
    connect(actionMaximizeWindow, SIGNAL(triggered()), this, SLOT(maximizeWindow()));

    actionResizeWindow = new QAction(QIcon(QPixmap(":/resize.xpm")), tr("Re&size Window..."), this);
    connect(actionResizeWindow, SIGNAL(triggered()), this, SLOT(resizeWindow()));

    actionPrintWindow = new QAction(QIcon(QPixmap(":/fileprint.xpm")), tr("&Print Window"), this);
    connect(actionPrintWindow, SIGNAL(triggered()), this, SLOT(printWindow()));

    actionShowPlotGeometryDialog =
            new QAction(QIcon(QPixmap(":/resize.xpm")), tr("&Layer Geometry"), this);
    connect(actionShowPlotGeometryDialog, SIGNAL(triggered()), this,
            SLOT(showPlotGeometryDialog()));

    actionEditSurfacePlot = new QAction(tr("&Surface..."), this);
    connect(actionEditSurfacePlot, SIGNAL(triggered()), this, SLOT(editSurfacePlot()));

    actionAdd3DData = new QAction(tr("&Data Set..."), this);
    connect(actionAdd3DData, SIGNAL(triggered()), this, SLOT(add3DData()));

    actionInvertMatrix = new QAction(tr("&Invert"), this);
    connect(actionInvertMatrix, SIGNAL(triggered()), this, SLOT(invertMatrix()));

    actionMatrixDeterminant = new QAction(tr("&Determinant"), this);
    connect(actionMatrixDeterminant, SIGNAL(triggered()), this, SLOT(matrixDeterminant()));

    actionConvertMatrix = new QAction(tr("&Convert to Table"), this);
    connect(actionConvertMatrix, SIGNAL(triggered()), this, SLOT(convertMatrixToTable()));

    actionConvertTable = new QAction(tr("Convert to &Matrix"), this);
    connect(actionConvertTable, SIGNAL(triggered()), this, SLOT(convertTableToMatrix()));

    actionPlot3DWireFrame =
            new QAction(QIcon(QPixmap(":/lineMesh.xpm")), tr("3D &Wire Frame"), this);
    connect(actionPlot3DWireFrame, SIGNAL(triggered()), this, SLOT(plot3DWireframe()));

    actionPlot3DHiddenLine =
            new QAction(QIcon(QPixmap(":/grid_only.xpm")), tr("3D &Hidden Line"), this);
    connect(actionPlot3DHiddenLine, SIGNAL(triggered()), this, SLOT(plot3DHiddenLine()));

    actionPlot3DPolygons = new QAction(QIcon(QPixmap(":/no_grid.xpm")), tr("3D &Polygons"), this);
    connect(actionPlot3DPolygons, SIGNAL(triggered()), this, SLOT(plot3DPolygons()));

    actionPlot3DWireSurface =
            new QAction(QIcon(QPixmap(":/grid_poly.xpm")), tr("3D Wire &Surface"), this);
    connect(actionPlot3DWireSurface, SIGNAL(triggered()), this, SLOT(plot3DWireSurface()));

    actionColorMap =
            new QAction(QIcon(QPixmap(":/color_map.xpm")), tr("Contour - &Color Fill"), this);
    connect(actionColorMap, SIGNAL(triggered()), this, SLOT(plotColorMap()));

    actionContourMap = new QAction(QIcon(QPixmap(":/contour_map.xpm")), tr("Contour &Lines"), this);
    connect(actionContourMap, SIGNAL(triggered()), this, SLOT(plotContour()));

    actionGrayMap = new QAction(QIcon(QPixmap(":/gray_map.xpm")), tr("&Gray Scale Map"), this);
    connect(actionGrayMap, SIGNAL(triggered()), this, SLOT(plotGrayScale()));

    actionCorrelate = new QAction(tr("Co&rrelate"), this);
    connect(actionCorrelate, SIGNAL(triggered()), this, SLOT(correlate()));

    actionAutoCorrelate = new QAction(tr("&Autocorrelate"), this);
    connect(actionAutoCorrelate, SIGNAL(triggered()), this, SLOT(autoCorrelate()));

    actionConvolute = new QAction(tr("&Convolute"), this);
    connect(actionConvolute, SIGNAL(triggered()), this, SLOT(convolute()));

    actionDeconvolute = new QAction(tr("&Deconvolute"), this);
    connect(actionDeconvolute, SIGNAL(triggered()), this, SLOT(deconvolute()));

    actionTranslateHor = new QAction(tr("&Horizontal"), this);
    connect(actionTranslateHor, SIGNAL(triggered()), this, SLOT(translateCurveHor()));

    actionTranslateVert = new QAction(tr("&Vertical"), this);
    connect(actionTranslateVert, SIGNAL(triggered()), this, SLOT(translateCurveVert()));

    actionBoxPlot = new QAction(QIcon(QPixmap(":/boxPlot.xpm")), tr("&Box Plot"), this);
    connect(actionBoxPlot, SIGNAL(triggered()), d_plot_mapper, SLOT(map()));
    d_plot_mapper->setMapping(actionBoxPlot, Graph::Box);

    actionMultiPeakGauss = new QAction(tr("&Gaussian..."), this);
    connect(actionMultiPeakGauss, SIGNAL(triggered()), this, SLOT(fitMultiPeakGauss()));

    actionMultiPeakLorentz = new QAction(tr("&Lorentzian..."), this);
    connect(actionMultiPeakLorentz, SIGNAL(triggered()), this, SLOT(fitMultiPeakLorentz()));

#ifdef SEARCH_FOR_UPDATES
    actionCheckUpdates = new QAction(tr("Search for &Updates"), this);
    connect(actionCheckUpdates, SIGNAL(triggered()), this, SLOT(searchForUpdates()));
#endif // defined SEARCH_FOR_UPDATES

    actionHomePage = new QAction(tr("&Makhber Homepage"), this);
    connect(actionHomePage, SIGNAL(triggered()), this, SLOT(showHomePage()));

    actionHelpForums = new QAction(tr("Makhber &Forums"), this);
    connect(actionHelpForums, SIGNAL(triggered()), this, SLOT(showForums()));

    actionHelpBugReports = new QAction(tr("Report a &Bug"), this);
    connect(actionHelpBugReports, SIGNAL(triggered()), this, SLOT(showBugTracker()));

#ifdef DOWNLOAD_LINKS
    actionDownloadManual = new QAction(tr("Download &Manual"), this);
    connect(actionDownloadManual, SIGNAL(triggered()), this, SLOT(downloadManual()));
#endif

#ifdef SCRIPTING_DIALOG
    actionScriptingLang = new QAction(tr("Scripting &Language"), this);
    connect(actionScriptingLang, SIGNAL(triggered()), this, SLOT(showScriptingLangDialog()));
#endif

    actionRestartScripting = new QAction(tr("&Restart Scripting"), this);
    connect(actionRestartScripting, SIGNAL(triggered()), this, SLOT(restartScriptingEnv()));

    actionNoteExecute = new QAction(tr("E&xecute"), this);
    actionNoteExecute->setShortcut(tr("Ctrl+J"));

    actionNoteExecuteAll = new QAction(tr("Execute &All"), this);
    actionNoteExecuteAll->setShortcut(tr("Ctrl+Shift+J"));

    actionNoteEvaluate = new QAction(tr("&Evaluate Expression"), this);
    actionNoteEvaluate->setShortcut(tr("Ctrl+Return"));

    actionShowCurvePlotDialog = new QAction(tr("&Plot details..."), this);
    connect(actionShowCurvePlotDialog, SIGNAL(triggered()), this, SLOT(showCurvePlotDialog()));

    actionShowCurveWorksheet = new QAction(tr("&Worksheet"), this);
    connect(actionShowCurveWorksheet, SIGNAL(triggered()), this, SLOT(showCurveWorksheet()));

    actionCurveFullRange = new QAction(tr("&Reset to Full Range"), this);
    connect(actionCurveFullRange, SIGNAL(triggered()), this, SLOT(setCurveFullRange()));

    actionEditCurveRange = new QAction(tr("Edit &Range..."), this);
    connect(actionEditCurveRange, SIGNAL(triggered()), this, SLOT(showCurveRangeDialog()));

    actionRemoveCurve = new QAction(QPixmap(":/close.xpm"), tr("&Delete"), this);
    connect(actionRemoveCurve, SIGNAL(triggered()), this, SLOT(removeCurve()));

    actionHideCurve = new QAction(tr("&Hide"), this);
    connect(actionHideCurve, SIGNAL(triggered()), this, SLOT(hideCurve()));

    actionHideOtherCurves = new QAction(tr("Hide &Other Curves"), this);
    connect(actionHideOtherCurves, SIGNAL(triggered()), this, SLOT(hideOtherCurves()));

    actionShowAllCurves = new QAction(tr("&Show All Curves"), this);
    connect(actionShowAllCurves, SIGNAL(triggered()), this, SLOT(showAllCurves()));

    actionEditFunction = new QAction(tr("&Edit Function..."), this);
    connect(actionEditFunction, SIGNAL(triggered()), this, SLOT(showFunctionDialog()));

    actionCopyStatusBarText = new QAction(tr("&Copy status bar text"), this);
    connect(actionCopyStatusBarText, SIGNAL(triggered()), this, SLOT(copyStatusBarText()));
}

void ApplicationWindow::translateActionsStrings()
{
    actionShowCurvePlotDialog->setText(tr("&Plot details..."));
    actionShowCurveWorksheet->setText(tr("&Worksheet"));
    actionRemoveCurve->setText(tr("&Delete"));
    actionEditFunction->setText(tr("&Edit Function..."));
    actionCopyStatusBarText->setText(tr("&Copy status bar text"));

    actionCurveFullRange->setText(tr("&Reset to Full Range"));
    actionEditCurveRange->setText(tr("Edit &Range..."));
    actionHideCurve->setText(tr("&Hide"));
    actionHideOtherCurves->setText(tr("Hide &Other Curves"));
    actionShowAllCurves->setText(tr("&Show All Curves"));

    actionNewProject->setText(tr("New &Project"));
    actionNewProject->setToolTip(tr("Open a new project"));
    actionNewProject->setShortcut(tr("Ctrl+N"));

    actionNewGraph->setText(tr("New &Graph"));
    actionNewGraph->setToolTip(tr("Create an empty 2D plot"));
    actionNewGraph->setShortcut(tr("Ctrl+G"));

    actionNewNote->setText(tr("New &Note / Script"));
    actionNewNote->setShortcut(tr("Ctrl+ALT+N"));
    actionNewNote->setToolTip(tr("Create an empty note / script window"));

    actionNewTable->setText(tr("New &Table"));
    actionNewTable->setShortcut(tr("Ctrl+T"));
    actionNewTable->setToolTip(tr("New table"));

    actionNewMatrix->setText(tr("New &Matrix"));
    actionNewMatrix->setShortcut(tr("Ctrl+M"));
    actionNewMatrix->setToolTip(tr("New matrix"));

    actionNewFunctionPlot->setText(tr("New &Function Plot"));
    actionNewFunctionPlot->setToolTip(tr("Create a new 2D function plot"));
    actionNewFunctionPlot->setShortcut(tr("Ctrl+F"));

    actionNewSurfacePlot->setText(tr("New 3D &Surface Plot"));
    actionNewSurfacePlot->setToolTip(tr("Create a new 3D surface plot"));
    actionNewSurfacePlot->setShortcut(tr("Ctrl+ALT+Z"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionOpen->setText(tr("&Open") + "...");
    actionOpen->setShortcut(tr("Ctrl+O"));
    actionOpen->setToolTip(tr("Open project"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionLoadImage->setText(tr("Open Image &File") + "...");
    actionLoadImage->setShortcut(tr("Ctrl+I"));

    actionImportImage->setText(tr("Import I&mage..."));

    actionSaveProject->setText(tr("&Save Project"));
    actionSaveProject->setToolTip(tr("Save project"));
    actionSaveProject->setShortcut(tr("Ctrl+S"));

    actionSaveProjectAs->setText(tr("Save Project &As..."));

    actionOpenTemplate->setText(tr("Open Te&mplate..."));
    actionOpenTemplate->setToolTip(tr("Open template"));

    actionSaveTemplate->setText(tr("Save As &Template..."));
    actionSaveTemplate->setToolTip(tr("Save window as template"));

    actionLoad->setText(tr("&Import ASCII..."));
    actionLoad->setToolTip(tr("Import data file(s)"));
    actionLoad->setShortcut(tr("Ctrl+K"));

    actionUndo->setText(tr("&Undo"));
    actionUndo->setToolTip(tr("Undo changes"));
    actionUndo->setShortcut(tr("Ctrl+Z"));

    actionRedo->setText(tr("&Redo"));
    actionRedo->setToolTip(tr("Redo changes"));
    actionRedo->setShortcut(tr("Ctrl+R"));

    actionCopyWindow->setText(tr("&Duplicate"));
    actionCopyWindow->setToolTip(tr("Duplicate window"));

    actionCutSelection->setText(tr("Cu&t Selection"));
    actionCutSelection->setToolTip(tr("Cut selection"));
    actionCutSelection->setShortcut(tr("Ctrl+X"));

    actionCopySelection->setText(tr("&Copy Selection"));
    actionCopySelection->setToolTip(tr("Copy selection"));
    actionCopySelection->setShortcut(tr("Ctrl+C"));

    actionPasteSelection->setText(tr("&Paste Selection"));
    actionPasteSelection->setToolTip(tr("Paste selection"));
    actionPasteSelection->setShortcut(tr("Ctrl+V"));

    actionClearSelection->setText(tr("&Delete Selection"));
    actionClearSelection->setToolTip(tr("Delete selection"));
    actionClearSelection->setShortcut(tr("Del", "delete key"));

    actionShowExplorer->setText(tr("Project &Explorer"));
    actionShowExplorer->setShortcut(tr("Ctrl+E"));
    actionShowExplorer->setToolTip(tr("Show project explorer"));

    actionShowLog->setText(tr("Results &Log"));
    actionShowLog->setToolTip(tr("Show analysis results"));

#ifdef SCRIPTING_CONSOLE
    actionShowConsole->setText(tr("&Console"));
    actionShowConsole->setToolTip(tr("Show Scripting console"));
#endif

    actionAddLayer->setText(tr("Add La&yer"));
    actionAddLayer->setToolTip(tr("Add Layer"));
    actionAddLayer->setShortcut(tr("ALT+L"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionShowLayerDialog->setText(tr("Arran&ge Layers") + "...");
    actionShowLayerDialog->setToolTip(tr("Arrange Layers"));
    actionShowLayerDialog->setShortcut(tr("ALT+A"));

    actionAutomaticLayout->setText(tr("Automatic Layout"));
    actionAutomaticLayout->setToolTip(tr("Automatic Layout"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportGraph->setText(tr("&Current") + "...");
    actionExportGraph->setShortcut(tr("Alt+G"));
    actionExportGraph->setToolTip(tr("Export current graph"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportAllGraphs->setText(tr("&All") + "...");
    actionExportAllGraphs->setShortcut(tr("Alt+X"));
    actionExportAllGraphs->setToolTip(tr("Export all graphs"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionExportPDF->setText(tr("&Export PDF") + "...");
    actionExportPDF->setShortcut(tr("Ctrl+Alt+P"));
    actionExportPDF->setToolTip(tr("Export to PDF"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionPrint->setText(tr("&Print") + "...");
    actionPrint->setShortcut(tr("Ctrl+P"));
    actionPrint->setToolTip(tr("Print window"));

    // FIXME: "..." should be added before translating, but this would break translations
    actionPrintAllPlots->setText(tr("Print All Plo&ts") + "...");
    // FIXME: "..." should be added before translating, but this would break translations
    actionShowExportASCIIDialog->setText(tr("E&xport ASCII") + "...");

    actionCloseAllWindows->setText(tr("&Quit"));
    actionCloseAllWindows->setShortcut(tr("Ctrl+Q"));

    actionClearLogInfo->setText(tr("Clear &Log Information"));
    actionDeleteFitTables->setText(tr("Delete &Fit Tables"));

    actionShowPlotWizard->setText(tr("Plot &Wizard"));
    actionShowPlotWizard->setShortcut(tr("Ctrl+Alt+W"));
    toolbarsMenu->setTitle(tr("Toolbars"));

    actionShowConfigureDialog->setText(tr("&Preferences..."));

    actionShowCurvesDialog->setText(tr("Add/Remove &Curve..."));
    actionShowCurvesDialog->setShortcut(tr("ALT+C"));
    actionShowCurvesDialog->setToolTip(tr("Add curve to graph"));

    actionAddErrorBars->setText(tr("Add &Error Bars..."));
    actionAddErrorBars->setToolTip(tr("Add Error Bars..."));
    actionAddErrorBars->setShortcut(tr("Ctrl+B"));

    actionAddFunctionCurve->setText(tr("Add &Function..."));
    actionAddFunctionCurve->setToolTip(tr("Add Function..."));
    actionAddFunctionCurve->setShortcut(tr("Ctrl+Alt+F"));

    actionUnzoom->setText(tr("&Rescale to Show All"));
    actionUnzoom->setShortcut(tr("Ctrl+Shift+R"));
    actionUnzoom->setToolTip(tr("Best fit"));

    actionNewLegend->setText(tr("New &Legend"));
    actionNewLegend->setShortcut(tr("Ctrl+L"));
    actionNewLegend->setToolTip(tr("Add new legend"));

    actionTimeStamp->setText(tr("Add Time Stamp"));
    actionTimeStamp->setShortcut(tr("Ctrl+ALT+T"));
    actionTimeStamp->setToolTip(tr("Date & time "));

    actionAddImage->setText(tr("Add &Image"));
    actionAddImage->setToolTip(tr("Add Image"));
    actionAddImage->setShortcut(tr("ALT+I"));

    actionPlotL->setText(tr("&Line"));
    actionPlotL->setToolTip(tr("Plot as line"));

    actionPlotP->setText(tr("&Scatter"));
    actionPlotP->setToolTip(tr("Plot as symbols"));

    actionPlotLP->setText(tr("Line + S&ymbol"));
    actionPlotLP->setToolTip(tr("Plot as line + symbols"));

    actionPlotVerticalDropLines->setText(tr("Vertical &Drop Lines"));

    actionPlotSpline->setText(tr("&Spline"));
    actionPlotVertSteps->setText(tr("&Vertical Steps"));
    actionPlotHorSteps->setText(tr("&Horizontal Steps"));

    actionPlotVerticalBars->setText(tr("&Vertical Bars"));
    actionPlotVerticalBars->setToolTip(tr("Plot with vertical bars"));

    actionPlotHorizontalBars->setText(tr("&Horizontal Bars"));
    actionPlotHorizontalBars->setToolTip(tr("Plot with horizontal bars"));

    actionPlotArea->setText(tr("&Area"));
    actionPlotArea->setToolTip(tr("Plot area"));

    actionPlotPie->setText(tr("&Pie"));
    actionPlotPie->setToolTip(tr("Plot pie"));

    actionPlotVectXYXY->setText(tr("Vectors &XYXY"));
    actionPlotVectXYXY->setToolTip(tr("Vectors XYXY"));

    actionPlotVectXYAM->setText(tr("Vectors XY&AM"));
    actionPlotVectXYAM->setToolTip(tr("Vectors XYAM"));

    actionPlotHistogram->setText(tr("&Histogram"));
    actionPlotStackedHistograms->setText(tr("&Stacked Histogram"));
    actionPlot2VerticalLayers->setText(tr("&Vertical 2 Layers"));
    actionPlot2HorizontalLayers->setText(tr("&Horizontal 2 Layers"));
    actionPlot4Layers->setText(tr("&4 Layers"));
    actionPlotStackedLayers->setText(tr("&Stacked Layers"));

    actionPlot3DRibbon->setText(tr("&Ribbon"));
    actionPlot3DRibbon->setToolTip(tr("Plot 3D ribbon"));

    actionPlot3DBars->setText(tr("&Bars"));
    actionPlot3DBars->setToolTip(tr("Plot 3D bars"));

    actionPlot3DScatter->setText(tr("&Scatter"));
    actionPlot3DScatter->setToolTip(tr("Plot 3D scatter"));

    actionPlot3DTrajectory->setText(tr("&Trajectory"));
    actionPlot3DTrajectory->setToolTip(tr("Plot 3D trajectory"));

    actionColorMap->setText(tr("Contour + &Color Fill"));
    actionColorMap->setToolTip(tr("Contour Lines + Color Fill"));

    actionContourMap->setText(tr("Contour &Lines"));
    actionContourMap->setToolTip(tr("Contour Lines"));

    actionGrayMap->setText(tr("&Gray Scale Map"));
    actionGrayMap->setToolTip(tr("Gray Scale Map"));

    actionShowColStatistics->setText(tr("Statistics on &Columns"));
    actionShowColStatistics->setToolTip(tr("Selected columns statistics"));

    actionShowRowStatistics->setText(tr("Statistics on &Rows"));
    actionShowRowStatistics->setToolTip(tr("Selected rows statistics"));
    actionShowIntDialog->setText(tr("&Integrate ..."));
    actionInterpolate->setText(tr("Inte&rpolate ..."));
    actionLowPassFilter->setText(tr("&Low Pass..."));
    actionHighPassFilter->setText(tr("&High Pass..."));
    actionBandPassFilter->setText(tr("&Band Pass..."));
    actionBandBlockFilter->setText(tr("&Band Block..."));
    actionFFT->setText(tr("&FFT..."));
    actionSmoothSavGol->setText(tr("&Savitzky-Golay..."));
    actionSmoothFFT->setText(tr("&FFT Filter..."));
    actionSmoothAverage->setText(tr("Moving Window &Average..."));
    actionDifferentiate->setText(tr("&Differentiate"));
    actionFitLinear->setText(tr("Fit &Linear"));
    actionShowFitPolynomDialog->setText(tr("Fit &Polynomial ..."));
    actionShowExpDecayDialog->setText(tr("&First Order ..."));
    actionShowTwoExpDecayDialog->setText(tr("&Second Order ..."));
    actionShowExpDecay3Dialog->setText(tr("&Third Order ..."));
    actionFitExpGrowth->setText(tr("Fit Exponential Gro&wth ..."));
    actionFitSigmoidal->setText(tr("Fit &Boltzmann (Sigmoidal)"));
    actionFitGauss->setText(tr("Fit &Gaussian"));
    actionFitLorentz->setText(tr("Fit Lorent&zian"));

    actionShowFitDialog->setText(tr("Fit &Wizard..."));
    actionShowFitDialog->setShortcut(tr("Ctrl+Y"));

    actionShowPlotDialog->setText(tr("&Plot ..."));
    actionShowScaleDialog->setText(tr("&Scales..."));
    actionShowAxisDialog->setText(tr("&Axes..."));
    actionShowGridDialog->setText(tr("&Grid ..."));
    actionShowTitleDialog->setText(tr("&Title ..."));

    actionAbout->setText(tr("&About Makhber"));
    actionAbout->setShortcut(tr("F1"));

    actionShowHelp->setText(tr("&Help"));
    actionShowHelp->setShortcut(tr("Ctrl+H"));

#ifdef DYNAMIC_MANUAL_PATH
    actionChooseHelpFolder->setText(tr("&Choose Help Folder..."));
#endif

    actionRename->setText(tr("&Rename Window"));

    actionCloseWindow->setText(tr("Close &Window"));
    actionCloseWindow->setShortcut(tr("Ctrl+W"));

    actionDeleteLayer->setText(tr("&Remove Layer"));
    actionDeleteLayer->setShortcut(tr("Alt+R"));

    actionResizeActiveWindow->setText(tr("Window &Geometry..."));
    actionHideActiveWindow->setText(tr("&Hide Window"));
    actionShowMoreWindows->setText(tr("More Windows..."));
    actionPixelLineProfile->setText(tr("&View Pixel Line Profile"));
    actionIntensityTable->setText(tr("&Intensity Table"));
    actionShowLineDialog->setText(tr("&Properties"));
    actionShowImageDialog->setText(tr("&Properties"));
    actionShowTextDialog->setText(tr("&Properties"));
    actionActivateWindow->setText(tr("&Activate Window"));
    actionMinimizeWindow->setText(tr("Mi&nimize Window"));
    actionMaximizeWindow->setText(tr("Ma&ximize Window"));
    actionResizeWindow->setText(tr("Re&size Window..."));
    actionPrintWindow->setText(tr("&Print Window"));
    actionShowPlotGeometryDialog->setText(tr("&Layer Geometry"));
    actionEditSurfacePlot->setText(tr("&Surface..."));
    actionAdd3DData->setText(tr("&Data Set..."));
    actionInvertMatrix->setText(tr("&Invert"));
    actionMatrixDeterminant->setText(tr("&Determinant"));
    actionConvertMatrix->setText(tr("&Convert to Table"));
    actionConvertTable->setText(tr("Convert to &Matrix"));
    actionPlot3DWireFrame->setText(tr("3D &Wire Frame"));
    actionPlot3DHiddenLine->setText(tr("3D &Hidden Line"));
    actionPlot3DPolygons->setText(tr("3D &Polygons"));
    actionPlot3DWireSurface->setText(tr("3D Wire &Surface"));
    actionCorrelate->setText(tr("Co&rrelate"));
    actionAutoCorrelate->setText(tr("&Autocorrelate"));
    actionConvolute->setText(tr("&Convolute"));
    actionDeconvolute->setText(tr("&Deconvolute"));
    actionTranslateHor->setText(tr("&Horizontal"));
    actionTranslateVert->setText(tr("&Vertical"));

    actionBoxPlot->setText(tr("&Box Plot"));
    actionBoxPlot->setToolTip(tr("Box and whiskers plot"));

    actionMultiPeakGauss->setText(tr("&Gaussian..."));
    actionMultiPeakLorentz->setText(tr("&Lorentzian..."));
    actionHomePage->setText(tr("&Makhber Homepage"));
#ifdef SEARCH_FOR_UPDATES
    actionCheckUpdates->setText(tr("Search for &Updates"));
#endif
    actionHelpForums->setText(tr("Visit Makhber &Forums"));
    actionHelpBugReports->setText(tr("Report a &Bug"));
#ifdef DOWNLOAD_LINKS
    actionDownloadManual->setText(tr("Download &Manual"));
#endif

#ifdef SCRIPTING_DIALOG
    actionScriptingLang->setText(tr("Scripting &Language"));
#endif
    actionRestartScripting->setText(tr("&Restart Scripting"));

    actionNoteExecute->setText(tr("E&xecute"));
    actionNoteExecute->setShortcut(tr("Ctrl+J"));

    actionNoteExecuteAll->setText(tr("Execute &All"));
    actionNoteExecuteAll->setShortcut(tr("Ctrl+Shift+J"));

    actionNoteEvaluate->setText(tr("&Evaluate Expression"));
    actionNoteEvaluate->setShortcut(tr("Ctrl+Return"));

    btnPointer->setText(tr("Disable &tools"));
    btnPointer->setToolTip(tr("Pointer"));

    btnZoomIn->setText(tr("&Zoom In"));
    btnZoomIn->setShortcut(tr("Ctrl++"));
    btnZoomIn->setToolTip(tr("Zoom In"));

    btnZoomOut->setText(tr("Zoom &Out"));
    btnZoomOut->setShortcut(tr("Ctrl+-"));
    btnZoomOut->setToolTip(tr("Zoom Out"));

    btnCursor->setText(tr("&Data Reader"));
    btnCursor->setShortcut(tr("CTRL+D"));
    btnCursor->setToolTip(tr("Data reader"));

    btnSelect->setText(tr("&Select Data Range"));
    btnSelect->setShortcut(tr("ALT+S"));
    btnSelect->setToolTip(tr("Select data range"));

    btnPicker->setText(tr("S&creen Reader"));
    btnPicker->setToolTip(tr("Screen reader"));

    btnMovePoints->setText(tr("&Move Data Points..."));
    btnMovePoints->setShortcut(tr("Ctrl+ALT+M"));
    btnMovePoints->setToolTip(tr("Move data points"));

    btnRemovePoints->setText(tr("Remove &Bad Data Points..."));
    btnRemovePoints->setShortcut(tr("Alt+B"));
    btnRemovePoints->setToolTip(tr("Remove data points"));

    actionAddText->setText(tr("Add &Text"));
    actionAddText->setToolTip(tr("Add Text"));
    actionAddText->setShortcut(tr("ALT+T"));

    actionNextWindow->setText(tr("&Next", "next window"));
    actionPrevWindow->setText(tr("&Previous", "previous window"));

    btnArrow->setText(tr("Draw &Arrow"));
    btnArrow->setShortcut(tr("CTRL+ALT+A"));
    btnArrow->setToolTip(tr("Draw arrow"));

    btnLine->setText(tr("Draw &Line"));
    btnLine->setShortcut(tr("CTRL+ALT+L"));
    btnLine->setToolTip(tr("Draw line"));

    // FIXME: is setText necessary for action groups?
    //	coord->setText( tr( "Coordinates" ) );
    //	coord->setIconText( tr( "&Coord" ) );
    //  coord->setStatusTip( tr( "Coordinates" ) );
    Box->setText(tr("Box"));
    Box->setIconText(tr("Box"));
    Box->setToolTip(tr("Box"));
    Box->setStatusTip(tr("Box"));
    Frame->setText(tr("Frame"));
    Frame->setIconText(tr("&Frame"));
    Frame->setToolTip(tr("Frame"));
    Frame->setStatusTip(tr("Frame"));
    None->setText(tr("No Axes"));
    None->setIconText(tr("No Axes"));
    None->setToolTip(tr("No axes"));
    None->setStatusTip(tr("No axes"));

    front->setToolTip(tr("Front grid"));
    back->setToolTip(tr("Back grid"));
    right->setToolTip(tr("Right grid"));
    left->setToolTip(tr("Left grid"));
    ceil->setToolTip(tr("Ceiling grid"));
    floor->setToolTip(tr("Floor grid"));

    wireframe->setText(tr("Wireframe"));
    wireframe->setIconText(tr("Wireframe"));
    wireframe->setToolTip(tr("Wireframe"));
    wireframe->setStatusTip(tr("Wireframe"));
    hiddenline->setText(tr("Hidden Line"));
    hiddenline->setIconText(tr("Hidden Line"));
    hiddenline->setToolTip(tr("Hidden line"));
    hiddenline->setStatusTip(tr("Hidden line"));
    polygon->setText(tr("Polygon Only"));
    polygon->setIconText(tr("Polygon Only"));
    polygon->setToolTip(tr("Polygon only"));
    polygon->setStatusTip(tr("Polygon only"));
    filledmesh->setText(tr("Mesh & Filled Polygons"));
    filledmesh->setIconText(tr("Mesh & Filled Polygons"));
    filledmesh->setToolTip(tr("Mesh & filled Polygons"));
    filledmesh->setStatusTip(tr("Mesh & filled Polygons"));
    pointstyle->setText(tr("Dots"));
    pointstyle->setIconText(tr("Dots"));
    pointstyle->setToolTip(tr("Dots"));
    pointstyle->setStatusTip(tr("Dots"));
    barstyle->setText(tr("Bars"));
    barstyle->setIconText(tr("Bars"));
    barstyle->setToolTip(tr("Bars"));
    barstyle->setStatusTip(tr("Bars"));
    conestyle->setText(tr("Cones"));
    conestyle->setIconText(tr("Cones"));
    conestyle->setToolTip(tr("Cones"));
    conestyle->setStatusTip(tr("Cones"));
    crossHairStyle->setText(tr("Crosshairs"));
    crossHairStyle->setIconText(tr("Crosshairs"));
    crossHairStyle->setToolTip(tr("Crosshairs"));
    crossHairStyle->setStatusTip(tr("Crosshairs"));

    // floorstyle->setText( tr( "Floor Style" ) );
    // floorstyle->setIconText( tr( "Floor Style" ) );
    // floorstyle->setStatusTip( tr( "Floor Style" ) );
    floordata->setText(tr("Floor Data Projection"));
    floordata->setIconText(tr("Floor Data Projection"));
    floordata->setToolTip(tr("Floor data projection"));
    floordata->setStatusTip(tr("Floor data projection"));
    flooriso->setText(tr("Floor Isolines"));
    flooriso->setIconText(tr("Floor Isolines"));
    flooriso->setToolTip(tr("Floor isolines"));
    flooriso->setStatusTip(tr("Floor isolines"));
    floornone->setText(tr("Empty Floor"));
    floornone->setIconText(tr("Empty Floor"));
    floornone->setToolTip(tr("Empty floor"));
    floornone->setStatusTip(tr("Empty floor"));

    actionAnimate->setText(tr("Animation"));
    actionAnimate->setIconText(tr("Animation"));
    actionAnimate->setToolTip(tr("Animation"));
    actionAnimate->setStatusTip(tr("Animation"));

    actionPerspective->setText(tr("Enable perspective"));
    actionPerspective->setIconText(tr("Enable perspective"));
    actionPerspective->setToolTip(tr("Enable perspective"));
    actionPerspective->setStatusTip(tr("Enable perspective"));

    actionResetRotation->setText(tr("Reset rotation"));
    actionResetRotation->setIconText(tr("Reset rotation"));
    actionResetRotation->setToolTip(tr("Reset rotation"));
    actionResetRotation->setStatusTip(tr("Reset rotation"));

    actionFitFrame->setText(tr("Fit frame to window"));
    actionFitFrame->setIconText(tr("Fit frame to window"));
    actionFitFrame->setToolTip(tr("Fit frame to window"));
    actionFitFrame->setStatusTip(tr("Fit frame to window"));
}

Graph3D *ApplicationWindow::openMatrixPlot3D(const QString &caption, const QString &matrix_name,
                                             double xl, double xr, double yl, double yr, double zl,
                                             double zr)
{
    QString name = matrix_name;
    name.remove("matrix<", Qt::CaseSensitive);
    name.remove(">");
    Matrix *m = matrix(name);
    if (!m)
        return 0;

    Graph3D *plot = new Graph3D("", &d_workspace, 0, Qt::Widget);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->setWindowTitle(caption);
    plot->setName(caption);
    plot->addMatrixData(m, xl, xr, yl, yr, zl, zr);
    plot->update();

    initPlot3D(plot);
    return plot;
}

void ApplicationWindow::plot3DMatrix(int style)
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Matrix"))
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString label = generateUniqueName(tr("Graph"));

    Graph3D *plot = new Graph3D("", &d_workspace, 0);
    plot->setAttribute(Qt::WA_DeleteOnClose);
    plot->addMatrixData((Matrix *)d_workspace.activeSubWindow());
    plot->customPlotStyle(style);
    customPlot3D(plot);
    plot->update();

    plot->resize(500, 400);
    plot->setWindowTitle(label);
    plot->setName(label);
    initPlot3D(plot);

    emit modified();
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::plotGrayScale()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Matrix"))
        return;

    plotSpectrogram((Matrix *)d_workspace.activeSubWindow(), Graph::GrayMap);
}

MultiLayer *ApplicationWindow::plotGrayScale(Matrix *m)
{
    if (!m)
        return 0;

    return plotSpectrogram(m, Graph::GrayMap);
}

void ApplicationWindow::plotContour()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Matrix"))
        return;

    plotSpectrogram((Matrix *)d_workspace.activeSubWindow(), Graph::ContourMap);
}

MultiLayer *ApplicationWindow::plotContour(Matrix *m)
{
    if (!m)
        return 0;

    return plotSpectrogram(m, Graph::ContourMap);
}

void ApplicationWindow::plotColorMap()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Matrix"))
        return;

    plotSpectrogram((Matrix *)d_workspace.activeSubWindow(), Graph::ColorMap);
}

MultiLayer *ApplicationWindow::plotColorMap(Matrix *m)
{
    if (!m)
        return 0;

    return plotSpectrogram(m, Graph::ColorMap);
}

MultiLayer *ApplicationWindow::plotSpectrogram(Matrix *m, Graph::CurveType type)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MultiLayer *g = multilayerPlot(generateUniqueName(tr("Graph")));
    Graph *plot = g->addLayer();
    setPreferences(plot);

    plot->plotSpectrogram(m, type);
    g->showNormal();

    emit modified();
    QApplication::restoreOverrideCursor();
    return g;
}

ApplicationWindow *ApplicationWindow::importOPJ(const QString &filename [[maybe_unused]])
{
#ifdef ORIGIN_IMPORT
    auto codec = getSettings().value("/General/Dialogs/LastUsedOriginLocale", "").toString();
    if (filename.endsWith(".opj", Qt::CaseInsensitive)
        || filename.endsWith(".ogg", Qt::CaseInsensitive)
        || filename.endsWith(".org", Qt::CaseInsensitive)) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        ApplicationWindow *app = new ApplicationWindow();
        app->applyUserSettings();
        app->setWindowTitle("Makhber - " + filename);
        app->showMaximized();
        app->projectname = filename;
        app->recentProjects.removeAll(filename);
        app->recentProjects.push_front(filename);
        app->updateRecentProjectsList();

        ImportOPJ(app, filename, codec);

        QApplication::restoreOverrideCursor();
        return app;
    } else if (filename.endsWith(".ogm", Qt::CaseInsensitive)
               || filename.endsWith(".ogw", Qt::CaseInsensitive)) {
        ImportOPJ(this, filename, codec);
        recentProjects.removeAll(filename);
        recentProjects.push_front(filename);
        updateRecentProjectsList();
        return this;
    } else
        return 0;
#else
    return NULL;
#endif
}

void ApplicationWindow::deleteFitTables()
{
    QList<MyWidget *> *mLst = new QList<MyWidget *>();
    QList<MyWidget *> windows = windowsList();
    for (int i = 0; i < int(windows.count()); i++) {
        if (windows.at(i)->inherits("MultiLayer"))
            mLst->append(windows.at(i));
    }

    foreach (MyWidget *ml, *mLst) {
        if (ml->inherits("MultiLayer")) {
            QWidgetList lst = ((MultiLayer *)ml)->graphPtrs();
            foreach (QWidget *widget, lst) {
                QList<QwtPlotCurve *> curves = ((Graph *)widget)->fitCurvesList();
                foreach (QwtPlotCurve *c, curves) {
                    if (((PlotCurve *)c)->type() != Graph::Function) {
                        Table *t = ((DataCurve *)c)->table();
                        if (!t)
                            continue;

                        t->askOnCloseEvent(false);
                        t->close();
                    }
                }
            }
        }
    }
    delete mLst;
}

QList<MyWidget *> ApplicationWindow::windowsList()
{
    QList<MyWidget *> lst = QList<MyWidget *>();

    Folder *project_folder = projectFolder();
    FolderListItem *item = project_folder->folderListItem();
    int initial_depth = item->depth();
    QTreeWidgetItemIterator it(item);
    while (item && item->depth() >= initial_depth) {
        QList<MyWidget *> folderWindows = item->folder()->windowsList();
        foreach (MyWidget *w, folderWindows)
            lst.append(w);
        it++;
        item = (FolderListItem *)(*it);
    }

    for (MyWidget *w : hiddenWindows)
        lst.append(w);
    for (MyWidget *w : outWindows)
        lst.append(w);

    return lst;
}

void ApplicationWindow::updateRecentProjectsList()
{
    if (recentProjects.isEmpty())
        return;

    while ((int)recentProjects.size() > MaxRecentProjects)
        recentProjects.pop_back();

    foreach (QAction *action, recent->actions())
        action->deleteLater();

    for (int i = 0; i < (int)recentProjects.size(); i++)
        connect(recent->addAction("&" + QString::number(i + 1) + " " + recentProjects[i]),
                SIGNAL(triggered()), this, SLOT(openRecentProject()));
}

void ApplicationWindow::translateCurveHor()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)w;
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g)
        return;

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));

        btnPointer->setChecked(true);
        return;
    } else if (g->validCurvesDataSize()) {
        btnPointer->setChecked(true);
        g->setActiveTool(new TranslateCurveTool(g, this, TranslateCurveTool::Horizontal,
                                                d_status_info, SLOT(setText(const QString &))));
    }
}

void ApplicationWindow::translateCurveVert()
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)w;
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g)
        return;

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));

        btnPointer->setChecked(true);
        return;
    } else if (g->validCurvesDataSize()) {
        btnPointer->setChecked(true);
        g->setActiveTool(new TranslateCurveTool(g, this, TranslateCurveTool::Vertical,
                                                d_status_info, SLOT(setText(const QString &))));
    }
}

void ApplicationWindow::fitMultiPeakGauss()
{
    fitMultiPeak((int)MultiPeakFit::Gauss);
}

void ApplicationWindow::fitMultiPeakLorentz()
{
    fitMultiPeak((int)MultiPeakFit::Lorentz);
}

void ApplicationWindow::fitMultiPeak(int profile)
{
    QWidget *w = d_workspace.activeSubWindow();
    if (!w || !w->inherits("MultiLayer"))
        return;

    MultiLayer *plot = (MultiLayer *)w;
    if (plot->isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("<h4>There are no plot layers available in this window.</h4>"
                                "<p><h4>Please add a layer and try again!</h4>"));
        btnPointer->setChecked(true);
        return;
    }

    Graph *g = (Graph *)plot->activeGraph();
    if (!g || !g->validCurvesDataSize())
        return;

    if (g->isPiePlot()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("This functionality is not available for pie plots!"));
        return;
    } else {
        bool ok;
        int peaks = QInputDialog::getInt(this, tr("Enter the number of peaks"), tr("Peaks"), 2, 2,
                                         1000000, 1, &ok);
        if (ok && peaks) {
            g->setActiveTool(new MultiPeakFitTool(g, this, (MultiPeakFit::PeakProfile)profile,
                                                  peaks, d_status_info,
                                                  SLOT(setText(const QString &))));
        }
    }
}

#ifdef DOWNLOAD_LINKS
void ApplicationWindow::downloadManual()
{
    QDesktopServices::openUrl(QUrl(MANUAL_URI));
}
#endif // defined DOWNLOAD_LINKS

void ApplicationWindow::showHomePage()
{
    QDesktopServices::openUrl(QUrl(HOMEPAGE_URI));
}

void ApplicationWindow::showForums()
{
    QDesktopServices::openUrl(QUrl(FORUM_URI));
}

void ApplicationWindow::showBugTracker()
{
    QDesktopServices::openUrl(QUrl(BUGREPORT_URI));
}

void ApplicationWindow::parseCommandLineArguments(const QStringList &args)
{
    int num_args = args.count();
    if (num_args == 0)
        return;

    QString str;
    bool exec = false;
    int scriptArg = 0;
    //	foreach(str, args){
    for (int i = 0; i < num_args; ++i) {
        str = args[i];
        if ((str == "-a" || str == "--about") || (str == "-m" || str == "--manual")) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("<b> %1 </b>: This command line option must be used without "
                                     "other arguments!")
                                          .arg(str));
        } else if (str == "-v" || str == "--version") {
            QString s = Makhber::versionString() + Makhber::extraVersion() + "\n";
            s += QObject::tr("Built") + ": " + Makhber::buildDateString() + "\n";
            s += Makhber::copyrightString() + "\n";

#ifdef Q_OS_WIN
            hide();
            QMessageBox::information(this, tr("Makhber") + " - " + tr("Version"), s);
#else
            std::cout << s.toStdString();
#endif
            ::exit(0);
        } else if (str == "-h" || str == "--help") {
            QString s = "\n" + tr("Usage") + ": ";
            s += "makhber [" + tr("options") + "] [" + tr("file") + "_" + tr("name") + "]\n\n";
            s += tr("Valid options are") + ":\n";
            s += "-a " + tr("or") + " --about: " + tr("show about dialog and exit") + "\n";
            s += "-h " + tr("or") + " --help: " + tr("show command line options") + "\n";
            s += "-l=XX " + tr("or") + " --lang=XX: " + tr("start Makhber in language")
                    + " XX ('en', 'fr', 'de', ...)\n";
            s += "-m " + tr("or") + " --manual: " + tr("show Makhber manual in a standalone window")
                    + "\n";
            s += "-v " + tr("or") + " --version: " + tr("print Makhber version and release date")
                    + "\n";
            s += "-x " + tr("or") + " --execute: " + tr("execute the script file given as argument")
                    + "\n\n";
#ifdef ORIGIN_IMPORT
            s += "'" + tr("file") + "_" + tr("name") + "' "
                    + tr("can be any .sciprj, .sciprj.gz, .qti, qti.gz, .opj, .ogm, .ogw, .ogg, "
                         ".org, .py or ASCII file")
                    + "\n";
#else
            s += "'" + tr("file") + "_" + tr("name") + "' "
                    + tr("can be any .sciprj, .sciprj.gz, .qti, qti.gz, .py or ASCII file") + "\n";
#endif
#ifdef Q_OS_WIN
            hide();
            QMessageBox::information(this, tr("Makhber - Help"), s);
#else
            std::cout << s.toStdString();
#endif
            ::exit(0);
        } else if (str.startsWith("--lang=") || str.startsWith("-l=")) {
            QString locale = str.mid(str.indexOf('=') + 1);
            if (locales.contains(locale))
                switchToLanguage(locale);

            if (!locales.contains(locale))
                QMessageBox::critical(
                        this, tr("Error"),
                        tr("<b> %1 </b>: Wrong locale option or no translation available!")
                                .arg(locale));
        } else if (str.startsWith("--execute") || str.startsWith("-x"))
            exec = true;
        else if (str.startsWith("-") || str.startsWith("--")) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("<b> %1 </b> unknown command line option!").arg(str) + "\n"
                                          + tr("Type %1 to see the list of the valid options.")
                                                    .arg("'makhber -h'"));
        }
        if (str.startsWith("-"))
            scriptArg = i; // save last flag
    }

    if (scriptArg < num_args - 1)
        scriptArg++;
    QString file_name = args[scriptArg]; // last argument

    QStringList scriptArgs;
    for (auto i = scriptArg + 1; i < num_args; ++i)
        scriptArgs << args[i];

    if (file_name.startsWith("-"))
        return; // no file name given

    if (!file_name.isEmpty()) {
        QFileInfo fi(file_name);
        if (fi.isDir()) {
            QMessageBox::critical(
                    this, tr("File opening error"),
                    tr("<b>%1</b> is a directory, please specify a file name!").arg(file_name));
            return;
        } else if (!fi.isReadable()) {
            QMessageBox::critical(this, tr("File opening error"),
                                  tr("You don't have the permission to open this file: <b>%1</b>")
                                          .arg(file_name));
            return;
        } else if (!fi.exists()) {
            QMessageBox::critical(this, tr("File opening error"),
                                  tr("The file: <b>%1</b> doesn't exist!").arg(file_name));
            return;
        }

        workingDir = fi.absolutePath();
        saveSettings(); // the recent projects must be saved

        ApplicationWindow *a;
        if (exec)
            a = loadScript(file_name, scriptArgs, exec);
        else
            a = open(file_name, scriptArgs);

        if (a) {
            a->workingDir = workingDir;
            close();
        }
    }
}

void ApplicationWindow::createLanguagesList()
{
    appTranslator = new QTranslator(this);
    qtTranslator = new QTranslator(this);
    qApp->installTranslator(appTranslator);
    qApp->installTranslator(qtTranslator);

    qmPath = qApp->applicationDirPath() + TS_PATH;

    QString lng; // lang, as en_GB
    QString slng; // short lang, as en
    lng = QLocale().name();
    {
        if (lng == "C")
            lng = "en";
        int i = lng.indexOf(QString("."));
        if (i >= 0)
            lng = lng.left(i);
        i = lng.indexOf(QString("_"));
        if (i >= 0)
            slng = lng.left(i);
        else
            slng = lng;
    }
    if (slng.size() > 2)
        slng = slng.left(2);

    QDir dir(qmPath);
    QStringList fileNames = dir.entryList(QStringList("makhber_*.qm"));
    if (fileNames.size() == 0) {
        // fall back to looking in the executable's directory
        qmPath = QFileInfo(QCoreApplication::applicationFilePath()).path() + "/translations";
        dir.setPath(qmPath);
        fileNames = dir.entryList(QStringList("makhber_*.qm"));
    }
    for (int i = 0; i < (int)fileNames.size(); i++) {
        QString locale = fileNames[i];
        locale = locale.mid(locale.indexOf('_') + 1);
        locale.truncate(locale.indexOf('.'));
        locales.push_back(locale);
    }
    locales.push_back("en");
    locales.sort();

    if (appLanguage != "en") {
        if (!appTranslator->load("makhber_" + appLanguage, qmPath))
            if (!appTranslator->load("makhber_" + appLanguage))
                if (!appTranslator->load("makhber_" + lng, qmPath))
                    if (!appTranslator->load("makhber_" + lng))
                        if (!appTranslator->load("makhber_" + slng, qmPath))
                            appTranslator->load("makhber_" + slng);
        if (!qtTranslator->load("qt_" + appLanguage, qmPath + "/qt"))
            if (!qtTranslator->load("qt_" + appLanguage))
                if (!qtTranslator->load("qt_" + lng, qmPath + "/qt"))
                    if (!qtTranslator->load("qt_" + lng))
                        if (!qtTranslator->load("qt_" + slng, qmPath + "/qt"))
                            qtTranslator->load("qt_" + slng);
    }
}

void ApplicationWindow::switchToLanguage(int param)
{
    if (param < (int)locales.size())
        switchToLanguage(locales[param]);
}

void ApplicationWindow::switchToLanguage(const QString &locale)
{
    if (!locales.contains(locale) || appLanguage == locale)
        return;

    appLanguage = locale;
    if (locale == "en") {
        qApp->removeTranslator(appTranslator);
        qApp->removeTranslator(qtTranslator);
        delete appTranslator;
        delete qtTranslator;
        appTranslator = new QTranslator(this);
        qtTranslator = new QTranslator(this);
        qApp->installTranslator(appTranslator);
        qApp->installTranslator(qtTranslator);
    } else {
        if (!appTranslator->load("makhber_" + appLanguage, qmPath))
            appTranslator->load("makhber_" + appLanguage);
        if (!qtTranslator->load("qt_" + appLanguage, qmPath + "/qt"))
            qtTranslator->load("qt_" + appLanguage);
    }
    insertTranslatedStrings();
}

QStringList ApplicationWindow::matrixNames()
{
    QStringList names;
    QList<MyWidget *> windows = windowsList();
    foreach (MyWidget *w, windows) {
        if (w->inherits("Matrix"))
            names << static_cast<Matrix *>(w)->name();
    }
    return names;
}

bool ApplicationWindow::alreadyUsedName(const QString &label)
{
    QList<MyWidget *> windows = windowsList();
    bool used = false;
    foreach (MyWidget *widget, windows) {
        if (widget && widget->name() == label) {
            used = true;
            break;
        }
    }
    return used;
}

bool ApplicationWindow::projectHasMatrices()
{
    QList<MyWidget *> windows = windowsList();
    bool has = false;
    foreach (MyWidget *w, windows) {
        if (w->inherits("Matrix")) {
            has = true;
            break;
        }
    }
    return has;
}

bool ApplicationWindow::projectHas2DPlots()
{
    QList<MyWidget *> windows = windowsList();
    bool hasPlots = false;
    foreach (MyWidget *w, windows) {
        if (w->inherits("MultiLayer")) {
            hasPlots = true;
            break;
        }
    }
    return hasPlots;
}

bool ApplicationWindow::projectHas3DPlots()
{
    QList<MyWidget *> windows = windowsList();
    bool has3DPlots = false;
    foreach (MyWidget *w, windows) {
        if (w->inherits("Graph3D")) {
            has3DPlots = true;
            break;
        }
    }
    return has3DPlots;
}

void ApplicationWindow::appendProject()
{
    OpenProjectDialog *open_dialog = new OpenProjectDialog(this, false);
    open_dialog->setDirectory(workingDir);
    open_dialog->setExtensionWidget(0);
    if (open_dialog->exec() != QDialog::Accepted || open_dialog->selectedFiles().isEmpty())
        return;
    workingDir = open_dialog->directory().path();
    appendProject(open_dialog->selectedFiles()[0]);
}

void ApplicationWindow::appendProject(const QString &fn)
{
    if (fn.isEmpty())
        return;

    QFile *file;

    QFileInfo fi(fn);
    workingDir = fi.absolutePath();

    if (fn.contains(".sciprj") || fn.contains(".qti") || fn.contains(".opj", Qt::CaseInsensitive)
        || fn.contains(".ogm", Qt::CaseInsensitive) || fn.contains(".ogw", Qt::CaseInsensitive)
        || fn.contains(".ogg", Qt::CaseInsensitive) || fn.contains(".org", Qt::CaseInsensitive)) {
        QFileInfo f(fn);
        if (!f.exists()) {
            QMessageBox::critical(this, tr("File opening error"),
                                  tr("The file: <b>%1</b> doesn't exist!").arg(fn));
            return;
        }
    } else {
        QMessageBox::critical(
                this, tr("File opening error"),
                tr("The file: <b>%1</b> is not a Makhber or Origin project file!").arg(fn));
        return;
    }

    if (fn.endsWith(".gz", Qt::CaseInsensitive) || fn.endsWith(".gz~", Qt::CaseInsensitive)) {
        file = openCompressedFile(fn);
        if (!file)
            return;
    } else {
        file = new QFile(fn);
        file->open(QIODevice::ReadOnly);
    }

    recentProjects.removeAll(fn);
    recentProjects.push_front(fn);
    updateRecentProjectsList();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    Folder *cf = current_folder;
    FolderListItem *item = (FolderListItem *)current_folder->folderListItem();
    folders.blockSignals(true);
    blockSignals(true);

    QString baseName = fi.baseName();
    QStringList lst = current_folder->subfolders();
    int n = lst.count(baseName);
    if (n) { // avoid identical subfolder names
        while (lst.count(baseName + QString::number(n)))
            n++;
        baseName += QString::number(n);
    }

    current_folder = &current_folder->addChild<Folder>(baseName);
    FolderListItem *fli = new FolderListItem(item, current_folder);
    current_folder->setFolderListItem(fli);

    if (fn.contains(".opj", Qt::CaseInsensitive) || fn.contains(".ogm", Qt::CaseInsensitive)
        || fn.contains(".ogw", Qt::CaseInsensitive) || fn.contains(".ogg", Qt::CaseInsensitive)
        || fn.contains(".org", Qt::CaseInsensitive))
#ifdef ORIGIN_IMPORT
    {
        auto codec = getSettings().value("/General/Dialogs/LastUsedOriginLocale", "").toString();
        ImportOPJ(this, fn, codec);
    }
#else
    {
        QMessageBox::critical(
                this, tr("File opening error"),
                tr("Makhber currently does not support Origin import. If you are interested in "
                   "reviving and maintaining an Origin import filter, contact the developers.")
                        .arg(fn));
        return;
    }
#endif
    else {
        QTextStream t(file);
        t.setCodec(QTextCodec::codecForName("UTF-8"));

        QString s = t.readLine();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        lst = s.split(QRegExp("\\s"), Qt::SkipEmptyParts);
        QString version = lst[1];
        lst = version.split(".", Qt::SkipEmptyParts);
#else
        lst = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
        QString version = lst[1];
        lst = version.split(".", QString::SkipEmptyParts);
#endif
        if (fn.endsWith(".qti", Qt::CaseInsensitive) || fn.endsWith(".qti.gz", Qt::CaseInsensitive))
            d_file_version = 100 * (lst[0]).toInt() + 10 * (lst[1]).toInt() + (lst[2]).toInt();
        else
            d_file_version = ((lst[0]).toInt() << 16) + ((lst[1]).toInt() << 8) + (lst[2]).toInt();

        t.readLine();
        if (d_file_version < 73)
            t.readLine();

        // process tables and matrix information
        while (!t.atEnd()) {
            s = t.readLine(4096); // workaround for safely reading very big lines
            lst.clear();
            if (s.left(8) == "<folder>") {
                lst = s.split("\t");
                Folder &f = current_folder->addChild<Folder>(lst[1]);
                f.setBirthDate(lst[2]);
                f.setModificationDate(lst[3]);
                if (lst.count() > 4)
                    if (lst[4] == "current")
                        cf = &f;

                FolderListItem *fli = new FolderListItem(current_folder->folderListItem(), &f);
                fli->setText(0, lst[1]);
                f.setFolderListItem(fli);

                current_folder = &f;
            } else if (s == "<table>") {
                openTable(this, t);
            } else if (s == "<matrix>") {
                while (s != "</matrix>") {
                    s = t.readLine();
                    lst << s;
                }
                lst.pop_back();
                openMatrix(this, lst);
            } else if (s == "<note>") {
                for (int i = 0; i < 3; i++) {
                    s = t.readLine();
                    lst << s;
                }
                Note *m = openNote(this, lst);
                QStringList cont;
                while (s != "</note>") {
                    s = t.readLine();
                    cont << s;
                }
                cont.pop_back();
                m->restore(cont);
            } else if (s == "</folder>") {
                Folder *parent = (Folder *)current_folder->parent();
                if (!parent)
                    current_folder = projectFolder();
                else
                    current_folder = parent;
            }
        }

        // process the rest
        t.seek(0);

        MultiLayer *plot = 0;
        while (!t.atEnd()) {
            s = t.readLine(4096); // workaround for safely reading very big lines
            if (s.left(8) == "<folder>") {
                lst = s.split("\t");
                current_folder = current_folder->findSubfolder(lst[1]);
            } else if (s == "<multiLayer>") { // process multilayers information
                s = t.readLine();
                QStringList graph = s.split("\t");
                QString caption = graph[0];
                plot = multilayerPlot(caption);
                plot->setCols(graph[1].toInt());
                plot->setRows(graph[2].toInt());
                setListViewDate(caption, graph[3]);
                plot->setBirthDate(graph[3]);
                plot->blockSignals(true);

                restoreWindowGeometry(this, plot, t.readLine());

                if (d_file_version > 71) {
                    QStringList lst = t.readLine().split("\t");
                    plot->setWindowLabel(lst[1]);
                    setListViewLabel(plot->name(), lst[1]);
                    plot->setCaptionPolicy((MyWidget::CaptionPolicy)lst[2].toInt());
                }

                if (d_file_version > 83) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                    QStringList lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                    plot->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(),
                                     lst[4].toInt());
                    lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                    plot->setSpacing(lst[1].toInt(), lst[2].toInt());
                    lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                    plot->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                    lst = t.readLine().split("\t", Qt::SkipEmptyParts);
                    plot->setAlignement(lst[1].toInt(), lst[2].toInt());
#else
                    QStringList lst = t.readLine().split("\t", QString::SkipEmptyParts);
                    plot->setMargins(lst[1].toInt(), lst[2].toInt(), lst[3].toInt(),
                                     lst[4].toInt());
                    lst = t.readLine().split("\t", QString::SkipEmptyParts);
                    plot->setSpacing(lst[1].toInt(), lst[2].toInt());
                    lst = t.readLine().split("\t", QString::SkipEmptyParts);
                    plot->setLayerCanvasSize(lst[1].toInt(), lst[2].toInt());
                    lst = t.readLine().split("\t", QString::SkipEmptyParts);
                    plot->setAlignement(lst[1].toInt(), lst[2].toInt());
#endif
                }

                while (s != "</multiLayer>") { // open layers
                    s = t.readLine();
                    if (s.left(7) == "<graph>") {
                        lst.clear();
                        while (s != "</graph>") {
                            s = t.readLine();
                            lst << s;
                        }
                        openGraph(this, plot, lst);
                    }
                }
                plot->blockSignals(false);
            } else if (s == "<SurfacePlot>") { // process 3D plots information
                lst.clear();
                while (s != "</SurfacePlot>") {
                    s = t.readLine();
                    lst << s;
                }
                openSurfacePlot(this, lst);
            } else if (s == "</folder>") {
                Folder *parent = (Folder *)current_folder->parent();
                if (!parent)
                    current_folder = projectFolder();
                else
                    current_folder = parent;
            }
        }
        file->close();
        delete file;
    }

    folders.blockSignals(false);
    // change folder to user defined current folder
    changeFolder(cf);
    blockSignals(false);
    renamedTables = QStringList();
    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::rawSaveFolder(Folder *folder, QIODevice *device)
{
    QTextStream stream(device);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    foreach (MyWidget *w, folder->windowsList()) {
        Table *t = qobject_cast<Table *>(w);
        if (t)
            t->saveToDevice(device, windowGeometryInfo(w));
        else
            stream << w->saveToString(windowGeometryInfo(w));
    }
    foreach (Folder *subfolder, folder->folders()) {
        stream << "<folder>\t" + QString(subfolder->name()) + "\t" + subfolder->birthDate() + "\t"
                        + subfolder->modificationDate();
        if (subfolder == current_folder)
            stream << "\tcurrent\n";
        else
            stream << "\n"; // FIXME: Having no 5th string here is not a good idea
        stream.flush();
        rawSaveFolder(subfolder, device);
        stream << "</folder>\n";
    }
}

void ApplicationWindow::saveFolder(Folder *folder, const QString &fn)
{
    // file saving procedure follows
    // https://bugs.launchpad.net/ubuntu/+source/linux/+bug/317781/comments/54
    QFile f(fn + ".new");
    while (!f.open(QIODevice::WriteOnly)) {
        if (f.isOpen())
            f.close();
        // The following message is slightly misleading, since it may be that fn.new can't be opened
        // _at_all_. However, changing this would break translations, in a bugfix release.
        // TODO: rephrase message for next minor release
        switch (QMessageBox::critical(
                this, tr("File save error"),
                tr("The file: <br><b>%1</b> is opened in read-only mode").arg(fn + ".new"),
                QMessageBox::Retry | QMessageBox::Default,
                QMessageBox::Abort | QMessageBox::Escape)) {
        case QMessageBox::Abort:
            return;
        }
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QTextStream t(&f);
    t.setCodec(QTextCodec::codecForName("UTF-8"));
    t << Makhber::schemaVersion() + " project file\n";
    t << "<scripting-lang>\t" + QString(scriptEnv->objectName()) + "\n";
    t << "<windows>\t" + QString::number(folder->windowCount(true)) + "\n";
    t.flush();
    rawSaveFolder(folder, &f);
    t << "<log>\n" + logInfo + "</log>";

    // second part of secure file saving (see comment at the start of this method)
#ifdef Q_OS_WIN
    // this one was taken from
    // http://support.microsoft.com/kb/148505/en-us
    // http://msdn.microsoft.com/en-us/library/17618685(VS.80).aspx
    if (!f.flush() || _commit(f.handle()) != 0) {
#else
    if (!f.flush() || fsync(f.handle()) != 0) {
#endif
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(
                this, tr("Error writing data to disk"),
                tr("<html>%1<br><br>Your data may or may not have ended up in <em>%2</em> (%3). \
					If there already was a version of this project on disk, it has not been touched.</html>")
                        .arg(QString::fromLocal8Bit(strerror(errno)))
                        .arg(fn + ".new")
                        .arg(f.handle()));
        f.close();
        return;
    }
    f.close();
#ifdef Q_OS_WIN
    // unfortunately, Windows doesn't support atomic renames; so Windows users will have to live
    // with the risk of losing the file in case of a crash between remove and rename
    if ((QFile::exists(fn)
         && ((QFile::exists(fn + "~") && !QFile::remove(fn + "~")) || !QFile::rename(fn, fn + "~")))
        || !QFile::rename(fn + ".new", fn)) {
#else
    // we want to atomically replace existing files, so we can't use QFile::rename()
    if ((QFile::exists(fn) && rename(QFile::encodeName(fn), QFile::encodeName(fn + "~")) != 0)
        || rename(QFile::encodeName(fn + ".new"), QFile::encodeName(fn)) != 0) {
#endif
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(
                this, tr("Error renaming backup files"),
                tr("<html>%1<br><br>Data was written to <em>%2</em>, but saving the original file as <em>%3</em>\
					and moving the new file to <em>%4</em> failed. In case you wonder why the original file hasn't\
					been simply replaced, see here:\
					<a href=\"http://bugs.launchpad.net/ubuntu/+source/linux/+bug/317781/comments/54\">\
					http://bugs.launchpad.net/ubuntu/+source/linux/+bug/317781/comments/54</a>.</html>")
                        .arg(QString::fromLocal8Bit(strerror(errno)))
                        .arg(fn + ".new")
                        .arg(fn + "~")
                        .arg(fn));
        return;
    }

    QApplication::restoreOverrideCursor();
}

void ApplicationWindow::saveAsProject()
{
    saveFolderAsProject(current_folder);
}

void ApplicationWindow::saveFolderAsProject(Folder *f)
{
    QString filter = tr("Makhber project") + " (*.sciprj);;";
    filter += tr("Compressed Makhber project") + " (*.sciprj.gz)";

    QString selectedFilter;
    QString fn = QFileDialog::getSaveFileName(this, tr("Save project as"), workingDir, filter,
                                              &selectedFilter);
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        workingDir = fi.absolutePath();
        QString baseName = fi.fileName();
        if (!baseName.endsWith(".sciprj") && !baseName.endsWith(".sciprj.gz")) {
            fn.append(".sciprj");
        }
        bool compress = false;
        if (fn.endsWith(".gz")) {
            fn = fn.left(fn.length() - 3);
            compress = true;
        }

        saveFolder(f, fn);
        if (selectedFilter.contains(".gz") || compress)
            file_compress(QFile::encodeName(fn).constData(), "wb9");
    }
}

void ApplicationWindow::showFolderPopupMenu(const QPoint &p, bool fromFolders)
{
    QMenu *cm = nullptr;
    if (fromFolders) {
        cm = showFolderPopupMenuImpl(folders.itemAt(p), fromFolders);
        if (cm)
            cm->exec(folders.mapToGlobal(p));
    } else {
        cm = showFolderPopupMenuImpl(lv.itemAt(p), fromFolders);
        if (cm)
            cm->exec(lv.mapToGlobal(p));
    }
}

QMenu *ApplicationWindow::showFolderPopupMenuImpl(QTreeWidgetItem *it, bool fromFolders)
{
    if (!it || folders.isRenaming())
        return nullptr;

    QMenu *cm = new QMenu(this);
    cm->addAction(tr("&Find..."), this, SLOT(showFindDialogue()));
    cm->addSeparator();
    cm->addAction(tr("App&end Project..."), this, SLOT(appendProject()));
    if (((FolderListItem *)it)->folder()->parent())
        cm->addAction(tr("Save &As Project..."), this, SLOT(saveAsProject()));
    else
        cm->addAction(tr("Save Project &As..."), this, SLOT(saveProjectAs()));
    cm->addSeparator();

    if (fromFolders && show_windows_policy != HideAll) {
        cm->addAction(tr("&Show All Windows"), this, SLOT(showAllFolderWindows()));
        cm->addAction(tr("&Hide All Windows"), this, SLOT(hideAllFolderWindows()));
        cm->addSeparator();
    }

    if (((FolderListItem *)it)->folder()->parent()) {
        cm->addAction(QPixmap(":/close.xpm"), tr("&Delete Folder"), this, SLOT(deleteFolder()),
                      Qt::Key_F8);
        cm->addAction(tr("&Rename"), this, SLOT(startRenameFolder()), Qt::Key_F2);
        cm->addSeparator();
    }

    if (fromFolders) {
        QMenu *window = cm->addMenu(tr("New &Window"));
        window->addAction(actionNewTable);
        window->addAction(actionNewMatrix);
        window->addAction(actionNewNote);
        window->addAction(actionNewGraph);
        window->addAction(actionNewFunctionPlot);
        window->addAction(actionNewSurfacePlot);
    }

    cm->addAction(QPixmap(":/newfolder.xpm"), tr("New F&older"), this, SLOT(addFolder()),
                  Qt::Key_F7);
    cm->addSeparator();

    QMenu *viewWindowsMenu = cm->addMenu(tr("&View Windows"));
    QStringList lst;
    lst << tr("&None") << tr("&Windows in Active Folder")
        << tr("Windows in &Active Folder && Subfolders");
    for (int i = 0; i < 3; ++i) {
        QAction *actId = viewWindowsMenu->addAction(lst[i], this, SLOT(setShowWindowsPolicy(bool)));
        actId->setData(i);
        actId->setCheckable(true);
        actId->setChecked(show_windows_policy == i);
    }
    cm->addSeparator();
    cm->addAction(tr("&Properties..."), this, SLOT(folderProperties()));
    return cm;
}

void ApplicationWindow::setShowWindowsPolicy(bool checked)
{
    Q_UNUSED(checked)

    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;
    int p = act->data().toInt();

    if (show_windows_policy == (ShowWindowsPolicy)p)
        return;

    show_windows_policy = (ShowWindowsPolicy)p;
    if (show_windows_policy == HideAll) {
        QList<MyWidget *> lst = windowsList();
        foreach (MyWidget *widget, lst) {
            if (!widget)
                continue;
            hiddenWindows.append(widget);
            widget->hide();
            setListView(widget->name(), tr("Hidden"));
        }
    } else
        showAllFolderWindows();
}

void ApplicationWindow::showFindDialogue()
{
    FindDialog *fd = new FindDialog(this);
    fd->setAttribute(Qt::WA_DeleteOnClose);
    fd->exec();
}

void ApplicationWindow::startRenameFolder()
{
    FolderListItem *fi = current_folder->folderListItem();
    if (!fi)
        return;

    disconnect(&folders, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this,
               SLOT(folderItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    fi->setFlags(fi->flags() | Qt::ItemIsEditable);
    fi->treeWidget()->editItem(fi, 0);
}

void ApplicationWindow::startRenameFolder(QTreeWidgetItem *item, int column)
{
    if (!item || item == folders.topLevelItem(0))
        return;

    if (item->treeWidget() == &lv && item->type() == FolderListItem::FolderType) {
        disconnect(
                &folders,
                SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *, QTreeWidgetItem *)),
                this, SLOT(folderItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
        current_folder = ((FolderListItem *)item)->folder();
        FolderListItem *it = current_folder->folderListItem();
        it->treeWidget()->setCurrentItem(it, 0);
        it->setFlags(it->flags() | Qt::ItemIsEditable);
        it->treeWidget()->editItem(it, column);
    } else {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->treeWidget()->editItem(item, 0);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

void ApplicationWindow::renameFolder(QTreeWidgetItem *it, int col, const QString &text)
{
    Q_UNUSED(col)

    if (!it)
        return;

    if (it->text(0) == text)
        return;

    Folder *parent = (Folder *)current_folder->parent();
    if (!parent) // the parent folder is the project folder (it always exists)
        parent = projectFolder();

    if (text.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid name!"));
        it->setFlags(it->flags() | Qt::ItemIsEditable);
        return;
    }

    QStringList lst = parent->subfolders();
    lst.removeAll(current_folder->name());
    while (lst.contains(text)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Name already exists!") + "\n"
                                      + tr("Please choose another name!"));

        it->setFlags(it->flags() | Qt::ItemIsEditable);
        it->setText(0, current_folder->name());
        it->treeWidget()->editItem(it, 0);
        return;
    }

    current_folder->setName(text);
    it->setText(0, text);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    connect(&folders, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this,
            SLOT(folderItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    folders.scrollToItem(parent->folderListItem(), QAbstractItemView::EnsureVisible);
    folders.setCurrentItem(parent->folderListItem()); // update the list views
}

void ApplicationWindow::showAllFolderWindows()
{
    QList<MyWidget *> lst = current_folder->windowsList();
    foreach (MyWidget *w, lst) { // force show all windows in current folder
        if (w) {
            updateWindowLists(w);
            switch (w->status()) {
            case MyWidget::Hidden:
                w->showNormal();
                break;

            case MyWidget::Normal:
                w->showNormal();
                break;

            case MyWidget::Minimized:
                w->showMinimized();
                break;

            case MyWidget::Maximized:
                w->showMaximized();
                break;
            }
        }
    }

    if ((current_folder->children()).isEmpty())
        return;

    FolderListItem *fi = current_folder->folderListItem();
    FolderListItem *item = (FolderListItem *)fi->child(0);
    int initial_depth = item->depth();
    QTreeWidgetItemIterator it(item);
    while (item && item->depth() >= initial_depth) { // show/hide windows in all subfolders
        lst = ((Folder *)item->folder())->windowsList();
        foreach (MyWidget *w, lst) {
            if (w && show_windows_policy == SubFolders) {
                updateWindowLists(w);
                switch (w->status()) {
                case MyWidget::Hidden:
                    w->showNormal();
                    break;

                case MyWidget::Normal:
                    w->showNormal();
                    break;

                case MyWidget::Minimized:
                    w->showMinimized();
                    break;

                case MyWidget::Maximized:
                    w->showMaximized();
                    break;
                }
            } else
                w->hide();
        }

        it++;
        item = (FolderListItem *)(*it);
    }
}

void ApplicationWindow::hideAllFolderWindows()
{
    QList<MyWidget *> lst = current_folder->windowsList();
    foreach (MyWidget *w, lst)
        hideWindow(w);

    if ((current_folder->children()).isEmpty())
        return;

    if (show_windows_policy == SubFolders) {
        FolderListItem *fi = current_folder->folderListItem();
        FolderListItem *item = (FolderListItem *)fi->child(0);
        int initial_depth = item->depth();
        QTreeWidgetItemIterator it(item);
        while (item && item->depth() >= initial_depth) {
            lst = item->folder()->windowsList();
            foreach (MyWidget *w, lst)
                hideWindow(w);

            it++;
            item = (FolderListItem *)(*it);
        }
    }
}

void ApplicationWindow::projectProperties()
{
    QString s = QString(current_folder->name()) + "\n\n";
    s += "\n\n\n";
    s += tr("Type") + ": " + tr("Project") + "\n\n";
    if (projectname != "untitled") {
        s += tr("Path") + ": " + projectname + "\n\n";

        QFileInfo fi(projectname);
        s += tr("Size") + ": " + QString::number(fi.size()) + " " + tr("bytes") + "\n\n";
    }

    s += tr("Contents") + ": " + QString::number(windowsList().count()) + " " + tr("windows");

    s += ", " + QString::number(current_folder->subfolders().count()) + " " + tr("folders")
            + "\n\n";
    s += "\n\n\n";

    if (projectname != "untitled") {
        QFileInfo fi(projectname);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        s += tr("Created") + ": " + QLocale().toString(fi.created()) + "\n\n";
#else
        s += tr("Created") + ": " + QLocale().toString(fi.birthTime()) + "\n\n";
#endif
        s += tr("Modified") + ": " + QLocale().toString(fi.lastModified()) + "\n\n";
    } else
        s += tr("Created") + ": " + current_folder->birthDate() + "\n\n";

    QMessageBox *mbox = new QMessageBox(tr("Properties"), s, QMessageBox::NoIcon, QMessageBox::Ok,
                                        QMessageBox::NoButton, QMessageBox::NoButton, this);

    mbox->setIconPixmap(QPixmap(":/appicon"));
    mbox->show();
}

void ApplicationWindow::folderProperties()
{
    if (!current_folder->parent()) {
        projectProperties();
        return;
    }

    QString s = current_folder->name() + "\n\n";
    s += "\n\n\n";
    s += tr("Type") + ": " + tr("Folder") + "\n\n";
    s += tr("Path") + ": " + current_folder->path() + "\n\n";
    s += tr("Contents") + ": " + QString::number(current_folder->windowsList().count()) + " "
            + tr("windows");
    s += ", " + QString::number(current_folder->subfolders().count()) + " " + tr("folders")
            + "\n\n";
    // s += "\n\n\n";
    s += tr("Created") + ": " + current_folder->birthDate() + "\n\n";
    // s += tr("Modified") + ": " + current_folder->modificationDate() + "\n\n";

    QMessageBox *mbox = new QMessageBox(tr("Properties"), s, QMessageBox::NoIcon, QMessageBox::Ok,
                                        QMessageBox::NoButton, QMessageBox::NoButton, this);

    mbox->setIconPixmap(QPixmap(":/folder_open.xpm"));
    mbox->show();
}

void ApplicationWindow::addFolder()
{
    QStringList lst = current_folder->subfolders();
    QString name = tr("New Folder");
    lst = lst.filter(name);
    if (!lst.isEmpty())
        name += " (" + QString::number(lst.size() + 1) + ")";

    Folder &f = current_folder->addChild<Folder>(name);
    addFolderListViewItem(&f);

    FolderListItem *fi = new FolderListItem(current_folder->folderListItem(), &f);
    f.setFolderListItem(fi);
    fi->setFlags(fi->flags() | Qt::ItemIsEditable);
    fi->treeWidget()->setCurrentItem(fi, 0);
    fi->treeWidget()->editItem(fi, 0);
    fi->treeWidget()->resizeColumnToContents(0);
}

bool ApplicationWindow::deleteFolder(Folder *f)
{
    if (confirmCloseFolder
        && QMessageBox::information(
                this, tr("Delete folder?"),
                tr("Delete folder '%1' and all the windows it contains?").arg(f->name()), tr("Yes"),
                tr("No"), 0, 0)) {
        return false;
    } else {
        FolderListItem *fi = f->folderListItem();
        foreach (MyWidget *w, f->windowsList())
            closeWindow(w);

        if (!(f->children()).isEmpty()) {
            FolderListItem *item = (FolderListItem *)fi->child(0);
            int initial_depth = item->depth();
            QTreeWidgetItemIterator it(item);
            while (item && item->depth() >= initial_depth) {
                Folder *subFolder = (Folder *)item->folder();
                if (subFolder) {
                    foreach (MyWidget *w, subFolder->windowsList()) {
                        removeWindowFromLists(w);
                        subFolder->removeWindow(w);
                        delete w;
                    }

                    FolderListItem *old_item = item;
                    it++;
                    item = (FolderListItem *)(*it);
                    delete subFolder;
                    delete old_item;
                }
            }
        }

        delete f;
        delete fi;
        return true;
    }
}

void ApplicationWindow::deleteFolder()
{
    Folder *parent = (Folder *)current_folder->parent();
    if (!parent)
        parent = projectFolder();

    folders.blockSignals(true);

    if (deleteFolder(current_folder)) {
        current_folder = parent;
        folders.setCurrentItem(parent->folderListItem());
        changeFolder(parent, true);
    }

    folders.blockSignals(false);
    folders.setFocus();
}

void ApplicationWindow::folderItemDoubleClicked(QTreeWidgetItem *it, int column)
{
    Q_UNUSED(column)
    if (!it)
        return;

    if (it->type() == FolderListItem::FolderType) {
        FolderListItem *item = ((FolderListItem *)it)->folder()->folderListItem();
        folders.setCurrentItem(item);
    } else if (it->type() == WindowListItem::WindowType) {
        MyWidget *w = ((WindowListItem *)it)->window();
        if (!w)
            return;
        if (d_workspace.activeSubWindow() != w)
            activateSubWindow(w);
        else {
            if (!w->isMaximized())
                w->setMaximized();
            else
                w->setNormal();
        }
    }
}

void ApplicationWindow::folderItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)

    if (!current)
        return;

    if (current == previous)
        return;

    current->setExpanded(true);
    changeFolder(((FolderListItem *)current)->folder());
    folders.scrollToItem(folders.currentItem(), QAbstractItemView::EnsureVisible);
    folders.setFocus();
}

void ApplicationWindow::hideFolderWindows(Folder *f)
{
    QList<MyWidget *> lst = f->windowsList();
    foreach (MyWidget *w, lst)
        w->hide();

    if ((f->children()).isEmpty())
        return;

    FolderListItem *fi = f->folderListItem();
    FolderListItem *item = (FolderListItem *)fi->child(0);
    if (!item)
        return;
    int initial_depth = item->depth();
    QTreeWidgetItemIterator it(item);
    while (item && item->depth() >= initial_depth) {
        lst = item->folder()->windowsList();
        foreach (MyWidget *w, lst)
            w->hide();
        it++;
        item = (FolderListItem *)(*it);
    }
}

bool ApplicationWindow::changeFolder(Folder *newFolder, bool force)
{
    if (current_folder == newFolder && !force)
        return false;

    deactivateFolders();
    newFolder->folderListItem()->setActive(true);

    Folder *oldFolder = current_folder;
    MyWidget::Status old_active_window_state = MyWidget::Normal;
    MyWidget *old_active_window = oldFolder->activeWindow();
    if (old_active_window)
        old_active_window_state = old_active_window->status();

    MyWidget::Status active_window_state = MyWidget::Normal;
    MyWidget *active_window = newFolder->activeWindow();
    if (active_window)
        active_window_state = active_window->status();

    d_workspace.blockSignals(true);
    hideFolderWindows(oldFolder);
    current_folder = newFolder;

    lv.clear();

    QObjectList folderLst = newFolder->children();
    if (!folderLst.isEmpty()) {
        foreach (QObject *f, folderLst)
            addFolderListViewItem(static_cast<Folder *>(f));
    }

    QList<MyWidget *> lst = newFolder->windowsList();
    foreach (MyWidget *w, lst) {
        w->blockSignals(true);
        if (!hiddenWindows.contains(w) && !outWindows.contains(w)
            && show_windows_policy != HideAll) {
            // show only windows in the current folder which are not hidden by the user
            if (w->status() == MyWidget::Normal)
                w->showNormal();
            else if (w->status() == MyWidget::Minimized)
                w->showMinimized();
        } else
            w->setStatus(MyWidget::Hidden);

        addListViewItem(w);
    }

    if (!(newFolder->children()).isEmpty()) {
        FolderListItem *fi = newFolder->folderListItem();
        FolderListItem *item = (FolderListItem *)fi->child(0);
        if (!item)
            return false;
        int initial_depth = item->depth();
        QTreeWidgetItemIterator it(item);
        while (item && item->depth() >= initial_depth) { // show/hide windows in subfolders
            lst = ((Folder *)item->folder())->windowsList();
            foreach (MyWidget *w, lst) {
                if (!hiddenWindows.contains(w) && !outWindows.contains(w)) {
                    if (show_windows_policy == SubFolders) {
                        if (w->status() == MyWidget::Normal || w->status() == MyWidget::Maximized)
                            w->showNormal();
                        else if (w->status() == MyWidget::Minimized)
                            w->showMinimized();
                    } else if (w->isVisible())
                        w->hide();
                }
            }
            it++;
            item = (FolderListItem *)(*it);
        }
    }

    d_workspace.blockSignals(false);

    if (active_window) {
        d_workspace.setActiveSubWindow(active_window);
        if (active_window_state == MyWidget::Minimized)
            active_window->showMinimized(); // d_workspace.setActiveWindow() makes minimized windows
                                            // to be shown normally
        else if (active_window_state == MyWidget::Maximized) {
            if (active_window->inherits("Graph3D"))
                ((Graph3D *)active_window)->setIgnoreFonts(true);
            active_window->showMaximized();
            if (active_window->inherits("Graph3D"))
                ((Graph3D *)active_window)->setIgnoreFonts(false);
        }
        current_folder->setActiveWindow(active_window);
        customMenu(active_window);
        customToolBars(active_window);
    }

    if (old_active_window) {
        old_active_window->setStatus(old_active_window_state);
        oldFolder->setActiveWindow(old_active_window);
    }

    foreach (MyWidget *w, newFolder->windowsList())
        w->blockSignals(false);

    return true;
}

void ApplicationWindow::deactivateFolders()
{
    QTreeWidgetItemIterator it(&folders);
    while (*it) {
        ((FolderListItem *)(*it))->setActive(false);
        it++;
    }
}

void ApplicationWindow::addListViewItem(MyWidget *w)
{
    if (!w)
        return;

    WindowListItem *it = new WindowListItem(&lv, w);
    if (w->inherits("Matrix")) {
        it->setIcon(0, QIcon(QPixmap(":/matrix.xpm")));
        it->setText(1, tr("Matrix"));
    } else if (w->inherits("Table")) {
        it->setIcon(0, QIcon(QPixmap(":/worksheet.xpm")));
        it->setText(1, tr("Table"));
    } else if (w->inherits("Note")) {
        it->setIcon(0, QIcon(QPixmap(":/note.xpm")));
        it->setText(1, tr("Note"));
    } else if (w->inherits("MultiLayer")) {
        it->setIcon(0, QIcon(QPixmap(":/graph.xpm")));
        it->setText(1, tr("Graph"));
    } else if (w->inherits("Graph3D")) {
        it->setIcon(0, QIcon(QPixmap(":/trajectory.xpm")));
        it->setText(1, tr("3D Graph"));
    }

    it->setText(0, w->name());
    it->setText(2, w->aspect());
    it->setText(3, w->birthDate());
    it->setText(4, w->windowLabel());
}

void ApplicationWindow::windowProperties()
{
    WindowListItem *it = (WindowListItem *)lv.currentItem();
    MyWidget *w = it->window();
    if (!w)
        return;

    QMessageBox *mbox =
            new QMessageBox(tr("Properties"), QString(), QMessageBox::NoIcon, QMessageBox::Ok,
                            QMessageBox::NoButton, QMessageBox::NoButton, this);

    QString s = QString(w->name()) + "\n\n";
    s += "\n\n\n";

    s += tr("Label") + ": " + ((MyWidget *)w)->windowLabel() + "\n\n";

    if (w->inherits("Matrix")) {
        mbox->setIconPixmap(QPixmap(":/matrix.xpm"));
        s += tr("Type") + ": " + tr("Matrix") + "\n\n";
    } else if (w->inherits("Table")) {
        mbox->setIconPixmap(QPixmap(":/worksheet.xpm"));
        s += tr("Type") + ": " + tr("Table") + "\n\n";
    } else if (w->inherits("Note")) {
        mbox->setIconPixmap(QPixmap(":/note.xpm"));
        s += tr("Type") + ": " + tr("Note") + "\n\n";
    } else if (w->inherits("MultiLayer")) {
        mbox->setIconPixmap(QPixmap(":/graph.xpm"));
        s += tr("Type") + ": " + tr("Graph") + "\n\n";
    } else if (w->inherits("Graph3D")) {
        mbox->setIconPixmap(QPixmap(":/trajectory.xpm"));
        s += tr("Type") + ": " + tr("3D Graph") + "\n\n";
    }
    s += tr("Path") + ": " + current_folder->path() + "\n\n";
    s += tr("Created") + ": " + w->birthDate() + "\n\n";
    s += tr("Status") + ": " + it->text(2) + "\n\n";
    mbox->setText(s);
    mbox->show();
}

void ApplicationWindow::addFolderListViewItem(Folder *f)
{
    if (!f)
        return;

    FolderListItem *it = new FolderListItem(&lv, f);
    it->setActive(false);
    it->setText(0, f->name());
    it->setText(1, tr("Folder"));
    it->setText(3, f->birthDate());
}

void ApplicationWindow::find(const QString &s, bool windowNames, bool labels, bool folderNames,
                             bool caseSensitive, bool partialMatch, bool subfolders)
{
    if (windowNames || labels) {
        MyWidget *w =
                current_folder->findWindow(s, windowNames, labels, caseSensitive, partialMatch);
        if (w) {
            activateSubWindow(w);
            return;
        }

        if (subfolders) {
            FolderListItem *item = (FolderListItem *)folders.currentItem()->child(0);
            QTreeWidgetItemIterator it(item);
            while (item) {
                Folder *f = item->folder();
                MyWidget *w = f->findWindow(s, windowNames, labels, caseSensitive, partialMatch);
                if (w) {
                    folders.setCurrentItem(f->folderListItem());
                    activateSubWindow(w);
                    return;
                }
                it++;
                item = (FolderListItem *)(*it);
            }
        }
    }

    if (folderNames) {
        Folder *f = current_folder->findSubfolder(s, caseSensitive, partialMatch);
        if (f) {
            folders.setCurrentItem(f->folderListItem());
            return;
        }

        if (subfolders) {
            FolderListItem *item = (FolderListItem *)folders.currentItem()->child(0);
            QTreeWidgetItemIterator it(item);
            while (item) {
                Folder *f = item->folder()->findSubfolder(s, caseSensitive, partialMatch);
                if (f) {
                    folders.setCurrentItem(f->folderListItem());
                    return;
                }

                it++;
                item = (FolderListItem *)(*it);
            }
        }
    }

    QMessageBox::warning(this, tr("No match found"),
                         tr("Sorry, no match found for string: '%1'").arg(s));
}

void ApplicationWindow::dropFolderItems(QTreeWidgetItem *dest)
{
    if (!dest || draggedItems.isEmpty())
        return;

    Folder *dest_f = ((FolderListItem *)dest)->folder();

    QTreeWidgetItem *it;
    QStringList subfolders = dest_f->subfolders();

    foreach (it, draggedItems) {
        if (it->type() == FolderListItem::FolderType) {
            Folder *f = ((FolderListItem *)it)->folder();
            FolderListItem *src = f->folderListItem();
            if (dest_f == f) {
                QMessageBox::critical(this, "Error", tr("Cannot move an object to itself!"));
                return;
            }

            if (((FolderListItem *)dest)->isChildOf(src)) {
                QMessageBox::critical(this, "Error",
                                      tr("Cannot move a parent folder into a child folder!"));
                draggedItems.clear();
                folders.setCurrentItem(current_folder->folderListItem());
                return;
            }

            Folder *parent = (Folder *)f->parent();
            if (!parent)
                parent = projectFolder();
            if (dest_f == parent)
                return;

            if (subfolders.contains(f->name())) {
                QMessageBox::critical(this, tr("Makhber") + " - " + tr("Skipped moving folder"),
                                      tr("The destination folder already contains a folder called "
                                         "'%1'! Folder skipped!")
                                              .arg(f->name()));
            } else
                moveFolder(src, (FolderListItem *)dest);
        } else {
            if (dest_f == current_folder)
                return;

            MyWidget *w = ((WindowListItem *)it)->window();
            if (w) {
                current_folder->removeWindow(w);
                w->hide();
                dest_f->addWindow(w);
                delete it;
            }
        }
    }

    draggedItems.clear();
    current_folder = dest_f;
    folders.setCurrentItem(dest_f->folderListItem());
    changeFolder(dest_f, true);
    folders.setFocus();
    modifiedProject();
}

void ApplicationWindow::moveFolder(FolderListItem *src, FolderListItem *dest)
{
    folders.blockSignals(true);

    Folder *dest_f = dest->folder();
    Folder *src_f = src->folder();

    dest_f = &dest_f->addChild<Folder>(src_f->name());
    dest_f->setBirthDate(src_f->birthDate());
    dest_f->setModificationDate(src_f->modificationDate());

    FolderListItem *copy_item = new FolderListItem(dest, dest_f);
    copy_item->setText(0, src_f->name());
    dest_f->setFolderListItem(copy_item);

    QList<MyWidget *> lst = src_f->windowsList();
    foreach (MyWidget *w, lst) {
        src_f->removeWindow(w);
        w->hide();
        dest_f->addWindow(w);
    }

    if (!(src_f->children()).isEmpty()) {
        FolderListItem *item = (FolderListItem *)src->child(0);
        int initial_depth = item->depth();
        QTreeWidgetItemIterator it(item);
        while (item && item->depth() >= initial_depth) {
            src_f = (Folder *)item->folder();

            dest_f = &dest_f->addChild<Folder>(src_f->name());
            dest_f->setBirthDate(src_f->birthDate());
            dest_f->setModificationDate(src_f->modificationDate());

            copy_item = new FolderListItem(copy_item, dest_f);
            copy_item->setText(0, src_f->name());
            dest_f->setFolderListItem(copy_item);

            lst = QList<MyWidget *>(src_f->windowsList());
            foreach (MyWidget *w, lst) {
                src_f->removeWindow(w);
                w->hide();
                dest_f->addWindow(w);
            }

            it++;
            item = (FolderListItem *)(*it);
        }
    }

    src_f = src->folder();
    delete src_f;
    delete src;
    folders.blockSignals(false);
}

#ifdef SEARCH_FOR_UPDATES

void ApplicationWindow::searchForUpdates()
{
    int choice = QMessageBox::question(
            this, versionString(),
            tr("Makhber will now try to determine whether a new version of Makhber is available. "
               "Please modify your firewall settings in order to allow Makhber to connect to the "
               "internet.")
                    + "\n" + tr("Do you wish to continue?"),
            QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);

    if (choice == QMessageBox::Yes) {
        http.get(QNetworkRequest(QUrl("https://api.github.com/repos/Makhber/makhber/tags")));
    }
}

void ApplicationWindow::receivedVersionFile(QNetworkReply *netreply)
{
    if (netreply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(
                this, tr("HTTP get version file"),
                tr("Error while fetching version file with HTTP: %1.").arg(netreply->error()));
        return;
    }

    version_buffer = netreply->readAll();

    if (version_buffer.size() > 0) {
        QJsonDocument json = QJsonDocument::fromJson(version_buffer);
        QString available_versionString = json[0]["name"].toString();
        QStringList list = available_versionString.split(".");
        bool intok {};
        int available_version = (list.at(0).toInt() << 16) + (list.at(1).toInt() << 8)
                        + list.at(2).toInt(&intok);

        if (intok) {
            if (available_version > Makhber::version()) {
                if (QMessageBox::question(this, tr("Updates Available"),
                                          tr("There is a newer version of Makhber (%1) available "
                                             "for download. Would you like to download it now?")
                                                  .arg(available_versionString),
                                          QMessageBox::Yes | QMessageBox::Default,
                                          QMessageBox::No | QMessageBox::Escape)
                    == QMessageBox::Yes)
                    QDesktopServices::openUrl(QUrl(DOWNLOAD_URI));
            } else {
                QMessageBox::information(
                        this, versionString(),
                        tr("No updates available.\n"
                           "You are already running the latest version: \"%1\"")
                            .arg(available_versionString));
            }
        } else
            QMessageBox::information(this, tr("Invalid version file"),
                                     tr("The version file could not be decoded "
                                        "into a valid version number."));
        autoSearchUpdatesRequest = false;
    }
}

#endif // defined SEARCH_FOR_UPDATES

/*!
  Turns 3D animation on or off
  */
void ApplicationWindow::toggle3DAnimation(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D"))
        ((Graph3D *)d_workspace.activeSubWindow())->animate(on);
}

QString ApplicationWindow::generateUniqueName(const QString &name, bool increment)
{
    int index = 0;
    QList<MyWidget *> windows = windowsList();
    QStringList lst;

    for (int i = 0; i < windows.count(); i++) {
        MyWidget *widget = windows.at(i);
        if (!widget)
            continue;
        lst << widget->name();
        if (widget->name().startsWith(name))
            index++;
    }

    QString newName = name;
    if (increment) // force return of a different name
        newName += QString::number(++index);
    else {
        if (index > 0)
            newName += QString::number(index);
    }

    while (lst.contains(newName))
        newName = name + QString::number(++index);
    return newName;
}

void ApplicationWindow::clearTable()
{
    Table *t = (Table *)d_workspace.activeSubWindow();
    if (!t || !t->inherits("Table"))
        return;

    if (QMessageBox::question(this, tr("Warning"),
                              tr("This will clear the contents of all the data associated with the "
                                 "table. Are you sure?"),
                              tr("&Yes"), tr("&No"), QString(), 0, 1))
        return;
    else
        t->clear();
}

/*!
  Turns perspective mode on or off
  */
void ApplicationWindow::togglePerspective(bool on)
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->setOrtho(!on);
    }
}

/*!
  Resets rotation of 3D plots to default values
  */
void ApplicationWindow::resetRotation()
{
    if (d_workspace.activeSubWindow() && d_workspace.activeSubWindow()->inherits("Graph3D")) {
        ((Graph3D *)d_workspace.activeSubWindow())->setRotation(30, 0, 15);
    }
}

/*!
  Finds best layout for the 3D plot
  */
void ApplicationWindow::fitFrameToLayer()
{
    if (!d_workspace.activeSubWindow() || !d_workspace.activeSubWindow()->inherits("Graph3D"))
        return;

    ((Graph3D *)d_workspace.activeSubWindow())->findBestLayout();
}

ApplicationWindow::~ApplicationWindow()
{
    if (lastCopiedLayer)
        delete lastCopiedLayer;

    QApplication::clipboard()->clear(QClipboard::Clipboard);
}

QString ApplicationWindow::versionString()
{
    return Makhber::versionString();
}

unsigned int ApplicationWindow::convertOldToNewColorIndex(unsigned int cindex)
{
    if ((cindex == 13) || (cindex == 14)) // white and light gray
        return cindex + 4;

    if (cindex == 15) // dark gray
        return cindex + 8;

    return cindex;
}

void ApplicationWindow::cascade()
{
    QList<QMdiSubWindow *> windows = d_workspace.subWindowList(QMdiArea::StackingOrder);

    const int xoffset = 13;
    const int yoffset = 20;

    int x = 0;
    int y = 0;

    foreach (QWidget *w, windows) {
        w->activateWindow();
        w->showNormal();
        ((MyWidget *)w)->setStatus(MyWidget::Normal);
        updateWindowStatus((MyWidget *)w);

        w->setGeometry(x, y, w->width(), w->height());
        w->raise();
        x += xoffset;
        y += yoffset;
    }
    modifiedProject();
}

ApplicationWindow *ApplicationWindow::loadScript(const QString &fn, const QStringList &args,
                                                 bool execute)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ApplicationWindow *app = new ApplicationWindow();
    app->applyUserSettings();
    bool isPython = fn.endsWith(".py", Qt::CaseInsensitive);
    app->m_batch = true;
    if (isPython)
        app->setScriptingLangForBatch("Python");
    else
        app->setScriptingLangForBatch("muParser");
    app->showMaximized();
    Note *script_note = app->newNote(fn);
    if (isPython) {
        // copy any arguments into the sys.argv array
        script_note->insert("import sys\n");
        QString prologue = "sys.argv=['" + fn + "'";
        for (auto &a : args)
            prologue += ",'" + a + "'";
        prologue += "]\n";
        script_note->insert(prologue);
    }
    script_note->importASCII(fn);
    QApplication::restoreOverrideCursor();
    if (execute) {
        // we need to disable the redirect of stdio, as this is batch processing
        Script *script = nullptr;
        if (auto scriptEdit = script_note->findChild<ScriptEdit *>())
            if ((script = scriptEdit->findChild<Script *>()))
                script->batchMode = true;
        if (!script_note->executeAll())
            exit(1);
        if (script)
            script->batchMode = false;
    }
    app->m_batch = false;
    return app;
}

QMenu *ApplicationWindow::createToolbarsMenu()
{
    QMenu *menu = 0;
    QList<QToolBar *> toolbars = this->findChildren<QToolBar *>();
    if (toolbars.size()) {
        menu = new QMenu(this);
        foreach (QToolBar *toolbar, toolbars) {
            if (toolbar->parentWidget() == this)
                menu->addAction(toolbar->toggleViewAction());
        }
    }
    return menu;
}

void ApplicationWindow::setStatusBarText(const QString &text)
{
    d_status_info->setText(text);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void ApplicationWindow::copyStatusBarText()
{
    QApplication::clipboard()->setText(d_status_info->text());
}

void ApplicationWindow::showStatusBarContextMenu(const QPoint &pos)
{
    QMenu cm(this);
    cm.addAction(actionCopyStatusBarText);
    cm.exec(d_status_info->mapToGlobal(pos));
}

QMenu *ApplicationWindow::showWindowMenuImpl(MyWidget *widget)
{
    d_workspace.setActiveSubWindow(widget); // FIXME not user-friendly, but can't be simply removed

    QMenu *cm = new QMenu(this);
    QMenu *depend_menu = new QMenu(this);

    if (widget->inherits("Table"))
        cm->addAction(actionShowExportASCIIDialog);
    else if (widget->inherits("Note"))
        cm->addAction(actionSaveNote);
    else
        cm->addAction(actionSaveTemplate);
    cm->addAction(actionPrint);
    cm->addAction(actionCopyWindow);
    cm->addSeparator();
    cm->addAction(actionRename);
    cm->addAction(actionCloseWindow);
    if (!hidden(widget))
        cm->addAction(actionHideActiveWindow);
    cm->addAction(actionActivateWindow);
    cm->addAction(actionMinimizeWindow);
    cm->addAction(actionMaximizeWindow);
    cm->addAction(actionResizeWindow);
    cm->addSeparator();
    cm->addAction(tr("&Properties..."), this, SLOT(windowProperties()));

    int n;
    if (widget->inherits("Table")) {
        QStringList graphs = dependingPlots(widget->name());
        n = graphs.count();
        if (n > 0) {
            cm->addSeparator();
            for (int i = 0; i < n; i++)
                depend_menu->addAction(graphs[i], this, SLOT(setActiveWindowFromAction()));

            depend_menu->setTitle(tr("D&epending Graphs"));
            cm->addMenu(depend_menu);
        }
    } else if (widget->inherits("Matrix")) {
        QStringList graphs = depending3DPlots((Matrix *)widget);
        n = graphs.count();
        if (n > 0) {
            cm->addSeparator();
            for (int i = 0; i < n; i++)
                depend_menu->addAction(graphs[i], this, SLOT(setActiveWindowFromAction()));

            depend_menu->setTitle(tr("D&epending 3D Graphs"));
            cm->addMenu(depend_menu);
        }
    } else if (widget->inherits("MultiLayer")) {
        QStringList tbls = multilayerDependencies(widget);
        n = tbls.count();
        if (n > 0) {
            cm->addSeparator();
            for (int i = 0; i < n; i++)
                depend_menu->addAction(tbls[i], this, SLOT(setActiveWindowFromAction()));

            depend_menu->setTitle(tr("D&epends on"));
            cm->addMenu(depend_menu);
        }
    } else if (widget->inherits("Graph3D")) {
        Graph3D *sp = (Graph3D *)widget;
        Matrix *m = sp->matrix();
        QString formula = sp->formula();
        if (!formula.isEmpty()) {
            cm->addSeparator();
            if (formula.contains("_")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                QStringList tl = formula.split("_", Qt::SkipEmptyParts);
#else
                QStringList tl = formula.split("_", QString::SkipEmptyParts);
#endif

                depend_menu->addAction(tl[0], this, SLOT(setActiveWindowFromAction()));

                depend_menu->setTitle(tr("D&epends on"));
                cm->addMenu(depend_menu);
            } else if (m) {
                depend_menu->addAction(m->name(), this, SLOT(setActiveWindowFromAction()));
                depend_menu->setTitle(tr("D&epends on"));
                cm->addMenu(depend_menu);
            }
        }
    }

    return cm;
}

void ApplicationWindow::setActiveWindowFromAction()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        activateSubWindow(qobject_cast<MyWidget *>(window(action->text())));
}

bool ApplicationWindow::validFor3DPlot(Table *table)
{
    if (table->numCols() < 2) {
        QMessageBox::critical(0, tr("Error"),
                              tr("You need at least two columns for this operation!"));
        return false;
    }
    if (table->firstSelectedColumn() < 0
        || table->colPlotDesignation(table->firstSelectedColumn()) != Makhber::Z) {
        QMessageBox::critical(0, tr("Error"), tr("Please select a Z column for this operation!"));
        return false;
    }
    if (table->noXColumn()) {
        QMessageBox::critical(0, tr("Error"), tr("You need to define a X column first!"));
        return false;
    }
    if (table->noYColumn()) {
        QMessageBox::critical(0, tr("Error"), tr("You need to define a Y column first!"));
        return false;
    }
    return true;
}

bool ApplicationWindow::validFor2DPlot(Table *table, int type)
{
    switch (type) {
    case Graph::Histogram:
    case Graph::Pie:
    case Graph::Box:
        if (table->selectedColumnCount() < 1) {
            QMessageBox::warning(this, tr("Error"), tr("Please select a column to plot!"));
            return false;
        }
        break;
    default:
        if (table->selectedColumnCount(Makhber::Y) < 1) {
            QMessageBox::warning(this, tr("Error"), tr("Please select a Y column to plot!"));
            return false;
        } else if (table->numCols() < 2) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You need at least two columns for this operation!"));
            return false;
        } else if (table->noXColumn()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Please set a default X column for this table, first!"));
            return false;
        }
        break;
    }

    return true;
}

void ApplicationWindow::selectPlotType(int type)
{
    if (!d_workspace.activeSubWindow())
        return;

    Table *table = qobject_cast<Table *>(d_workspace.activeSubWindow());
    if (table && validFor2DPlot(table, type)) {
        switch (type) {
        case Graph::Histogram:
        case Graph::Pie:
        case Graph::Box:
            multilayerPlot(table, table->selectedColumns(), (Graph::CurveType)type,
                           table->firstSelectedRow(), table->lastSelectedRow());
            break;
        default:
            multilayerPlot(table, table->drawableColumnSelection(), (Graph::CurveType)type,
                           table->firstSelectedRow(), table->lastSelectedRow());
            break;
        }
        return;
    }

    MultiLayer *ml = qobject_cast<MultiLayer *>(d_workspace.activeSubWindow());
    if (ml) {
        Graph *g = ml->activeGraph();
        if (g->curves() > 0)
            g->setCurveType(g->curves() - 1, (Graph::CurveType)type);
    }
}

void ApplicationWindow::handleAspectAdded(const AbstractAspect *parent, int index)
{
    AbstractAspect *aspect = parent->child(index);
    ::future::Matrix *matrix = qobject_cast<::future::Matrix *>(aspect);
    if (matrix) {
        // initMatrix(static_cast<Matrix *>(matrix->view()));
        return;
    }
    ::future::Table *table = qobject_cast<::future::Table *>(aspect);
    if (table) {
        // initTable(static_cast<Table *>(table->view()));
        return;
    }
}

void ApplicationWindow::handleAspectAboutToBeRemoved(const AbstractAspect *parent, int index)
{
    AbstractAspect *aspect = parent->child(index);
    ::future::Matrix *matrix = qobject_cast<::future::Matrix *>(aspect);
    if (matrix) {
        closeWindow(static_cast<Matrix *>(matrix->view()));
        return;
    }
    ::future::Table *table = qobject_cast<::future::Table *>(aspect);
    if (table) {
        closeWindow(static_cast<Table *>(table->view()));
        return;
    }
}

void ApplicationWindow::showHistory()
{
    if (!d_project->undoStack())
        return;
    QDialog dialog;
    QVBoxLayout layout(&dialog);

    QDialogButtonBox button_box;
    button_box.setOrientation(Qt::Horizontal);
    button_box.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::NoButton
                                  | QDialogButtonBox::Ok);
    QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

    int index = d_project->undoStack()->index();
    QUndoView undo_view(d_project->undoStack());

    layout.addWidget(&undo_view);
    layout.addWidget(&button_box);

    dialog.setWindowTitle(tr("Undo/Redo History"));
    if (dialog.exec() == QDialog::Accepted)
        return;

    d_project->undoStack()->setIndex(index);
}

QStringList ApplicationWindow::tableWindows()
{
    QList<AbstractAspect *> tables = d_project->descendantsThatInherit("future::Table");
    QStringList result;
    foreach (AbstractAspect *aspect, tables)
        result.append(aspect->name());
    return result;
}

QSettings &ApplicationWindow::getSettings()
{
#ifdef Q_OS_MAC // Mac
    static QSettings d_settings(QSettings::IniFormat, QSettings::UserScope, "Makhber", "Makhber");
#else
    static QSettings d_settings(QSettings::NativeFormat, QSettings::UserScope, "Makhber",
                                "Makhber");
#endif
    return d_settings;
}

// initialize singleton
static auto &MakhberSettingsSingleton = ApplicationWindow::getSettings();
