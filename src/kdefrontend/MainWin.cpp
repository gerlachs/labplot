/***************************************************************************
    File                 : MainWin.cc
    Project              : LabPlot
    Description          : Main window of the application
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "MainWin.h"

#include "backend/core/Project.h"
#include "backend/core/Folder.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Workbook.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datasources/FileDataSource.h"
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#include "backend/datapicker/Datapicker.h"
#include "backend/note/Note.h"
#include "backend/lib/macros.h"

#include "commonfrontend/ProjectExplorer.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#ifdef HAVE_CANTOR_LIBS
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#endif
#include "commonfrontend/datapicker/DatapickerView.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "commonfrontend/note/NoteView.h"

#include "kdefrontend/datasources/ImportFileDialog.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/HistoryDialog.h"
#include "kdefrontend/SettingsDialog.h"
#include "kdefrontend/GuiObserver.h"
#include "kdefrontend/widgets/FITSHeaderEditDialog.h"

#include <QMdiArea>
#include <QMenu>
#include <QDockWidget>
#include <QStackedWidget>
#include <QUndoStack>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QElapsedTimer>
#include <QHash>
#include <QDebug>	// qWarning()

#include <KActionCollection>
#include <KFileDialog>
#include <KStandardAction>
#include <kxmlguifactory.h>
#include <KMessageBox>
#include <KToolBar>
#include <KStatusBar>
#include <KLocalizedString>
#include <KFilterDev>

#ifdef HAVE_CANTOR_LIBS
#include <cantor/backend.h>
#endif

/*!
\class MainWin
\brief Main application window.

\ingroup kdefrontend
*/

MainWin::MainWin(QWidget *parent, const QString& filename)
	: KXmlGuiWindow(parent),
	m_currentSubWindow(0),
	m_project(0),
	m_aspectTreeModel(0),
	m_projectExplorer(0),
	m_projectExplorerDock(0),
	m_propertiesDock(0),
	m_currentAspect(0),
	m_currentFolder(0),
	m_suppressCurrentSubWindowChangedEvent(false),
	m_closing(false),
	m_autoSaveActive(false),
	m_visibilityMenu(0),
	m_newMenu(0),
	m_editMenu(0),
	axisDock(0),
	notesDock(0),
	cartesianPlotDock(0),
	cartesianPlotLegendDock(0),
	columnDock(0),
	matrixDock(0),
	spreadsheetDock(0),
	projectDock(0),
	xyCurveDock(0),
	xyEquationCurveDock(0),
	xyDataReductionCurveDock(0),
	xyDifferentiationCurveDock(0),
	xyIntegrationCurveDock(0),
	xyInterpolationCurveDock(0),
	xySmoothCurveDock(0),
	xyFitCurveDock(0),
	xyFourierFilterCurveDock(0),
	xyFourierTransformCurveDock(0),
	worksheetDock(0),
	textLabelDock(0),
	customPointDock(0),
	datapickerImageDock(0),
	datapickerCurveDock(0),
#ifdef HAVE_CANTOR_LIBS
	cantorWorksheetDock(0),
#endif
	m_guiObserver(0) {

// 	QTimer::singleShot( 0, this, SLOT(initGUI(filename)) );  //TODO doesn't work anymore
	initGUI(filename);
	setAcceptDrops(true);
}

MainWin::~MainWin() {
	//write settings
	m_recentProjectsAction->saveEntries( KSharedConfig::openConfig()->group("Recent Files") );
// 	qDebug()<<"SAVED m_recentProjectsAction->urls()="<<m_recentProjectsAction->urls()<<endl;
	//etc...

	KSharedConfig::openConfig()->sync();

	if (m_project != 0) {
		m_mdiArea->closeAllSubWindows();
		disconnect(m_project, 0, this, 0);
		delete m_project;
	}

	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	if (m_guiObserver)
		delete m_guiObserver;
}

AspectTreeModel* MainWin::model() const {
	return m_aspectTreeModel;
}

void MainWin::initGUI(const QString& fileName) {
	m_mdiArea = new QMdiArea;
	setCentralWidget(m_mdiArea);
	connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
		this, SLOT(handleCurrentSubWindowChanged(QMdiSubWindow*)));
#ifdef _WIN32
	QIcon::setThemeName("hicolor");
#endif

	statusBar()->showMessage(i18nc("%1 is the LabPlot version", "Welcome to LabPlot %1", QLatin1String(LVERSION)));
	initActions();
#ifdef __APPLE__
	setupGUI(Default, QLatin1String("/Applications/labplot2.app/Contents/share/labplot2ui.rc"));
#else
	setupGUI(Default, QLatin1String("labplot2ui.rc"));
#endif
	initMenus();
	setWindowIcon(QIcon::fromTheme("LabPlot2"));
	setAttribute( Qt::WA_DeleteOnClose );

	//make the status bar of a fixed size in order to avoid height changes when placing a ProgressBar there.
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	statusBar()->setFixedHeight(fm.height()+5);

	//load recently used projects
	m_recentProjectsAction->loadEntries( KSharedConfig::openConfig()->group("Recent Files") );

	//set the view mode of the mdi area
	KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );
	int viewMode = group.readEntry("ViewMode", 0);
	if (viewMode == 1) {
		m_mdiArea->setViewMode(QMdiArea::TabbedView);
		int tabPosition = group.readEntry("TabPosition", 0);
		m_mdiArea->setTabPosition(QTabWidget::TabPosition(tabPosition));
		m_mdiArea->setTabsClosable(true);
		m_mdiArea->setTabsMovable(true);
		m_tileWindows->setVisible(false);
		m_cascadeWindows->setVisible(false);
	}

	//auto-save
	m_autoSaveActive = group.readEntry<bool>("AutoSave", 0);
	int interval = group.readEntry("AutoSaveInterval", 1);
	interval = interval*60*1000;
	m_autoSaveTimer.setInterval(interval);
	connect(&m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSaveProject()));

	if (!fileName.isEmpty())
		openProject(fileName);
	else {
		//There is no file to open. Depending on the settings do nothing,
		//create a new project or open the last used project.
		int load = group.readEntry("LoadOnStart", 0);
		if (load == 1)   //create new project
			newProject();
		else if (load == 2) { //create new project with a worksheet
			newProject();
			newWorksheet();
		} else if (load == 3) { //open last used project
			if (!m_recentProjectsAction->urls().isEmpty()) {
				QDEBUG("TO OPEN m_recentProjectsAction->urls() =" << m_recentProjectsAction->urls().first());
				openRecentProject( m_recentProjectsAction->urls().first() );
			}
		}
	}
	updateGUIOnProjectChanges();
}

void MainWin::initActions() {
	// ******************** File-menu *******************************
	//add some standard actions
	KStandardAction::openNew(this, SLOT(newProject()),actionCollection());
	KStandardAction::open(this, SLOT(openProject()),actionCollection());
	m_recentProjectsAction = KStandardAction::openRecent(this, SLOT(openRecentProject(QUrl)),actionCollection());
	m_closeAction = KStandardAction::close(this, SLOT(closeProject()),actionCollection());
	m_saveAction = KStandardAction::save(this, SLOT(saveProject()),actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, SLOT(saveProjectAs()),actionCollection());
	m_printAction = KStandardAction::print(this, SLOT(print()),actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()),actionCollection());
	KStandardAction::fullScreen(this, SLOT(toggleFullScreen()), this, actionCollection());
	//New Folder/Workbook/Spreadsheet/Matrix/Worksheet/Datasources
	m_newWorkbookAction = new QAction(QIcon::fromTheme("labplot-workbook-new"),i18n("Workbook"),this);
	actionCollection()->addAction("new_workbook", m_newWorkbookAction);
	connect(m_newWorkbookAction, SIGNAL(triggered()), SLOT(newWorkbook()));

	m_newDatapickerAction = new QAction(QIcon::fromTheme("color-picker-black"), i18n("Datapicker"), this);
	actionCollection()->addAction("new_datapicker", m_newDatapickerAction);
	connect(m_newDatapickerAction, SIGNAL(triggered()), SLOT(newDatapicker()));

	m_newSpreadsheetAction = new QAction(QIcon::fromTheme("labplot-spreadsheet-new"),i18n("Spreadsheet"),this);

// 	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_spreadsheet", m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, SIGNAL(triggered()), SLOT(newSpreadsheet()));

	m_newMatrixAction = new QAction(QIcon::fromTheme("labplot-matrix-new"),i18n("Matrix"),this);
// 	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_matrix", m_newMatrixAction);
	connect(m_newMatrixAction, SIGNAL(triggered()), SLOT(newMatrix()));

	m_newWorksheetAction= new QAction(QIcon::fromTheme("labplot-worksheet-new"),i18n("Worksheet"),this);
// 	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	actionCollection()->addAction("new_worksheet", m_newWorksheetAction);
	connect(m_newWorksheetAction, SIGNAL(triggered()), SLOT(newWorksheet()));

	m_newNotesAction= new QAction(QIcon::fromTheme("document-new"),i18n("Note"),this);
	actionCollection()->addAction("new_notes", m_newNotesAction);
	connect(m_newNotesAction, SIGNAL(triggered()), SLOT(newNotes()));

// 	m_newScriptAction = new QAction(QIcon::fromTheme("insert-text"),i18n("Note/Script"),this);
// 	actionCollection()->addAction("new_script", m_newScriptAction);
// 	connect(m_newScriptAction, SIGNAL(triggered()),SLOT(newScript()));

	m_newFolderAction = new QAction(QIcon::fromTheme("folder-new"),i18n("Folder"),this);
	actionCollection()->addAction("new_folder", m_newFolderAction);
	connect(m_newFolderAction, SIGNAL(triggered()), SLOT(newFolder()));

	//"New file datasources"
	m_newFileDataSourceAction = new QAction(QIcon::fromTheme("application-octet-stream"),i18n("File Data Source"),this);
	actionCollection()->addAction("new_file_datasource", m_newFileDataSourceAction);
	connect(m_newFileDataSourceAction, SIGNAL(triggered()), this, SLOT(newFileDataSourceActionTriggered()));

	//"New database datasources"
// 	m_newSqlDataSourceAction = new QAction(QIcon::fromTheme("server-database"),i18n("SQL Data Source "),this);
// 	actionCollection()->addAction("new_database_datasource", m_newSqlDataSourceAction);
// 	connect(m_newSqlDataSourceAction, SIGNAL(triggered()), this, SLOT(newSqlDataSourceActionTriggered()));

	m_importAction = new QAction(QIcon::fromTheme("document-import"), i18n("Import"), this);
	actionCollection()->setDefaultShortcut(m_importAction, Qt::CTRL+Qt::SHIFT+Qt::Key_I);
	actionCollection()->addAction("import", m_importAction);
	connect(m_importAction, SIGNAL(triggered()), SLOT(importFileDialog()));

	m_exportAction = new QAction(QIcon::fromTheme("document-export"), i18n("Export"), this);
	actionCollection()->setDefaultShortcut(m_exportAction, Qt::CTRL+Qt::SHIFT+Qt::Key_E);
	actionCollection()->addAction("export", m_exportAction);
	connect(m_exportAction, SIGNAL(triggered()), SLOT(exportDialog()));

	m_editFitsFileAction = new QAction(i18n("FITS Metadata Editor"), this);
	actionCollection()->addAction("edit_fits", m_editFitsFileAction);
	connect(m_editFitsFileAction, SIGNAL(triggered()), SLOT(editFitsFileDialog()));

	// Edit
	//Undo/Redo-stuff
	m_undoAction = KStandardAction::undo(this, SLOT(undo()), actionCollection());
	m_redoAction = KStandardAction::redo(this, SLOT(redo()), actionCollection());

	m_historyAction = new QAction(QIcon::fromTheme("view-history"), i18n("Undo/Redo History"),this);
	actionCollection()->addAction("history", m_historyAction);
	connect(m_historyAction, SIGNAL(triggered()), SLOT(historyDialog()));

	// TODO: more menus
	//  Appearance
	// Analysis: see WorksheetView.cpp
	// Drawing
	// Script

	//Windows
	QAction* action  = new QAction(i18n("&Close"), this);
	actionCollection()->setDefaultShortcut(action, Qt::CTRL+Qt::Key_C);
	action->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction("close window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(closeActiveSubWindow()));

	action = new QAction(i18n("Close &All"), this);
	action->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction("close all windows", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(closeAllSubWindows()));

	m_tileWindows = new QAction(i18n("&Tile"), this);
	m_tileWindows->setStatusTip(i18n("Tile the windows"));
	actionCollection()->addAction("tile windows", m_tileWindows);
	connect(m_tileWindows, SIGNAL(triggered()), m_mdiArea, SLOT(tileSubWindows()));

	m_cascadeWindows = new QAction(i18n("&Cascade"), this);
	m_cascadeWindows->setStatusTip(i18n("Cascade the windows"));
	actionCollection()->addAction("cascade windows", m_cascadeWindows);
	connect(m_cascadeWindows, SIGNAL(triggered()), m_mdiArea, SLOT(cascadeSubWindows()));
	action = new QAction(QIcon::fromTheme("go-next-view"), i18n("Ne&xt"), this);
	action->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction("next window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(activateNextSubWindow()));

	action = new QAction(QIcon::fromTheme("go-previous-view"), i18n("Pre&vious"), this);
	action->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction("previous window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(activatePreviousSubWindow()));

	//"Standard actions"
	KStandardAction::preferences(this, SLOT(settingsDialog()), actionCollection());
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	//Actions for window visibility
	QActionGroup* windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);

	m_visibilityFolderAction = new QAction(QIcon::fromTheme("folder"), i18n("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setData(Project::folderOnly);

	m_visibilitySubfolderAction = new QAction(QIcon::fromTheme("folder-documents"), i18n("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(Project::folderAndSubfolders);

	m_visibilityAllAction = new QAction(i18n("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(Project::allMdiWindows);

	connect(windowVisibilityActions, SIGNAL(triggered(QAction*)), this, SLOT(setMdiWindowVisibility(QAction*)));

	//Actions for hiding/showing the dock widgets
	QActionGroup * docksActions = new QActionGroup(this);
	docksActions->setExclusive(false);

	m_toggleProjectExplorerDockAction = new QAction(QIcon::fromTheme("view-list-tree"), i18n("Project explorer"), docksActions);
	m_toggleProjectExplorerDockAction->setCheckable(true);
	m_toggleProjectExplorerDockAction->setChecked(true);
	actionCollection()->addAction("toggle_project_explorer_dock", m_toggleProjectExplorerDockAction);

	m_togglePropertiesDockAction = new QAction(QIcon::fromTheme("view-list-details"), i18n("Properties explorer"), docksActions);
	m_togglePropertiesDockAction->setCheckable(true);
	m_togglePropertiesDockAction->setChecked(true);
	actionCollection()->addAction("toggle_properties_explorer_dock", m_togglePropertiesDockAction);

	connect(docksActions, SIGNAL(triggered(QAction*)), this, SLOT(toggleDockWidget(QAction*)));
}

void MainWin::initMenus() {
	//menu for adding new aspects
	m_newMenu = new QMenu(i18n("Add new"), this);
	m_newMenu->setIcon(QIcon::fromTheme("document-new"));
	m_newMenu->addAction(m_newFolderAction);
	m_newMenu->addAction(m_newWorkbookAction);
	m_newMenu->addAction(m_newSpreadsheetAction);
	m_newMenu->addAction(m_newMatrixAction);
	m_newMenu->addAction(m_newWorksheetAction);
	m_newMenu->addAction(m_newNotesAction);
	m_newMenu->addAction(m_newDatapickerAction);
	m_newMenu->addSeparator();
	m_newMenu->addAction(m_newFileDataSourceAction);

#ifdef HAVE_CANTOR_LIBS
	m_newMenu->addSeparator();
	m_newCantorWorksheetMenu = new QMenu(i18n("CAS Worksheet"));
	m_newCantorWorksheetMenu->setIcon(QIcon::fromTheme("archive-insert"));

	//"Adding Cantor backends to menue and context menu"
	QStringList m_availableBackend = Cantor::Backend::listAvailableBackends();
	if(m_availableBackend.count() > 0) {
		unplugActionList(QLatin1String("backends_list"));
		QList<QAction*> newBackendActions;
		foreach (Cantor::Backend* backend, Cantor::Backend::availableBackends()) {
			if (!backend->isEnabled()) continue;
			QAction* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(),this);
			action->setData(backend->name());
			newBackendActions << action;
			m_newCantorWorksheetMenu->addAction(action);
		}

		connect(m_newCantorWorksheetMenu, SIGNAL(triggered(QAction*)), this, SLOT(newCantorWorksheet(QAction*)));
		plugActionList(QLatin1String("backends_list"), newBackendActions);
	}
	m_newMenu->addMenu(m_newCantorWorksheetMenu);
#else
	delete this->guiFactory()->container("cas_worksheet", this);
	delete this->guiFactory()->container("new_casWorksheet", this);
	delete this->guiFactory()->container("cas_worksheet_toolbar", this);
#endif

// 	m_newMenu->addAction(m_newSqlDataSourceAction);

	//menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window visibility policy"), this);
	m_visibilityMenu->setIcon(QIcon::fromTheme("window-duplicate"));
	m_visibilityMenu ->addAction(m_visibilityFolderAction);
	m_visibilityMenu ->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu ->addAction(m_visibilityAllAction);

	//menu for editing files
	m_editMenu = new QMenu(i18n("Edit"), this);
	m_editMenu->addAction(m_editFitsFileAction);
}

/*!
	Asks to save the project if it was modified.
	\return \c true if the project still needs to be saved ("cancel" clicked), \c false otherwise.
 */
bool MainWin::warnModified() {
	if (m_project->hasChanged()) {
		int want_save = KMessageBox::warningYesNoCancel( this,
		                i18n("The current project %1 has been modified. Do you want to save it?", m_project->name()),
		                i18n("Save Project"));
		switch (want_save) {
		case KMessageBox::Yes:
			return !saveProject();
		case KMessageBox::No:
			break;
		case KMessageBox::Cancel:
			return true;
		}
	}

	return false;
}

/*!
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * on project changes (project closes and opens)
 */
void MainWin::updateGUIOnProjectChanges() {
	if (m_closing)
		return;

	KXMLGUIFactory* factory = this->guiFactory();
	if (factory->container("worksheet", this) == NULL) {
		//no worksheet menu found, most probably labplot2ui.rc
		//was not properly installed -> return here in order not to crash
		return;
	}

	//disable all menus if there is no project
	bool b = (m_project == 0);
	m_saveAction->setEnabled(!b);
	m_saveAsAction->setEnabled(!b);
	m_printAction->setEnabled(!b);
	m_printPreviewAction->setEnabled(!b);
	m_importAction->setEnabled(!b);
	m_exportAction->setEnabled(!b);
	m_newWorkbookAction->setEnabled(!b);
	m_newSpreadsheetAction->setEnabled(!b);
	m_newMatrixAction->setEnabled(!b);
	m_newWorksheetAction->setEnabled(!b);
	m_newDatapickerAction->setEnabled(!b);
	m_closeAction->setEnabled(!b);
	m_toggleProjectExplorerDockAction->setEnabled(!b);
	m_togglePropertiesDockAction->setEnabled(!b);

	if (!m_mdiArea->currentSubWindow()) {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("matrix", this)->setEnabled(false);
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->hide();
		factory->container("worksheet_toolbar", this)->hide();
		factory->container("cartesian_plot_toolbar", this)->hide();
		factory->container("datapicker_toolbar", this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->hide();
#endif
	}

	factory->container("new", this)->setEnabled(!b);
	factory->container("edit", this)->setEnabled(!b);

	if (b)
		setCaption("LabPlot2");
	else
		setCaption(m_project->name());

	// undo/redo actions are disabled in both cases - when the project is closed or opened
	m_undoAction->setEnabled(false);
	m_redoAction->setEnabled(false);
}

/*
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * depending on the currently active window (worksheet or spreadsheet).
 */
void MainWin::updateGUI() {
	if (m_closing)
		return;

	KXMLGUIFactory* factory = this->guiFactory();
	if (factory->container("worksheet", this) == NULL) {
		//no worksheet menu found, most probably labplot2ui.rc
		//was not properly installed -> return here in order not to crash
		return;
	}

	if (!m_mdiArea->currentSubWindow()) {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("matrix", this)->setEnabled(false);
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->hide();
		factory->container("worksheet_toolbar", this)->hide();
		factory->container("cartesian_plot_toolbar", this)->hide();
		factory->container("datapicker_toolbar", this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->hide();
#endif
		return;
	}

	//for the toolbar worksheet_toolbar, spreadsheet_toolbar and cartesian_plot_toolbar
	//the default desktop style is ignored and the buttons are shown with icons and texts.
	//We need to set the style explicitly when the toolbar is shown for the first time
	//(no subgroup in the group "MainWindow" available)
	//TODO: is this the usual behaviour for toolbars defined in the rc-file?
	KConfigGroup group = KSharedConfig::openConfig()->group("MainWindow");

	//Handle the Worksheet-object
	Worksheet* w = this->activeWorksheet();
	if (w != 0) {
		//enable worksheet related menus
		factory->container("worksheet", this)->setEnabled(true);
		factory->container("analysis", this)->setEnabled(true);
//TODO 		factory->container("drawing", this)->setEnabled(true);

		//disable spreadsheet and matrix related menus
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("matrix", this)->setEnabled(false);

		//populate worksheet menu
		WorksheetView* view=qobject_cast<WorksheetView*>(w->view());
		QMenu* menu = qobject_cast<QMenu*>(factory->container("worksheet", this));
		menu->clear();
		view->createContextMenu(menu);

		//populate analysis menu
		menu = qobject_cast<QMenu*>(factory->container("analysis", this));
		menu->clear();
		view->createAnalysisMenu(menu);

		//populate worksheet-toolbar
		QToolBar* toolbar = qobject_cast<QToolBar*>(factory->container("worksheet_toolbar", this));
		if (group.groupList().indexOf("Toolbar worksheet_toolbar") == -1)
			toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);

		//populate the toolbar for cartesian plots
		toolbar=qobject_cast<QToolBar*>(factory->container("cartesian_plot_toolbar", this));
		if (group.groupList().indexOf("Toolbar cartesian_plot_toolbar") == -1)
			toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

		toolbar->clear();
		view->fillCartesianPlotToolBar(toolbar);
		toolbar->setVisible(true);

		//hide the spreadsheet toolbar
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
	} else {
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
//		factory->container("drawing", this)->setEnabled(false);
		factory->container("worksheet_toolbar", this)->setVisible(false);
		factory->container("cartesian_plot_toolbar", this)->setVisible(false);
	}

	//Handle the Spreadsheet-object
	const  Spreadsheet* spreadsheet = this->activeSpreadsheet();
	if (spreadsheet) {
		//enable spreadsheet related menus
		factory->container("spreadsheet", this)->setEnabled(true);

		//populate spreadsheet-menu
		SpreadsheetView* view = qobject_cast<SpreadsheetView*>(spreadsheet->view());
		QMenu* menu = qobject_cast<QMenu*>(factory->container("spreadsheet", this));
		menu->clear();
		view->createContextMenu(menu);

		//populate spreadsheet-toolbar
		QToolBar* toolbar = qobject_cast<QToolBar*>(factory->container("spreadsheet_toolbar", this));
		if (group.groupList().indexOf("Toolbar spreadsheet_toolbar") == -1)
			toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
	}

	//Handle the Matrix-object
	const  Matrix* matrix = this->activeMatrix();
	if (matrix) {
		factory->container("matrix", this)->setEnabled(true);

		//populate matrix-menu
		MatrixView* view = qobject_cast<MatrixView*>(matrix->view());
		QMenu* menu = qobject_cast<QMenu*>(factory->container("matrix", this));
		menu->clear();
		view->createContextMenu(menu);
	} else
		factory->container("matrix", this)->setEnabled(false);

#ifdef HAVE_CANTOR_LIBS
	CantorWorksheet* cantorworksheet = this->activeCantorWorksheet();
	if(cantorworksheet) {
		// enable Cantor Worksheet related menues
		factory->container("cas_worksheet", this)->setEnabled(true);
		CantorWorksheetView* view=qobject_cast<CantorWorksheetView*>(cantorworksheet->view());
		QMenu* menu=qobject_cast<QMenu*>(factory->container("cas_worksheet", this));
		menu->clear();
		view->createContextMenu(menu);
		QToolBar* toolbar=qobject_cast<QToolBar*>(factory->container("cas_worksheet_toolbar", this));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		//no cantor worksheet selected -> deactivate cantor worksheet related menu and toolbar
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->setVisible(false);
	}
#endif

	const Datapicker* datapicker = this->activeDatapicker();
	if (datapicker) {
		factory->container("datapicker", this)->setEnabled(true);
		//populate datapicker-menu
		DatapickerView* view = qobject_cast<DatapickerView*>(datapicker->view());
		QMenu* menu = qobject_cast<QMenu*>(factory->container("datapicker", this));
		menu->clear();
		view->createContextMenu(menu);

		//populate spreadsheet-toolbar
		QToolBar* toolbar = qobject_cast<QToolBar*>(factory->container("datapicker_toolbar", this));
		if (group.groupList().indexOf("Toolbar datapicker_toolbar") == -1)
			toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);
	} else {
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("datapicker_toolbar", this)->setVisible(false);
	}
}

/*!
	creates a new empty project. Returns \c true, if a new project was created.
*/
bool MainWin::newProject() {
	//close the current project, if available
	if (!closeProject())
		return false;

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	if (m_project)
		delete m_project;

	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	m_project = new Project();
	m_currentAspect = m_project;
	m_currentFolder = m_project;

	KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );
	Project::MdiWindowVisibility vis = Project::MdiWindowVisibility(group.readEntry("MdiWindowVisibility", 0));
	m_project->setMdiWindowVisibility( vis );
	if (vis == Project::folderOnly)
		m_visibilityFolderAction->setChecked(true);
	else if (vis == Project::folderAndSubfolders)
		m_visibilitySubfolderAction->setChecked(true);
	else
		m_visibilityAllAction->setChecked(true);

	m_aspectTreeModel = new AspectTreeModel(m_project, this);

	//newProject is called for the first time, there is no project explorer yet
	//-> initialize the project explorer,  the GUI-observer and the dock widgets.
	if (m_projectExplorer == 0) {
		m_projectExplorerDock = new QDockWidget(this);
		m_projectExplorerDock->setObjectName("projectexplorer");
		m_projectExplorerDock->setWindowTitle(i18n("Project Explorer"));
		addDockWidget(Qt::LeftDockWidgetArea, m_projectExplorerDock);

		m_projectExplorer = new ProjectExplorer(m_projectExplorerDock);
		m_projectExplorerDock->setWidget(m_projectExplorer);

		connect(m_projectExplorer, SIGNAL(currentAspectChanged(AbstractAspect*)),
		        this, SLOT(handleCurrentAspectChanged(AbstractAspect*)));
		connect(m_projectExplorerDock, SIGNAL(visibilityChanged(bool)), SLOT(projectExplorerDockVisibilityChanged(bool)));

		//Properties dock
		m_propertiesDock = new QDockWidget(this);
		m_propertiesDock->setObjectName("aspect_properties_dock");
		m_propertiesDock->setWindowTitle(i18n("Properties"));
		addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

		QScrollArea* sa = new QScrollArea(m_propertiesDock);
		stackedWidget = new QStackedWidget(sa);
		sa->setWidget(stackedWidget);
		sa->setWidgetResizable(true);
		m_propertiesDock->setWidget(sa);

		connect(m_propertiesDock, SIGNAL(visibilityChanged(bool)), SLOT(propertiesDockVisibilityChanged(bool)));

		//GUI-observer;
		m_guiObserver = new GuiObserver(this);
	}

	m_projectExplorer->setModel(m_aspectTreeModel);
	m_projectExplorer->setProject(m_project);
	m_projectExplorer->setCurrentAspect(m_project);

	m_projectExplorerDock->show();
	m_propertiesDock->show();
	updateGUIOnProjectChanges();

	connect(m_project, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_project, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
	        this, SLOT(handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
	        this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_project, SIGNAL(statusInfo(QString)), statusBar(), SLOT(showMessage(QString)));
	connect(m_project, SIGNAL(changed()), this, SLOT(projectChanged()));
	connect(m_project, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_project, SIGNAL(requestFolderContextMenu(const Folder*,QMenu*)), this, SLOT(createFolderContextMenu(const Folder*,QMenu*)));
	connect(m_project, SIGNAL(mdiWindowVisibilityChanged()), this, SLOT(updateMdiWindowVisibility()));

	m_undoViewEmptyLabel = i18n("Project %1 created", m_project->name());
	setCaption(m_project->name());

	return true;
}

void MainWin::openProject() {
	KConfigGroup conf(KSharedConfig::openConfig(), "MainWin");
	QString dir = conf.readEntry("LastOpenDir", "");
	QString path = QFileDialog::getOpenFileName(this,i18n("Open project"), dir,
	               i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ)"));

	if (!path.isEmpty()) {
		this->openProject(path);

		int pos = path.lastIndexOf(QDir::separator());
		if (pos != -1) {
			QString newDir = path.left(pos);
			if (newDir != dir)
				conf.writeEntry("LastOpenDir", newDir);
		}
	}
}

void MainWin::openProject(const QString& filename) {
	if (filename == m_currentFileName) {
		KMessageBox::information(this, i18n("The project file %1 is already opened.", filename), i18n("Open project"));
		return;
	}

	QIODevice *file;
	// first try gzip compression, because projects can be gzipped and end with .lml
	if (filename.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		file = new KCompressionDevice(filename,KFilterDev::compressionTypeForMimeType("application/x-gzip"));
	else	// opens filename using file ending
		file = new KFilterDev(filename);

	if (file == 0)
		file = new QFile(filename);

	if (!file->open(QIODevice::ReadOnly)) {
		KMessageBox::error(this, i18n("Sorry. Could not open file for reading."));
		return;
	}

	//TODO: this solves the problem with empty files (the application doesn't hang anymore)
	//but it crashes from time to time at unexpected places within Qt.
// 	char* c;
// 	bool rc = file->getChar(c);
// 	if (!rc) {
// 		KMessageBox::error(this, "The project file is empty.", i18n("Error opening project"));
// 		file->close();
// 		delete file;
// 		return;
// 	}
// 	file->seek(0);

	if (!newProject()) {
		file->close();
		delete file;
		return;
	}

	WAIT_CURSOR;
	QElapsedTimer timer;
	timer.start();
	bool rc = openXML(file);
	file->close();
	delete file;
	if (!rc) {
		closeProject();
		return;
	}

	m_currentFileName = filename;
	m_project->setFileName(filename);
	m_project->undoStack()->clear();
	m_undoViewEmptyLabel = i18n("Project %1 opened", m_project->name());
	m_recentProjectsAction->addUrl( QUrl(filename) );
	setCaption(m_project->name());
	updateGUIOnProjectChanges();
	updateGUI(); //there are most probably worksheets or spreadsheets in the open project -> update the GUI
	m_saveAction->setEnabled(false);

	statusBar()->showMessage( i18n("Project successfully opened (in %1 seconds).", (float)timer.elapsed()/1000) );

	if (m_autoSaveActive)
		m_autoSaveTimer.start();

	RESET_CURSOR;
}

void MainWin::openRecentProject(const QUrl& url) {
	if (url.isLocalFile())	// fix for Windows
		this->openProject(url.toLocalFile());
	else
		this->openProject(url.path());
}

bool MainWin::openXML(QIODevice *file) {
	XmlStreamReader reader(file);
	if (m_project->load(&reader) == false) {
		RESET_CURSOR;
		QString msg_text = reader.errorString();
		KMessageBox::error(this, msg_text, i18n("Error when opening the project"));
		statusBar()->showMessage(msg_text);
		return false;
	}

	if (reader.hasWarnings()) {
		QString msg = i18n("The following problems occurred when loading the project file:\n");
		const QStringList& warnings = reader.warningStrings();
		foreach (const QString& str, warnings)
			msg += str + '\n';

		qWarning() << msg;
//TODO: show warnings in a kind of "log window" but not in message box
// 		KMessageBox::error(this, msg, i18n("Project loading partly failed"));
// 		statusBar()->showMessage(msg);
	}

	return true;
}

/*!
	Closes the current project, if available. Return \c true, if the project was closed.
*/
bool MainWin::closeProject() {
	if (m_project == 0)
		return true; //nothing to close

	if (warnModified())
		return false;

	delete m_aspectTreeModel;
	m_aspectTreeModel = 0;
	delete m_project;
	m_project = 0;
	m_currentFileName = "";

	//update the UI if we're just closing a project
	//and not closing(quitting) the application
	if (!m_closing) {
		m_projectExplorerDock->hide();
		m_propertiesDock->hide();
		m_currentAspect = 0;
		m_currentFolder = 0;
		updateGUIOnProjectChanges();

		if (m_autoSaveActive)
			m_autoSaveTimer.stop();
	}

	return true;
}

bool MainWin::saveProject() {
	const QString& fileName = m_project->fileName();
	if (fileName.isEmpty())
		return saveProjectAs();
	else
		return save(fileName);
}

bool MainWin::saveProjectAs() {
	KConfigGroup conf(KSharedConfig::openConfig(), "MainWin");
	QString dir = conf.readEntry("LastOpenDir", "");
	QString fileName = QFileDialog::getSaveFileName(this, i18n("Save project as"), dir,
		i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ)"));

	if (fileName.isEmpty())// "Cancel" was clicked
		return false;

	if (fileName.contains(QLatin1String(".lml"), Qt::CaseInsensitive) == false)
		fileName.append(QLatin1String(".lml"));

	return save(fileName);
}

/*!
 * auxillary function that does the actual saving of the project
 */
bool MainWin::save(const QString& fileName) {
	WAIT_CURSOR;
	// use file ending to find out how to compress file
	QIODevice* file;
	// if ending is .lml, do gzip compression anyway
	if (fileName.endsWith(QLatin1String(".lml")))
		file = new KCompressionDevice(fileName, KCompressionDevice::GZip);
	else
		file = new KFilterDev(fileName);

	if (file == 0)
		file = new QFile(fileName);

	bool ok;
	if (file->open(QIODevice::WriteOnly)) {
		m_project->setFileName(fileName);

		QXmlStreamWriter writer(file);
		m_project->save(&writer);
		m_project->undoStack()->clear();
		m_project->setChanged(false);
		file->close();

		setCaption(m_project->name());
		statusBar()->showMessage(i18n("Project saved"));
		m_saveAction->setEnabled(false);
		m_recentProjectsAction->addUrl( QUrl(fileName) );
		ok = true;

		//if the project dock is visible, refresh the shown content
		//(version and modification time might have been changed)
		if (stackedWidget->currentWidget() == projectDock)
			projectDock->setProject(m_project);

		//we have a file name now
		// -> auto save can be activated now if not happened yet
		if (m_autoSaveActive && !m_autoSaveTimer.isActive())
			m_autoSaveTimer.start();
	} else {
		KMessageBox::error(this, i18n("Sorry. Could not open file for writing."));
		ok = false;
	}

	delete file;

	RESET_CURSOR;
	return ok;
}

/*!
 * automatically saves the project in the specified time interval.
 */
void MainWin::autoSaveProject() {
	//don't auto save when there are no changes or the file name
	//was not provided yet (the project was never explicitly saved yet).
	if ( !m_project->hasChanged() || m_project->fileName().isEmpty())
		return;

	this->saveProject();
}

/*!
	prints the current sheet (worksheet, spreadsheet or matrix)
*/
void MainWin::print() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	statusBar()->showMessage(i18n("Preparing printing of %1", part->name()));
	if (part->printView())
		statusBar()->showMessage(i18n("%1 printed", part->name()));
	else
		statusBar()->showMessage("");
}

void MainWin::printPreview() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	statusBar()->showMessage(i18n("Preparing printing of %1", part->name()));
	if (part->printPreview())
		statusBar()->showMessage(i18n("%1 printed", part->name()));
	else
		statusBar()->showMessage("");
}

/**************************************************************************************/

/*!
	adds a new Folder to the project.
*/
void MainWin::newFolder() {
	Folder* folder = new Folder(i18n("Folder"));
	this->addAspectToProject(folder);
}

/*!
	adds a new Workbook to the project.
*/
void MainWin::newWorkbook() {
	Workbook* workbook = new Workbook(0, i18n("Workbook"));
	this->addAspectToProject(workbook);
}

/*!
	adds a new Datapicker to the project.
*/
void MainWin::newDatapicker() {
	Datapicker* datapicker = new Datapicker(0, i18n("Datapicker"));
	this->addAspectToProject(datapicker);
}
/*!
	adds a new Spreadsheet to the project.
*/
void MainWin::newSpreadsheet() {
	Spreadsheet* spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));

	//if the current active window is a workbook and no folder/project is selected in the project explorer,
	//add the new spreadsheet to the workbook
	Workbook* workbook = activeWorkbook();
	if (workbook) {
		QModelIndex index = m_projectExplorer->currentIndex();
		AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		if (!aspect->inherits("Folder")) {
			workbook->addChild(spreadsheet);
			return;
		}
	}

	this->addAspectToProject(spreadsheet);
}

/*!
	adds a new Matrix to the project.
*/
void MainWin::newMatrix() {
	Matrix* matrix = new Matrix(0, i18n("Matrix"));

	//if the current active window is a workbook and no folder/project is selected in the project explorer,
	//add the new spreadsheet to the workbook
	Workbook* workbook = activeWorkbook();
	if (workbook) {
		QModelIndex index = m_projectExplorer->currentIndex();
		AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		if (!aspect->inherits("Folder")) {
			workbook->addChild(matrix);
			return;
		}
	}

	this->addAspectToProject(matrix);
}

/*!
	adds a new Worksheet to the project.
*/
void MainWin::newWorksheet() {
	Worksheet* worksheet= new Worksheet(0, i18n("Worksheet"));
	this->addAspectToProject(worksheet);
}

void MainWin::newNotes() {
	Note* notes = new Note(i18n("Note"));
	this->addAspectToProject(notes);
}

/*!
	returns a pointer to a Workbook-object, if the currently active Mdi-Subwindow is \a WorkbookView.
	Otherwise returns \a 0.
*/
Workbook* MainWin::activeWorkbook() const {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return 0;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	Q_ASSERT(part);
	return dynamic_cast<Workbook*>(part);
}

/*!
	returns a pointer to a Datapicker-object, if the currently active Mdi-Subwindow is \a DatapickerView.
	Otherwise returns \a 0.
*/
Datapicker* MainWin::activeDatapicker() const {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return 0;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	Q_ASSERT(part);
	return dynamic_cast<Datapicker*>(part);
}

/*!
	returns a pointer to a \c Spreadsheet object, if the currently active Mdi-Subwindow
	or if the currently selected tab in a \c WorkbookView is a \c SpreadsheetView
	Otherwise returns \c 0.
*/
Spreadsheet* MainWin::activeSpreadsheet() const {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return 0;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	Q_ASSERT(part);
	Spreadsheet* spreadsheet = 0;
	const Workbook* workbook = dynamic_cast<const Workbook*>(part);
	if (workbook) {
		spreadsheet = workbook->currentSpreadsheet();
		if (!spreadsheet) {
			//potentially, the spreadsheet was not selected in workbook yet since the selection in project explorer
			//arrives in workbook's slot later than in this function
			//->check whether we have a spreadsheet or one of its columns currently selected in the project explorer
			spreadsheet = dynamic_cast<Spreadsheet*>(m_currentAspect);
			if (!spreadsheet) {
				if (m_currentAspect->parentAspect())
					spreadsheet = dynamic_cast<Spreadsheet*>(m_currentAspect->parentAspect());
			}
		}
	} else
		spreadsheet = dynamic_cast<Spreadsheet*>(part);

	return spreadsheet;
}

/*!
	returns a pointer to a \c Matrix object, if the currently active Mdi-Subwindow
	or if the currently selected tab in a \c WorkbookView is a \c MatrixView
	Otherwise returns \c 0.
*/
Matrix* MainWin::activeMatrix() const {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return 0;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	Q_ASSERT(part);
	Matrix* matrix = 0;
	Workbook* workbook = dynamic_cast<Workbook*>(part);
	if (workbook) {
		matrix = workbook->currentMatrix();
		if (!matrix) {
			//potentially, the matrix was not selected in workbook yet since the selection in project explorer
			//arrives in workbook's slot later than in this function
			//->check whether we have a matrix currently selected in the project explorer
			matrix = dynamic_cast<Matrix*>(m_currentAspect);
		}
	} else
		matrix = dynamic_cast<Matrix*>(part);

	return matrix;
}

/*!
	returns a pointer to a Worksheet-object, if the currently active Mdi-Subwindow is \a WorksheetView
	Otherwise returns \a 0.
*/
Worksheet* MainWin::activeWorksheet() const {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return 0;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	Q_ASSERT(part);
	return dynamic_cast<Worksheet*>(part);
}

#ifdef HAVE_CANTOR_LIBS
/*
    adds a new Cantor Spreadsheet to the project.
*/
void MainWin::newCantorWorksheet(QAction* action) {
	CantorWorksheet* cantorworksheet = new CantorWorksheet(0, action->data().toString());
	this->addAspectToProject(cantorworksheet);
}

/********************************************************************************/
/*!
    returns a pointer to a CantorWorksheet-object, if the currently active Mdi-Subwindow is \a CantorWorksheetView
    Otherwise returns \a 0.
*/
CantorWorksheet* MainWin::activeCantorWorksheet() const{
    QMdiSubWindow* win = m_mdiArea->currentSubWindow();
    if (!win)
        return 0;

    AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
    Q_ASSERT(part);
    return dynamic_cast<CantorWorksheet*>(part);
}
#endif

/*!
	called if there were changes in the project.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged() {
	setCaption(i18n("%1    [Changed]", m_project->name()));
	m_saveAction->setEnabled(true);
	m_undoAction->setEnabled(true);
	return;
}

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win) {
	if (!win)
		return;

	PartMdiView *view = qobject_cast<PartMdiView*>(win);
	if (!view) {
		updateGUI();
		return;
	}

	if (view == m_currentSubWindow) {
		//do nothing, if the current sub-window gets selected again.
		//This event happens, when labplot loses the focus (modal window is opened or the user switches to another application)
		//and gets it back (modal window is closed or the user switches back to labplot).
		return;
	} else
		m_currentSubWindow = view;

	updateGUI();
	if (!m_suppressCurrentSubWindowChangedEvent)
		m_projectExplorer->setCurrentAspect(view->part());
}

void MainWin::handleAspectAdded(const AbstractAspect* aspect) {
	const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		//TODO: export, print and print preview should be handled in the views and not in MainWin.
		connect(part, SIGNAL(exportRequested()), this, SLOT(exportDialog()));
		connect(part, SIGNAL(printRequested()), this, SLOT(print()));
		connect(part, SIGNAL(printPreviewRequested()), this, SLOT(printPreview()));
		connect(part, SIGNAL(showRequested()), this, SLOT(handleShowSubWindowRequested()));
	}
}

void MainWin::handleAspectRemoved(const AbstractAspect* parent,const AbstractAspect* before,const AbstractAspect* aspect) {
	Q_UNUSED(before);
	Q_UNUSED(aspect);
	m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (!part) return;

	const Workbook* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
	const Datapicker* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
	if (!datapicker)
		datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

	if (!workbook && !datapicker) {
		PartMdiView* win = part->mdiSubWindow();
		if (win)
			m_mdiArea->removeSubWindow(win);
	}
}

/*!
    called when the current aspect in the tree of the project explorer was changed.
    Selects the new aspect.
*/
void MainWin::handleCurrentAspectChanged(AbstractAspect *aspect) {
	if (!aspect)
		aspect = m_project; // should never happen, just in case

	m_suppressCurrentSubWindowChangedEvent = true;
	if (aspect->folder() != m_currentFolder) {
		m_currentFolder = aspect->folder();
		updateMdiWindowVisibility();
	}

	m_currentAspect = aspect;

	//activate the corresponding MDI sub window for the current aspect
	activateSubWindowForAspect(aspect);
	m_suppressCurrentSubWindowChangedEvent = false;

	updateGUI();
}

void MainWin::activateSubWindowForAspect(const AbstractAspect* aspect) const {
	const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		//for FileDataSource we currently don't show any view
		if (dynamic_cast<const FileDataSource*>(part))
			return;

		PartMdiView* win;

		//for aspects being children of a Workbook, we show workbook's window, otherwise the window of the selected part
		const Workbook* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
		const Datapicker* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
		if (!datapicker)
			datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

		if (workbook)
			win = workbook->mdiSubWindow();
		else if (datapicker)
			win = datapicker->mdiSubWindow();
		else
			win = part->mdiSubWindow();

		if (m_mdiArea->subWindowList().indexOf(win) == -1) {
			if (dynamic_cast<const Note*>(part))
				m_mdiArea->addSubWindow(win, Qt::Tool);
			else
				m_mdiArea->addSubWindow(win);
			win->show();
		}
		m_mdiArea->setActiveSubWindow(win);
	} else {
		//activate the mdiView of the parent, if a child was selected
		const AbstractAspect* parent = aspect->parentAspect();
		if (parent) {
			activateSubWindowForAspect(parent);

			//if the parent's parent is a Workbook (a column of a spreadsheet in workbook was selected),
			//we need to select the corresponding tab in WorkbookView too
			if (parent->parentAspect()) {
				Workbook* workbook = dynamic_cast<Workbook*>(parent->parentAspect());
				Datapicker* datapicker = dynamic_cast<Datapicker*>(parent->parentAspect());
				if (!datapicker)
					datapicker = dynamic_cast<Datapicker*>(parent->parentAspect()->parentAspect());

				if (workbook)
					workbook->childSelected(parent);
				else if (datapicker)
					datapicker->childSelected(parent);
			}
		}
	}
	return;
}

void MainWin::setMdiWindowVisibility(QAction * action) {
	m_project->setMdiWindowVisibility((Project::MdiWindowVisibility)(action->data().toInt()));
}

/*!
	shows the sub window of a worksheet, matrix or a spreadsheet.
	Used if the window was closed before and the user asks to show
	the window again via the context menu in the project explorer.
*/
void MainWin::handleShowSubWindowRequested() {
	activateSubWindowForAspect(m_currentAspect);
}

/*!
	this is called on a right click on the root folder in the project explorer
*/
void MainWin::createContextMenu(QMenu* menu) const {
	menu->addMenu(m_newMenu);

	//The tabbed view collides with the visibility policy for the subwindows.
	//Hide the menus for the visibility policy if the tabbed view is used.
	if (m_mdiArea->viewMode() != QMdiArea::TabbedView)
		menu->addMenu(m_visibilityMenu);
}

/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder* folder, QMenu* menu) const {
	Q_UNUSED(folder);

	//Folder provides it's own context menu. Add a separator before adding additional actions.
	menu->addSeparator();
	this->createContextMenu(menu);
}

void MainWin::undo() {
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	if (m_project->undoStack()->index()==0) {
		setCaption(m_project->name());
		m_saveAction->setEnabled(false);
		m_undoAction->setEnabled(false);
		m_project->setChanged(false);
	}
	m_redoAction->setEnabled(true);
	RESET_CURSOR;
}

void MainWin::redo() {
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	projectChanged();
	if (m_project->undoStack()->index() == m_project->undoStack()->count())
		m_redoAction->setEnabled(false);
	RESET_CURSOR;
}

/*!
	Shows/hides mdi sub-windows depending on the current visibility policy.
*/
void MainWin::updateMdiWindowVisibility() const {
	QList<QMdiSubWindow *> windows = m_mdiArea->subWindowList();
	PartMdiView* part_view;
	switch (m_project->mdiWindowVisibility()) {
	case Project::allMdiWindows:
		foreach (QMdiSubWindow* window, windows)
			window->show();

		break;
	case Project::folderOnly:
		foreach (QMdiSubWindow *window, windows) {
			part_view = qobject_cast<PartMdiView *>(window);
			Q_ASSERT(part_view);
			if (part_view->part()->folder() == m_currentFolder)
				part_view->show();
			else
				part_view->hide();
		}
		break;
	case Project::folderAndSubfolders:
		foreach (QMdiSubWindow *window, windows) {
			part_view = qobject_cast<PartMdiView *>(window);
			if (part_view->part()->isDescendantOf(m_currentFolder))
				part_view->show();
			else
				part_view->hide();
		}
		break;
	}
}

void MainWin::toggleDockWidget(QAction* action) const {
	if (action->objectName() == "toggle_project_explorer_dock") {
		if (m_projectExplorerDock->isVisible())
			m_projectExplorerDock->hide();
		else
			m_projectExplorerDock->show();
	} else if (action->objectName() == "toggle_properties_explorer_dock") {
		if (m_propertiesDock->isVisible())
			m_propertiesDock->hide();
		else
			m_propertiesDock->show();
	}
}

void MainWin::projectExplorerDockVisibilityChanged(bool visible) {
	m_toggleProjectExplorerDockAction->setChecked(visible);
}

void MainWin::propertiesDockVisibilityChanged(bool visible) {
	m_togglePropertiesDockAction->setChecked(visible);
}

void MainWin::toggleFullScreen() {
	if (this->windowState() == Qt::WindowFullScreen)
		this->setWindowState(m_lastWindowState);
	else {
		m_lastWindowState = this->windowState();
		this->showFullScreen();
	}
}

void MainWin::closeEvent(QCloseEvent* event) {
	m_closing = true;
	if (!this->closeProject()) {
		m_closing = false;
		event->ignore();
	}
}

void MainWin::dragEnterEvent(QDragEnterEvent* event) {
	event->accept();
}

void MainWin::dropEvent(QDropEvent* event) {
	if (!m_project)
		newProject();

	QUrl url = event->mimeData()->urls().at(0);
	importFileDialog(url.toLocalFile());
}

void MainWin::handleSettingsChanges() {
	const KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );

	QMdiArea::ViewMode viewMode = QMdiArea::ViewMode(group.readEntry("ViewMode", 0));
	if (m_mdiArea->viewMode() != viewMode) {
		m_mdiArea->setViewMode(viewMode);
		if (viewMode == QMdiArea::SubWindowView)
			this->updateMdiWindowVisibility();
	}

	if (m_mdiArea->viewMode() == QMdiArea::TabbedView) {
		m_tileWindows->setVisible(false);
		m_cascadeWindows->setVisible(false);
		QTabWidget::TabPosition tabPosition = QTabWidget::TabPosition(group.readEntry("TabPosition", 0));
		if (m_mdiArea->tabPosition() != tabPosition)
			m_mdiArea->setTabPosition(tabPosition);
	} else {
		m_tileWindows->setVisible(true);
		m_cascadeWindows->setVisible(true);
	}

	//autosave
	bool autoSave = group.readEntry("AutoSave", 0);
	if (m_autoSaveActive != autoSave) {
		m_autoSaveActive = autoSave;
		if (autoSave)
			m_autoSaveTimer.start();
		else
			m_autoSaveTimer.stop();
	}

	int interval = group.readEntry("AutoSaveInterval", 1);
	interval *= 60*1000;
	if (interval != m_autoSaveTimer.interval())
		m_autoSaveTimer.setInterval(interval);
}

/***************************************************************************************/
/************************************** dialogs ***************************************/
/***************************************************************************************/
/*!
  shows the dialog with the Undo-history.
*/
void MainWin::historyDialog() {
	if (!m_project->undoStack())
		return;

	HistoryDialog* dialog = new HistoryDialog(this, m_project->undoStack(), m_undoViewEmptyLabel);
	int index = m_project->undoStack()->index();
	if (dialog->exec() != QDialog::Accepted) {
		if (m_project->undoStack()->count() != 0)
			m_project->undoStack()->setIndex(index);
	}

	//disable undo/redo-actions if the history was cleared
	//(in both cases, when accepted or rejected in the dialog)
	if (m_project->undoStack()->count() == 0) {
		m_undoAction->setEnabled(false);
		m_redoAction->setEnabled(false);
	}
}

/*!
  Opens the dialog to import data to the selected workbook, spreadsheet or matrix
*/
void MainWin::importFileDialog(const QString& fileName) {
	DEBUG("MainWin::importFileDialog()");
	m_importFileDialog = new ImportFileDialog(this, false, fileName);

	// explicitly do not delete on close (Windows does this!)
	m_importFileDialog->setAttribute(Qt::WA_DeleteOnClose, false);

	// select existing container
	if (m_currentAspect->inherits("Spreadsheet") || m_currentAspect->inherits("Matrix") || m_currentAspect->inherits("Workbook"))
		m_importFileDialog->setCurrentIndex( m_projectExplorer->currentIndex());
	else if (m_currentAspect->inherits("Column")) {
		if (m_currentAspect->parentAspect()->inherits("Spreadsheet"))
			m_importFileDialog->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));
	}

	if (m_importFileDialog->exec() == QDialog::Accepted) {
		m_importFileDialog->importTo(statusBar());
		m_project->setChanged(true);
	}

	delete m_importFileDialog;
	m_importFileDialog = 0;
	DEBUG("MainWin::importFileDialog() DONE");
}

/*!
  opens the dialog for the export of the currently active worksheet, spreadsheet or matrix.
 */
void MainWin::exportDialog() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
	if (part->exportView())
		statusBar()->showMessage(i18n("%1 exported", part->name()));
}

void MainWin::editFitsFileDialog() {
	FITSHeaderEditDialog* editDialog = new FITSHeaderEditDialog(this);
	if (editDialog->exec() == KDialog::Accepted) {
		if (editDialog->saved())
			statusBar()->showMessage(i18n("FITS files saved"));
	}
}

/*!
  adds a new file data source to the current project.
*/
void MainWin::newFileDataSourceActionTriggered() {
	ImportFileDialog* dlg = new ImportFileDialog(this, true);
	if (dlg->exec() == QDialog::Accepted) {
		FileDataSource* dataSource = new FileDataSource(0,  i18n("File data source%1", 1));
		dlg->importToFileDataSource(dataSource, statusBar());
		this->addAspectToProject(dataSource);
	}
	delete dlg;
}

/*!
  adds a new SQL data source to the current project.
*/
void MainWin::newSqlDataSourceActionTriggered() {
	//TODO
}

void MainWin::addAspectToProject(AbstractAspect* aspect) {
	QModelIndex index = m_projectExplorer->currentIndex();
	if (!index.isValid())
		m_project->addChild(aspect);
	else {
		AbstractAspect* parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		// every aspect contained in the project should have a folder
		Q_ASSERT(parent_aspect->folder());
		parent_aspect->folder()->addChild(aspect);
	}
}

void MainWin::settingsDialog() {
	SettingsDialog* dlg = new SettingsDialog(this);
	connect (dlg, SIGNAL(settingsChanged()), this, SLOT(handleSettingsChanges()));
	dlg->exec();
}
