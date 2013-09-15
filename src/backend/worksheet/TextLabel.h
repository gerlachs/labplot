/***************************************************************************
    File                 : ScalableTextLabel.h
    Project              : LabPlot/SciDAVis
    Description          : A one-line text label supporting floating point font sizes.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2012-2013 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QObject>
#include <QFont>
#include <QBrush>
#include <QPen>
#include "backend/lib/macros.h"
#include "tools/TeXRenderer.h"
#include "backend/worksheet/AbstractWorksheetElement.h"

class TextLabelPrivate;
class TextLabel : public AbstractWorksheetElement{
	Q_OBJECT

	public:
		enum HorizontalPosition {hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom};
		enum VerticalPosition {vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom};
		
		enum HorizontalAlignment {hAlignLeft, hAlignCenter, hAlignRight};
		enum VerticalAlignment {vAlignTop, vAlignCenter, vAlignBottom};

		struct TextWrapper{
			TextWrapper() : teXUsed(false){}
			TextWrapper(const QString& t, bool b) : text(t), teXUsed(b){}
			TextWrapper(const QString& t) : text(t), teXUsed(false){}

			QString text;
			bool teXUsed;
		};

		struct PositionWrapper{
			QPointF 		   point;
			HorizontalPosition horizontalPosition;
			VerticalPosition   verticalPosition;
		};

		TextLabel(const QString &name);
		~TextLabel();
		
		virtual QGraphicsItem *graphicsItem() const;
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);
		
		CLASS_D_ACCESSOR_DECL(TextWrapper, text, Text);
		BASIC_D_ACCESSOR_DECL(qreal, teXFontSize, TeXFontSize);
		BASIC_D_ACCESSOR_DECL(QColor, teXFontColor, TeXFontColor);
		CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position);
		void setPosition(const QPointF&);
		void setPositionInvalid(bool);
		BASIC_D_ACCESSOR_DECL(HorizontalAlignment, horizontalAlignment, HorizontalAlignment);
		BASIC_D_ACCESSOR_DECL(VerticalAlignment, verticalAlignment, VerticalAlignment);
		BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle);
		
		virtual void setVisible(bool on);
		virtual bool isVisible() const;

		typedef TextLabelPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	private slots:
		void updateTeXImage();

	protected:
		TextLabelPrivate * const d_ptr;
		TextLabel(const QString &name, TextLabelPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(TextLabel)
		void init();

	signals:
		friend class TextLabelSetTextCmd;
		friend class TextLabelSetTeXFontSizeCmd;
		friend class TextLabelSetTeXFontColorCmd;
		friend class TextLabelSetPositionCmd;
		friend class TextLabelSetHorizontalAlignmentCmd;
		friend class TextLabelSetVerticalAlignmentCmd;
		friend class TextLabelSetRotationAngleCmd;
		void textWrapperChanged(const TextLabel::TextWrapper&);
		void teXFontSizeChanged(const qreal);
		void teXFontColorChanged(const QColor);
		void positionChanged(const TextLabel::PositionWrapper&);
		void horizontalAlignmentChanged(TextLabel::HorizontalAlignment);
		void verticalAlignmentChanged(TextLabel::VerticalAlignment);
		void rotationAngleChanged(float);
		void visibleChanged(bool);

		void changed();
};

#endif
