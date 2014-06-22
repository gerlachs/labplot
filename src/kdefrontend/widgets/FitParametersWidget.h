/***************************************************************************
    File                 : FitParametersWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)
    Description          : widget for editing the fit parameters

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
#ifndef FITPARAMETERSWIDGET_H
#define FITPARAMETERSWIDGET_H

#include <QWidget>
#include <QStringList>

#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_fitparameterswidget.h"

class FitParametersWidget: public QWidget {
	Q_OBJECT

public:
	explicit FitParametersWidget(QWidget*, XYFitCurve::FitData*);

private:
	Ui::FitParametersWidget ui;
	XYFitCurve::FitData* m_fitData;

signals:
	void finished();

private slots:
	void applyClicked();
};

#endif //FITPARAMETERSWIDGET_H
