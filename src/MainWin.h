/***************************************************************************
    File                 : MainWin.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : main class
                           
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
#ifndef MAINWIN_H
#define MAINWIN_H
#include <KXmlGuiWindow>
#include <QMdiArea>
#include <KTextEdit>
#include <KAction>

#include "Spreadsheet.h"
#include "Worksheet.h"
#include "elements/Set.h"
#include "plots/Plot.h"

class AbstractAspect;
class Folder;
class ProjectExplorer;
class Table;
class Project;
#include "core/PartMdiView.h"

class MainWin : public KXmlGuiWindow
{
	Q_OBJECT
public:
	MainWin(QWidget *parent=0,QString filename=0);
	~MainWin();
	QMdiArea* getMdi() const { return m_mdi_area; }
	Spreadsheet* activeSpreadsheet() const;			//!< get active spreadsheet
	Spreadsheet* getSpreadsheet(QString title) const;	//!< get Spreadsheet of name title
	Worksheet* activeWorksheet() const;			//!< get active worksheet
	Worksheet* getWorksheet(QString name) const;		//!< get Worksheet of name title
	Project* getProject() const { return m_project; }
	void setProject(Project *p) { m_project=p; }
	void updateGUI();		//!< update GUI of main window
	void updateSheetList();		//!< update dynamic sheet list menu
	void updateSetList();		//!< update dynamic set list menu
	void openXML(QIODevice *file);	//!< do the actual opening
	void saveXML(QIODevice *file);	//!< do the actual saving

private:
	QMdiArea *m_mdi_area;
	Project *m_project;
	QMenu *spreadsheetmenu;
	KAction *spreadaction;
	KAction *m_folderAction;
	KAction *m_historyAction;
	KAction *m_undoAction;
	KAction *m_redoAction;
	void setupActions();
	void initProjectExplorer();
	bool warnModified();
	void addSet(Set s, const int sheet, const Plot::PlotType ptype);
	AbstractAspect * m_current_aspect;
	Folder * m_current_folder;
	ProjectExplorer * m_project_explorer;
	QDockWidget * m_project_explorer_dock;
	void handleAspectAddedInternal(AbstractAspect *aspect);

public slots:
	Table* newSpreadsheet();
	Folder* newFolder();
	Worksheet* newWorksheet();
	void save(QString filename=0);	//!< save project (.lml format)
	void open(QString filename);	//!< open project (.lml format)
	void saveAs();	//!< save as different file name (.lml format)
	void showHistory();
	void createContextMenu(QMenu * menu) const;
	void createFolderContextMenu(const Folder * folder, QMenu * menu) const;
	void undo();
	void redo();
	//! Show/hide mdi windows depending on the currend folder
	void updateMdiWindowVisibility();

private slots:
	void openNew();
	void print();
	void SpreadsheetMenu();
	void importDialog();
	void projectDialog();
	void functionActionTriggered(QAction*);
	void titleDialog();
	void axesDialog();
	void legendDialog();
	void settingsDialog();

signals:
	void partActivated(AbstractPart*);
	

private slots:
	void handleAspectAdded(const AbstractAspect *parent, int index);
	void handleAspectAboutToBeRemoved(const AbstractAspect *parent, int index);
	void handleAspectRemoved(const AbstractAspect *parent, int index);
	void handleAspectDescriptionChanged(const AbstractAspect *aspect);
	void handleCurrentAspectChanged(AbstractAspect *aspect);
	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to);
	void setMdiWindowVisibility(QAction * action);
};

#endif
