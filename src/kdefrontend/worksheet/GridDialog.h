/***************************************************************************
    File                 : GridDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : dialog for editing the grid properties for the worksheet view
                           
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

#ifndef GRIDDIALOG_H
#define GRIDDIALOG_H

#include <KDialog>
#include "worksheet/WorksheetView.h"

class QComboBox;
class QSpinBox;
class KColorButton;

class GridDialog: public KDialog {
	Q_OBJECT

public:
	GridDialog(QWidget*);
	void save(WorksheetView::GridSettings&);

private:
	QComboBox* cbStyle;
	QSpinBox* sbHorizontalSpacing;
	QSpinBox* sbVerticalSpacing;
	KColorButton* kcbColor;
	QSpinBox* sbOpacity;

private slots:
};

#endif