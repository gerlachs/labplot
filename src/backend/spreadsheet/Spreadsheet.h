/***************************************************************************
    File                 : Spreadsheet.h
    Project              : SciDAVis
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "datasources/AbstractDataSource.h"
#include "core/column/Column.h"
#include <QList>

class Spreadsheet : public AbstractDataSource{
	Q_OBJECT

	public:
		Spreadsheet(AbstractScriptingEngine *engine, int rows, int columns, const QString &name);
		~Spreadsheet();

		virtual QIcon icon() const;
		virtual bool fillProjectMenu(QMenu * menu);
		virtual QMenu *createContextMenu();
		virtual QWidget *view() const;

		int columnCount() const;
		int columnCount(SciDAVis::PlotDesignation pd) const;
		Column* column(int index) const;
		Column* column(const QString &name) const;
		int rowCount() const;

		void removeRows(int first, int count);
		void insertRows(int before, int count);
		void removeColumns(int first, int count);
		void insertColumns(int before, int count);

		int colX(int col);
		int colY(int col);
		QString text(int row, int col) const;

		void copy(Spreadsheet * other);

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);
		
	public slots:		
		void appendRows(int count);
		void appendRow();
		void appendColumns(int count);
		void appendColumn();

		void setColumnCount(int new_size);
		void setRowCount(int new_size);

		void clear();
		void clearMasks();

		void moveColumn(int from, int to);
		void sortColumns(Column * leading, QList<Column*> cols, bool ascending);

	signals:
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		void requestProjectMenu(QMenu *menu, bool *rc);
		void requestProjectContextMenu(QMenu *menu);
#endif

	private:
		mutable QWidget *m_view;
};

#endif
