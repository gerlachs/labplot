/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de

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

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/widgets/TreeViewComboBox.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <kmessagebox.h>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeddes \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
 
ImportFileDialog::ImportFileDialog(QWidget* parent) : KDialog(parent) {
	mainWidget = new QWidget(this);
	vLayout = new QVBoxLayout(mainWidget);
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(0,0,0,0);
	
	importFileWidget = new ImportFileWidget( mainWidget );
	vLayout->addWidget(importFileWidget);
	
	setMainWidget( mainWidget );
	
    setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
	setButtonText(KDialog::User1,i18n("Show Options"));
// 	enableButtonOk(false);
	
	connect(this,SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));

	setCaption(i18n("Import data to spreadsheet/matrix"));
	setWindowIcon(KIcon("document-import-database"));
	resize( QSize(500,0).expandedTo(minimumSize()) );
}

/*!
	creates widgets for the frame "Add-To" and sets the current model in the combobox to \c model.
 */
void ImportFileDialog::setModel(QAbstractItemModel * model){
  //Frame for the "Add To"-Stuff
  frameAddTo = new QGroupBox(this);
  frameAddTo->setTitle(i18n("Import  to"));
  QHBoxLayout* hLayout = new QHBoxLayout(frameAddTo);
  hLayout->addWidget( new QLabel(i18n("Spreadsheet"),  frameAddTo) );
	
  cbAddTo = new TreeViewComboBox(frameAddTo);
  cbAddTo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  QList<const char *> list;
  list<<"Folder"<<"Spreadsheet";
  cbAddTo->setTopLevelClasses(list);
  hLayout->addWidget( cbAddTo);
  connect( cbAddTo, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(currentAddToIndexChanged(const QModelIndex&)) );
  
  bNewSpreadsheet = new QPushButton(frameAddTo);
  bNewSpreadsheet->setIcon(KIcon("insert-table"));
  bNewSpreadsheet->setToolTip(i18n("Add new spreadsheet"));
  hLayout->addWidget( bNewSpreadsheet);
  connect( bNewSpreadsheet, SIGNAL(clicked()), this, SLOT(newSpreadsheet()));
  
  hLayout->addItem( new QSpacerItem(50,10, QSizePolicy::Preferred, QSizePolicy::Fixed) );
  
  lPosition = new QLabel(i18n("Position"),  frameAddTo);
  lPosition->setEnabled(false);
  hLayout->addWidget(lPosition);
  
  cbPosition = new QComboBox(frameAddTo);
  cbPosition->setEnabled(false);
  cbPosition->addItem(i18n("Append"));
  cbPosition->addItem(i18n("Prepend"));
  cbPosition->addItem(i18n("Replace"));

  cbPosition->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  hLayout->addWidget( cbPosition);
  
  vLayout->addWidget(frameAddTo);
  cbAddTo->setModel(model);
  
  //hide the data-source related widgets
  importFileWidget->hideDataSource();
}


void ImportFileDialog::setCurrentIndex(const QModelIndex& index){
  cbAddTo->setCurrentModelIndex(index);
  this->currentAddToIndexChanged(index);
}


/*!
  triggers data import to the file data source \c source
*/
void ImportFileDialog::importToFileDataSource(FileDataSource* source) const{
	importFileWidget->saveSettings(source);
	source->read();
}

/*!
  triggers data import to the currently selected spreadsheet
*/
void ImportFileDialog::importToSpreadsheet() const{
  AbstractAspect * aspect = static_cast<AbstractAspect *>(cbAddTo->currentModelIndex().internalPointer());
  Spreadsheet* sheet = qobject_cast<Spreadsheet*>(aspect);
  QString fileName = importFileWidget->fileName();
  AbstractFileFilter* filter = importFileWidget->currentFileFilter();
  AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());
  
  filter->read(fileName, sheet, mode);
  delete filter;
}
  

void ImportFileDialog::toggleOptions(){
	if (importFileWidget->toggleOptions()){
		setButtonText(KDialog::User1,i18n("Hide Options"));
	}else{
		setButtonText(KDialog::User1,i18n("Show Options"));
	}

	//resize the dialog
	mainWidget->resize(layout()->minimumSize());
	layout()->activate();
 	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

void ImportFileDialog::currentAddToIndexChanged(QModelIndex index){
	AbstractAspect * aspect = static_cast<AbstractAspect *>(index.internalPointer());
	if (!aspect)
		return;
	
	 if ( aspect->inherits("Spreadsheet") ){
		 lPosition->setEnabled(true);
		 cbPosition->setEnabled(true);
		 bNewSpreadsheet->setEnabled(false);
		 enableButtonOk(true);
	 }else{
		 lPosition->setEnabled(false);
		 cbPosition->setEnabled(false);
		 bNewSpreadsheet->setEnabled(true);
		 enableButtonOk(false);
	 }
}

void ImportFileDialog::newSpreadsheet(){
	KMessageBox::information(this, i18n("Not implemented yet. Please create a new spreadsheet in the main window."), 
							 i18n("Not implemented yet"));
}
