/***************************************************************************
    File                 : CartesianPlotDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for cartesian plot properties
                           
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

#ifndef CARTESIANPLOTDOCK_H
#define CARTESIANPLOTDOCK_H

#include <QList>
#include "ui_cartesianplotdock.h"
#include "backend/worksheet/Worksheet.h"

class CartesianPlot;
class LabelWidget;

class CartesianPlotDock: public QWidget{
	Q_OBJECT
	
public:
	CartesianPlotDock(QWidget *parent);
	void setPlots(QList<CartesianPlot*>);
	void activateTitleTab();
	
private:
	Ui::CartesianPlotDock ui;
	QList<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot;
	LabelWidget* labelWidget;
	bool m_initializing;
	
private slots:
	void init();
	void retranslateUi();
  
	//SLOTs for changes triggered in CartesianPlotDock
	//"General"-tab
	void nameChanged();
	void commentChanged();
	void visibilityChanged(int);
	void geometryChanged();
	void layoutChanged(Worksheet::Layout);

	//"Coordinate system"-tab
	void toggleXBreak(int);
	void toggleYBreak(int);
	
	//"Plot area"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);
  	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double value);
	void borderOpacityChanged(int);
	void horizontalPaddingChanged(double);
	void verticalPaddingChanged(double);
	
	//SLOTs for changes triggered in CartesianPlot
	void plotPositionChanged();
	
	//save/load
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif
