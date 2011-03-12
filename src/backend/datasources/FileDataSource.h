/***************************************************************************
    File                 : FileDataSource.h
    Project              : LabPlot/SciDAVis
    Description 		: Represents file data source
    --------------------------------------------------------------------
    Copyright		            : (C) 2009 Alexander Semke
    Email (use @ for *) 	: alexander.semke*web.de

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
#ifndef ASCIIDATASOURCE_H
#define ASCIIDATASOURCE_H

#include "AbstractDataSource.h"
#include "filters/AbstractFileFilter.h"
#include <QString>

class FileDataSource	:  public AbstractDataSource {
    Q_OBJECT

  public:
	FileDataSource(AbstractScriptingEngine *engine,  const QString &name);
	~FileDataSource();

	enum FileType{AsciiVector, BinaryVector, AsciiMatrix, BinaryMatrix, Image, Sound};

	static QStringList fileTypes();
	static QString fileInfoString(const QString&);

	void read();

	void setFileType(const FileType);
	FileType fileType() const;

	void setFileWatched(const bool);
	bool isFileWatched() const;

	void setFileLinked(const bool);
	bool isFileLinked() const;

	void setFileName(const QString &);
	QString fileName() const;

	void setFilter(AbstractFileFilter*);
	QIcon icon() const;
	QMenu *createContextMenu() const;
	virtual QWidget *view() const;
	
  private:
	QString m_fileName;
	FileType m_fileType;
	bool m_fileWatched;
	bool m_fileLinked;
	AbstractFileFilter* m_filter;

  signals:
	void dataChanged();
	void dataUpdated();
  };

#endif