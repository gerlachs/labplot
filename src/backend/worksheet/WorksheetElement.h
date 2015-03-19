/***************************************************************************
    File                 : WorksheetElement.h
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef WORKSHEETELEMENT_H
#define WORKSHEETELEMENT_H

#include "backend/core/AbstractAspect.h"
#include <QGraphicsItem>
#include <QPen>

class QAction;

class WorksheetElement: public AbstractAspect {
	Q_OBJECT

	public:
		explicit WorksheetElement(const QString&);
		virtual ~WorksheetElement();

		enum WorksheetElementName {NameCartesianPlot = 1};

		virtual QGraphicsItem* graphicsItem() const = 0;

		virtual void setVisible(bool on) = 0;
		virtual bool isVisible() const = 0;
		virtual bool isFullyVisible() const;
		virtual void setPrinting(bool) = 0;
		virtual QMenu* createContextMenu();

		static QPainterPath shapeFromPath(const QPainterPath&, const QPen&);

	public slots:
		virtual void retransform() = 0;
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	private:
		QMenu* m_drawingOrderMenu;
		QMenu* m_moveBehindMenu;
		QMenu* m_moveInFrontOfMenu;

	protected:
		static QPen selectedPen;
		static float selectedOpacity;
		static QPen hoveredPen;
		static float hoveredOpacity;

	private slots:
		void prepareMoveBehindMenu();
		void prepareMoveInFrontOfMenu();
		void execMoveBehind(QAction*);
		void execMoveInFrontOf(QAction*);

	signals:
		friend class AbstractPlotSetHorizontalPaddingCmd;
		friend class AbstractPlotSetVerticalPaddingCmd;
		void horizontalPaddingChanged(float);
		void verticalPaddingChanged(float);

		void hovered();
		void unhovered();
};

#endif
